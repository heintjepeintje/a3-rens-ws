#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/range.hpp"
#include <chrono>
#include <random>

using namespace std::chrono_literals;

class MultiUltrasonicPublisher : public rclcpp::Node
{
public:
  MultiUltrasonicPublisher() : Node("multi_ultrasonic_publisher")
  {
    // Publishers for 3 ultrasonic sensors
    pub1_ = create_publisher<sensor_msgs::msg::Range>("/ultrasonic/range1", 10);
    pub2_ = create_publisher<sensor_msgs::msg::Range>("/ultrasonic/range2", 10);
    pub3_ = create_publisher<sensor_msgs::msg::Range>("/ultrasonic/range3", 10);

    // Timer to publish at 5 Hz
    timer_ = create_wall_timer(200ms, std::bind(&MultiUltrasonicPublisher::timer_callback, this));

    // Random generator for dummy sensor data
    rng_ = std::mt19937(rd_());
    dist_ = std::uniform_real_distribution<float>(0.05, 2.0); // 5 cm – 2 m
  }

private:
  void timer_callback()
  {
    auto msg1 = sensor_msgs::msg::Range();
    msg1.header.stamp = now();
    msg1.header.frame_id = "ultrasonic_1";
    msg1.radiation_type = sensor_msgs::msg::Range::ULTRASOUND;
    msg1.field_of_view = 0.5; // radians
    msg1.min_range = 0.02;
    msg1.max_range = 4.0;
    msg1.range = dist_(rng_);
    pub1_->publish(msg1);

    auto msg2 = msg1;
    msg2.header.frame_id = "ultrasonic_2";
    msg2.range = dist_(rng_);
    pub2_->publish(msg2);

    auto msg3 = msg1;
    msg3.header.frame_id = "ultrasonic_3";
    msg3.range = dist_(rng_);
    pub3_->publish(msg3);

    RCLCPP_INFO(get_logger(),
      "Published: Sensor1 %.2f m, Sensor2 %.2f m, Sensor3 %.2f m",
      msg1.range, msg2.range, msg3.range);
  }

  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr pub1_;
  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr pub2_;
  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr pub3_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::random_device rd_;
  std::mt19937 rng_;
  std::uniform_real_distribution<float> dist_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MultiUltrasonicPublisher>());
  rclcpp::shutdown();
  return 0;
}
