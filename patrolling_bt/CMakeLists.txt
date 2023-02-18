cmake_minimum_required(VERSION 3.5)
project(patrolling_bt)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)
find_package(action_msgs REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(ament_index_cpp REQUIRED)

find_package(ZMQ)
if(ZMQ_FOUND)
    message(STATUS "ZeroMQ found.")
    add_definitions(-DZMQ_FOUND)
else()
  message(WARNING "ZeroMQ NOT found. Not including PublisherZMQ.")
endif()

include_directories(include ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

set(dependencies
  rclcpp
  rclcpp_lifecycle
  rclcpp_action
  behaviortree_cpp_v3
  action_msgs
  lifecycle_msgs
  geometry_msgs
  nav2_msgs
  ament_index_cpp
)

include_directories(include ${ZMQ_INCLUDE_DIRS})

add_library(patrol_node SHARED src/patrolling_bt/Patrol.cpp)
add_library(move_node SHARED src/patrolling_bt/Move.cpp)
add_library(get_waypoint_node SHARED src/patrolling_bt/GetWaypoint.cpp)


list(APPEND plugin_libs
  patrol_node
  move_node
  get_waypoint_node
)

foreach(bt_plugin ${plugin_libs})
  ament_target_dependencies(${bt_plugin} ${dependencies})
  target_compile_definitions(${bt_plugin} PRIVATE BT_PLUGIN_EXPORT)
endforeach()

add_executable(patrolling_bt src/patrolling_bt_main.cpp)
ament_target_dependencies(patrolling_bt ${dependencies})
target_link_libraries(patrolling_bt ${ZMQ_LIBRARIES})

install(TARGETS
  ${plugin_libs}
  patrolling_bt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(DIRECTORY behavior_tree_xml
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_include_directories(include)
ament_export_dependencies(${dependencies})

ament_package()