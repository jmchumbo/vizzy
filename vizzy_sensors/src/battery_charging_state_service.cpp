#include "ros/ros.h"
#include <vizzy_msgs/BatteryChargingState.h>
#include <vizzy_sensors/VoltageCurrentSerialInterface.hpp>

std::string *sensor_port;
std::string *segway_port;
bool check_segway()
{
    //segway_rmp::Path path = ros::topic::waitForMessage("/path_planned/edge");
    return false;
}

bool check_batteries()
{
    VoltageCurrentSerialInterface voltage_reader;
    if (voltage_reader.initComm(*sensor_port))
    {
        double voltage;
        double current;
        if (voltage_reader.getSystemPowerSupply(current, voltage) == 0)
            cout << "Hello" << endl;
    }
    return false;

}


uint8_t battery_charging_state()
{
    //Compute state
    if (check_segway() && check_batteries())
        return vizzy_msgs::BatteryChargingStateResponse::CHARGING;
    else
        return vizzy_msgs::BatteryChargingStateResponse::NOT_CHARGING;
    
}

bool query_charging_state(vizzy_msgs::BatteryChargingStateRequest &req,
                 vizzy_msgs::BatteryChargingStateResponse &res)
{
    res.battery_charging_state = battery_charging_state();
    ROS_INFO("sending back response: [%ld]", res.battery_charging_state);
    return true;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "battery_charging_state_server");
    if (argc != 3)
    {
        ROS_INFO("usage: battery_charging_state_server <id-mind port, /dev/ttyUSBX> <segway topic /my_topic>");
        return 1;
    }
    ros::NodeHandle n;
    sensor_port = new std::string(argv[1]);
    segway_port = new std::string(argv[2]);
    cout << "Id-mind Port: " << *sensor_port << " Segway port: " << *segway_port << endl;
    ros::ServiceServer service = n.advertiseService("battery_charging_state", query_charging_state);
    ROS_INFO("Ready to check Vizzy's charging state.");
    ros::spin();

    return 0;
}