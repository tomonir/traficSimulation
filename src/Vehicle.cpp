#include <iostream>
#include <random>
#include "Cloud.h"
#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

#define DISTANCE_TO_COLLISION 200 

Vehicle::Vehicle(const double max_allowed_speed)
{
    _currStreet = nullptr;
    _cloud = nullptr;
    _posStreet = 0.0;
    _type = ObjectType::objectVehicle;
    _speed = 400; // m/s
    _max_allowed_speed = max_allowed_speed;
    _currentState = VehicleStates::moving;
}


void Vehicle::setCurrentDestination(std::shared_ptr<Intersection> destination)
{
    // update destination
    _currDestination = destination;

    // reset simulation parameters
    _posStreet = 0.0;
}

void Vehicle::simulate()
{
    // launch drive function in a thread
    threads.emplace_back(std::thread(&Vehicle::drive, this));
}


void Vehicle::processIntersection(bool &hasEnteredIntersection,
                                 const long timeSinceLastUpdate,
                                 double completion)
{
            // update position with a constant velocity motion model
            //_posStreet += _speed * timeSinceLastUpdate / 1000;

            // compute completion rate of current street
            //double completion = _posStreet / _currStreet->getLength();

            // compute current pixel position on street based on driving direction
            std::shared_ptr<Intersection> i1, i2;
            i2 = _currDestination;
            i1 = i2->getID() == _currStreet->getInIntersection()->getID() ? _currStreet->getOutIntersection() : _currStreet->getInIntersection();

            double x1, y1, x2, y2, xv, yv, dx, dy, l;
            i1->getPosition(x1, y1);
            i2->getPosition(x2, y2);
            dx = x2 - x1;
            dy = y2 - y1;
            l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (x1 - x2));
            xv = x1 + completion * dx; // new position based on line equation in parameter form
            yv = y1 + completion * dy;
            this->setPosition(xv, yv);

            // check wether halting position in front of destination has been reached
            if (completion >= 0.85 && !hasEnteredIntersection)
            {
               
                // request entry to the current intersection (using async)
                auto ftrEntryGranted = std::async(&Intersection::addVehicleToQueue, _currDestination, get_shared_this());
                this->setCurrentState(VehicleStates::waiting);
                // wait until entry has been granted

                std::unique_lock<std::mutex> lck(_mtx);
                std::cout << "Vehicle #" << _id << " Is waiting at signal"  << std::endl;
                lck.unlock();

                ftrEntryGranted.get();
                lck.lock();
                std::cout << "Vehicle #" << _id << " clear signal"  << std::endl;
                lck.unlock();
                setCurrentState(VehicleStates::crossingIntersection);
                /*give signal to the vehicles who are wating for the vehicle in question*/
                _cloud->sendWeakUPMessage(this->getID());
                // slow down and set intersection flag
                _speed = _max_allowed_speed / 10.0;
                
                hasEnteredIntersection = true;
                
            }

            // check wether intersection has been crossed
            if (completion >= 1.0 && hasEnteredIntersection)
            {
                
                // choose next street and destination
                std::vector<std::shared_ptr<Street>> streetOptions = _currDestination->queryStreets(_currStreet);
                std::shared_ptr<Street> nextStreet;
                if (streetOptions.size() > 0)
                {
                    // pick one street at random and query intersection to enter this street
                    std::random_device rd;
                    std::mt19937 eng(rd());
                    std::uniform_int_distribution<> distr(0, streetOptions.size() - 1);
                    nextStreet = streetOptions.at(distr(eng));
                }
                else
                {
                    // this street is a dead-end, so drive back the same way
                    nextStreet = _currStreet;
                }

               
                
                // pick the one intersection at which the vehicle is currently not
                std::shared_ptr<Intersection> nextIntersection = nextStreet->getInIntersection()->getID() == _currDestination->getID() ? nextStreet->getOutIntersection() : nextStreet->getInIntersection(); 

                // send signal to intersection that vehicle has left the intersection
                _currDestination->vehicleHasLeft(get_shared_this());

                // assign new street and destination
                this->setCurrentDestination(nextIntersection);
                this->setCurrentStreet(nextStreet);

                // reset speed and intersection flag
                _speed = _max_allowed_speed;
                hasEnteredIntersection = false;
                setCurrentState(VehicleStates::moving);
                //_currentState = VehicleStates::moving;
            }    
}
void Vehicle::processCloseVehicle(std::shared_ptr<TrafficObject> other_object,
                                  const bool hasEnteredIntersection,
                                  double completion  )
{
        double vehicle_destination_x;
        double vehicle_destination_y;

        double other_vehicle_currentPosition_x;
        double other_vehicle_currentPosition_y;

        double this_vehicle_currentPosition_x;
        double this_vehicle_currentPosition_y;

        double this_vehicle_distance_to_destination;
        double other_vehicle_distance_to_destination;
    
    
    if ((!hasEnteredIntersection))
    
    {
        std::shared_ptr<Vehicle> other_vehicle = std::dynamic_pointer_cast<Vehicle>(other_object);


        /*since both vehicle have same destination; any one can be used to calculate the destination position*/
        other_vehicle->getCurrentDestination()->getPosition(vehicle_destination_x,vehicle_destination_y);
        other_vehicle->getPosition(other_vehicle_currentPosition_x,other_vehicle_currentPosition_y);
        this->getPosition(this_vehicle_currentPosition_x,this_vehicle_currentPosition_y);

        this_vehicle_distance_to_destination = _cloud->getDistanceBetweenPoints(this_vehicle_currentPosition_x,
        this_vehicle_currentPosition_y, vehicle_destination_x,vehicle_destination_y);

        other_vehicle_distance_to_destination = _cloud->getDistanceBetweenPoints(other_vehicle_currentPosition_x,
        other_vehicle_currentPosition_y, vehicle_destination_x,vehicle_destination_y);


        if ((this->getCurrenStreet()->getID()== other_vehicle->getCurrenStreet()->getID()) &&
           ((this->getCurrentState()!= VehicleStates::crossingIntersection) || (other_vehicle->getCurrentState()!= VehicleStates::crossingIntersection)) && 
           (other_vehicle_distance_to_destination < this_vehicle_distance_to_destination) 
          )
          {

              //based on distance gap reduce speed;
               double distance_between_vehicles =  _cloud->getDistanceBetweenPoints(this_vehicle_currentPosition_x,
        this_vehicle_currentPosition_y, other_vehicle_currentPosition_x,other_vehicle_currentPosition_y);
             

                //after threshold ; 
                //check other vehicle is in wating state; and this vehicle goes to waiting state
                //if not request for speed; and follow the speed
                if (distance_between_vehicles > (100))
                {
                    _speed = _max_allowed_speed / 10.0;

                } else if (other_vehicle->getCurrentState()== VehicleStates::waiting)
                {
                    /* go to waiting state , and send signal that it is waiting for weakup message */
                    this->setCurrentState(VehicleStates::waiting);
                    _cloud->sendWaitingForYouMessage(this->getID(),other_vehicle->getID());

                    std::unique_lock<std::mutex> lck(_mtx);
                    std::cout << "Vehicle #" << _id << " Is waiting -> Vehicle#" << other_vehicle->getID() << std::endl;
                    lck.unlock();

                    _cloud->waitForWeakupMessage(this->getID()); 

                    lck.lock();
                    std::cout << "Vehicle #" << _id << " weakup signal <- Vehicle#" << other_vehicle->getID() << std::endl;
                    lck.unlock();
                    //as soon as waiting done it send weakup message
                    _cloud->sendWeakUPMessage(this->getID());
                    this->setCurrentState(VehicleStates::moving); 
                    
                } else
                {
                    /*just follow the other vehicle*/
                    _speed = other_vehicle->getCurrentSpeed();   
                }
                
                

          }
        
    }
}
// virtual function which is executed in a thread
void Vehicle::drive()
{
    // print id of the current thread
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Vehicle #" << _id << "::drive: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();

    // initalize variables
    bool hasEnteredIntersection = false;
    double cycleDuration = 1; // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        

        
        if (timeSinceLastUpdate >= cycleDuration)
        {
            //_cloud->sendSpeedIfRequested(this->getID(),_speed);
            //_cloud->printAllMessage();
            // update position with a constant velocity motion model
            _posStreet += _speed * timeSinceLastUpdate / 1000;

            // compute completion rate of current street
            double completion = _posStreet / _currStreet->getLength();
            
            processIntersection(hasEnteredIntersection,timeSinceLastUpdate,completion);


            double looking_distance = DISTANCE_TO_COLLISION;
            //std::vector<std::shared_ptr<TrafficObject>> close_objects;
            auto close_objects = _cloud->getCloseObjects(this->getID(),_posX,_posY,looking_distance);

            this->setCloseVehicleId(-1);
            for (auto it : close_objects)
            {
                if ((it->getType() == ObjectType::objectVehicle) &&
                   (std::abs(this->getMovingAngle()-it->getMovingAngle()) <10)
                   )
                {
                    this->setCloseVehicleId(it->getID());
                    processCloseVehicle(it,hasEnteredIntersection,completion);
                }
            }

            
            
            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
        }
    } // eof simulation loop
}
