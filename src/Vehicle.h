#ifndef VEHICLE_H
#define VEHICLE_H

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Street;
class Intersection;
class Cloud;

class Vehicle : public TrafficObject, public std::enable_shared_from_this<Vehicle>
{
public:
    // constructor / desctructor
    Vehicle();

    // getters / setters
    void setCurrentStreet(std::shared_ptr<Street> street) { _currStreet = street; };
    void setCloud(std::shared_ptr<Cloud> cloud){_cloud=cloud;};
    void setCurrentDestination(std::shared_ptr<Intersection> destination);

    // typical behaviour methods
    void simulate();
    

    // miscellaneous
    std::shared_ptr<Vehicle> get_shared_this() { return shared_from_this(); }

private:
    // typical behaviour methods
    void drive();
    void processIntersection(bool &hasEnteredIntersection,const long timeSinceLastUpdate);

    std::shared_ptr<Cloud> _cloud;
    std::shared_ptr<Street> _currStreet;            // street on which the vehicle is currently on
    std::shared_ptr<Intersection> _currDestination; // destination to which the vehicle is currently driving
    double _posStreet;                              // position on current street
    double _speed;                                  // ego speed in m/s
};

#endif