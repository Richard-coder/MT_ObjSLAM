//
// Created by khuang on 8/1/18.
//

#include "ObjectView.h"

#include "ObjectInstance.h"
#include "ObjSLAMCamera.h"

namespace ObjSLAM {

template<typename TVoxel, typename TIndex>
ObjCameraPose ObjectView<TVoxel, TIndex>::GetCameraPose() {
  return *camera_Pose;
}

template<typename TVoxel, typename TIndex>
void ObjectView<TVoxel, TIndex>::SetCameraPose(ObjCameraPose _pose) {
  camera_Pose = new ObjCameraPose(_pose.GetSE3Pose());
}

template<typename TVoxel, typename TIndex>
void ObjectView<TVoxel, TIndex>::SetCameraPose(const ORUtils::SE3Pose *_se3pose) {
  this->camera_Pose = new ObjCameraPose(*_se3pose);
}

//returns the new label if it is new. if the same class already exists, return the old label instance and discard the new one.
template<typename TVoxel, typename TIndex>
std::shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>> ObjectView<TVoxel, TIndex>::AddLabelToVector(
        std::vector<shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>>> &label_ptr_vector,
        std::shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>> new_label) {

  if (label_ptr_vector.size() == 0) {
    label_ptr_vector.push_back(new_label);

    return new_label;
  } else {
    bool isNew = true;
    for (size_t t = 0; t < label_ptr_vector.size(); ++t) {
      if (label_ptr_vector.at(t)->GetLabelIndex() == new_label->GetLabelIndex()) {
        isNew = false;
        return label_ptr_vector.at(t);
      }
    }
    if (isNew) {
      label_ptr_vector.push_back(new_label);
      return new_label;
    }
  }
}


template<typename TVoxel, typename TIndex>
void ObjectView<TVoxel, TIndex>::SetListOfObjects(
        std::vector<shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>>> &label_ptr_vector) {

  cout << "Setting Obj List...";

  int pixel_count_TH=1600;

  auto label_ptr_bg_new = std::make_shared<ObjectClassLabel_Group<TVoxel, TIndex>>
      (0);
  shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>>
      label_ptr_bg = AddLabelToVector(label_ptr_vector,
                                      label_ptr_bg_new);


  if(!is_RGB_D_aligned){

    std::shared_ptr<ITMLib::ITMRGBDCalib> new_calib_ptr = std::make_shared<ITMLib::ITMRGBDCalib>();
    *new_calib_ptr = calibration;
    auto *cam = new ObjSLAMCamera(new_calib_ptr, imgSize_d);
    d_to_rgb_correspondence = cam->ProjectDepthPixelToRGB(this->depth_Image);
    delete cam;
  }/*else{
    for(int y = 0; y < d_to_rgb_correspondence->noDims.y;++y)for(int x = 0; x < d_to_rgb_correspondence->noDims.x;++x){
      int locId =
      d_to_rgb_correspondence->GetData(MEMORYDEVICE_CPU)[i].y=y;
      d_to_rgb_correspondence->GetData(MEMORYDEVICE_CPU)[i].x=x;
    }
  }*/

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for (LabelImgVector::iterator it = label_img_vector.begin(); it != label_img_vector.end(); ++it) {

    //skip labels with pixels less than count_TH=40;
    int count_non0_pix=0;
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
    //it over pixels

    for (int i = 0; i < (*it)->dataSize; ++i) {
      if((*it)->GetElement(i, MEMORYDEVICE_CPU)!=0){

//          #pragma omp critical{
                count_non0_pix++;
//              }

      }
    }
    if(count_non0_pix<pixel_count_TH) continue;


    int labelIndex = 0;

    auto single_obj_ITMView = std::make_shared<ITMLib::ITMView>(calibration, imgSize_rgb, imgSize_d, false);

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
    //it over pixels
    for (int i = 0; i < (*it)->dataSize; ++i) {



      int idx_rgb = -1;
      if(is_RGB_D_aligned){
        idx_rgb=i;
      } else {
        Vector2i rgb_pixel_loc  = d_to_rgb_correspondence->GetElement(i, MEMORYDEVICE_CPU);
        idx_rgb = (rgb_pixel_loc.x != -1 && rgb_pixel_loc.y != -1) ? (rgb_pixel_loc.y * imgSize_d.x + rgb_pixel_loc.x) : -1;
      }

//TODO optimize it!!!
      if (idx_rgb != -1) {
        //if the label is not empty
        if ((*it)->GetElement(idx_rgb, MEMORYDEVICE_CPU) != 0) {
          //if label index == 0 it means it is the first labeled pixel, only one possible number beside the 0s in one label img
          if (labelIndex == 0) { labelIndex = (*it)->GetElement(idx_rgb, MEMORYDEVICE_CPU); }

          //Set value of the each pixel in the segmented itm view
          //TODO depth img is shifted, needed to shift the itm view the tracker is using

          single_obj_ITMView->depth->GetData(MEMORYDEVICE_CPU)[i] = this->depth_Image->GetData(
              MEMORYDEVICE_CPU)[i];
          single_obj_ITMView->rgb->GetData(MEMORYDEVICE_CPU)[i] = this->rgb_Image->GetData(
              MEMORYDEVICE_CPU)[idx_rgb];
        }
      }
    }

    //set all object instance map
    if (labelIndex != 0) {
      //label
      auto label_ptr_new = std::make_shared<ObjectClassLabel_Group<TVoxel, TIndex>>
          (labelIndex);
      //returns the new label if it is new. if the same class already exists, return the old label instance and discard the new one.
      auto label_ptr = AddLabelToVector(label_ptr_vector, label_ptr_new);


      //create a object instance
      auto new_obj_instance = std::make_shared<ObjectInstance<TVoxel, TIndex>>
          (label_ptr);

      new_obj_instance->SetAnchorView(this->shared_from_this());
      new_obj_instance->SetAnchorView_ITM(single_obj_ITMView);

      Object_View_Tup<TVoxel, TIndex> object_view_tuple(new_obj_instance, single_obj_ITMView);

      this->obj_view_tup_vec.push_back(object_view_tuple);
    }
  }

  //background
  bg_itmview = make_shared<ITMLib::ITMView>(calibration, imgSize_rgb, imgSize_d, false);

  auto new_obj_instance = std::make_shared<ObjectInstance<TVoxel, TIndex>>
      (label_ptr_bg);


#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < this->depth_Image->dataSize; i++) {
    int idx_rgb = -1;
    if(is_RGB_D_aligned){
      idx_rgb=i;
    } else {
      Vector2i rgb_pixel_loc  = d_to_rgb_correspondence->GetElement(i, MEMORYDEVICE_CPU);
      idx_rgb = (rgb_pixel_loc.x != -1 && rgb_pixel_loc.y != -1) ? (rgb_pixel_loc.y * imgSize_d.x + rgb_pixel_loc.x) : -1;
      if (rgb_pixel_loc.x == -1 || rgb_pixel_loc.y == -1) {
        idx_rgb = -1;
      }
    }

    if (idx_rgb != -1) {
      bool is_background = true;
      for (LabelImgVector::iterator it = label_img_vector.begin(); it != label_img_vector.end(); it++) {
        if ((*it)->GetElement(idx_rgb, MEMORYDEVICE_CPU) != 0) {
          is_background = false;
          break;
        }
      }
      if (is_background) {
        bg_itmview->depth->GetData(MEMORYDEVICE_CPU)[i] = this->depth_Image->GetData(
            MEMORYDEVICE_CPU)[i];
        bg_itmview->rgb->GetData(MEMORYDEVICE_CPU)[i] = this->rgb_Image->GetData(MEMORYDEVICE_CPU)[idx_rgb];
      }
    }
  }


  new_obj_instance->SetAnchorView_ITM(bg_itmview);

  Object_View_Tup<TVoxel, TIndex> object_view_tuple(new_obj_instance, bg_itmview);
  this->obj_view_tup_vec.push_back(object_view_tuple);

//  SaveImageToFile(single_obj_ITMView_bg->depth,"test.ppm");
//  return label_ptr_vector;

  cout << "Finished! \n";
}


template<typename TVoxel, typename TIndex>
void ObjectView<TVoxel, TIndex>::setListOfViews() {

  for (int i = 0; i < segmentation_Mask->dataSize; i++) {
//    if(segmentation_Mask->GetElement(i, MEMORYDEVICE_CPU))
  }

}

template<typename TVoxel, typename TIndex>
std::vector<Object_View_Tup<TVoxel, TIndex>>& ObjectView<TVoxel, TIndex>::GetObjVec() {
  return this->obj_view_tup_vec;
}

template<typename TVoxel, typename TIndex>
std::shared_ptr<ITMLib::ITMView> ObjectView<TVoxel, TIndex>::GetBackgroundView() {
  if (bg_itmview.get() != NULL) {
    return this->bg_itmview;
  }

}

}