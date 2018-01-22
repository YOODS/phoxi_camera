//
// Created by controller on 1/11/18.
//

#include "phoxi_camera/RosInterface.h"
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/fill_image.h>
#include <phoxi_camera/PhoXiException.h>

RosInterface::RosInterface() : nh("~"), dynamicReconfigureServer(dynamicReconfigureMutex), child_frame("base_link")  {

    //create service servers
    getDeviceListService = nh.advertiseService("get_device_list", &RosInterface::getDeviceList, this);
    connectCameraService =nh.advertiseService("connect_camera", &RosInterface::connectCamera, this);
    isConnectedService = nh.advertiseService("is_connected", (bool (RosInterface::*)(phoxi_camera::IsConnected::Request&, phoxi_camera::IsConnected::Response&))&RosInterface::isConnected, this);
    isAcquiringService = nh.advertiseService("is_acquiring", (bool (RosInterface::*)(phoxi_camera::IsAcquiring::Request&, phoxi_camera::IsAcquiring::Response&))&RosInterface::isAcquiring, this);
    isConnectedServiceV2 = nh.advertiseService("V2/is_connected", (bool (RosInterface::*)(phoxi_camera::Bool::Request&, phoxi_camera::Bool::Response&))&RosInterface::isConnected, this);
    isAcquiringServiceV2 = nh.advertiseService("V2/is_acquiring", (bool (RosInterface::*)(phoxi_camera::Bool::Request&, phoxi_camera::Bool::Response&))&RosInterface::isAcquiring, this);
    startAcquisitionService = nh.advertiseService("start_acquisition", (bool (RosInterface::*)(std_srvs::Empty::Request&, std_srvs::Empty::Response&))&RosInterface::startAcquisition, this);
    stopAcquisitionService = nh.advertiseService("stop_acquisition", (bool (RosInterface::*)(std_srvs::Empty::Request&, std_srvs::Empty::Response&))&RosInterface::stopAcquisition, this);
    startAcquisitionServiceV2 = nh.advertiseService("V2/start_acquisition", (bool (RosInterface::*)(phoxi_camera::Empty::Request&, phoxi_camera::Empty::Response&))&RosInterface::startAcquisition, this);
    stopAcquisitionServiceV2 = nh.advertiseService("V2/stop_acquisition", (bool (RosInterface::*)(phoxi_camera::Empty::Request&, phoxi_camera::Empty::Response&))&RosInterface::startAcquisition, this);
    triggerImageService =nh.advertiseService("trigger_image", &RosInterface::triggerImage, this);
    getFrameService = nh.advertiseService("get_frame", &RosInterface::getFrame, this);
    saveFrameService = nh.advertiseService("save_frame", &RosInterface::saveFrame, this);
    disconnectCameraService = nh.advertiseService("disconnect_camera", &RosInterface::disconnectCamera, this);
    getHardwareIdentificationService = nh.advertiseService("get_hardware_indentification", &RosInterface::getHardwareIdentification, this);
    getSupportedCapturingModesService = nh.advertiseService("get_supported_capturing_modes", &RosInterface::getSupportedCapturingModes, this);
    setCoordianteSpaceService = nh.advertiseService("V2/set_transformation",&RosInterface::setCoordianteSpace, this);
    setTransformationService = nh.advertiseService("V2/set_coordination_space",&RosInterface::setTransformation, this);

    //create publishers
    cloudPub = nh.advertise <pcl::PointCloud<pcl::PointXYZ >>("pointcloud", 1);
    normalMapPub = nh.advertise < sensor_msgs::Image > ("normal_map", 1);
    confidenceMapPub = nh.advertise < sensor_msgs::Image > ("confidence_map", 1);
    texturePub = nh.advertise < sensor_msgs::Image > ("texture", 1);

    //set dynamic reconfigure callback
    dynamicReconfigureServer.setCallback(boost::bind(&RosInterface::dynamicReconfigureCallback,this, _1, _2));
}

bool RosInterface::getDeviceList(phoxi_camera::GetDeviceList::Request &req, phoxi_camera::GetDeviceList::Response &res){
    try {
        res.out = PhoXiInterface::cameraList();
        res.len = res.out.size();
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}
bool RosInterface::connectCamera(phoxi_camera::ConnectCamera::Request &req, phoxi_camera::ConnectCamera::Response &res){
    try {
        PhoXiInterface::connectCamera(req.name);
        dynamicReconfigureServer.getConfigDefault(dynamicReconfigureConfig);
        dynamicReconfigureServer.updateConfig(dynamicReconfigureConfig);
        this->dynamicReconfigureCallback(dynamicReconfigureConfig,std::numeric_limits<uint32_t>::max());
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}
bool RosInterface::isConnected(phoxi_camera::IsConnected::Request &req, phoxi_camera::IsConnected::Response &res){
    res.connected = PhoXiInterface::isConnected();
    return true;
}
bool RosInterface::isAcquiring(phoxi_camera::IsAcquiring::Request &req, phoxi_camera::IsAcquiring::Response &res){
    res.is_acquiring = PhoXiInterface::isAcquiring();
    return true;
}
bool RosInterface::isConnected(phoxi_camera::Bool::Request &req, phoxi_camera::Bool::Response &res){
    res.value = PhoXiInterface::isConnected();
    res.message = "ok"; //todo tot este premysliet
    res.success = true;
    return true;
}
bool RosInterface::isAcquiring(phoxi_camera::Bool::Request &req, phoxi_camera::Bool::Response &res){
    res.value = PhoXiInterface::isAcquiring();
    res.message = "ok"; //todo tot este premysliet
    res.success = true;
    return true;
}
bool RosInterface::startAcquisition(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res){
    try {
        PhoXiInterface::startAcquisition();
    }catch (PhoXiInterfaceException &e){
        ROS_ERROR("%s",e.what());
    }
    return true;
}
bool RosInterface::stopAcquisition(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res){
    try {
        PhoXiInterface::stopAcquisition();
    }catch (PhoXiInterfaceException &e){
        ROS_ERROR("%s",e.what());
    }
    return true;
}
bool RosInterface::startAcquisition(phoxi_camera::Empty::Request &req, phoxi_camera::Empty::Response &res){
    try {
        //todo
        PhoXiInterface::startAcquisition();
        res.message = "Ok";
        res.success = true;
    }catch (PhoXiInterfaceException &e){
        res.message = e.what();
        res.success = false;
    }
    return true;
}
bool RosInterface::stopAcquisition(phoxi_camera::Empty::Request &req, phoxi_camera::Empty::Response &res){
    try {
        PhoXiInterface::stopAcquisition();
        res.message = "Ok";
        res.success = true;
    }catch (PhoXiInterfaceException &e){
        res.message = e.what();
        res.success = false;
    }
    return true;
}
bool RosInterface::triggerImage(phoxi_camera::TriggerImage::Request &req, phoxi_camera::TriggerImage::Response &res){
    try {
        res.id = RosInterface::triggerImage();
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}
bool RosInterface::getFrame(phoxi_camera::GetFrame::Request &req, phoxi_camera::GetFrame::Response &res){
    try {
        pho::api::PFrame frame = getPFrame(req.in);
        publishFrame(frame);
        if(!frame){
            res.success = false;
            res.message = "Null frame!";
        }
        else{
            res.success = true;
            res.message = "Ok";
        }
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}
bool RosInterface::saveFrame(phoxi_camera::SaveFrame::Request &req, phoxi_camera::SaveFrame::Response &res){
    try {
        pho::api::PFrame frame = RosInterface::getPFrame(req.in);
        if(!frame){
            res.success = false;
            ROS_INFO("aaa");
            res.message = "Null frame!";
            return true;
        }
        frame->SaveAsPly(req.path);
        res.message = "Ok";
        res.success = true;
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        ROS_INFO("rrr %s",e.what());
        res.message = e.what();
    }
    return true;
}
bool RosInterface::disconnectCamera(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res){
    try {
        PhoXiInterface::disconnectCamera();
    }catch (PhoXiInterfaceException &e){
        //scanner is already disconnected on exception
    }
    return true;
}
bool RosInterface::getHardwareIdentification(phoxi_camera::GetHardwareIdentification::Request &req, phoxi_camera::GetHardwareIdentification::Response &res){
    try {
        PhoXiInterface::getHardwareIdentification();
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}
bool RosInterface::getSupportedCapturingModes(phoxi_camera::GetSupportedCapturingModes::Request &req, phoxi_camera::GetSupportedCapturingModes::Response &res){
    try {
        std::vector<pho::api::PhoXiCapturingMode> modes = PhoXiInterface::getSupportedCapturingModes();
        for(int i =0; i < modes.size(); i++){
            phoxi_camera::PhoXiSize size;
            size.Height = modes[i].Resolution.Height;
            size.Width = modes[i].Resolution.Width;
            res.supported_capturing_modes.push_back(size);
        }
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}

void RosInterface::publishFrame(pho::api::PFrame frame) {
    if (!frame) {
        ROS_WARN("NUll frame!");
        return;
    }
    if (frame->PointCloud.Empty()){
        ROS_WARN("Empty point cloud!");
    }
    if (frame->DepthMap.Empty()){
        ROS_WARN("Empty depth map!");
    }
    if (frame->Texture.Empty()){
        ROS_WARN("Empty texture!");
    }
    if (frame->ConfidenceMap.Empty()){
        ROS_WARN("Empty confidence map!");
    }
    if (frame->NormalMap.Empty()){
        ROS_WARN("Empty normal map!");
    }
    sensor_msgs::Image texture, confidence_map, normal_map;
    ros::Time timeNow = ros::Time::now();
    texture.header.stamp = timeNow;
    texture.header.frame_id = child_frame;
    confidence_map.header.stamp = timeNow;
    confidence_map.header.frame_id = child_frame;
    normal_map.header.stamp = timeNow;
    normal_map.header.frame_id = child_frame;
    texture.encoding = "32FC1";
    sensor_msgs::fillImage(texture, sensor_msgs::image_encodings::TYPE_32FC1,
                           frame->Texture.Size.Height, // height
                           frame->Texture.Size.Width, // width
                           frame->Texture.Size.Width * sizeof(float), // stepSize
                           frame->Texture.operator[](0));
    confidence_map.encoding = "32FC1";
    sensor_msgs::fillImage(confidence_map,
                           sensor_msgs::image_encodings::TYPE_32FC1,
                           frame->ConfidenceMap.Size.Height, // height
                           frame->ConfidenceMap.Size.Width, // width
                           frame->ConfidenceMap.Size.Width * sizeof(float), // stepSize
                           frame->ConfidenceMap.operator[](0));
    normal_map.encoding = "32FC3";
    sensor_msgs::fillImage(normal_map,
                           sensor_msgs::image_encodings::TYPE_32FC3,
                           frame->NormalMap.Size.Height, // height
                           frame->NormalMap.Size.Width, // width
                           frame->NormalMap.Size.Width * sizeof(float) * 3, // stepSize
                           frame->NormalMap.operator[](0));
    std::shared_ptr<pcl::PointCloud<pcl::PointNormal>> cloud = PhoXiInterface::getPointCloudFromFrame(frame);
    sensor_msgs::PointCloud2 output_cloud;
    pcl::toROSMsg(*cloud,output_cloud);
    output_cloud.header.frame_id = "base_link";
    output_cloud.header.stamp = ros::Time::now();
    cloudPub.publish(output_cloud);
    normalMapPub.publish(normal_map);
    confidenceMapPub.publish(confidence_map);
    texturePub.publish(texture);
}

bool RosInterface::setCoordianteSpace(phoxi_camera::SetCoordinatesSpace::Request &req, phoxi_camera::SetCoordinatesSpace::Response &res){
    try {
        PhoXiInterface::setCoordinateSpace(req.coordinates_space);
        //update dynamic reconfigure
        dynamicReconfigureConfig.coordination_space = req.coordinates_space;
        dynamicReconfigureServer.updateConfig(dynamicReconfigureConfig);
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}

bool RosInterface::setTransformation(phoxi_camera::TransformationMatrix::Request &req, phoxi_camera::TransformationMatrix::Response &res){
    if(req.matrix.size()!= 16){
        res.success = false;
        res.message = "Bad matrix dimensions!";
        return true;
    }
    try {
        PhoXiInterface::setTransformation(req.matrix,req.coordinates_space,req.set_space,req.save_settings);
        //update dynamic reconfigure
        dynamicReconfigureConfig.coordination_space = req.coordinates_space;
        dynamicReconfigureServer.updateConfig(dynamicReconfigureConfig);
        res.success = true;
        res.message = "Ok";
    }catch (PhoXiInterfaceException &e){
        res.success = false;
        res.message = e.what();
    }
    return true;
}

void RosInterface::dynamicReconfigureCallback(phoxi_camera::phoxi_cameraConfig &config, uint32_t level) {
    ROS_INFO("Reconfigure Request");
    if(!PhoXiInterface::isConnected()){
        ROS_WARN("Scanner not connected!");
        return;
    }
    if (level & (1 << 1)) {
        try {
            switch (config.resolution){
                case 0:
                    PhoXiInterface::setLowResolution();
                    break;
                case 1:
                    PhoXiInterface::setHighResolution();
                    break;
                default:
                    ROS_WARN("Resolution not supported!");
                    break;
            }
        }catch(PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 2)) {
        try{
            this->isOk();
            scanner->CapturingSettings->ScanMultiplier = config.scan_multiplier;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 3)) {
        try{
            this->isOk();
            scanner->CapturingSettings->ShutterMultiplier = config.shutter_multiplier;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 4)) {
        try{
           PhoXiInterface::setTriggerMode(config.trigger_mode,false);
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 5)) {
        try{
            this->isOk();
            scanner->Timeout = config.timeout;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 6)) {
        try{
            this->isOk();
            scanner->ProcessingSettings->Confidence = config.confidence;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 7)) {
        try{
            this->isOk();
            scanner->OutputSettings->SendPointCloud = config.send_point_cloud;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 8)) {
        try{
            this->isOk();
            scanner->OutputSettings->SendNormalMap = config.send_normal_map;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 9)) {
        try{
            this->isOk();
            scanner->OutputSettings->SendConfidenceMap = config.send_confidence_map;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }
    
    if (level & (1 << 10)) {
        try{
            this->isOk();
            scanner->OutputSettings->SendTexture = config.send_texture;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }

    if (level & (1 << 11)) {
        try{
            this->isOk();
            scanner->OutputSettings->SendTexture = config.send_deapth_map;
        }catch (PhoXiInterfaceException &e){
            ROS_WARN("%s",e.what());
        }
    }
}

pho::api::PFrame RosInterface::getPFrame(int id){
    pho::api::PFrame frame = PhoXiInterface::getPFrame(id);
    //update dynamic reconfigure
    dynamicReconfigureConfig.coordination_space = pho::api::PhoXiTriggerMode::Software;
    dynamicReconfigureServer.updateConfig(dynamicReconfigureConfig);
    return frame;
}

int RosInterface::triggerImage(){
    int id = PhoXiInterface::triggerImage();
    //update dynamic reconfigure
    dynamicReconfigureConfig.coordination_space = pho::api::PhoXiTriggerMode::Software;
    dynamicReconfigureServer.updateConfig(dynamicReconfigureConfig);
    return id;
}


