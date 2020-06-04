#include <iostream>

#include "Cloud.h"
#include "Intersection.h"
#include <math.h>
#include <string> 


std::mutex Cloud::_cloud_mtx;


template <typename T>
int VehicleMessageQueue<T>::getWeakUpMsgIndex(const int recevier_object_id)
{
    int foundIndex= -1;
    int index=0;
    for (auto &msg :_queue)
    {
        std::cout<<"sender#"<<msg.sender_id<<"recever#"<<msg.receiver_id<<"Msg="<<msg.msgType<<std::endl;
        if ((msg.receiver_id == recevier_object_id) &&
          (msg.msgType == VehicleMessageTypes::wakeUP) )
        {
            foundIndex = index;
            break;
        }
        index++;    
    }
    std::cout<<"WAKEUP FOUND AT"<<foundIndex<<" For #"<<recevier_object_id<<std::ends;
    return foundIndex;
}

template <typename T>
T VehicleMessageQueue<T>::receiveWeakup(const int recevier_object_id)
{

    std::unique_lock<std::mutex> uLock(_mutex);

    _cond.wait(uLock, [this,recevier_object_id] { return ((!_queue.empty()) && (getWeakUpMsgIndex(recevier_object_id)>-1)); });
    int foundIndex = getWeakUpMsgIndex(recevier_object_id);
    T msg = std::move(_queue.at(foundIndex));
    _queue.erase(_queue.begin()+foundIndex);
    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void VehicleMessageQueue<T>::send(T &&msg)
{
   
        std::lock_guard<std::mutex> uLock(_mutex);
        _queue.push_back(std::move(msg));
        _cond.notify_one(); // notify client after pushing new Vehicle into vector

}


template <typename T>
std::deque<T> VehicleMessageQueue<T>::getMessage(const int recevier_object_id,VehicleMessageTypes msgType)
{
    std::deque<T> return_queue;
    std::unique_lock<std::mutex> uLock(_mutex);
    if (!_queue.empty())
    {
        for ( auto iter = _queue.begin(); iter != _queue.end() ; )
        {
            
            if ((iter->receiver_id == recevier_object_id) && 
               (iter->msgType == msgType))
               {
                   return_queue.push_back(*iter);
                   _queue.erase(iter);
               }else
               {
                   ++iter;
               }             
        }
            

    }
    return return_queue;
} 

template <typename T>
bool VehicleMessageQueue<T>::isWaitingForMe(const int me_id,const int you_id)
 {

    bool isFound = false; 
    std::unique_lock<std::mutex> uLock(_mutex); 
    if (!_queue.empty())
    {
        for (auto &msg :_queue)
        {
            if ((msg.receiver_id == me_id) &&
                (msg.sender_id == you_id) &&
            (msg.msgType == VehicleMessageTypes::waitingForYou) )
            {
                isFound = true;
                break;
            }
             
        }        
    }
    //std::cout<<"Waiting for you is "<<isFound<<std::endl;
    return isFound;
 } 


double getDistance(const double & aPoint_x,const double & aPoint_y,
        const double & bPoint_x,const double & bPoint_y) 
{
    double return_distance=0;
    const double x_diff = bPoint_x - aPoint_x;
    const double y_diff = bPoint_y - aPoint_y;
    return_distance = std::sqrt(x_diff * x_diff + y_diff * y_diff);

    //std::cout<<"return distnace is "<<return_distance<<std::endl;
    return return_distance;
}


std::vector<std::shared_ptr<TrafficObject>> Cloud::getCloseObjects(const int object_id, const double x, const double y, const double distance)
{

    std::vector<std::shared_ptr<TrafficObject>> return_objects;
    double posx, posy;
    for (auto it : _trafficObjects)
    {
        it->getPosition(posx, posy);
        if ((it->getID()!= object_id)&& 
            getDistance(x,y,posx,posy)< distance)
        {
            return_objects.push_back(it);
        }
    }

    return return_objects;

}

void Cloud::sendWeakUPMessage(const int receiver_id)
{
     auto veh_msg = _vehicle_messages_queue.getMessage(receiver_id,VehicleMessageTypes::waitingForYou);
     
     //std::unique_lock<std::mutex> lck(_cloud_mtx);
     //std::cout<<"WeakupMessage for " <<receiver_id<< " is total "<<  veh_msg.size()<<std::endl; 
     //lck.unlock();
     for (auto &msg:veh_msg)
     {
         VehicleMessage temp_msg;
         temp_msg.sender_id = receiver_id;
         temp_msg.receiver_id = msg.sender_id;
         temp_msg.msgType = VehicleMessageTypes::wakeUP;
         
         sendVehicleMessage(std::move(temp_msg));
     }


}

void Cloud::sendWaitingForYouMessage(const int sender_id, const int receiver_id)
{
     
    VehicleMessage temp_msg;
    temp_msg.sender_id = sender_id;
    temp_msg.receiver_id = receiver_id;
    temp_msg.msgType = VehicleMessageTypes::waitingForYou;
    std::cout<<"Vehicle #"<<temp_msg.sender_id<<" to Vehicle#"<<temp_msg.receiver_id<<" Waiting for you message"<<std::endl;
    sendVehicleMessage(std::move(temp_msg));

}


void Cloud::waitForWeakupMessage(const int receiver_id)
{
    while (true)
    {
        std::cout<<"Vehicle#"<<receiver_id<<"Wait for weakup....................."<<std::endl;
        VehicleMessage weakupmsg = _vehicle_messages_queue.receiveWeakup(receiver_id);
        std::cout<<"Vehicle#"<<receiver_id<<"weakedup";
        return;
    }
}

bool Cloud::isYouWaitingForMe(const int me_id, const int you_id)
{
     
    return _vehicle_messages_queue.isWaitingForMe(me_id,you_id);

}

void Cloud::sendVehicleMessage(VehicleMessage &&msg)
{
    _vehicle_messages_queue.send(std::move(msg));
}



