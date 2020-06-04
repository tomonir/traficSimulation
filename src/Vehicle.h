#ifndef VEHICLE_H
#define VEHICLE_H

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Street;
class Intersection;
class Cloud;

enum VehicleStates { moving, waiting,slowing,crossingIntersection };

class Vehicle : public TrafficObject, public std::enable_shared_from_this<Vehicle>
{
public:
    // constructor / desctructor
    Vehicle();

    // getters / setters
    void setCurrentStreet(std::shared_ptr<Street> street) { _currStreet = street; };
    std::shared_ptr<Street> getCurrenStreet(){return _currStreet;}
    void setCloud(std::shared_ptr<Cloud> cloud){_cloud=cloud;};
    void setCurrentDestination(std::shared_ptr<Intersection> destination);
    void setCurrentState(VehicleStates current_sate){ _currentState = current_sate;}
    VehicleStates getCurrentState(){return _currentState;}
    // typical behaviour methods
    void simulate();
    

    // miscellaneous
    std::shared_ptr<Vehicle> get_shared_this() { return shared_from_this(); }

private:
    // typical behaviour methods
    void drive();
    void processIntersection(bool &hasEnteredIntersection,const long timeSinceLastUpdate,
    double completion);
    void processCloseVehicle(std::shared_ptr<TrafficObject> other_object,
                             const bool hasEnteredIntersection,
                             double completion);

    std::shared_ptr<Cloud> _cloud; // information source
    VehicleStates _currentState;  // current state of the vehicle
    std::shared_ptr<Street> _currStreet;            // street on which the vehicle is currently on
    std::shared_ptr<Intersection> _currDestination; // destination to which the vehicle is currently driving
    double _posStreet;                              // position on current street
    double _speed;                                  // ego speed in m/s
};

#endif