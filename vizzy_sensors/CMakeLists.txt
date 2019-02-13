# http://ros.org/doc/groovy/api/catkin/html/user_guide/supposed.html
cmake_minimum_required(VERSION 2.8.3)
project(vizzy_sensors)

find_package(catkin REQUIRED
  COMPONENTS
    roscpp
    laser_filters
    message_generation
    nodelet
    pcl_ros
    pluginlib
    roscpp
    roslint
    sensor_msgs
    tf
    vizzy_msgs
    )

catkin_package(
  CATKIN_DEPENDS
    hokuyo_node
    laser_filters
    message_runtime
    nodelet
    pcl_ros
    sensor_msgs
    tf
    serial
  INCLUDE_DIRS
    include
  LIBRARIES
    vizzy_sensors_filters
)

include_directories(include ${CMAKE_INSTALL_PREFIX}/include ${catkin_INCLUDE_DIRS} )


## Declare a cpp executable


add_library(vizzy_sensors_filters src/vizzy_sensors_filters.cpp)
target_link_libraries(vizzy_sensors_filters 
  ${Boost_LIBRARIES} 
  ${catkin_LIBRARIES}
)

add_library(vizzy_battery_monitor_interface src/VoltageCurrentSerialInterface.cpp)
target_link_libraries(vizzy_battery_monitor_interface 
${Boost_LIBRARIES} 
  ${catkin_LIBRARIES}
  ${CMAKE_INSTALL_PREFIX}/lib/libserial.so
)
#add_executable(battery_monitor_example src/VoltageCurrentSerialInterface.cpp)

add_executable(battery_state_service src/battery_state_service.cpp)

target_link_libraries(battery_state_service 
  ${Boost_LIBRARIES} 
  ${catkin_LIBRARIES}
  vizzy_battery_monitor_interface
  ${CMAKE_INSTALL_PREFIX}/lib/libserial.so
)

add_executable(battery_charging_state_service src/battery_charging_state_service.cpp)

target_link_libraries(battery_charging_state_service 
  ${Boost_LIBRARIES} 
  ${catkin_LIBRARIES}
  vizzy_battery_monitor_interface
  ${CMAKE_INSTALL_PREFIX}/lib/libserial.so
)

add_executable(kokam_power_publisher src/kokam_power_publisher.cpp)

target_link_libraries(kokam_power_publisher 
  ${Boost_LIBRARIES} 
  ${catkin_LIBRARIES}
  vizzy_battery_monitor_interface
  ${CMAKE_INSTALL_PREFIX}/lib/libserial.so
)

# Install the plugin and the plugin description
install (DIRECTORY include/vizzy_sensors/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})

install (TARGETS vizzy_sensors_filters
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})

install(FILES vizzy_sensors_plugins.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

# Install runtime launch and configuration files
foreach(dir config launch)
  install(DIRECTORY ${dir}/ 
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach()

# unit tests are enabled selectively
if (CATKIN_ENABLE_TESTING)
  find_package(roslaunch REQUIRED)
  roslaunch_add_file_check(launch)
  roslaunch_add_file_check(launch/hokuyo)
endif()
