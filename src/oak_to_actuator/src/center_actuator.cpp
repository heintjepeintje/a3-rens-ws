#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>
#include <string>

using namespace std;

class CenterActuator : public rclcpp::Node
{
public:
    CenterActuator() : Node("center_actuator")
    {
        // Subscriber naar het detectie-topic
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/detection/triangle_position", 10,
            std::bind(&CenterActuator::triangleCallback, this, std::placeholders::_1));

        // Publisher naar micro ROS Arduino
        publisher_ = this->create_publisher<std_msgs::msg::Int32>("/actuator_subscriber", 10);
    }

private:
    void triangleCallback(const std_msgs::msg::String::SharedPtr msg)
    {
        std_msgs::msg::Int32 status_msg;

        if (msg->data == "Center") // Check of triangle in center is
        {
            status_msg.data = 1; // extend
        }
        else
        {
            status_msg.data = 0; // retract
        }

        publisher_->publish(status_msg);

        RCLCPP_INFO(this->get_logger(), "Triangle Position: %s -> Status: %d",
                    msg->data.c_str(), status_msg.data);
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CenterActuator>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
