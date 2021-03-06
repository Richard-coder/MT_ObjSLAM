//
// Created by khuang on 6/11/18.
//

#ifndef DATASETREADER_LPD_DATASET_H
#define DATASETREADER_LPD_DATASET_H

#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <memory>
#include "ObjSLAMDataTypes.h"
#include "ObjCameraPose.h"

#include "../../External/InfiniTAM/InfiniTAM/ORUtils/FileUtils.h"
#include "External/InfiniTAM/InfiniTAM/ITMLib/Objects/Camera/ITMRGBDCalib.h"
#include "DatasetReader.h"
#include <eigen3/Eigen/Geometry>
#include <eigen3/Eigen/Dense>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>


using namespace std;

/**
 * @brief Dataset reader for the RealisticRenderingDataset by LPD
 */
class LPD_Dataset_Reader : public DatasetReader {
 private:
//  int width, height;
//  Vector2i imgSize;
//  ITMLib::ITMRGBDCalib *calib;
  ifstream pose_in;
  ObjSLAM::ObjCameraPose *pose_wc;
//  string path;
 public:
//  ObjSLAM::ObjUChar4Image *rgb_img;
//  ObjSLAM::ObjFloatImage *depth_img;
//  std::shared_ptr<ObjSLAM::ObjUIntImage> label_img;
//  LabelImgVector label_img_vector;

//  ObjSLAM::ObjUChar4Image *rgb_img_prev;
//  ObjSLAM::ObjFloatImage *depth_img_prev;
//  ObjSLAM::ObjUIntImage *label_img_prev;


//  ObjSLAM::ObjCameraPose* pose_cw_prev;

  int img_number = 1;

 public:
//  LPD_Dataset_Reader() {};

//  LPD_Dataset_Reader(string _path, int w, int h) : path(_path), width(w), height(h) {
//    imgSize = Vector2i(w, h);
//    setCalib_LPD();
//  };

  LPD_Dataset_Reader(string _path, Vector2i _imgSize) : DatasetReader(_path,
                                                                             _imgSize)/* path(_path), imgSize(_imgSize)*/ {
//    width = imgSize.x;
//    height = imgSize.y;
    readCalib();
  };

  //destructor
  ~LPD_Dataset_Reader() {
    deleteVariables();
  }

  int ReadNext() {
    cout << "img_number = " << img_number << endl;
//    if (img_number > 1) {
//      deleteVariables();
//    }
    if (label_img_vector.size() != 0) {
      label_img_vector.clear();
    }

    //TODO make the path using os path join instead of slash
    string depth_path = path + "/depth/cam0/" + to_string(img_number) + ".exr";
    string rgb_path = path + "/rgb/cam0/" + to_string(img_number) + ".ppm";
    string normal_path = path + "/normal/cam0/" + to_string(img_number) + ".png";
    string label_path = path + "/pixel_label/cam0/" + to_string(img_number) + ".txt";
    string pose_path = path + "/groundTruthPoseVel_imu.txt";

    std::vector<string> fileNames = GetFileNames(path + "/pixel_label/cam0/");
    std::vector<string> filteredNames;

    for (int i = 0; i < fileNames.size(); i++) {
      string prefix = to_string(img_number) + ".";
      if (boost::starts_with(fileNames.at(i), prefix) && fileNames.at(i) != prefix) {
        filteredNames.push_back(fileNames.at(i));
      }
    }

    std::sort(filteredNames.begin(), filteredNames.end());

    for (int i = 0; i < filteredNames.size(); i++) {
      label_img_vector.push_back(ReadLabel_OneFile(path + "/pixel_label/cam0/" + filteredNames.at(i)));
    }

    if (!pose_in.is_open()) {
      pose_in.open(pose_path);
//      cout << "ifstream open: " << pose_path << endl;
    }

    //depth
    shared_ptr<ObjSLAM::ObjFloatImage> ray_depth_img = ReadOneDepth(depth_path);

    depth_img = convertRayDepthToZDepth(ray_depth_img.get());

//    delete ray_depth_img;
//    string name = to_string(img_number)+".ppm";
//    SaveImageToFile(depth_img,name.c_str());
    rgb_img = ReadOneRGB(rgb_path);

    double time = img_number * 0.1;

    ObjSLAM::LPD_RAW_Pose *raw_pose = ReadLPDRawPose(pose_in, time);
    //T_bw
    ObjSLAM::ObjCameraPose *T_bw = convertRawPose_to_Pose(raw_pose);
//    cout<<T_bw->GetSE3Pose().GetM();
    //T_cb
    ObjSLAM::ObjCameraPose *T_cb = new ObjSLAM::ObjCameraPose(0.5, -0.5, 0.5, -0.5, 0, 0, 0);
//    cout<<T_cb->GetSE3Pose().GetM();
    //Cam to World
//    ORUtils::SE3Pose T_cw_SE3 = T_cb->getSE3Pose().GetM() * T_bw->GetSE3Pose().GetM();
    ORUtils::SE3Pose T_cw_SE3 = T_bw->GetSE3Pose().GetM() * T_cb->GetSE3Pose().GetM();

    ORUtils::SE3Pose T_wc_SE3(T_cw_SE3.GetInvM());

    //world to cam
//    pose_wc = new ObjSLAM::ObjCameraPose(T_wc_SE3);

    delete raw_pose;
    delete T_bw;
    delete T_cb;
//    delete T_cw_SE3;


    return img_number++;
  }

 shared_ptr<ObjSLAM::ObjFloatImage> convertRayDepthToZDepth(ObjSLAM::ObjFloatImage *in) {
//    ORUtils::Vector2<int> imgSize(width, height);
//    auto *res = new ObjSLAM::ObjFloatImage(imgSize, MEMORYDEVICE_CPU);

    auto res = std:: make_shared<ObjSLAM::ObjFloatImage>(imgSize, MEMORYDEVICE_CPU);

    Eigen::Matrix3f K;/* = Eigen::Matrix3d::Zero();*/
    K(0, 0) = this->calib->intrinsics_d.projectionParamsSimple.fx;

    K(1, 1) = this->calib->intrinsics_d.projectionParamsSimple.fy;
    K(0, 2) = this->calib->intrinsics_d.projectionParamsSimple.px;
    K(1, 2) = this->calib->intrinsics_d.projectionParamsSimple.py;
    K(2, 2) = 1;

    Eigen::Matrix3f K_inv = K.inverse();

    //TODO: Make this a camera Projection function
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        Eigen::Vector3f Pix(j, i, 1.0);
        Eigen::Vector3f WorldVec = K_inv * Pix;
        WorldVec.normalize();
        WorldVec = WorldVec * in->GetElement(i * width + j, MEMORYDEVICE_CPU);
        res->GetData(MEMORYDEVICE_CPU)[i * width + j] = WorldVec(2);
      }
    }

    return res;
  }


  //Read Normal
/*  ObjSLAM::ObjFloat4Image* ReadNormal(std::string Path) {

    ifstream in;

    in.open(Path);
    //read rgb from png file


//    ORUtils::Vector2<int> imgSize(width, height);
    auto *res = new ObjSLAM::ObjFloat4Image(imgSize, MEMORYDEVICE_CPU);


    res->ChangeDims(imgSize);

    ReadImageFromFile(res, Path.c_str());


    return res;
  }*/



  ObjSLAM::LPD_RAW_Pose *ReadLPDRawPose(ifstream &in, double t) {

    /*    ifstream in;
    in.open(Path);*/

    ObjSLAM::LPD_RAW_Pose *res = new ObjSLAM::LPD_RAW_Pose();
    double currentT = 0.0;
    string currentLine;
    double TH = 0.0005;

    while (getline(in, currentLine)) {
      istringstream iss(currentLine);

      iss >> currentT;
//      cout<<"Current T"<<currentT<<endl;

      if (abs(currentT - t) < TH) {
        iss >> res->qw;
        iss >> res->qx;
        iss >> res->qy;
        iss >> res->qz;
//        cout << "read q " << res->qw << res->qx << res->qy << res->qz << endl;
        iss >> res->x;

        iss >> res->y;

        iss >> res->z;

        iss >> res->vx;
        iss >> res->vy;
        iss >> res->vz;
        iss >> res->p;
        iss >> res->q;
        iss >> res->r;
        iss >> res->ax;
        iss >> res->ay;
        iss >> res->az;

        break;
      }
    }
    return res;
  }

  ObjSLAM::ObjCameraPose *convertRawPose_to_Pose(ObjSLAM::LPD_RAW_Pose *_rawPose) {
    auto *res = new ObjSLAM::ObjCameraPose(
        _rawPose->qw,
        _rawPose->qx,
        _rawPose->qy,
        _rawPose->qz,
        _rawPose->x,
        _rawPose->y,
        _rawPose->z
    );

    return res;

  }

  ObjSLAM::ObjShortImage *calculateAffineDFromDepth(ObjSLAM::ObjFloatImage *depth) {

//    ORUtils::Vector2<int> imgSize(width, height);
    auto *disparity = new ObjSLAM::ObjShortImage(imgSize, MEMORYDEVICE_CPU);

    float fx = this->calib->intrinsics_d.projectionParamsSimple.fx;
//    cout<<"fx"<<fx;
    float bxf = 8.0f * this->calib->disparityCalib.GetParams().y * fx;

    for (int y = 0; y < imgSize.y; y++) {
      for (int x = 0; x < imgSize.x; x++) {
        int locId = x + y * imgSize.x;
//
//        float depth_pixel = depth->GetData(MEMORYDEVICE_CPU)[locId];
//        float disparity_tmp = bxf / depth_pixel;
//        short disparity_pixel = this->calib->disparityCalib.GetParams().x - disparity_tmp;

        short short_D_pixel = depth->GetData(MEMORYDEVICE_CPU)[locId] / this->calib->disparityCalib.GetParams().x;

        disparity->GetData(MEMORYDEVICE_CPU)[locId] = short_D_pixel;

      }
    }
    return disparity;
  }

  bool readCalib() {
    setCalib_LPD();
    return true;
  }

  void setCalib_LPD() {
//    calib = new ITMLib::ITMRGBDCalib();
    calib = std::make_shared<ITMLib::ITMRGBDCalib>();
    calib->intrinsics_rgb.SetFrom(640, 480, 320, 320, 320, 240);
    calib->intrinsics_d.SetFrom(640, 480, 320, 320, 320, 240);


    //Calib Extrinsics is between RGB and D, for LPD Dataset it is identity
    ObjSLAM::ObjMatrix4f mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    calib->trafo_rgb_to_depth.SetFrom(mat);

    //disparity calib a b physical meanings?
    calib->disparityCalib.SetFrom(/*1135.09*//*0.0002*/1.0,
                                                       0.0,
                                                       ITMLib::ITMDisparityCalib::TRAFO_AFFINE); //TODO get the values


  }

/*//  void readExtrnsics(string Path){
//
//    ObjSLAM::ObjMatrix4f calib_Ext;
//    Eigen::Quaterniond R(0.5, -0.5,0.5,-0.5);
//    Eigen::Matrix3d eigen_mat = R.normalized().toRotationMatrix();
//
//    double m00 = eigen_mat(0,0);double m01 = eigen_mat(0,1);double m02 = eigen_mat(0,2);
//    double m10 = eigen_mat(1,0);double m11 = eigen_mat(1,1);double m12 = eigen_mat(1,2);
//    double m20 = eigen_mat(2,0);double m21 = eigen_mat(2,1);double m22 = eigen_mat(2,2);
//    ObjSLAM::ObjMatrix4f mat(1,0,0, 0,0,1,0, 0,0,0,1,0,0,0,0,1);
//    calib->trafo_rgb_to_depth.SetFrom(mat);
//  }*/



//  ITMLib::ITMRGBDCalib *GetCalib() {
//    return calib;
//  }

  void deleteVariables() {

//    delete(this->label_img);
//    delete(this->rgb_img);
//    delete(this->depth_img);
//    delete(this->pose_cw);
//    delete this->label_img;
//    delete this->rgb_img;
//    delete this->depth_img;
    delete this->pose_wc;
    label_img_vector.clear();
  }

  void setWidth(int w) {
    width = w;
  }
  void setHeight(int h) {
    height = h;
  }
  int getWidth() {
    return width;
  }
  int getHeight() {
    return height;
  }
  Vector2i GetSize() {
    return imgSize;
  }

  ObjSLAM::ObjCameraPose *getPose() { return pose_wc; }

};

#endif //DATASETREADER__LPD_DATASET_H
