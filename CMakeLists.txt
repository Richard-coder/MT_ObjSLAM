cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)

project(MT_ObjSLAM)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set (PROJECT_DIR "/local/MT/MT_ObjSLAM")
set (SOURCE_DIR "src/ObjSLAM" CACHE STRING "some info" FORCE)
set (HEADER_DIR "src/ObjSLAM" CACHE STRING "some info" FORCE)

#set(BUILD_PANGOLIN_EIGEN ON CACHE BOOL "")
find_package(Pangolin REQUIRED)

find_package( Boost REQUIRED)
find_package( OpenMP REQUIRED)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


#if(WITH_GPROF)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
#endif()

if(WITH_CUDA)
    include_directories(/usr/local/cuda/include)
endif()

include_directories($(HEADER_DIR)

        /usr/include
        External/InfiniTAM/InfiniTAM/InputSource
        External/InfiniTAM/InfiniTAM/ITMLib
        ${Pangolin_INCLUDE_DIRS}
        )



add_subdirectory(External/InfiniTAM/InfiniTAM EXCLUDE_FROM_ALL TRUE)
INCLUDE(External/InfiniTAM/InfiniTAM/cmake/UseCUDA.cmake)

#add_subdirectory(External/g2o/g2o)


set(TrackerMapper_Sources
        src/ObjSLAM/ObjSLAM_TrackerMapper_Test.cpp

        src/ObjSLAM/ObjectInstanceScene.tpp src/ObjSLAM/ObjectInstanceScene.h
        src/ObjSLAM/ObjSLAMDataTypes.h

        src/ObjSLAM/LPD_Dataset_Reader.h
        src/ObjSLAM/ObjCameraPose.cpp src/ObjSLAM/ObjCameraPose.h
        src/ObjSLAM/LPD_RAW_Pose.cpp src/ObjSLAM/LPD_RAW_Pose.h
        src/ObjSLAM/ObjSLAMMappingEngine.tpp src/ObjSLAM/ObjSLAMMappingEngine.h
 #       src/DatasetIO/IOUtils.cpp src/DatasetIO/IOUtils.h
        src/ObjSLAM/ObjSLAMCamera.cpp src/ObjSLAM/ObjSLAMCamera.h
        src/ObjSLAM/ObjectInstance.tpp src/ObjSLAM/ObjectInstance.h
        src/ObjSLAM/ObjectClassLabel_Group.tpp src/ObjSLAM/ObjectClassLabel_Group.h
        src/ObjSLAM/ObjectView.tpp src/ObjSLAM/ObjectView.h
        src/ObjSLAM/ObjSLAMTrackingEngine.cpp src/ObjSLAM/ObjSLAMTrackingEngine.h
        src/ObjSLAM/DatasetReader.h src/ObjSLAM/DatasetReader.cpp src/ObjSLAM/TeddyReader.cpp
        src/ObjSLAM/TeddyReader.h src/ObjSLAM/TUM_Reader.cpp src/ObjSLAM/TUM_Reader.h
        src/ObjSLAM/ObjSLAMVoxelSceneParams.h
        src/ObjSLAM/ObjSLAMUI.cpp src/ObjSLAM/ObjSLAMUI.h
        src/ObjSLAM/ObjSLAMMainEngine.cpp src/ObjSLAM/ObjSLAMMainEngine.h
        )



set(TrackerMapperCLI_Sources
        src/ObjSLAM/ObjSLAM_TrackerMapper_CLI.cpp


        src/ObjSLAM/ObjectInstanceScene.tpp src/ObjSLAM/ObjectInstanceScene.h
        src/ObjSLAM/ObjSLAMDataTypes.h

        src/ObjSLAM/LPD_Dataset_Reader.h
        src/ObjSLAM/ObjCameraPose.cpp src/ObjSLAM/ObjCameraPose.h
        src/ObjSLAM/LPD_RAW_Pose.cpp src/ObjSLAM/LPD_RAW_Pose.h
        src/ObjSLAM/ObjSLAMMappingEngine.tpp src/ObjSLAM/ObjSLAMMappingEngine.h
        src/DatasetIO/IOUtils.cpp src/DatasetIO/IOUtils.h
        src/ObjSLAM/ObjSLAMCamera.cpp src/ObjSLAM/ObjSLAMCamera.h
        src/ObjSLAM/ObjectInstance.tpp src/ObjSLAM/ObjectInstance.h
        src/ObjSLAM/ObjectClassLabel_Group.tpp src/ObjSLAM/ObjectClassLabel_Group.h
        src/ObjSLAM/ObjectView.tpp src/ObjSLAM/ObjectView.h
        src/ObjSLAM/ObjSLAMTrackingEngine.cpp src/ObjSLAM/ObjSLAMTrackingEngine.h
        src/ObjSLAM/DatasetReader.h src/ObjSLAM/DatasetReader.cpp src/ObjSLAM/TeddyReader.cpp
        src/ObjSLAM/TeddyReader.h src/ObjSLAM/TUM_Reader.cpp src/ObjSLAM/TUM_Reader.h
        src/ObjSLAM/ObjSLAMVoxelSceneParams.h
        src/ObjSLAM/ObjSLAMMainEngine.cpp src/ObjSLAM/ObjSLAMMainEngine.h
        )

#add_executable(ObjSLAM ${ObjSLAM_Sources})
#TARGET_LINK_LIBRARIES(ObjSLAM cudart InputSource ITMLib MiniSlamGraphLib ORUtils FernRelocLib Threads::Threads png)



add_executable(TrackerMapper ${TrackerMapper_Sources}  /usr/include)
#add_executable(TrackerMapper_CLI ${TrackerMapperCLI_Sources}  /usr/include)

TARGET_LINK_LIBRARIES(TrackerMapper  boost_system  boost_filesystem  InputSource ITMLib MiniSlamGraphLib ORUtils FernRelocLib Threads::Threads ${OpenMP_CXX_FLAGS}  png ${Pangolin_LIBRARIES})
#TARGET_LINK_LIBRARIES(TrackerMapper_CLI  boost_system  boost_filesystem  InputSource ITMLib MiniSlamGraphLib ORUtils FernRelocLib Threads::Threads ${OpenMP_CXX_FLAGS}  png ${Pangolin_LIBRARIES})
#TARGET_LINK_LIBRARIES(Mapper  boost_system  boost_filesystem  cudart InputSource ITMLib MiniSlamGraphLib ORUtils FernRelocLib Threads::Threads ${OpenMP_CXX_FLAGS}  png)
#looks like it needs link cudart
