#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Street;
class Intersection;


class Pedestrian : public TrafficObject, public std::enable_shared_from_this<Pedestrian>
{
public:
    // constructor / desctructor
    Pedestrian(const double speed);

    // getters / setters
    void setCurrentStreet(std::shared_ptr<Street> street) { _currStreet = street; };
    std::shared_ptr<Street> getCurrenStreet(){return _currStreet;}
    
    
    void setCurrentDestination(std::shared_ptr<Intersection> destination);
    std::shared_ptr<Intersection> getCurrentDestination(){return _currDestination;};


    // typical behaviour methods
    void simulate();
    double getCurrentSpeed() {return _speed;}

    // miscellaneous
    std::shared_ptr<Pedestrian> get_shared_this() { return shared_from_this(); }

private:
    // typical behaviour methods
    void move();
    double _posStreet;                              // position on current street                  
    std::shared_ptr<Street> _currStreet;            // street on which the vehicle is currently on
    std::shared_ptr<Intersection> _currDestination; // destination to which the vehicle is currently driving
    double _speed;                                  // ego speed in m/s

};

#endif
