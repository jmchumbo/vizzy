#include "ros/ros.h"
#include <vizzy_msgs/KokamBatteryPower.h>
#include <std_msgs/Time.h>
#include <vizzy_sensors/VoltageCurrentSerialInterface.hpp>

#include <sstream>

/**
 * This tutorial demonstrates simple sending of messages over the ROS system.
 */
int main(int argc, char **argv)
{
  /**
   * The ros::init() function needs to see argc and argv so that it can perform
   * any ROS arguments and name remapping that were provided at the command line.
   * For programmatic remappings you can use a different version of init() which takes
   * remappings directly, but for most command-line programs, passing argc and argv is
   * the easiest way to do it.  The third argument to init() is the name of the node.
   *
   * You must call one of the versions of ros::init() before using any other
   * part of the ROS system.
   */
  ros::init(argc, argv, "kokam_power_publisher");

  /**
   * NodeHandle is the main access point to communications with the ROS system.
   * The first NodeHandle constructed will fully initialize this node, and the last
   * NodeHandle destructed will close down the node.
   */
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
  /**
   * The advertise() function is how you tell ROS that you want to
   * publish on a given topic name. This invokes a call to the ROS
   * master node, which keeps a registry of who is publishing and who
   * is subscribing. After this advertise() call is made, the master
   * node will notify anyone who is trying to subscribe to this topic name,
   * and they will in turn negotiate a peer-to-peer connection with this
   * node.  advertise() returns a Publisher object which allows you to
   * publish messages on that topic through a call to publish().  Once
   * all copies of the returned Publisher object are destroyed, the topic
   * will be automatically unadvertised.
   *
   * The second parameter to advertise() is the size of the message queue
   * used for publishing messages.  If messages are published more quickly
   * than we can send them, the number here specifies how many messages to
   * buffer up before throwing some away.
   */
  ros::Publisher chatter_pub = n.advertise<vizzy_msgs::KokamBatteryPower>("kokam_power", 1000);

  ros::Rate loop_rate(rate_);

  /**
   * A count of how many messages we have sent. This is used to create
   * a unique string for each message.
   */
  int count = 0;
  if (!voltage_reader.initComm(*sensor_port))
    return 1;
  
        
  while (ros::ok())
  {
    /**
     * This is a message object. You stuff it with data, and then publish it.
     */
    vizzy_msgs::KokamBatteryPower msg;
    double current=-1.0,voltage=-1.0;
    if (voltage_reader.getSystemPowerSupply(current, voltage) == 0){
        msg.voltage = voltage;
        msg.current = current;
    }
    ros::Time my_time_ = ros::Time::now();
    std_msgs::Time my_time;
    msg.time_stamp.nsec = my_time_.nsec;
    msg.time_stamp.sec = my_time_.nsec;
    //msg.time_stamp.n = my_time;

    /**
     * The publish() function is how you send messages. The parameter
     * is the message object. The type of this object must agree with the type
     * given as a template parameter to the advertise<>() call, as was done
     * in the constructor above.
     */
    chatter_pub.publish(msg);

    ros::spinOnce();

    loop_rate.sleep();
    ++count;
  }


  return 0;
}