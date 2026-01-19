#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class VelJoyPublisher : public rclcpp::Node
{
public:
  VelJoyPublisher() : Node("vel_joy_publisher")
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "/vel_joy", 10);

    timer_ = this->create_wall_timer(
      100ms, std::bind(&VelJoyPublisher::publishCmd, this));

    RCLCPP_INFO(this->get_logger(), "VelJoy publisher started");
  }

private:
  void publishCmd()
  {
    geometry_msgs::msg::Twist cmd;
    cmd.linear.x = 0.3;   // forward speed (m/s)
    cmd.angular.z = 0.0; // no turning

    publisher_->publish(cmd);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VelJoyPublisher>());
  rclcpp::shutdown();
  return 0;
}
