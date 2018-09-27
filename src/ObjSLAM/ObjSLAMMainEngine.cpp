//
// Created by khuang on 9/24/18.
//

#include "ObjSLAMMainEngine.h"


//static global variables
//bool saveSTL = false;
//int STL_Frequency = 1;
//int reader_SkipFrames = 0;
//int numthreads = 4;
//int totFrames;


int ObjSLAMMainEngine::readNext() {
   imgNum = reader->readNext();
   if(imgNum==-1) return -1; //no more image
   sceneIsBackground=true;

   wholeView->depth->SetFrom(reader->depth_img,ORUtils::Image<float>::CPU_TO_CPU);
   wholeView->rgb ->SetFrom(reader->rgb_img,ORUtils::Image<Vector4u>::CPU_TO_CPU);
   bool isKeyFrame=true;
//   if(isKeyFrame){
      label_img_vector=reader->label_img_vector;
      depth_img=reader->depth_img;
      rgb_img=reader->rgb_img;
//   }
    framesElapsedBeforeMapping++;
   return imgNum;
}

void ObjSLAMMainEngine::trackFrame() {
   t_state=trackingEngine->TrackFrame(wholeView.get());
}


void ObjSLAMMainEngine::mapFrame() {
   mappingEngine->ProcessFrame();

   mapperFree=true;
}

void ObjSLAMMainEngine::outputPics(){
    mappingEngine->outputAllObjImages();
}

void ObjSLAMMainEngine::updateMappingEngine(){

  mapperFree=false;
  mappingEngine->UpdateImgNumber(imgNum);
  framesElapsedBeforeMapping = 0;
  mappingEngine->UpdateTrackingState(t_state);
  mappingEngine->CreateView(depth_img, rgb_img, label_img_vector);
}

ObjSLAM::ObjUChar4Image* ObjSLAMMainEngine::getImage(int n){
    if(n<this->mappingEngine->number_activeObjects){
        return mappingEngine->getImage(n);
    }else{
        return mappingEngine->getImage(0);
    }
}

ObjSLAM::ObjUChar4Image* ObjSLAMMainEngine::getInputImage() {
    return this->wholeView->rgb;
}

ObjSLAM::ObjUChar4Image* ObjSLAMMainEngine::getAboveImage() {
    return this->mappingEngine->getImageFromAbove();
}