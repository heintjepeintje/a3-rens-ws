// https://www.embeddedhow.com/post/shape-detection-using-opencv-c


#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <std_msgs/msg/string.hpp> 

#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

struct TriangleInformation
{
    bool found = false;
    Rect bounding_box;
};

TriangleInformation get_triangle(const Mat& dial_img, Mat& in_img)
{
    TriangleInformation info;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(dial_img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Scalar lower_pink(140, 50, 50);
    Scalar upper_pink(170, 255, 255);

    Mat hsv_img;
    cvtColor(in_img, hsv_img, COLOR_BGR2HSV);

    Size img_size = in_img.size();
    int max_size = img_size.width / 2;

    for (size_t i = 0; i < contours.size(); ++i)
    {
        double c_area = contourArea(contours[i]);

        if (c_area > 1000)
        {
            double peri = arcLength(contours[i], true);
            vector<Point> approx;
            approxPolyDP(contours[i], approx, 0.02 * peri, true);

            if (approx.size() == 3)
            {
                info.bounding_box = boundingRect(approx);

                if (info.bounding_box.width > max_size || info.bounding_box.height > max_size)
                {
                    continue;
                }

                Mat mask = Mat::zeros(in_img.size(), CV_8UC1);
                drawContours(mask, vector<vector<Point>>{approx}, -1, Scalar(255), FILLED);

                Mat roi = hsv_img(info.bounding_box);
                Mat mask_roi;
                inRange(roi, lower_pink, upper_pink, mask_roi);

                if (countNonZero(mask_roi) > 0)
                {
                    drawContours(in_img, vector<vector<Point>>{approx}, -1, Scalar(255, 0, 255), 2);
                    rectangle(in_img, info.bounding_box, Scalar(0, 255, 0), 3);
                    putText(in_img, "Triangle", Point(info.bounding_box.x, info.bounding_box.y - 5),
                            FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);

                    info.found = true;
                    break;
                }
            }
        }
    }

    return info;
}



class CameraPreviewNode : public rclcpp::Node
{
public:
    CameraPreviewNode() : Node("camera_detection_node")
    {
        subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/rgb/image_raw", 10,
            std::bind(&CameraPreviewNode::imageCallback, this, std::placeholders::_1));

        position_publisher_ = this->create_publisher<std_msgs::msg::String>("triangle_position", 10);

        RCLCPP_INFO(this->get_logger(), "Detection Started: Looking for Triangles");
    }

private:
    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        try
        {
            Mat img = cv_bridge::toCvCopy(msg, "bgr8")->image;
            Mat img_gray, img_blur, img_canny, img_dilate, img_erode;

            cvtColor(img, img_gray, COLOR_BGR2GRAY);
            GaussianBlur(img_gray, img_blur, Size(3, 3), 3, 0);
            Canny(img_blur, img_canny, 25, 75);

            Mat se1 = getStructuringElement(MORPH_RECT, Size(3, 3));
            dilate(img_canny, img_dilate, se1);

            TriangleInformation triangle = get_triangle(img_dilate, img);

            if (triangle.found)
            {
                std::string positionX_message = "";
                std::string positionY_message = "";
                std::string position_message = "";

                Rect center_box(0, 0, img.cols, img.rows);
                center_box.x = img.cols / 2 - 100;
                center_box.y = img.rows / 2 - 100;
                center_box.width = 200;
                center_box.height = 200;

                if ((triangle.bounding_box & center_box).area() > 0)
                {
                    position_message = "Center";
                }
                else
                {
                    if (triangle.bounding_box.x + triangle.bounding_box.width / 2 < img.cols / 2)
                    {
                        positionX_message = "Left";
                    }
                    else
                    {
                        positionX_message = "Right";
                    }

                    if (triangle.bounding_box.y + triangle.bounding_box.height / 2 < img.rows / 2)
                    {
                        positionY_message = "Top";
                    }
                    else
                    {
                        positionY_message = "Bottom";
                    }

                    position_message = positionX_message + " " + positionY_message;
                }

                auto message = std_msgs::msg::String();
                message.data = position_message;
                position_publisher_->publish(message);

                RCLCPP_INFO(this->get_logger(), "%s", message.data.c_str());
            }

            imshow("OAK-D Camera Triangles Preview", img);
            waitKey(1);
        }
        catch (cv_bridge::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        }
    }

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr position_publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraPreviewNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
