#ifndef SPEEDLIMIT_H
#define SPEEDLIMIT_H

#include <vector>

#include <memory>
#include "TrafficObject.h"



class SpeedLimit : public TrafficObject
{
public:
    // constructor / desctructor
    SpeedLimit();

    // getters / setters
    void setSpeed(double speed){_speed = speed;};
    double getSpeed(){return _speed;};


private:

    double _speed;
};

#endif
