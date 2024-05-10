#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_bot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    uint32_t width = img.step;
    uint16_t sector_width = width/3;
    uint16_t sectorL = sector_width;
    uint16_t sectorFL = sector_width + 1;
    uint16_t sectorFR = 2 * sector_width;
    uint16_t sectorR = (2 * sector_width)+1;

    const std::vector<uint8_t>& imgData = img.data;
    uint32_t imgDataSize = imgData.size();

    bool white = false;
    //for loop to look at the data 
    for(uint32_t i = 0; i < imgDataSize; i+=3){

        //if the value is 255 send drive command
        if(imgData[i] == 255 && imgData[i+1] == 255 && imgData[i+2] == 255)
        {
            //white is in the picture
            white = true;
            //determine section 
            uint16_t locale = i % width;
            //Left Turn
            if(locale < sectorR) drive_robot(0.0, 0.075);
            //Forward
            if(locale >= sectorFL && locale <= sectorFR) drive_robot(0.15, 0.0);
            //Right Turn
            if(locale >= sectorR) drive_robot(0.0, -0.075);
            //command motors sent. Break the loop. 
            break;
        }
    }
    //No white in the image
    if(white == false) drive_robot(0.0,0.0);


    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ROS_INFO("Ready to find ball and send velocity commands.");
    ros::spin();

    return 0;
}
