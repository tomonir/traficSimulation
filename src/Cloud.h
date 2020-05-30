#ifndef CLOUD_H
#define CLOUD_H

#include <string>
#include <vector>
#include "TrafficObject.h"

class Cloud
{
public:
    // constructor / desctructor

    // getters / setters
    void setTrafficObjects(std::vector<std::shared_ptr<TrafficObject>> &trafficObjects) { _trafficObjects = trafficObjects; };

    // typical behaviour methods
    std::vector<std::shared_ptr<TrafficObject>> getCloseObjects(const double x, const double y, const double distance);

private:
    // typical behaviour methods

    // member variables
    std::vector<std::shared_ptr<TrafficObject>> _trafficObjects;
};

#endif
