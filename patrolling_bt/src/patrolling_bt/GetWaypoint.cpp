// Copyright 2021 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <iostream>
#include <vector>

#include "patrolling_bt/GetWaypoint.hpp"

#include "behaviortree_cpp_v3/behavior_tree.h"

#include "geometry_msgs/msg/pose_stamped.hpp"

#include "rclcpp/rclcpp.hpp"

namespace patrolling_bt
{

int GetWaypoint::current_ = 0;
int waypoints_number_ = 4;

GetWaypoint::GetWaypoint(
  const std::string & xml_tag_name,
  const BT::NodeConfiguration & conf)
: BT::ActionNodeBase(xml_tag_name, conf)
{
  rclcpp::Node::SharedPtr node;
  config().blackboard->get("node", node);

  geometry_msgs::msg::PoseStamped wp;
  wp.header.frame_id = "map";
  wp.pose.orientation.w = 1.0;

  //Con el mapa se obtiene los waypoints
  // Wp final
  wp.pose.position.x = -1.85;
  wp.pose.position.y = -1.07;
  final_goal_ = wp;

  // wp1
  wp.pose.position.x = 6.79;
  wp.pose.position.y = -2.89;
  waypoints_.push_back(wp);

  // wp2
  wp.pose.position.x = 0.15;
  wp.pose.position.y = 2.18;
  waypoints_.push_back(wp);


  // wp3
  wp.pose.position.x = -4.86;
  wp.pose.position.y = -3.23;
  waypoints_.push_back(wp);

  // wp4
  wp.pose.position.x = -5.33;
  wp.pose.position.y = -0.02;
  waypoints_.push_back(wp);
}

void
GetWaypoint::halt()
{
}

BT::NodeStatus
GetWaypoint::tick()
{
  if (current_ < waypoints_number_) {
    setOutput("waypoint", waypoints_[current_++]);
  } else if (current_ == waypoints_number_) { // On last waypoint
    setOutput("waypoint", final_goal_);
    current_++;
  } else { // Arrived to final_goal_
    return BT::NodeStatus::FAILURE;
  }
  return BT::NodeStatus::SUCCESS;
}

}  // namespace patrolling_bt

#include "behaviortree_cpp_v3/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<patrolling_bt::GetWaypoint>("GetWaypoint");
}
