#include <iostream>

#include "Cloud.h"
#include "Intersection.h"
#include <math.h>
#include <string> 

double getDistance(const double & aPoint_x,const double & aPoint_y,
        const double & bPoint_x,const double & bPoint_y) 
{
    const double x_diff = bPoint_x - aPoint_x;
    const double y_diff = bPoint_y - aPoint_x;
    return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}


std::vector<std::shared_ptr<TrafficObject>> Cloud::getCloseObjects(const double x, const double y, const double distance)
{

    std::vector<std::shared_ptr<TrafficObject>> return_objects;
    double posx, posy;
    for (auto it : _trafficObjects)
    {
        it->getPosition(posx, posy);
        if (getDistance(x,y,posx,posy)< distance)
        {
            return_objects.push_back(it);
        }
    }

    return return_objects;

}





