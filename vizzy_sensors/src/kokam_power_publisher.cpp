#include "ros/ros.h"
#include <vizzy_msgs/KokamBatteryPower.h>
#include <std_msgs/Time.h>
#include <vizzy_sensors/VoltageCurrentSerialInterface.hpp>

#include <sstream>

int main(int argc, char **argv)
{
    ros::init(argc, argv, "kokam_power_publisher");
    ros::NodeHandle n;
    if (argc != 3)
    {
        ROS_INFO("usage: kokam_power_publisher <kokam battery sensor port, /dev/ttyUSBX> <rate>");
        return 1;
    }
    std::string *sensor_port;
    double rate_;
    sensor_port = new std::string(argv[1]);
    rate_ = atof(argv[2]);
    VoltageCurrentSerialInterface voltage_reader;
    ros::Publisher chatter_pub = n.advertise<vizzy_msgs::KokamBatteryPower>("kokam_power", 1000);

    ros::Rate loop_rate(rate_);

    int count = 0;
    if (!voltage_reader.initComm(*sensor_port))
        return 1;

    while (ros::ok())
    {
        vizzy_msgs::KokamBatteryPower msg;
        double current = -1.0, voltage = -1.0;
        if (voltage_reader.getSystemPowerSupply(current, voltage) == 0)
        {
            msg.voltage = voltage;
            msg.current = current;
            ros::Time my_time_ = ros::Time::now();
            std_msgs::Time my_time;
            msg.time_stamp.nsec = my_time_.nsec;
            msg.time_stamp.sec = my_time_.sec;
            chatter_pub.publish(msg);
        }

        ros::spinOnce();

        loop_rate.sleep();
        ++count;
    }

    return 0;
}