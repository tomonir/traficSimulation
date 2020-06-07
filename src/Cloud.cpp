#include <iostream>

#include "Cloud.h"
#include "Intersection.h"
#include <math.h>
#include <string> 


std::mutex Cloud::_cloud_mtx;


template <typename T>
void VehicleMessageQueue<T>::printCurrentQueue()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    if (_queue.size()>0) std::cout<<"#############################"<<std::endl; 
    for (auto &msg :_queue)
    {
        std::cout<<"QUEUE->> sender#"<<msg.sender_id<<"recever#"<<msg.receiver_id<<"Msg="<<msg.msgType<<std::endl;
    }
}


template <typename T>
int VehicleMessageQueue<T>::getWeakUpMsgIndex(const int recevier_object_id)
{
    int found = 0;
    if (!_queue.empty())
    {
        for ( auto iter = _queue.begin(); iter != _queue.end() ; )
        {
            
            if ((iter->receiver_id == recevier_object_id) && 
               (iter->msgType == VehicleMessageTypes::wakeUP))
               {
                   //return_queue.push_back(*iter);
                   _queue.erase(iter);
                   found = 1;
                    std::cout<<"QUEUE->> Weakup Found for Vehicle#"<<recevier_object_id<<std::endl;
                   //break;
               }else
               {
                   ++iter;
               }             
        }
            

    }

    return found;




}

template <typename T>
void VehicleMessageQueue<T>::waitForWeakUPMessage(const int recevier_object_id)
{

    std::unique_lock<std::mutex> uLock(_mutex);

    _cond.wait(uLock, [this,recevier_object_id] { return  (getWeakUpMsgIndex(recevier_object_id)>0); });
    
    return ;
}

template <typename T>
void VehicleMessageQueue<T>::send(T &&msg)
{
        
        std::lock_guard<std::mutex> uLock(_mutex);
        bool notify= false;
        if (msg.msgType == VehicleMessageTypes::wakeUP) notify= true;
        _queue.push_back(std::move(msg));
        if (notify) _cond.notify_all(); // notify client after pushing new Vehicle into vector
        

}


template <typename T>
std::deque<T> VehicleMessageQueue<T>::getMessage(const int recevier_object_id,VehicleMessageTypes msgType)
{
    
    std::unique_lock<std::mutex> uLock(_mutex);
    std::deque<T> return_queue;
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

     
    std::unique_lock<std::mutex> uLock(_mutex); 
    bool isFound = false;
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


double getDistance(const double aPoint_x,const double aPoint_y,
        const double bPoint_x,const double bPoint_y) 
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


double Cloud::getDistanceBetweenPoints(const double aPoint_x,const double aPoint_y,
        const double bPoint_x,const double bPoint_y)
{

return getDistance(aPoint_x,aPoint_y,bPoint_x,bPoint_y);

} 

void Cloud::sendSpeedIfRequested(const int sender_id,const double my_speed)
{
    auto veh_msg = _vehicle_messages_queue.getMessage(sender_id,VehicleMessageTypes::requestSpeed);

    for (auto &msg:veh_msg)
    {
        VehicleMessage temp_msg;
        temp_msg.sender_id = sender_id;
        temp_msg.receiver_id = msg.sender_id;
        temp_msg.requested_sppedMessage = my_speed;
        temp_msg.msgType = VehicleMessageTypes::returnSpeed;

        sendVehicleMessage(std::move(temp_msg));
    }
}


void Cloud::printAllMessage()
{
    _vehicle_messages_queue.printCurrentQueue();
}
double Cloud::getRequestedSpeed(const int receiver_id)
{
    double return_speed = 0;
    auto veh_msg = _vehicle_messages_queue.getMessage(receiver_id,VehicleMessageTypes::returnSpeed);

    for (auto &msg:veh_msg)
    {
        return msg.requested_sppedMessage;
    }
    return return_speed;
}

void Cloud::sendSpeedRequest(const int sender_id,const int receiver_id)
{
    VehicleMessage temp_msg;
    temp_msg.sender_id = sender_id;
    temp_msg.receiver_id = receiver_id;
    temp_msg.msgType = VehicleMessageTypes::requestSpeed;

    sendVehicleMessage(std::move(temp_msg));
}

void Cloud::sendWeakUPMessage(const int receiver_id)
{
     
     std::unique_lock<std::mutex> lck(_cloud_mtx);
     auto veh_msg = _vehicle_messages_queue.getMessage(receiver_id,VehicleMessageTypes::waitingForYou);
     
;
     for (auto &msg:veh_msg)
     {
         VehicleMessage temp_msg;
         temp_msg.sender_id = receiver_id;
         temp_msg.receiver_id = msg.sender_id;
         temp_msg.msgType = VehicleMessageTypes::wakeUP;
         

        std::cout<<"WeakupMessage from " <<temp_msg.sender_id<< " to Vehicle#"<< temp_msg.receiver_id<<std::endl; 

        sendVehicleMessage(std::move(temp_msg));
        
     }


}

void Cloud::sendWaitingForYouMessage(const int sender_id, const int receiver_id)
{
     
    VehicleMessage temp_msg;
    temp_msg.sender_id = sender_id;
    temp_msg.receiver_id = receiver_id;
    temp_msg.msgType = VehicleMessageTypes::waitingForYou;
    std::cout<<"Vehicle #"<<temp_msg.sender_id<<" -> Vehicle#"<<temp_msg.receiver_id<<" Waiting for you message"<<std::endl;
    sendVehicleMessage(std::move(temp_msg));

}


void Cloud::waitForWeakupMessage(const int receiver_id)
{
    while (true)
    {
        //std::cout<<"Vehicle#"<<receiver_id<<"Wait for weakup....................."<<std::endl;
        _vehicle_messages_queue.waitForWeakUPMessage(receiver_id);
        //std::cout<<"Vehicle#"<<receiver_id<<"weakedup";
        return;
    }
}



void Cloud::sendVehicleMessage(VehicleMessage &&msg)
{
    _vehicle_messages_queue.send(std::move(msg));
}






double Cloud::getDotProduct(const TwoDVector a, const TwoDVector b)
{
return (a.x*b.x)+(a.y*b.y);
}
double Cloud::getVectorMagnitude(const TwoDVector a)
{
return std::sqrt((a.x*a.x)+(a.y*a.y));
}
TwoDVector Cloud::getProjectionPoint(const TwoDVector a, const TwoDVector b )
{
    TwoDVector return_vector = a;
    
    double magnatude_a = getVectorMagnitude(a);
    double dotProduct_a_b = getDotProduct(a,b);

    return_vector.x *= (dotProduct_a_b/(magnatude_a*magnatude_a));
    return_vector.y *= (dotProduct_a_b/(magnatude_a*magnatude_a));

    return return_vector;
}

