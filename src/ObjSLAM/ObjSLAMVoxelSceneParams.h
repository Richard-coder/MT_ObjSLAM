//
// Created by khuang on 9/3/18.
//

#pragma once

extern bool sceneIsBackground;
//#pragma omp threadprivate(sceneIsBackground) //only active when using parallel track and map
extern bool saveSTL;
extern int STL_Frequency;
extern int reader_SkipFrames;
extern int numthreads;
extern int totFrames;
extern bool do_BG_cleanup;
extern bool do_Obj_cleanup;
//class ObjSLAMVoxelSceneParams{
//
// public:
//  static const unsigned int sdf_local_block_num = 0x10000; //0x10000
//  static const unsigned int sdf_bucket_num = sdf_local_block_num*4;
//  static const unsigned int sdf_hash_mask = sdf_bucket_num-1;
//  static const unsigned int sdf_excess_list_size=0x8000;
//
//  static const unsigned int background_sdf_local_block_num = 0x20000;
//  static const unsigned int background_sdf_bucket_num = background_sdf_local_block_num*4;
//  static const unsigned int background_sdf_hash_mask = background_sdf_bucket_num-1;
//
//
//
//};

 //MT_OBJSLAM_OBJSLAMVOXELSCENEPARAMS_H
