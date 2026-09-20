cmake_minimum_required(VERSION 3.8)
project(ur_viet_chu)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

add_executable(node_dieu_khien src/node_dieu_khien.cpp)
target_include_directories(node_dieu_khien PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(node_dieu_khien PUBLIC c_std_99 cxx_std_17)
ament_target_dependencies(
  node_dieu_khien
  rclcpp
  moveit_ros_planning_interface
  geometry_msgs
  visualization_msgs
)

install(TARGETS node_dieu_khien
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()
