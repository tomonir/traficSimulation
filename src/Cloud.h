#ifndef CLOUD_H
#define CLOUD_H

#include <string>
#include <vector>
#include "TrafficObject.h"

#include <mutex>
#include <deque>
#include <condition_variable>

enum VehicleMessageTypes {waitingForYou,wakeUP,requestSpeed,returnSpeed};

class VehicleMessage
{
   public:
    int sender_id;
    int receiver_id;
    double requested_sppedMessage;
    VehicleMessageTypes msgType; 
};

template <class T>
class VehicleMessageQueue
{
public:
    void send(T &&msg);
    void waitForWeakUPMessage(const int recevier_object_id);
    std::deque<T> getMessage(const int recevier_object_id,VehicleMessageTypes msgType); 
    bool isWaitingForMe(const int me_id,const int you_id);
    int getWeakUpMsgIndex(const int recevier_object_id);
    void printCurrentQueue();

private:
    std::mutex _mutex;
    std::condition_variable _cond;
    std::deque<T> _queue;
};


class Cloud
{
public:
    // constructor / desctructor

    // getters / setters
    void setTrafficObjects(std::vector<std::shared_ptr<TrafficObject>> &trafficObjects) { _trafficObjects = trafficObjects;
     };
    void  waitForWeakupMessage(const int receiver_id);
    void sendWeakUPMessage(const int receiver_id);
    void sendWaitingForYouMessage(const int sender_id, const int receiver_id);

    void sendSpeedIfRequested(const int sender_id,const double my_speed);
    void sendSpeedRequest(const int sender_id,const int receiver_id);
    double getRequestedSpeed(const int receiver_id);
    // typical behaviour methods
    std::vector<std::shared_ptr<TrafficObject>> getCloseObjects(const int object_id,const double x, const double y, const double distance);
    double getDistanceBetweenPoints(const double aPoint_x,const double aPoint_y,const double bPoint_x,const double bPoint_y);
    void printAllMessage(); 
private:
    // typical behaviour methods
    void sendVehicleMessage(VehicleMessage &&msg);

    // member variables
    std::vector<std::shared_ptr<TrafficObject>> _trafficObjects;
    VehicleMessageQueue<VehicleMessage> _vehicle_messages_queue;
    static std::mutex _cloud_mtx;  
};

#endif
