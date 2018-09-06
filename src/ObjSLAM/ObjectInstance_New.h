//
// Created by khuang on 8/1/18.
//
#pragma once
#ifndef MT_OBJSLAM_OBJECTINSTANCE_NEW_H
#define MT_OBJSLAM_OBJECTINSTANCE_NEW_H
#include <memory>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Objects/RenderStates/ITMRenderState.h>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Objects/Tracking/ITMTrackingState.h>
#include "ObjectClassLabel_Group.h"
#include "ObjectInstanceScene.h"
#include "External/InfiniTAM/InfiniTAM/ITMLib/Objects/Views/ITMView.h"

namespace ObjSLAM {
template<class TVoxel, class TIndex>
class ObjectView_New;

template<class TVoxel, class TIndex>
class ObjectInstance_New : public std::enable_shared_from_this<ObjectInstance_New<TVoxel, TIndex>> {

 private:
  std::shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>> label;
  std::shared_ptr<ObjectView_New<TVoxel, TIndex>> anchor_view;
  std::shared_ptr<ITMLib::ITMView> anchor_view_itm;
  std::shared_ptr<ObjectInstanceScene<TVoxel, TIndex>> scene;
  std::shared_ptr<ITMLib::ITMRenderState> r_state;
  std::shared_ptr<ITMLib::ITMTrackingState> t_state;
 public:
  bool isBackground = false;

  //Constructor
  ObjectInstance_New(std::shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>> _label) :
      label(_label) {
    isBackground = (this->getLabelIndex()==0);
  }

  void addObjectInstanceToLabel();

  void setScene(std::shared_ptr<ObjectInstanceScene<TVoxel, TIndex>> _scene) { scene = _scene; }

  void setAnchorView(std::shared_ptr<ObjectView_New<TVoxel, TIndex>> _anchor_view) { anchor_view = _anchor_view; }
  void setAnchorView(ObjectView_New<TVoxel, TIndex> *_anchor_view) { anchor_view =
                                                                         std::shared_ptr<ObjectView_New<TVoxel,
                                                                                                        TIndex>>(
                                                                             _anchor_view);
  }

  void setAnchorView_ITM(std::shared_ptr<ITMLib::ITMView> _anchor_view) { anchor_view_itm = _anchor_view; }

  void setRenderState(std::shared_ptr<ITMLib::ITMRenderState> _r_state) { r_state = _r_state; }
  void setTrackingState(std::shared_ptr<ITMLib::ITMTrackingState> _t_state){t_state=_t_state; }

  std::shared_ptr<ObjectView_New<TVoxel, TIndex>> getAnchorView() { return anchor_view; }
  ITMLib::ITMView *getAnchorView_ITM() { return anchor_view_itm.get(); }

  std::shared_ptr<ObjectInstanceScene<TVoxel, TIndex>> getScene() { return this->scene; }

  std::shared_ptr<ObjectClassLabel_Group<TVoxel, TIndex>> getClassLabel() { return label; }

  std::shared_ptr<ITMLib::ITMRenderState> getRenderState() { return this->r_state; }
  std::shared_ptr<ITMLib::ITMTrackingState> getTrackingState() { return this->t_state; }

  int getLabelIndex(){ return this->getClassLabel()->getLabelIndex();}

};

}
#include "ObjectInstance_New.tpp"
#endif //MT_OBJSLAM_OBJECTINSTANCE_NEW_H
