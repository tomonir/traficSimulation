#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Graphics.h"
#include "Intersection.h"
#include "Vehicle.h"
#include "Street.h"
#include "SpeedLimit.h"

#include <math.h>
#include <string> 

#define VEHICLE_WIDTH 60
#define VEHICLE_LENGTH 100 


void Graphics::simulate()
{
    this->loadBackgroundImg();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // update graphics
        this->drawTrafficObjects();
    }
}

void Graphics::loadBackgroundImg()
{
    // create window
    _windowName = "Concurrency Traffic Simulation";
    cv::namedWindow(_windowName, cv::WINDOW_NORMAL);

    // load image and create copy to be used for semi-transparent overlay
    cv::Mat background = cv::imread(_bgFilename);
    _images.push_back(background);         // first element is the original background
    _images.push_back(background.clone()); // second element will be the transparent overlay
    _images.push_back(background.clone()); // third element will be the result image for display
}


// Include center point of your rectangle, size of your rectangle and the degrees of rotation  
void DrawRotatedRectangle(cv::Mat& image, cv::Scalar color, cv::Point centerPoint, cv::Size rectangleSize, double rotationDegrees)
{
   // cv::Scalar color = cv::Scalar(255.0, 255.0, 255.0); // white

    // Create the rotated rectangle
    cv::RotatedRect rotatedRectangle(centerPoint, rectangleSize, rotationDegrees);

    // We take the edges that OpenCV calculated for us
    cv::Point2f vertices2f[4];
    rotatedRectangle.points(vertices2f);

    // Convert them so we can use them in a fillConvexPoly
    cv::Point vertices[4];    
    for(int i = 0; i < 4; ++i){
        vertices[i] = vertices2f[i];
    }

    // Now we can fill the rotated rectangle with our specified color
    cv::fillConvexPoly(image,
                       vertices,
                       4,
                       color);
}


void Graphics::drawTrafficObjects()
{
    // reset images
    _images.at(1) = _images.at(0).clone();
    _images.at(2) = _images.at(0).clone();

    
    // create overlay from all traffic objects
    for (auto it : _trafficObjects)
    {
        double posx, posy;
        it->getPosition(posx, posy);

        if (it->getType() == ObjectType::objectIntersection)
        {
            // cast object type from TrafficObject to Intersection
            std::shared_ptr<Intersection> intersection = std::dynamic_pointer_cast<Intersection>(it);

            // set color according to traffic light and draw the intersection as a circle
            cv::Scalar trafficLightColor = intersection->trafficLightIsGreen() == true ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 25, trafficLightColor, -1);
            cv::putText(_images.at(1),std::to_string(it->getID()),cv::Point2d(posx, posy),cv::FONT_HERSHEY_SIMPLEX,1,(255,255,255),2);
        }
        else if (it->getType() == ObjectType::objectVehicle)
        {
            double end_point_arrow_x = posx+ 50 * std::cos(it->getMovingAngle());
            double end_point_arrow_y = posy +50 * std::sin(it->getMovingAngle());
            std::shared_ptr<Vehicle> this_vehicle = std::dynamic_pointer_cast<Vehicle>(it);
            std::shared_ptr<Street> curr_street = std::dynamic_pointer_cast<Street>(this_vehicle->getCurrenStreet());

            
            cv::RNG rng(it->getID());
            int b = rng.uniform(0, 255);
            int g = rng.uniform(0, 255);
            int r = sqrt(255*255 - g*g - r*r); // ensure that length of color vector is always 255
            
            cv::Scalar vehicleColor = cv::Scalar(b,g,r);
            //cv::circle(_images.at(1), cv::Point2d(posx, posy), 50, vehicleColor, -1);
            
            
            //cv::Rect rect(posx, posy, VEHICLE_LENGTH, VEHICLE_WIDTH);
            //cv::rectangle(_images.at(1), rect, vehicleColor,VEHICLE_WIDTH);

            DrawRotatedRectangle(_images.at(1),vehicleColor,cv::Point(posx,posy),cv::Size2f(VEHICLE_LENGTH,VEHICLE_WIDTH),this_vehicle->getMovingAngle());

            std::string txt_to_dispaly = std::to_string(this_vehicle->getID())+"-"+std::to_string(this_vehicle->getCloseVehicleId());
            cv::putText(_images.at(1),txt_to_dispaly,cv::Point2d(posx-50, posy),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255.0, 255.0, 255.0),2);
            
            cv::putText(_images.at(1),std::to_string(this_vehicle->getCurrentSpeed()/10).substr(0,2),cv::Point2d(posx-40, posy+25),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255.0, 255.0, 255.0),2);
            //cv::putText(_images.at(1),std::to_string(this_vehicle->getMovingAngle()),cv::Point2d(posx-30, posy+40),cv::FONT_HERSHEY_SIMPLEX,1,(255,255,255),3);
           
        }else if (it->getType() == ObjectType::objectSpeedLimit)
        {
            std::shared_ptr<SpeedLimit> this_SpeedLimit = std::dynamic_pointer_cast<SpeedLimit>(it);
            double speed = this_SpeedLimit->getSpeed();
            cv::Scalar speedLimitColor = cv::Scalar(0,0,255);
            cv::Scalar whiteColor = cv::Scalar(255.0, 255.0, 255.0);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 50, speedLimitColor, 20);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 40, whiteColor, -1);
            
            cv::putText(_images.at(1),std::to_string(speed).substr(0,2),cv::Point2d(posx-40, posy+15),cv::FONT_HERSHEY_SIMPLEX,2,cv::Scalar(0.0,0.0,0.0),8);
        }
    }

    float opacity = 0.85;
    cv::addWeighted(_images.at(1), opacity, _images.at(0), 1.0 - opacity, 0, _images.at(2));

    // display background and overlay image
    cv::imshow(_windowName, _images.at(2));
    cv::waitKey(33);
}
