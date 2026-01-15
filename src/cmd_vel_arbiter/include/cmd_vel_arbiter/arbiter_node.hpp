#ifndef CMD_VEL_ARBITER__ARBITER_NODE_HPP_
#define CMD_VEL_ARBITER__ARBITER_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

class CmdVelArbiter : public rclcpp::Node
{
public:
  CmdVelArbiter();

private:
  void joyCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
  void ultrasonicCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
  void controlLoop();

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr joy_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr ultrasonic_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  geometry_msgs::msg::Twist joy_cmd_;
  geometry_msgs::msg::Twist ultrasonic_cmd_;

  rclcpp::Time last_ultrasonic_time_;
  rclcpp::Duration ultrasonic_timeout_;
  bool ultrasonic_active_;
};

#endif  // CMD_VEL_ARBITER__ARBITER_NODE_HPP_
