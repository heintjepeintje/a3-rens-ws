#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class CameraPreviewNode : public rclcpp::Node
{
public:
    CameraPreviewNode() : Node("camera_preview_node")
    {
        subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/rgb/image_raw", 10,
            std::bind(&CameraPreviewNode::imageCallback, this, std::placeholders::_1));
        
        RCLCPP_INFO(this->get_logger(), "Camera Preview Node started");
        
    }

private:
    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        try
        {
            cv::Mat frame = cv_bridge::toCvCopy(msg, "bgr8")->image;

            cv::imshow("OAK-D Camera Preview", frame);
            cv::waitKey(1);
        }
        catch (cv_bridge::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        }
    }

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraPreviewNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
