// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM
// Edited by Kailin Huang 2018 ETH Zurich

#pragma once

#include "../Interface/ITMVisualisationEngine.h"

namespace ITMLib
{
	template<class TVoxel, class TIndex>
	class ITMVisualisationEngine_CPU : public ITMVisualisationEngine < TVoxel, TIndex >
	{
	public:
		explicit ITMVisualisationEngine_CPU(void) { }
		~ITMVisualisationEngine_CPU(void) { }

		ITMRenderState* CreateRenderState(const ITMScene<TVoxel, TIndex> *scene, const Vector2i & imgSize) const;
		void FindVisibleBlocks(const ITMScene<TVoxel,TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		/**
		 * Edit Kailin Huang
		 * @brief FindVisibleBlocks and then update the view count in the voxels
		 * @param scene
		 * @param pose
		 * @param intrinsics
		 * @param renderState
		 */
		void FindVisibleBlocksAndUpdateViewCount(ITMScene<TVoxel,TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		int CountVisibleBlocks(const ITMScene<TVoxel,TIndex> *scene,const ITMRenderState *renderState, int minBlockId, int maxBlockId) const;
		void CreateExpectedDepths(const ITMScene<TVoxel,TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		void RenderImage(const ITMScene<TVoxel,TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState,
			ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE,
			IITMVisualisationEngine::RenderRaycastSelection raycastType = IITMVisualisationEngine::RENDER_FROM_NEW_RAYCAST) const;
		/**
		 * Edit Kailin Huang
		 * @brief RenderImage for multiple objects
		 * @param obj_inst_ptr_vector Vector of Objects
		 * @param pose
		 * @param intrinsics
		 * @param renderState
		 * @param outputImage
		 * @param type
		 * @param raycastType
		 */
		void RenderImageMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>>& obj_inst_ptr_vector, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState,
							  ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE,
							  IITMVisualisationEngine::RenderRaycastSelection raycastType = IITMVisualisationEngine::RENDER_FROM_NEW_RAYCAST) const;
		void FindSurface(const ITMScene<TVoxel,TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState) const;
		void CreatePointCloud(const ITMScene<TVoxel,TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState, bool skipPoints) const;
		void CreateICPMaps(const ITMScene<TVoxel,TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState) const;
		/**
		 * Edit Kailin Huang
		 * @brief CreateICPMaps for multiple objects
		 * @param obj_inst_ptr_vector Vector of Objects
		 * @param view
		 * @param trackingState
		 * @param renderState
		 */
		void CreateICPMapsMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector, const ITMView *view,
				ITMTrackingState *trackingState,
		ITMRenderState *renderState) const;
		void ForwardRender(const ITMScene<TVoxel,TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState) const;
	};

	template<class TVoxel>
	class ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash> : public ITMVisualisationEngine < TVoxel, ITMVoxelBlockHash >
	{
	public:
		explicit ITMVisualisationEngine_CPU(void) { }
		~ITMVisualisationEngine_CPU(void) { }

		ITMRenderState_VH* CreateRenderState(const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const Vector2i & imgSize) const;
		void FindVisibleBlocks(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		/**
		 * Edit Kailin Huang
		 * @brief FindVisibleBlocks and then update the view count in the voxels
		 * @param scene
		 * @param pose
		 * @param intrinsics
		 * @param renderState
		 */
		void FindVisibleBlocksAndUpdateViewCount(ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		int CountVisibleBlocks(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ITMRenderState *renderState, int minBlockId, int maxBlockId) const;
		void CreateExpectedDepths(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, ITMRenderState *renderState) const;
		void RenderImage(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState,
			ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE,
			IITMVisualisationEngine::RenderRaycastSelection raycastType = IITMVisualisationEngine::RENDER_FROM_NEW_RAYCAST) const;
		/**
		 * Edit Kailin Huang
		 * @brief RenderImage for multiple objects
		 * @param obj_inst_ptr_vector Vector of Objects
		 * @param pose
		 * @param intrinsics
		 * @param renderState
		 * @param outputImage
		 * @param type
		 * @param raycastType
		 */
		void RenderImageMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, ITMVoxelBlockHash>>& obj_inst_ptr_vector, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState,
						 ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE,
						 IITMVisualisationEngine::RenderRaycastSelection raycastType = IITMVisualisationEngine::RENDER_FROM_NEW_RAYCAST) const;
		void FindSurface(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics, const ITMRenderState *renderState) const;
		void CreatePointCloud(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState, bool skipPoints) const;
		void CreateICPMaps(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState) const;
		/**
		 * Edit Kailin Huang
		 * @brief CreateICPMaps for multiple objects
		 * @param obj_inst_ptr_vector Vector of Objects
		 * @param view
		 * @param trackingState
		 * @param renderState
		 */
		void CreateICPMapsMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, ITMVoxelBlockHash>> &obj_inst_ptr_vector, const ITMView *view,
								ITMTrackingState *trackingState,
								ITMRenderState *renderState) const;
		void ForwardRender(const ITMScene<TVoxel,ITMVoxelBlockHash> *scene, const ITMView *view, ITMTrackingState *trackingState, ITMRenderState *renderState) const;
	};
}
