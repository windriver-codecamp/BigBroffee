################################################################################
# Set minimum required version of cmake, project name and compile options
################################################################################
cmake_minimum_required(VERSION 3.5)
project(t3_control)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

################################################################################
# Find ament packages and libraries for ament and system dependencies
################################################################################
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rcutils REQUIRED)
find_package(rcl REQUIRED)
find_package(turtlebot3_msgs REQUIRED)
find_package(yaml REQUIRED)
################################################################################
# Build
################################################################################
find_path(ASIO_INCLUDE_DIR NAMES)
include_directories(
  include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_STANDALONE") 

add_executable(t3_control ./control.cpp ./keyboard_event.cpp ./gamepad_event.cpp)
target_link_libraries(t3_control mosquitto ssl crypto json)
#modified

ament_target_dependencies( t3_control
  hlds_laser_publisher
  rclcpp
  std_msgs
  sensor_msgs
  rcl
  rclutils
  turtlebot3_msgs
  yaml
)

#added for testing purposes
add_executable(test_subscriber testsubscriber.cpp)
ament_target_dependencies(test_subscriber rclcpp sensor_msgs)

################################################################################
# Install
################################################################################

install(
  TARGETS t3_control
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

################################################################################
# Macro for ament package
################################################################################
ament_export_dependencies(rclcpp)
ament_export_dependencies(std_msgs)
ament_export_dependencies(sensor_msgs)
ament_export_include_directories(include)
ament_package()
