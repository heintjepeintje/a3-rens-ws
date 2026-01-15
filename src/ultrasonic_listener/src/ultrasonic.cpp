#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/range.hpp"
#include "std_msgs/msg/bool.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class MultiUltrasonicReader : public rclcpp::Node
{
public:
  MultiUltrasonicReader()
  : Node("multi_ultrasonic_reader"),
    stop_duration_(rclcpp::Duration::from_seconds(5.0))  // stop for 5 seconds
  {
    // Publish ONLY to ultrasonic cmd_vel (arbiter input)
    cmd_vel_publisher_ =
      create_publisher<geometry_msgs::msg::Twist>("/vel_ultrasonic", 10);

    // Buzzer publisher
    buzzer_pub_ = create_publisher<std_msgs::msg::Bool>("/buzzer", 10);

    // Subscriptions for ultrasonic sensors
    sub1_ = create_subscription<sensor_msgs::msg::Range>(
        "/ultrasonic/range1", 10,
        std::bind(&MultiUltrasonicReader::cb1, this, std::placeholders::_1));

    sub2_ = create_subscription<sensor_msgs::msg::Range>(
        "/ultrasonic/range2", 10,
        std::bind(&MultiUltrasonicReader::cb2, this, std::placeholders::_1));

    sub3_ = create_subscription<sensor_msgs::msg::Range>(
        "/ultrasonic/range3", 10,
        std::bind(&MultiUltrasonicReader::cb3, this, std::placeholders::_1));

    // 20 Hz safety evaluation
    timer_ = create_wall_timer(
      50ms, std::bind(&MultiUltrasonicReader::publishSafetyCmd, this));

    last_obstacle_time_ = now();

    RCLCPP_INFO(get_logger(),
      "MultiUltrasonicReader started (arbiter-compatible)");
  }

private:
  void publishSafetyCmd()
  {
    rclcpp::Time current_time = now();

    bool obstacle_detected =
      (range1_ < 0.30) ||
      (range2_ < 0.30) ||
      (range3_ < 0.30);

    if (obstacle_detected) {
      last_obstacle_time_ = current_time;
    }

    bool should_stop =
      (current_time - last_obstacle_time_) < stop_duration_;

    if (should_stop) {
      geometry_msgs::msg::Twist stop_cmd;
      stop_cmd.linear.x = 0.0;
      stop_cmd.linear.y = 0.0;
      stop_cmd.angular.z = 0.0;

      cmd_vel_publisher_->publish(stop_cmd);

      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 1000,
        "Obstacle detected → STOPPING robot | Ranges: %.2f, %.2f, %.2f",
        range1_, range2_, range3_);

      if (!last_buzzer_state_) {
        std_msgs::msg::Bool buzzer;
        buzzer.data = true;
        buzzer_pub_->publish(buzzer);
        last_buzzer_state_ = true;
      }
    } else {
      // No obstacle → ultrasonic node stays silent
      if (last_buzzer_state_) {
        std_msgs::msg::Bool buzzer;
        buzzer.data = false;
        buzzer_pub_->publish(buzzer);
        last_buzzer_state_ = false;
      }
    }
  }

  void cb1(const sensor_msgs::msg::Range::SharedPtr msg) { range1_ = msg->range; }
  void cb2(const sensor_msgs::msg::Range::SharedPtr msg) { range2_ = msg->range; }
  void cb3(const sensor_msgs::msg::Range::SharedPtr msg) { range3_ = msg->range; }

  // Subscriptions
  rclcpp::Subscription<sensor_msgs::msg::Range>::SharedPtr sub1_;
  rclcpp::Subscription<sensor_msgs::msg::Range>::SharedPtr sub2_;
  rclcpp::Subscription<sensor_msgs::msg::Range>::SharedPtr sub3_;

  // Publishers
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr buzzer_pub_;

  // Timer
  rclcpp::TimerBase::SharedPtr timer_;

  // Sensor values
  float range1_ = 100.0;
  float range2_ = 100.0;
  float range3_ = 100.0;

  // Buzzer state
  bool last_buzzer_state_ = false;

  // Stop duration tracking
  rclcpp::Time last_obstacle_time_;
  rclcpp::Duration stop_duration_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MultiUltrasonicReader>());
  rclcpp::shutdown();
  return 0;
}
