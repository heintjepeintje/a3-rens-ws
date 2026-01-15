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

// Struct to hold triangle for easy conclusion
struct ShapeInformation
{
    bool found = false;
    Rect bounding_box;
};

//function to get the pink triangles bounding box 
ShapeInformation get_triangle_box(const Mat& dial_img, Mat& in_img);
ShapeInformation get_center_box(int size, Mat& in_img); // in which format should i do size...
bool overlapping_triangle_center(ShapeInformation& triangle, ShapeInformation& center);
void adjust_contrast_brightness(Mat& image, double alpha, int beta);

class CameraDetectionNode : public rclcpp::Node
{
    public:
    CameraDetectionNode() : Node("camera_detection_node")
    {
        // create subscription to get oak-d camara images
        subscriber_camera = this->create_subscription<sensor_msgs::msg::Image>("/camera/rgb/image_raw", 10,
            [this](sensor_msgs::msg::Image::SharedPtr msg){ this->imageCallback(msg); }); //calls method everytime a new image arrives

        // create publisher to publish triangle location
        publisher_triangle_location = this->create_publisher<std_msgs::msg::String>("/detection/triangle_position", 10);
    }

    private:
        ShapeInformation center_box;

        void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
        {
            try
            {
                Mat img = cv_bridge::toCvCopy(msg, "bgr8")->image; // turns ros2 image into cv image
                
                Mat img_gray, img_blur, img_canny, img_dilate, img_erode; //decleration for filters

                if (!center_box.found)
                {
                    center_box = get_center_box(6, img);
                }

                // Adjust contrast and brightness
                double alpha = 3;
                int beta = -50;
                Mat img_contrast_bright = img.clone();
                adjust_contrast_brightness(img_contrast_bright, alpha, beta);

                // Convert the image to grayscale and apply Gaussian blur
                cvtColor(img_contrast_bright, img_gray, COLOR_BGR2GRAY);
                GaussianBlur(img_gray, img_blur, Size(3, 3), 3, 0);
                Canny(img_blur, img_canny, 25, 75);

                // Apply dilation to the edges
                Mat se1 = getStructuringElement(MORPH_RECT, Size(3, 3));
                dilate(img_canny, img_dilate, se1);

                // Detect the triangle
                ShapeInformation triangle_box = get_triangle_box(img_dilate, img);

                // Draw bounding box on the image
                rectangle(img, center_box.bounding_box, Scalar(0, 125, 0), 2);
                putText(img, "Center Box", Point(center_box.bounding_box.x, center_box.bounding_box.y - 5),
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);

                if(triangle_box.found)
                {
                    // Declare position messages
                    string positionX_message = "";
                    string positionY_message = "";
                    string position_message = "";

                    // Compare bounding boxes
                    if (overlapping_triangle_center(center_box, triangle_box))
                    {
                        position_message = "Center";
                    }
                    else
                    {
                        if (triangle_box.bounding_box.x + triangle_box.bounding_box.width / 2 < img.cols / 2)
                        {
                            positionX_message = "Left";
                        }
                        else
                        {
                            positionX_message = "Right";
                        }

                        if (triangle_box.bounding_box.y + triangle_box.bounding_box.height / 2 < img.rows / 2)
                        {
                            positionY_message = "Top";
                        }
                        else
                        {
                            positionY_message = "Bottom";
                        }

                        position_message = positionX_message + " " + positionY_message; // Combine the position information
                    }

                    // Publish the position information
                    auto message = std_msgs::msg::String();
                    message.data = position_message;
                    publisher_triangle_location->publish(message);

                    // Log the position information to the console as well
                    RCLCPP_INFO(this->get_logger(), "%s", message.data.c_str());
                }

                // Show the image with contrast and brightness applied
                imshow("Adjusted Image", img);
                waitKey(1);
            }
            catch (cv_bridge::Exception &e)
            {
                RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
            }
        }

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_camera;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_triangle_location;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraDetectionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

// Finds the red triangle in the image based on RGB values
ShapeInformation get_triangle_box(const Mat& dial_img, Mat& in_img)
{
    // Declare shape info
    ShapeInformation info;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    // Find contours
    findContours(dial_img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Maximum allowed size for the triangle (half of image width)
    Size img_size = in_img.size();
    int max_size = img_size.width / 2;

    // Loop through all contours
    for (size_t i = 0; i < contours.size(); ++i)
    {
        double c_area = contourArea(contours[i]);

        // Skip small contours
        if (c_area < 1000)
            continue;

        double peri = arcLength(contours[i], true);
        vector<Point> approx;
        approxPolyDP(contours[i], approx, 0.02 * peri, true);

        // Only process triangles
        if (approx.size() != 3)
            continue;

        // Get bounding box for the triangle
        info.bounding_box = boundingRect(approx);

        // Skip triangles that are too big
        if (info.bounding_box.width > max_size || info.bounding_box.height > max_size)
            continue;

        // Check if the triangle contains the specific RGB red color
        Mat roi = in_img(info.bounding_box);  
        bool contains_red = false;

        // Iterate over all pixels in the bounding box
        for (int y = 0; y < roi.rows; ++y) {
            for (int x = 0; x < roi.cols; ++x) {
                Vec3b pixel = roi.at<Vec3b>(y, x);

                // Check if the pixel has a strong red channel value
                if (pixel[2] > 150 && pixel[0] < 100 && pixel[1] < 100) {
                    contains_red = true;
                    break;
                }
            }
            if (contains_red) break; 
        }

        // If no red pixel is found, skip this contour
        if (!contains_red)
            continue;

        // Draw the triangle and bounding box on the image
        drawContours(in_img, vector<vector<Point>>{approx}, -1, Scalar(255, 0, 255), 2);
        rectangle(in_img, info.bounding_box, Scalar(0, 255, 0), 3);
        putText(in_img, "Triangle", Point(info.bounding_box.x, info.bounding_box.y - 5),
                FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);

        info.found = true;
        break; // stop after finding the first valid triangle
    }

    return info;
}

// Returns a square bounding box at the center of the image
ShapeInformation get_center_box(int size, Mat& in_img)
{
    // Declare the shape info to return
    ShapeInformation info;

    // Calculate square size based on input 'size' factor
    Size img_size = in_img.size();
    int square_size = img_size.width / size;

    // Define the centered square
    Rect center_box;
    center_box.x = img_size.width / 2 - square_size / 2;
    center_box.y = img_size.height / 2 - square_size / 2;
    center_box.width = square_size;
    center_box.height = square_size;

    // Assign to ShapeInformation
    info.bounding_box = center_box;
    info.found = true;  // mark as found since we always create it

    return info;
}

// Returns true if the triangle bounding box overlaps with the center box
bool overlapping_triangle_center(ShapeInformation& triangle, ShapeInformation& center)
{
    if (!triangle.found || !center.found)
        return false;

    // Calculate intersection between the two rectangles
    Rect intersection = triangle.bounding_box & center.bounding_box;

    // If intersection area > 0, they overlap
    return (intersection.area() > 0);
}

// Function to adjust contrast and brightness
void adjust_contrast_brightness(Mat& image, double alpha, int beta)
{
    for( int y = 0; y < image.rows; y++ ) 
    {
        for( int x = 0; x < image.cols; x++ ) 
        {
            for( int c = 0; c < image.channels(); c++ ) 
            {
                        image.at<Vec3b>(y,x)[c] = saturate_cast<uchar>(alpha * image.at<Vec3b>(y,x)[c] + beta);
            }
        }
    }
}