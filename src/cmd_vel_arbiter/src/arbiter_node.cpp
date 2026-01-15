#include "cmd_vel_arbiter/arbiter_node.hpp"

using std::placeholders::_1;

CmdVelArbiter::CmdVelArbiter()
: Node("cmd_vel_arbiter"),
  ultrasonic_timeout_(rclcpp::Duration::from_seconds(0.5)),
  ultrasonic_active_(false)
{
  joy_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
    "/vel_joy", 10,
    std::bind(&CmdVelArbiter::joyCallback, this, _1)
  );

  ultrasonic_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
    "/vel_ultrasonic", 10,
    std::bind(&CmdVelArbiter::ultrasonicCallback, this, _1)
  );

  cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
    "/cmd_vel", 10
  );

  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(20),
    std::bind(&CmdVelArbiter::controlLoop, this)
  );

  RCLCPP_INFO(get_logger(), "CmdVel Arbiter with HARD STOP safety started");
}

void CmdVelArbiter::joyCallback(
  const geometry_msgs::msg::Twist::SharedPtr msg)
{
  joy_cmd_ = *msg;
}

void CmdVelArbiter::ultrasonicCallback(
  const geometry_msgs::msg::Twist::SharedPtr /*msg*/)
{
  // Ultrasonic = EMERGENCY STOP trigger
  ultrasonic_active_ = true;
  last_ultrasonic_time_ = now();

  // 🚨 IMMEDIATE STOP
  geometry_msgs::msg::Twist stop;
  cmd_vel_pub_->publish(stop);
}

void CmdVelArbiter::controlLoop()
{
  auto current_time = now();

  if (ultrasonic_active_) {
    // Keep robot stopped
    if ((current_time - last_ultrasonic_time_) < ultrasonic_timeout_) {
      geometry_msgs::msg::Twist stop;
      cmd_vel_pub_->publish(stop);
      return;
    } else {
      ultrasonic_active_ = false;
    }
  }

  // Normal joystick control
  cmd_vel_pub_->publish(joy_cmd_);
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CmdVelArbiter>());
  rclcpp::shutdown();
  return 0;
}
