//
// Created by khuang on 8/16/18.
//

#ifndef MT_OBJSLAM_OBJSLAMTRACKINGENGINE_H
#define MT_OBJSLAM_OBJSLAMTRACKINGENGINE_H
#include <External/InfiniTAM/InfiniTAM/ITMLib/Objects/Tracking/ITMTrackingState.h>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Objects/RenderStates/ITMRenderState.h>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Utils/ITMSceneParams.h>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Core/ITMTrackingController.h>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Engines/LowLevel/Interface/ITMLowLevelEngine.h>
#include "ObjCameraPose.h"

namespace ObjSLAM {
using namespace ITMLib;
class ObjSLAMTrackingEngine {
 private:
  std::shared_ptr<ITMTrackingState> t_state;
//  ITMRenderState *r_state;
  std::unique_ptr<ITMSceneParams> params = make_unique<ITMSceneParams>(0.5, 4, 0.1, 0.1, 4.0, false);
//  ITMVisualisationEngine<TVoxel, TIndex> *visualisationEngine;
  std::shared_ptr<ITMTrackingController> t_controller;
  ITMTracker *tracker;
  ITMLowLevelEngine *lowEngine;

  Vector2i imgSize;
  const ITMLib::ITMLibSettings *settings;
  const ITMLib::ITMRGBDCalib *calib;
  int imgNumber = 1;

 public:
  ObjSLAMTrackingEngine(const ITMLib::ITMLibSettings *_settings,
                        const ITMLib::ITMRGBDCalib *_calib,
                        const Vector2i _imgSize);
  ~ObjSLAMTrackingEngine();
  shared_ptr<ITMLib::ITMTrackingState>  TrackFrame(ITMLib::ITMView * view);

  shared_ptr<ITMLib::ITMTrackingState>  getTrackingState();

  shared_ptr<ITMLib::ITMTrackingController> getTrackingController();

  void outputTrackingResults(std::string path);

  void outputTrackingResults(std::ofstream& of);
};

}

#endif //MT_OBJSLAM_OBJSLAMTRACKINGENGINE_H
