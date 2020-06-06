#include <algorithm>
#include <iostream>
#include <chrono>
#include "TrafficObject.h"
#include <math.h>
// init static variable
int TrafficObject::_idCnt = 0;

std::mutex TrafficObject::_mtx;

void TrafficObject::setPosition(double x, double y)
{

    _previous_posX = _posX;
    _previous_posY = _posY;

    _posX = x;
    _posY = y;
}


void TrafficObject::getPosition(double &x, double &y)
{
    x = _posX;
    y = _posY;
}

void TrafficObject::getPreviousPosition(double &x, double &y)
{
    x = _previous_posX;
    y = _previous_posY;
}
double TrafficObject::getMovingAngle()
{
    double degrees = std::atan2(_posY - _previous_posY, _posX - _previous_posX) * 180 / M_PI;

    /*
    if (degrees > 90)
    {
        degrees = 450 - degrees;
    }
    else
    {
        degrees = 90 - degrees;
    }*/

    return degrees;
    
}

TrafficObject::TrafficObject()
{
    _type = ObjectType::noObject;
    _id = _idCnt++;
}

TrafficObject::~TrafficObject()
{
    // set up thread barrier before this object is destroyed
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });
}
