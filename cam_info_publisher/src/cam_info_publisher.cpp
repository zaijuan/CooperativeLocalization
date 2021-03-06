#include "cam_info_publisher.hpp"

#include <iostream>
#include <sstream>

#include <ros/console.h>

#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <image_transport/image_transport.h>

using camera_info_manager::CameraInfoManager;

using namespace message_filters;
using namespace sensor_msgs;

CamInfoPubNode::CamInfoPubNode(ros::NodeHandle &nh_ ) {

  leftNs = ros::NodeHandle(nh_, "left");
  rightNs = ros::NodeHandle(nh_, "right");

  image_sub_left = new message_filters::Subscriber<Image>(nh_, "/B/left/image_raw", 1);
  image_sub_right = new message_filters::Subscriber<Image>(nh_, "/B/right/image_raw", 1);

  camPubLeft = image_transport::ImageTransport(leftNs).advertiseCamera("image_raw", 2);
  camPubRight = image_transport::ImageTransport(rightNs).advertiseCamera("image_raw", 2);

  cinfoLeft = new CameraInfoManager(leftNs, "boreas_left", camera_calibration_path);
  cinfoRight = new CameraInfoManager(rightNs, "boreas_right", camera_calibration_path);

  cinfoLeft->loadCameraInfo(camera_calibration_path);
  cinfoRight->loadCameraInfo(camera_calibration_path);

  sync = new TimeSynchronizer<Image, Image>(*image_sub_left, *image_sub_right, 10);
 
  sync->registerCallback(boost::bind(&CamInfoPubNode::callback, this, _1, _2));

  ROS_INFO("Left calibrated: %s", cinfoLeft->isCalibrated() ? "true" : "false");
  ROS_INFO("Right calibrated: %s", cinfoRight->isCalibrated() ? "true" : "false");
}

CamInfoPubNode::~CamInfoPubNode() {
  delete cinfoLeft;
  delete cinfoRight;
  delete image_sub_left;
  delete image_sub_right;
  delete sync;
}

void CamInfoPubNode::callback(const ImageConstPtr& image_left_msg, const ImageConstPtr& image_right_msg) {
    ros::Time triggerTime = ros::Time::now();

    leftCamInfo = cinfoLeft->getCameraInfo();
    leftCamInfo.header.stamp = triggerTime;
    leftCamInfo.header.frame_id = image_left_msg->header.frame_id;
    leftCamInfo.width = image_left_msg->width;
 
    rightCamInfo = cinfoRight->getCameraInfo();
    rightCamInfo.header.stamp = triggerTime;
    rightCamInfo.header.frame_id = image_right_msg->header.frame_id;
    rightCamInfo.width = image_right_msg->width;

    camPubLeft.publish(*image_left_msg, leftCamInfo);
    camPubRight.publish(*image_right_msg, rightCamInfo);
}

//ImageConstPtr

int main(int argc, char** argv)
{
  ros::init(argc, argv, "cam_info_publisher_node");
  ros::NodeHandle nh_;
  
  CamInfoPubNode cam_info_pub_node_(nh_);

  ros::spin();
  return 0;
}

