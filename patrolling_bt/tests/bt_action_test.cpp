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
#include <list>
#include <memory>
#include <vector>
#include <set>

#include "behaviortree_cpp_v3/behavior_tree.h"
#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/utils/shared_library.h"

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "geometry_msgs/msg/twist.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "lifecycle_msgs/msg/transition.hpp"
#include "lifecycle_msgs/msg/state.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "gtest/gtest.h"

using namespace std::placeholders;
using namespace std::chrono_literals;


class VelocitySinkNode : public rclcpp::Node
{
public:
  VelocitySinkNode()
  : Node("VelocitySink")
  {
    vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "/output_vel", 100, std::bind(&VelocitySinkNode::vel_callback, this, _1));
  }

  void vel_callback(geometry_msgs::msg::Twist::SharedPtr msg)
  {
    vel_msgs_.push_back(*msg);
  }

  std::list<geometry_msgs::msg::Twist> vel_msgs_;

private:
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr vel_sub_;
};

class Nav2FakeServer : public rclcpp::Node
{
  using NavigateToPose = nav2_msgs::action::NavigateToPose;
  using GoalHandleNavigateToPose = rclcpp_action::ServerGoalHandle<NavigateToPose>;

public:
  Nav2FakeServer()
  : Node("nav2_fake_server_node") {}

  void start_server()
  {
    move_action_server_ = rclcpp_action::create_server<NavigateToPose>(
      shared_from_this(),
      "navigate_to_pose",
      std::bind(&Nav2FakeServer::handle_goal, this, _1, _2),
      std::bind(&Nav2FakeServer::handle_cancel, this, _1),
      std::bind(&Nav2FakeServer::handle_accepted, this, _1));
  }

private:
  rclcpp_action::Server<NavigateToPose>::SharedPtr move_action_server_;

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const NavigateToPose::Goal> goal)
  {
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
  {
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
  {
    std::thread{std::bind(&Nav2FakeServer::execute, this, _1), goal_handle}.detach();
  }

  void execute(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
  {
    auto feedback = std::make_shared<NavigateToPose::Feedback>();
    auto result = std::make_shared<NavigateToPose::Result>();

    auto start = now();

    while ((now() - start) < 5s) {
      feedback->distance_remaining = 5.0 - (now() - start).seconds();
      goal_handle->publish_feedback(feedback);
    }

    goal_handle->succeed(result);
  }
};

class StoreWP : public BT::ActionNodeBase
{
public:
  explicit StoreWP(
    const std::string & xml_tag_name,
    const BT::NodeConfiguration & conf)
  : BT::ActionNodeBase(xml_tag_name, conf) {}

  void halt() {}
  BT::NodeStatus tick()
  {
    waypoints_.push_back(getInput<geometry_msgs::msg::PoseStamped>("in").value());
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return BT::PortsList(
    {
      BT::InputPort<geometry_msgs::msg::PoseStamped>("in")
    });
  }

  static std::vector<geometry_msgs::msg::PoseStamped> waypoints_;
};

std::vector<geometry_msgs::msg::PoseStamped> StoreWP::waypoints_;

TEST(bt_action, patrol_btn)
{
  auto node = rclcpp::Node::make_shared("patrol_btn_node");
  auto node_sink = std::make_shared<VelocitySinkNode>();

  BT::BehaviorTreeFactory factory;
  BT::SharedLibrary loader;

  factory.registerFromPlugin(loader.getOSName("patrol_node"));

  std::string xml_bt =
    R"(
    <root main_tree_to_execute = "MainTree" >
      <BehaviorTree ID="MainTree">
          <Patrol    name="patrol"/>
      </BehaviorTree>
    </root>)";

  auto blackboard = BT::Blackboard::create();
  blackboard->set("node", node);
  BT::Tree tree = factory.createTreeFromText(xml_bt, blackboard);

  rclcpp::Rate rate(10);

  bool finish = false;
  int counter = 0;
  while (!finish && rclcpp::ok()) {
    finish = tree.rootNode()->executeTick() == BT::NodeStatus::SUCCESS;
    rclcpp::spin_some(node_sink->get_node_base_interface());
    rate.sleep();
  }

  ASSERT_FALSE(node_sink->vel_msgs_.empty());
  ASSERT_NEAR(node_sink->vel_msgs_.size(), 150, 2);

  geometry_msgs::msg::Twist & one_twist = node_sink->vel_msgs_.front();

  ASSERT_GT(one_twist.angular.z, 0.1);
  ASSERT_NEAR(one_twist.linear.x, 0.0, 0.0000001);
}

TEST(bt_action, move_btn)
{
  auto node = rclcpp::Node::make_shared("move_node");
  auto nav2_fake_node = std::make_shared<Nav2FakeServer>();

  nav2_fake_node->start_server();

  bool finish = false;
  std::thread t([&]() {
      while (!finish) {rclcpp::spin_some(nav2_fake_node);}
    });


  BT::BehaviorTreeFactory factory;
  BT::SharedLibrary loader;

  factory.registerFromPlugin(loader.getOSName("move_node"));

  std::string xml_bt =
    R"(
    <root main_tree_to_execute = "MainTree" >
      <BehaviorTree ID="MainTree">
          <Move    name="move" goal="{goal}"/>
      </BehaviorTree>
    </root>)";

  auto blackboard = BT::Blackboard::create();
  blackboard->set("node", node);

  geometry_msgs::msg::PoseStamped goal;
  blackboard->set("goal", goal);

  BT::Tree tree = factory.createTreeFromText(xml_bt, blackboard);

  rclcpp::Rate rate(10);

  int counter = 0;
  while (!finish && rclcpp::ok()) {
    finish = tree.rootNode()->executeTick() == BT::NodeStatus::SUCCESS;
    rate.sleep();
  }

  t.join();
}

TEST(bt_action, get_waypoint_btn)
{
  auto node = rclcpp::Node::make_shared("get_waypoint_node");

  rclcpp::spin_some(node);

  {
    BT::BehaviorTreeFactory factory;
    BT::SharedLibrary loader;

    factory.registerFromPlugin(loader.getOSName("get_waypoint_node"));

    std::string xml_bt =
      R"(
      <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
          <GetWaypoint    name="final_pose" waypoint="{waypoint}"/>
        </BehaviorTree>
      </root>)";

    auto blackboard = BT::Blackboard::create();
    blackboard->set("node", node);

    BT::Tree tree = factory.createTreeFromText(xml_bt, blackboard);

    rclcpp::Rate rate(10);

    bool finish = false;
    int counter = 0;
    while (!finish && rclcpp::ok()) {
      finish = tree.rootNode()->executeTick() == BT::NodeStatus::SUCCESS;
      counter++;
      rate.sleep();
    }

    auto point = blackboard->get<geometry_msgs::msg::PoseStamped>("waypoint");

    ASSERT_EQ(counter, 1);
    ASSERT_NEAR(point.pose.position.x, 3.67, 0.0000001);
    ASSERT_NEAR(point.pose.position.y, -0.24, 0.0000001);
  }

  {
    BT::BehaviorTreeFactory factory;
    BT::SharedLibrary loader;

    factory.registerNodeType<StoreWP>("StoreWP");
    factory.registerFromPlugin(loader.getOSName("get_waypoint_node"));

    std::string xml_bt =
      R"(
      <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
          <Sequence name="root_sequence">
             <GetWaypoint    name="wp1" waypoint="{waypoint}"/>
             <StoreWP in="{waypoint}"/>
             <GetWaypoint    name="wp2" waypoint="{waypoint}"/>
             <StoreWP in="{waypoint}"/>
             <GetWaypoint    name="wp3" waypoint="{waypoint}"/>
             <StoreWP in="{waypoint}"/>
             <GetWaypoint    name="wp4" waypoint="{waypoint}"/>
             <StoreWP in="{waypoint}"/>
          </Sequence>
        </BehaviorTree>
      </root>)";

    auto blackboard = BT::Blackboard::create();
    blackboard->set("node", node);

    BT::Tree tree = factory.createTreeFromText(xml_bt, blackboard);

    rclcpp::Rate rate(10);

    bool finish = false;
    while (!finish && rclcpp::ok()) {
      finish = tree.rootNode()->executeTick() == BT::NodeStatus::SUCCESS;
      rate.sleep();
    }

    const auto & waypoints = StoreWP::waypoints_;
    ASSERT_EQ(waypoints.size(), 4);
    ASSERT_NEAR(waypoints[0].pose.position.x, 6.79, 0.0000001);
    ASSERT_NEAR(waypoints[0].pose.position.y, -2.89, 0.0000001);
    ASSERT_NEAR(waypoints[1].pose.position.x, 0.15, 0.0000001);
    ASSERT_NEAR(waypoints[1].pose.position.y, 2.18, 0.0000001);
    ASSERT_NEAR(waypoints[2].pose.position.x, -4.86, 0.0000001);
    ASSERT_NEAR(waypoints[2].pose.position.y, -3.23, 0.0000001);
    ASSERT_NEAR(waypoints[3].pose.position.x, -5.33, 0.0000001);
    ASSERT_NEAR(waypoints[3].pose.position.y, -0.02, 0.0000001);
  }
}


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
