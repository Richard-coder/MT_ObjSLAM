// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM
//Edited by Kailin Huang 2018 ETH Zurich

#include "ITMVisualisationEngine_CPU.h"

#include "../Shared/ITMVisualisationEngine_Shared.h"
#include "../../Reconstruction/Shared/ITMSceneReconstructionEngine_Shared.h"

#include <vector>
#include <cfloat>

using namespace ITMLib;

template<class TVoxel, class TIndex>
static int RenderPointCloud(Vector4u *outRendering, Vector4f *locations, Vector4f *colours, const Vector4f *ptsRay,
                            const TVoxel *voxelData, const typename TIndex::IndexData *voxelIndex, bool skipPoints,
                            float voxelSize,
                            Vector2i imgSize, Vector3f lightSource);

template<class TVoxel, class TIndex>
ITMRenderState *ITMVisualisationEngine_CPU<TVoxel, TIndex>::CreateRenderState(const ITMScene<TVoxel, TIndex> *scene,
                                                                              const Vector2i &imgSize) const {
    return new ITMRenderState(
            imgSize, scene->sceneParams->viewFrustum_min, scene->sceneParams->viewFrustum_max, MEMORYDEVICE_CPU
    );
}

template<class TVoxel>
ITMRenderState_VH *ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CreateRenderState(
        const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const Vector2i &imgSize) const {
    return new ITMRenderState_VH(
            (sceneIsBackground ? ITMVoxelBlockHash::noTotalEntries_BG : ITMVoxelBlockHash::noTotalEntries), imgSize,
            scene->sceneParams->viewFrustum_min, scene->sceneParams->viewFrustum_max, MEMORYDEVICE_CPU
    );
}

template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::FindVisibleBlocks(const ITMScene<TVoxel, TIndex> *scene,
                                                                   const ORUtils::SE3Pose *pose,
                                                                   const ITMIntrinsics *intrinsics,
                                                                   ITMRenderState *renderState) const {
}

template<class TVoxel>
void ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::FindVisibleBlocks(
        const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
        ITMRenderState *renderState) const {
    const ITMHashEntry *hashTable = scene->index.GetEntries();
    int noTotalEntries = (sceneIsBackground ? scene->index.noTotalEntries_BG : scene->index.noTotalEntries);
    float voxelSize = scene->sceneParams->voxelSize;
    Vector2i imgSize = renderState->renderingRangeImage->noDims;

    Matrix4f M = pose->GetM();
    Vector4f projParams = intrinsics->projectionParamsSimple.all;

    ITMRenderState_VH *renderState_vh = (ITMRenderState_VH *) renderState;

    int noVisibleEntries = 0;
    int *visibleEntryIDs = renderState_vh->GetVisibleEntryIDs();

    //build visible list
    for (int targetIdx = 0; targetIdx < noTotalEntries; targetIdx++) {
        unsigned char hashVisibleType = 0;// = entriesVisibleType[targetIdx];
        const ITMHashEntry &hashEntry = hashTable[targetIdx];

        if (hashEntry.ptr >= 0) {
            bool isVisible, isVisibleEnlarged;
            checkBlockVisibility<false>(isVisible, isVisibleEnlarged, hashEntry.pos, M, projParams, voxelSize, imgSize);
            hashVisibleType = isVisible;
        }

        if (hashVisibleType > 0) {
            visibleEntryIDs[noVisibleEntries] = targetIdx;
            noVisibleEntries++;
        }
    }

    renderState_vh->noVisibleEntries = noVisibleEntries;
}


template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::FindVisibleBlocksAndUpdateViewCount(ITMScene<TVoxel, TIndex> *scene,
                                                                                     const ORUtils::SE3Pose *pose,
                                                                                     const ITMIntrinsics *intrinsics,
                                                                                     ITMRenderState *renderState) const {
}

template<class TVoxel>
void ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::FindVisibleBlocksAndUpdateViewCount(
        ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
        ITMRenderState *renderState) const {
    const ITMHashEntry *hashTable = scene->index.GetEntries();
    int noTotalEntries = (sceneIsBackground ? scene->index.noTotalEntries_BG : scene->index.noTotalEntries);
    float voxelSize = scene->sceneParams->voxelSize;
    Vector2i imgSize = renderState->renderingRangeImage->noDims;

    Matrix4f M = pose->GetM();
    Vector4f projParams = intrinsics->projectionParamsSimple.all;

    ITMRenderState_VH *renderState_vh = (ITMRenderState_VH *) renderState;

    int noVisibleEntries = 0;
    int *visibleEntryIDs = renderState_vh->GetVisibleEntryIDs();
    TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();

    //build visible list
    for (int targetIdx = 0; targetIdx < noTotalEntries; targetIdx++) {
        unsigned char hashVisibleType = 0;// = entriesVisibleType[targetIdx];
        const ITMHashEntry &hashEntry = hashTable[targetIdx];

        if (hashEntry.ptr >= 0) {
            bool isVisible, isVisibleEnlarged;
            checkBlockVisibility<false>(isVisible, isVisibleEnlarged, hashEntry.pos, M, projParams, voxelSize, imgSize);
            hashVisibleType = isVisible;
        }

        if (hashVisibleType > 0) {
            visibleEntryIDs[noVisibleEntries] = targetIdx;

            //increment view count
            int blockIdx = hashEntry.ptr * SDF_BLOCK_SIZE3;
            for (int idx = blockIdx; idx < blockIdx + SDF_BLOCK_SIZE3; idx++) {

                TVoxel &vox = voxelData[idx];
                voxelData[idx].view_count++;

            }

            noVisibleEntries++;
        }
    }

    renderState_vh->noVisibleEntries = noVisibleEntries;
}

template<class TVoxel, class TIndex>
int ITMVisualisationEngine_CPU<TVoxel, TIndex>::CountVisibleBlocks(const ITMScene<TVoxel, TIndex> *scene,
                                                                   const ITMRenderState *renderState, int minBlockId,
                                                                   int maxBlockId) const {
    return 1;
}

template<class TVoxel>
int ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CountVisibleBlocks(
        const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ITMRenderState *renderState, int minBlockId,
        int maxBlockId) const {
    const ITMRenderState_VH *renderState_vh = (const ITMRenderState_VH *) renderState;

    int noVisibleEntries = renderState_vh->noVisibleEntries;
    const int *visibleEntryIDs = renderState_vh->GetVisibleEntryIDs();

    int ret = 0;
    for (int i = 0; i < noVisibleEntries; ++i) {
        int blockID = scene->index.GetEntries()[visibleEntryIDs[i]].ptr;
        if ((blockID >= minBlockId) && (blockID <= maxBlockId)) ++ret;
    }

    return ret;
}

template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::CreateExpectedDepths(const ITMScene<TVoxel, TIndex> *scene,
                                                                      const ORUtils::SE3Pose *pose,
                                                                      const ITMIntrinsics *intrinsics,
                                                                      ITMRenderState *renderState) const {
    Vector2i imgSize = renderState->renderingRangeImage->noDims;
    Vector2f *minmaxData = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);

    for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
        //TODO : this could be improved a bit...
        Vector2f &pixel = minmaxData[locId];
        pixel.x = 0.2f;
        pixel.y = 3.0f;
    }
}

template<class TVoxel>
void ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CreateExpectedDepths(
        const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
        ITMRenderState *renderState) const {
//	std::cout<<"pose"<<pose->GetM().m00<<std::endl;
    Vector2i imgSize = renderState->renderingRangeImage->noDims;
    Vector2f *minmaxData = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);

    for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
        Vector2f &pixel = minmaxData[locId];
        pixel.x = FAR_AWAY;
        pixel.y = VERY_CLOSE;
    }

    float voxelSize = scene->sceneParams->voxelSize;

    std::vector<RenderingBlock> renderingBlocks(MAX_RENDERING_BLOCKS);
    int numRenderingBlocks = 0;

    ITMRenderState_VH *renderState_vh = (ITMRenderState_VH *) renderState;

    const int *visibleEntryIDs = renderState_vh->GetVisibleEntryIDs();
    int noVisibleEntries = renderState_vh->noVisibleEntries;

    //go through list of visible 8x8x8 blocks
    for (int blockNo = 0; blockNo < noVisibleEntries; ++blockNo) {
        const ITMHashEntry &blockData(scene->index.GetEntries()[visibleEntryIDs[blockNo]]);


        Vector2i upperLeft, lowerRight;
        Vector2f zRange;
        bool validProjection = false;
        if (blockData.ptr >= 0) {
            validProjection = ProjectSingleBlock(blockData.pos, pose->GetM(), intrinsics->projectionParamsSimple.all,
                                                 imgSize, voxelSize, upperLeft, lowerRight, zRange);
        }
        if (!validProjection) continue;

        Vector2i requiredRenderingBlocks(
                (int) ceilf((float) (lowerRight.x - upperLeft.x + 1) / (float) renderingBlockSizeX),
                (int) ceilf((float) (lowerRight.y - upperLeft.y + 1) / (float) renderingBlockSizeY));
        int requiredNumBlocks = requiredRenderingBlocks.x * requiredRenderingBlocks.y;

        if (numRenderingBlocks + requiredNumBlocks >= MAX_RENDERING_BLOCKS) continue;
        int offset = numRenderingBlocks;
        numRenderingBlocks += requiredNumBlocks;

        CreateRenderingBlocks(&(renderingBlocks[0]), offset, upperLeft, lowerRight, zRange);
    }

    // go through rendering blocks
    for (int blockNo = 0; blockNo < numRenderingBlocks; ++blockNo) {
        // fill minmaxData
        const RenderingBlock &b(renderingBlocks[blockNo]);

        for (int y = b.upperLeft.y; y <= b.lowerRight.y; ++y) {
            for (int x = b.upperLeft.x; x <= b.lowerRight.x; ++x) {
                Vector2f &pixel(minmaxData[x + y * imgSize.x]);    //creating a reference
                if (pixel.x > b.zRange.x) pixel.x = b.zRange.x;
                if (pixel.y < b.zRange.y) pixel.y = b.zRange.y;
            }
        }
    }
}

template<class TVoxel, class TIndex>
static void GenericRaycast(const ITMScene<TVoxel, TIndex> *scene, const Vector2i &imgSize, const Matrix4f &invM,
                           const Vector4f &projParams, const ITMRenderState *renderState, bool updateVisibleList) {
    const Vector2f *minmaximg = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);
    float mu = scene->sceneParams->mu;
    float oneOverVoxelSize = 1.0f / scene->sceneParams->voxelSize;

    renderState->raycastResult->Clear();

    Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
    const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();
    const typename ITMVoxelBlockHash::IndexData *voxelIndex = scene->index.getIndexData();
    uchar *entriesVisibleType = NULL;
    if (updateVisibleList && (dynamic_cast<const ITMRenderState_VH *>(renderState) != NULL)) {
        entriesVisibleType = ((ITMRenderState_VH *) renderState)->GetEntriesVisibleType();
    }

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
    for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
        int y = locId / imgSize.x;
        int x = locId - y * imgSize.x;
        int locId2 =
                (int) floor((float) x / minmaximg_subsample) + (int) floor((float) y / minmaximg_subsample) * imgSize.x;

        if (entriesVisibleType != NULL)
            castRay<TVoxel, TIndex, true>(
                    pointsRay[locId],
                    entriesVisibleType,
                    x, y,
                    voxelData,
                    voxelIndex,
                    invM,
                    InvertProjectionParams(projParams),
                    oneOverVoxelSize,
                    mu,
                    minmaximg[locId2]
            );
        else
            castRay<TVoxel, TIndex, false>(
                    pointsRay[locId],
                    NULL,
                    x, y,
                    voxelData,
                    voxelIndex,
                    invM,
                    InvertProjectionParams(projParams),
                    oneOverVoxelSize,
                    mu,
                    minmaximg[locId2]
            );
    }
}

//Edit Kailin Huang
//RayCast for Multiple Objects
template<class TVoxel, class TIndex>
static void GenericRaycastMultiObj(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector,
                                   const Vector2i &imgSize, const Matrix4f &invM, const Vector4f &projParams,
                                   const ITMRenderState *renderState, bool updateVisibleList) {
    const Vector2f *minmaximg = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);
    float mu;
    float oneOverVoxelSize;

    std::vector<const CONSTPTR(TVoxel) *> voxelData_vec;

    std::vector</*const*/ ITMHashEntry *> voxelIndex_vec;

    std::vector<uchar *> entriesVisibleType_vec;

    Vector4f *pointsRay_final = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);

    typename std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>>::iterator it;

/*#ifdef WITH_OPENMP
#pragma omp parallel for private(sceneIsBackground)
#endif*/
    for(it = obj_inst_ptr_vector.begin(); it !=obj_inst_ptr_vector.end(); it++){


        ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex> obj_inst_ptr = *it; //obj_inst_ptr_vector.at(i);
        sceneIsBackground = obj_inst_ptr->CheckIsBackground();

        auto scene = obj_inst_ptr->GetScene();
        /*const*/ ITMHashEntry *voxelIndex = scene->index.getIndexData();

        const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();

        ITMRenderState *r_state_tmp = obj_inst_ptr->GetRenderState().get();
//TODO
//        r_state_tmp->raycastResult->Clear();

        Vector4f *pointsRay = r_state_tmp->raycastResult->GetData(MEMORYDEVICE_CPU);

        mu = scene->sceneParams->mu;
        oneOverVoxelSize = 1.0f / scene->sceneParams->voxelSize;

        uchar *entriesVisibleType = NULL;
        if (updateVisibleList && (dynamic_cast<const ITMRenderState_VH *>(r_state_tmp) != NULL)) {
            entriesVisibleType = ((ITMRenderState_VH *) r_state_tmp)->GetEntriesVisibleType();
        }
        //entriesVisibleType_vec.push_back(entriesVisibleType);
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
        for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
            int y = locId / imgSize.x;
            int x = locId - y * imgSize.x;
            int locId2 = (int) floor((float) x / minmaximg_subsample) +
                         (int) floor((float) y / minmaximg_subsample) * imgSize.x;

            if (entriesVisibleType != NULL)
                castRay<TVoxel, TIndex, true>(
                        pointsRay[locId],
                        entriesVisibleType,
                        x, y,
                        voxelData,
                        voxelIndex,
                        invM,
                        InvertProjectionParams(projParams),
                        oneOverVoxelSize,
                        mu,
                        minmaximg[locId2]
                );
            else
                castRay<TVoxel, TIndex, false>(
                        pointsRay[locId],
                        NULL,
                        x, y,
                        voxelData,
                        voxelIndex,
                        invM,
                        InvertProjectionParams(projParams),
                        oneOverVoxelSize,
                        mu,
                        minmaximg[locId2]
                );
        }
    }

    Matrix4f M;
    invM.inv(M);

//fuse the point clouds
#ifdef WITH_OPENMP
#pragma omp parallel for private(sceneIsBackground)
#endif
    for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
        int y = locId / imgSize.x;
        int x = locId - y * imgSize.x;

        double dist = DBL_MAX;

        typename std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>>::iterator it;
        int i = 0;
        for(it = obj_inst_ptr_vector.begin(); it !=obj_inst_ptr_vector.end(); it++,i++){

            sceneIsBackground = i == 0 ? true : false;
            ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex> obj_inst_ptr = *it; //obj_inst_ptr_vector.at(i);

            Vector4f *pt_ray_tmp = obj_inst_ptr->GetRenderState()->raycastResult->GetData(MEMORYDEVICE_CPU);

            Vector4f pt_tmp = pt_ray_tmp[locId];
            if (pt_tmp.w == 0) continue;

            Vector4f pt_tmp_tmp = pt_tmp;

            pt_tmp_tmp.w = 1.0f;

            Vector4f pt_cam_f = M * pt_tmp_tmp;
            double dist_new = NORM3(pt_cam_f);
            if (dist_new < dist) {
                dist = dist_new;
                pointsRay_final[locId]=pt_tmp;
            }
        }
    }

}

//Old version, unused
template<class TVoxel, class TIndex>
static void GenericRaycastMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector,
                                const Vector2i &imgSize, const Matrix4f &invM, const Vector4f &projParams,
                                const ITMRenderState *renderState, bool updateVisibleList) {
    const Vector2f *minmaximg = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);
    float mu;
    float oneOverVoxelSize;

    std::vector<const CONSTPTR(TVoxel) *> voxelData_vec;

    std::vector</*const*/ ITMHashEntry *> voxelIndex_vec;

    std::vector<uchar *> entriesVisibleType_vec;

    Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);


    //TODO this loop is very WEIRD... cannot push back to 2 vectors same time.
    for (size_t i = 0; i < obj_inst_ptr_vector.size(); ++i) {
        sceneIsBackground = i == 0 ? true : false;
        ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex> obj_inst_ptr = obj_inst_ptr_vector.at(i);

        auto scene = obj_inst_ptr->GetScene();
        /*const*/ ITMHashEntry *voxelIndex = scene->index.getIndexData();

        const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();

        voxelIndex_vec.push_back(voxelIndex);
        voxelData_vec.push_back(voxelData);

        mu = scene->sceneParams->mu;
        oneOverVoxelSize = 1.0f / scene->sceneParams->voxelSize;

        //TODO check why no value in voxelIndex, voxelData is good
//		uchar *entriesVisibleType = NULL;
//		if (updateVisibleList && (dynamic_cast<const ITMRenderState_VH *>(renderState) != NULL)) {
//			entriesVisibleType = ((ITMRenderState_VH *) renderState)->GetEntriesVisibleType();
//		}
//		entriesVisibleType_vec.push_back(entriesVisibleType);

    }


#ifdef WITH_OPENMP
#pragma omp parallel for private(sceneIsBackground)
#endif
    for (int locId = 0; locId < imgSize.x * imgSize.y; ++locId) {
        int y = locId / imgSize.x;
        int x = locId - y * imgSize.x;
        int locId2 =
                (int) floor((float) x / minmaximg_subsample) + (int) floor((float) y / minmaximg_subsample) * imgSize.x;
//TODO here the both vectors are empty again...why WTF!!!!
        /*if (entriesVisibleType!=NULL)*/
        castRayMulti<TVoxel, TIndex>(
                pointsRay[locId],
                entriesVisibleType_vec,
                x, y,
                voxelData_vec,
                voxelIndex_vec,
                invM,
                InvertProjectionParams(projParams),
                oneOverVoxelSize,
                mu,
                minmaximg[locId2]
        );
        /*else castRayMulti<TVoxel, TIndex, false>(
                    pointsRay[locId],
                    NULL,
                    x, y,
                    voxelData_vec,
                    voxelIndex_vec,
                    invM,
                    InvertProjectionParams(projParams),
                    oneOverVoxelSize,
                    mu,
                    minmaximg[locId2]
            );*/
    }

}


template<class TVoxel, class TIndex>
static void
RenderImage_common(const ITMScene<TVoxel, TIndex> *scene, const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
                   const ITMRenderState *renderState, ITMUChar4Image *outputImage,
                   IITMVisualisationEngine::RenderImageType type,
                   IITMVisualisationEngine::RenderRaycastSelection raycastType) {
    Vector2i imgSize = outputImage->noDims;
    Matrix4f invM = pose->GetInvM();

    Vector4f *pointsRay;
    if (raycastType == IITMVisualisationEngine::RENDER_FROM_OLD_RAYCAST)
        pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
    else {
        if (raycastType == IITMVisualisationEngine::RENDER_FROM_OLD_FORWARDPROJ)
            pointsRay = renderState->forwardProjection->GetData(MEMORYDEVICE_CPU);
        else {
            // this one is generally done for freeview visualisation, so
            // no, do not update the list of visible blocks
            GenericRaycast(scene, imgSize, invM, intrinsics->projectionParamsSimple.all, renderState, false);
            pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
        }
    }

    Vector3f lightSource = -Vector3f(invM.getColumn(2));
    Vector4u *outRendering = outputImage->GetData(MEMORYDEVICE_CPU);
    const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();
    const typename TIndex::IndexData *voxelIndex = scene->index.getIndexData();

    if ((type == IITMVisualisationEngine::RENDER_COLOUR_FROM_VOLUME) &&
        (!TVoxel::hasColorInformation))
        type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE;

    switch (type) {
        case IITMVisualisationEngine::RENDER_COLOUR_FROM_VOLUME:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
            for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                Vector4f ptRay = pointsRay[locId];
                processPixelColour<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData,
                                                   voxelIndex);
            }
            break;
        case IITMVisualisationEngine::RENDER_COLOUR_FROM_NORMAL:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
            for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                Vector4f ptRay = pointsRay[locId];
                processPixelNormal<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData,
                                                   voxelIndex, lightSource);
            }
            break;
        case IITMVisualisationEngine::RENDER_COLOUR_FROM_CONFIDENCE:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
            for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                Vector4f ptRay = pointsRay[locId];
                processPixelConfidence<TVoxel, TIndex>(outRendering[locId], ptRay, ptRay.w > 0, voxelData, voxelIndex,
                                                       lightSource);
            }
            break;
        case IITMVisualisationEngine::RENDER_SHADED_GREYSCALE_IMAGENORMALS:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
            for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                int y = locId / imgSize.x;
                int x = locId - y * imgSize.x;

                if (intrinsics->FocalLengthSignsDiffer()) {
                    processPixelGrey_ImageNormals<true, true>(outRendering, pointsRay, imgSize, x, y,
                                                              scene->sceneParams->voxelSize, lightSource);
                } else {
                    processPixelGrey_ImageNormals<true, false>(outRendering, pointsRay, imgSize, x, y,
                                                               scene->sceneParams->voxelSize, lightSource);
                }
            }
            break;
        case IITMVisualisationEngine::RENDER_SHADED_GREYSCALE:
        default:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
            for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                Vector4f ptRay = pointsRay[locId];
                processPixelGrey<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData,
                                                 voxelIndex, lightSource);
            }
    }
}

/**
 * Edit Kailin Huang
 * @brief RenderImage_common for multiple objects
 * @tparam TVoxel
 * @tparam TIndex
 * @param obj_inst_ptr_vector Vector of objects
 * @param pose
 * @param intrinsics
 * @param renderState
 * @param outputImage
 * @param type
 * @param raycastType
 */
template<class TVoxel, class TIndex>
static void RenderImage_common_multi(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector,
                                     const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
                                     const ITMRenderState *renderState, ITMUChar4Image *outputImage,
                                     IITMVisualisationEngine::RenderImageType type,
                                     IITMVisualisationEngine::RenderRaycastSelection raycastType) {
    Vector2i imgSize = outputImage->noDims;
    Matrix4f invM = pose->GetInvM();

    Vector4f *pointsRay;
    GenericRaycastMultiObj(obj_inst_ptr_vector, imgSize, invM, intrinsics->projectionParamsSimple.all, renderState, false);
    pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);

//    if (raycastType == IITMVisualisationEngine::RENDER_FROM_OLD_RAYCAST)
//        pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
//    else {
//        if (raycastType == IITMVisualisationEngine::RENDER_FROM_OLD_FORWARDPROJ)
//            pointsRay = renderState->forwardProjection->GetData(MEMORYDEVICE_CPU);
//        else {
//            // this one is generally done for freeview visualisation, so
//            // no, do not update the list of visible blocks
//            GenericRaycastMultiObj(obj_inst_ptr_vector, imgSize, invM, intrinsics->projectionParamsSimple.all, renderState, false);
//
//            pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
//        }
//    }


    Vector3f lightSource = -Vector3f(invM.getColumn(2));
    Vector4u *outRendering = outputImage->GetData(MEMORYDEVICE_CPU);

    int i = 0;

    typename std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>>::iterator it;

    for(it = obj_inst_ptr_vector.begin(); it !=obj_inst_ptr_vector.end(); it++,i++){
//    for (size_t i = 0; i < obj_inst_ptr_vector.size(); ++i) {
        sceneIsBackground = i == 0 ? true : false;
//sceneIsBackground=true;
        ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex> obj_inst_ptr = *it;//obj_inst_ptr_vector.at(0);

        const ITMScene<TVoxel, TIndex> *scene = obj_inst_ptr->GetScene().get();


        const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();
        const typename TIndex::IndexData *voxelIndex = scene->index.getIndexData();

        if ((type == IITMVisualisationEngine::RENDER_COLOUR_FROM_VOLUME) &&
            (!TVoxel::hasColorInformation))
            type = IITMVisualisationEngine::RENDER_SHADED_GREYSCALE;

        switch (type) {
            case IITMVisualisationEngine::RENDER_COLOUR_FROM_VOLUME:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
                for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                    Vector4f ptRay = pointsRay[locId];
                    if(ptRay.w>0) {
                        processPixelColour<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0,
                                                           voxelData,
                                                           voxelIndex);
                    }
                }
                break;
            case IITMVisualisationEngine::RENDER_COLOUR_FROM_NORMAL:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
                for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                    Vector4f ptRay = pointsRay[locId];
                    processPixelNormal<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData,
                                                       voxelIndex, lightSource);
                }
                break;
            case IITMVisualisationEngine::RENDER_COLOUR_FROM_CONFIDENCE:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
                for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                    Vector4f ptRay = pointsRay[locId];
                    processPixelConfidence<TVoxel, TIndex>(outRendering[locId], ptRay, ptRay.w > 0, voxelData,
                                                           voxelIndex, lightSource);
                }
                break;
            case IITMVisualisationEngine::RENDER_SHADED_GREYSCALE_IMAGENORMALS:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
                for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                    int y = locId / imgSize.x;
                    int x = locId - y * imgSize.x;

                    if (intrinsics->FocalLengthSignsDiffer()) {
                        processPixelGrey_ImageNormals<true, true>(outRendering, pointsRay, imgSize, x, y,
                                                                  scene->sceneParams->voxelSize, lightSource);
                    } else {
                        processPixelGrey_ImageNormals<true, false>(outRendering, pointsRay, imgSize, x, y,
                                                                   scene->sceneParams->voxelSize, lightSource);
                    }
                }
                break;
            case IITMVisualisationEngine::RENDER_SHADED_GREYSCALE:
            default:
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
                for (int locId = 0; locId < imgSize.x * imgSize.y; locId++) {
                    Vector4f ptRay = pointsRay[locId];
                   // ptRay.w = 1; //pure test to see if w=0 is the problem
                   if(ptRay.w>0){

                    processPixelGrey<TVoxel, TIndex>(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData,
                                                     voxelIndex, lightSource);
                   }
                }
        }

    }

}

template<class TVoxel, class TIndex>
static void
CreatePointCloud_common(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState,
                        ITMRenderState *renderState, bool skipPoints) {
    Vector2i imgSize = renderState->raycastResult->noDims;
    Matrix4f invM = trackingState->pose_d->GetInvM() * view->calib.trafo_rgb_to_depth.calib;

    // this one is generally done for the colour tracker, so yes, update
    // the list of visible blocks if possible
    GenericRaycast(scene, imgSize, invM, view->calib.intrinsics_rgb.projectionParamsSimple.all, renderState, true);
    trackingState->pose_pointCloud->SetFrom(trackingState->pose_d);

    trackingState->pointCloud->noTotalPoints = RenderPointCloud<TVoxel, TIndex>(
            trackingState->pointCloud->locations->GetData(MEMORYDEVICE_CPU),
            trackingState->pointCloud->colours->GetData(MEMORYDEVICE_CPU),
            renderState->raycastResult->GetData(MEMORYDEVICE_CPU),
            scene->localVBA.GetVoxelBlocks(),
            scene->index.getIndexData(),
            skipPoints,
            scene->sceneParams->voxelSize,
            imgSize,
            -Vector3f(invM.getColumn(2))
    );
}

template<class TVoxel, class TIndex>
static void
CreateICPMaps_common(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState,
                     ITMRenderState *renderState) {
    Vector2i imgSize = renderState->raycastResult->noDims;
    Matrix4f invM = trackingState->pose_d->GetInvM();

    // this one is generally done for the ICP tracker, so yes, update
    // the list of visible blocks if possible
    GenericRaycast(scene, imgSize, invM, view->calib.intrinsics_d.projectionParamsSimple.all, renderState, true);
    trackingState->pose_pointCloud->SetFrom(trackingState->pose_d);

    Vector3f lightSource = -Vector3f(invM.getColumn(2));
    Vector4f *normalsMap = trackingState->pointCloud->colours->GetData(MEMORYDEVICE_CPU);
    Vector4f *pointsMap = trackingState->pointCloud->locations->GetData(MEMORYDEVICE_CPU);
    Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
    float voxelSize = scene->sceneParams->voxelSize;

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < imgSize.y; y++)
        for (int x = 0; x < imgSize.x; x++) {
            if (view->calib.intrinsics_d.FocalLengthSignsDiffer()) {
                processPixelICP<true, true>(pointsMap, normalsMap, pointsRay, imgSize, x, y, voxelSize, lightSource);
            } else {
                processPixelICP<true, false>(pointsMap, normalsMap, pointsRay, imgSize, x, y, voxelSize, lightSource);
            }

        }
}

/**
 * Edit Kailin Huang
 * @brief CreateICPMaps_common for multiple objects
 * @tparam TVoxel
 * @tparam TIndex
 * @param obj_inst_ptr_vector vector of objects
 * @param view
 * @param trackingState
 * @param renderState
 */
template<class TVoxel, class TIndex>
static void
CreateICPMaps_common_MultiObj(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector, const ITMView *view, ITMTrackingState *trackingState,
                     ITMRenderState *renderState) {
    Vector2i imgSize = renderState->raycastResult->noDims;
    Matrix4f invM = trackingState->pose_d->GetInvM();


    GenericRaycastMultiObj(obj_inst_ptr_vector, imgSize, invM, view->calib.intrinsics_d.projectionParamsSimple.all, renderState, true);
    int i = 0;
    typename std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>>::iterator it;
    for(it = obj_inst_ptr_vector.begin(); it !=obj_inst_ptr_vector.end(); it++,i++){
//    for (size_t i = 0; i < obj_inst_ptr_vector.size(); ++i) {
        sceneIsBackground = i == 0 ? true : false;
//sceneIsBackground=true;
        ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex> obj_inst_ptr = *it;//obj_inst_ptr_vector.at(0);

        const ITMScene<TVoxel, TIndex> *scene = obj_inst_ptr->GetScene().get();
        //ITMTrackingState* trackingState = obj_inst_ptr->GetTrackingState().get();

        // this one is generally done for the ICP tracker, so yes, update
        // the list of visible blocks if possible

        trackingState->pose_pointCloud->SetFrom(trackingState->pose_d);

        Vector3f lightSource = -Vector3f(invM.getColumn(2));
        Vector4f *normalsMap = trackingState->pointCloud->colours->GetData(MEMORYDEVICE_CPU);
        Vector4f *pointsMap = trackingState->pointCloud->locations->GetData(MEMORYDEVICE_CPU);
        Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
        float voxelSize = scene->sceneParams->voxelSize;


#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
        for (int y = 0; y < imgSize.y; y++)
            for (int x = 0; x < imgSize.x; x++) {
                if (view->calib.intrinsics_d.FocalLengthSignsDiffer()) {
                    processPixelICP<true, true>(pointsMap, normalsMap, pointsRay, imgSize, x, y, voxelSize, lightSource);
                } else {
                    processPixelICP<true, false>(pointsMap, normalsMap, pointsRay, imgSize, x, y, voxelSize, lightSource);
                }

            }

    }


}

template<class TVoxel, class TIndex>
static void
ForwardRender_common(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view, ITMTrackingState *trackingState,
                     ITMRenderState *renderState) {
    Vector2i imgSize = renderState->raycastResult->noDims;
    Matrix4f M = trackingState->pose_d->GetM();
    Matrix4f invM = trackingState->pose_d->GetInvM();
    const Vector4f &projParams = view->calib.intrinsics_d.projectionParamsSimple.all;

    const Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
    Vector4f *forwardProjection = renderState->forwardProjection->GetData(MEMORYDEVICE_CPU);
    float *currentDepth = view->depth->GetData(MEMORYDEVICE_CPU);
    int *fwdProjMissingPoints = renderState->fwdProjMissingPoints->GetData(MEMORYDEVICE_CPU);
    const Vector2f *minmaximg = renderState->renderingRangeImage->GetData(MEMORYDEVICE_CPU);
    float voxelSize = scene->sceneParams->voxelSize;
    const TVoxel *voxelData = scene->localVBA.GetVoxelBlocks();
    const typename TIndex::IndexData *voxelIndex = scene->index.getIndexData();

    renderState->forwardProjection->Clear();

    for (int y = 0; y < imgSize.y; y++)
        for (int x = 0; x < imgSize.x; x++) {
            int locId = x + y * imgSize.x;
            Vector4f pixel = pointsRay[locId];

            int locId_new = forwardProjectPixel(pixel * voxelSize, M, projParams, imgSize);
            if (locId_new >= 0) forwardProjection[locId_new] = pixel;
        }

    int noMissingPoints = 0;
    for (int y = 0; y < imgSize.y; y++)
        for (int x = 0; x < imgSize.x; x++) {
            int locId = x + y * imgSize.x;
            int locId2 = (int) floor((float) x / minmaximg_subsample) +
                         (int) floor((float) y / minmaximg_subsample) * imgSize.x;

            Vector4f fwdPoint = forwardProjection[locId];
            Vector2f minmaxval = minmaximg[locId2];
            float depth = currentDepth[locId];

            if ((fwdPoint.w <= 0) && ((fwdPoint.x == 0 && fwdPoint.y == 0 && fwdPoint.z == 0) || (depth >= 0)) &&
                (minmaxval.x < minmaxval.y))
                //if ((fwdPoint.w <= 0) && (minmaxval.x < minmaxval.y))
            {
                fwdProjMissingPoints[noMissingPoints] = locId;
                noMissingPoints++;
            }
        }

    renderState->noFwdProjMissingPoints = noMissingPoints;
    const Vector4f invProjParams = InvertProjectionParams(projParams);

    for (int pointId = 0; pointId < noMissingPoints; pointId++) {
        int locId = fwdProjMissingPoints[pointId];
        int y = locId / imgSize.x, x = locId - y * imgSize.x;
        int locId2 =
                (int) floor((float) x / minmaximg_subsample) + (int) floor((float) y / minmaximg_subsample) * imgSize.x;

        castRay<TVoxel, TIndex, false>(forwardProjection[locId], NULL, x, y, voxelData, voxelIndex, invM, invProjParams,
                                       1.0f / scene->sceneParams->voxelSize, scene->sceneParams->mu, minmaximg[locId2]);
    }
}

template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::RenderImage(const ITMScene<TVoxel, TIndex> *scene,
                                                             const ORUtils::SE3Pose *pose,
                                                             const ITMIntrinsics *intrinsics,
                                                             const ITMRenderState *renderState,
                                                             ITMUChar4Image *outputImage,
                                                             IITMVisualisationEngine::RenderImageType type,
                                                             IITMVisualisationEngine::RenderRaycastSelection raycastType) const {
    RenderImage_common(scene, pose, intrinsics, renderState, outputImage, type, raycastType);
}

template<class TVoxel>
void
ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::RenderImage(const ITMScene<TVoxel, ITMVoxelBlockHash> *scene,
                                                                   const ORUtils::SE3Pose *pose,
                                                                   const ITMIntrinsics *intrinsics,
                                                                   const ITMRenderState *renderState,
                                                                   ITMUChar4Image *outputImage,
                                                                   IITMVisualisationEngine::RenderImageType type,
                                                                   IITMVisualisationEngine::RenderRaycastSelection raycastType) const {
    RenderImage_common(scene, pose, intrinsics, renderState, outputImage, type, raycastType);
}

//Edit Kailin Huang
template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::RenderImageMulti(
        std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector, const ORUtils::SE3Pose *pose,
        const ITMIntrinsics *intrinsics,
        const ITMRenderState *renderState, ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type,
        IITMVisualisationEngine::RenderRaycastSelection raycastType) const {
    RenderImage_common_multi(obj_inst_ptr_vector, pose, intrinsics, renderState, outputImage, type, raycastType);
}

//Edit Kailin Huang
template<class TVoxel>
void ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::RenderImageMulti(
        std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, ITMVoxelBlockHash>> &obj_inst_ptr_vector,
        const ORUtils::SE3Pose *pose, const ITMIntrinsics *intrinsics,
        const ITMRenderState *renderState, ITMUChar4Image *outputImage, IITMVisualisationEngine::RenderImageType type,
        IITMVisualisationEngine::RenderRaycastSelection raycastType) const {
    RenderImage_common_multi(obj_inst_ptr_vector, pose, intrinsics, renderState, outputImage, type, raycastType);
}

template<class TVoxel, class TIndex>
void ITMVisualisationEngine_CPU<TVoxel, TIndex>::FindSurface(const ITMScene<TVoxel, TIndex> *scene,
                                                             const ORUtils::SE3Pose *pose,
                                                             const ITMIntrinsics *intrinsics,
                                                             const ITMRenderState *renderState) const {
    // this one is generally done for freeview visualisation, so no, do not
    // update the list of visible blocks
    GenericRaycast(scene, renderState->raycastResult->noDims, pose->GetInvM(), intrinsics->projectionParamsSimple.all,
                   renderState, false);
}

template<class TVoxel>
void
ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::FindSurface(const ITMScene<TVoxel, ITMVoxelBlockHash> *scene,
                                                                   const ORUtils::SE3Pose *pose,
                                                                   const ITMIntrinsics *intrinsics,
                                                                   const ITMRenderState *renderState) const {
    // this one is generally done for freeview visualisation, so no, do not
    // update the list of visible blocks
    GenericRaycast(scene, renderState->raycastResult->noDims, pose->GetInvM(), intrinsics->projectionParamsSimple.all,
                   renderState, false);
}

template<class TVoxel, class TIndex>
void
ITMVisualisationEngine_CPU<TVoxel, TIndex>::CreatePointCloud(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view,
                                                             ITMTrackingState *trackingState,
                                                             ITMRenderState *renderState, bool skipPoints) const {
    CreatePointCloud_common(scene, view, trackingState, renderState, skipPoints);
}

template<class TVoxel>
void ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CreatePointCloud(
        const ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ITMView *view, ITMTrackingState *trackingState,
        ITMRenderState *renderState, bool skipPoints) const {
    CreatePointCloud_common(scene, view, trackingState, renderState, skipPoints);
}

template<class TVoxel, class TIndex>
void
ITMVisualisationEngine_CPU<TVoxel, TIndex>::CreateICPMaps(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view,
                                                          ITMTrackingState *trackingState,
                                                          ITMRenderState *renderState) const {
    CreateICPMaps_common(scene, view, trackingState, renderState);
}

template<class TVoxel>
void
ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CreateICPMaps(const ITMScene<TVoxel, ITMVoxelBlockHash> *scene,
                                                                     const ITMView *view,
                                                                     ITMTrackingState *trackingState,
                                                                     ITMRenderState *renderState) const {
    CreateICPMaps_common(scene, view, trackingState, renderState);
}

//Edit Kailin Huang
template<class TVoxel, class TIndex>
void
ITMVisualisationEngine_CPU<TVoxel, TIndex>::CreateICPMapsMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, TIndex>> &obj_inst_ptr_vector, const ITMView *view,
                                                          ITMTrackingState *trackingState,
                                                          ITMRenderState *renderState) const {
    CreateICPMaps_common_MultiObj(obj_inst_ptr_vector, view, trackingState, renderState);
}

//Edit Kailin Huang
template<class TVoxel>
void
ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::CreateICPMapsMulti(std::vector<ObjSLAM::ObjectInstance_ptr<TVoxel, ITMVoxelBlockHash>> &obj_inst_ptr_vector,
                                                                     const ITMView *view,
                                                                     ITMTrackingState *trackingState,
                                                                     ITMRenderState *renderState) const {
    CreateICPMaps_common_MultiObj(obj_inst_ptr_vector, view, trackingState, renderState);
}

template<class TVoxel, class TIndex>
void
ITMVisualisationEngine_CPU<TVoxel, TIndex>::ForwardRender(const ITMScene<TVoxel, TIndex> *scene, const ITMView *view,
                                                          ITMTrackingState *trackingState,
                                                          ITMRenderState *renderState) const {
    ForwardRender_common(scene, view, trackingState, renderState);
}

template<class TVoxel>
void
ITMVisualisationEngine_CPU<TVoxel, ITMVoxelBlockHash>::ForwardRender(const ITMScene<TVoxel, ITMVoxelBlockHash> *scene,
                                                                     const ITMView *view,
                                                                     ITMTrackingState *trackingState,
                                                                     ITMRenderState *renderState) const {
    ForwardRender_common(scene, view, trackingState, renderState);
}

template<class TVoxel, class TIndex>
static int RenderPointCloud(Vector4f *locations, Vector4f *colours, const Vector4f *ptsRay,
                            const TVoxel *voxelData, const typename TIndex::IndexData *voxelIndex, bool skipPoints,
                            float voxelSize,
                            Vector2i imgSize, Vector3f lightSource) {
    int noTotalPoints = 0;

    for (int y = 0, locId = 0; y < imgSize.y; y++)
        for (int x = 0; x < imgSize.x; x++, locId++) {
            Vector3f outNormal;
            float angle;
            Vector4f pointRay = ptsRay[locId];
            Vector3f point = pointRay.toVector3();
            bool foundPoint = pointRay.w > 0;

            computeNormalAndAngle<TVoxel, TIndex>(foundPoint, point, voxelData, voxelIndex, lightSource, outNormal,
                                                  angle);

            if (skipPoints && ((x % 2 == 0) || (y % 2 == 0))) foundPoint = false;

            if (foundPoint) {
                Vector4f tmp;
                tmp = VoxelColorReader<TVoxel::hasColorInformation, TVoxel, TIndex>::interpolate(voxelData, voxelIndex,
                                                                                                 point);
                if (tmp.w > 0.0f) {
                    tmp.x /= tmp.w;
                    tmp.y /= tmp.w;
                    tmp.z /= tmp.w;
                    tmp.w = 1.0f;
                }
                colours[noTotalPoints] = tmp;

                Vector4f pt_ray_out;
                pt_ray_out.x = point.x * voxelSize;
                pt_ray_out.y = point.y * voxelSize;
                pt_ray_out.z = point.z * voxelSize;
                pt_ray_out.w = 1.0f;
                locations[noTotalPoints] = pt_ray_out;

                noTotalPoints++;
            }
        }

    return noTotalPoints;
}
