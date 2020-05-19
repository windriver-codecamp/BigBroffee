################################################################################
# Set minimum required version of cmake, project name and compile options
################################################################################
cmake_minimum_required(VERSION 3.5)
project(obj_avoid_t3)

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

add_executable(obj_avoid_t3 ./control.cpp ./keyboard_event.cpp ./gamepad_event.cpp)
target_link_libraries(obj_avoid_t3 mosquitto ssl crypto json)
#modificat

ament_target_dependencies(obj_avoid_t3
  hlds_laser_publisher
  rclcpp
  std_msgs
  sensor_msgs
  rcl
  rclutils
  turtlebot3_msgs
  yaml
)

################################################################################
# Install
################################################################################

install(
  TARGETS obj_avoid_t3
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
