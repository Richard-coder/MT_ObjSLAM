//
// Created by khuang on 10/22/18.
//

//
// Created by khuang on 6/11/18.
//
#include <iostream>
#include <External/InfiniTAM/InfiniTAM/ITMLib/Objects/RenderStates/ITMRenderStateFactory.h>
#include <src/ObjSLAM/ObjSLAMVoxelSceneParams.h>
#include "LPD_Dataset_Reader.h"
#include "ObjectInstanceScene.h"
#include "../../External/InfiniTAM/InfiniTAM/ITMLib/ITMLibDefines.h"
#include "ObjSLAMMappingEngine.h"
#include "ObjSLAMTrackingEngine.h"
#include "ObjectView.h"
#include "TeddyReader.h"
#include "TUM_Reader.h"
#include "ObjSLAMMainEngine.h"
//#include "ObjSLAMUI.h"
#include <memory>

//#include <g2o/core/base_vertex.h>
//#include <g2o/core/base_unary_edge.h>
//#include <g2o/core/block_solver.h>
//#include <g2o/core/optimization_algorithm_levenberg.h>
//#include <g2o/core/optimization_algorithm_gauss_newton.h>
//#include <g2o/core/optimization_algorithm_dogleg.h>
//#include <g2o/solvers/dense/linear_solver_dense.h>
//
//#include <g2o/types/slam3d/types_slam3d.h>
//#include <g2o/types/slam3d_addons/types_slam3d_addons.h>

#include <ctime>
#include <sys/time.h>



using namespace std;



//static global variables
bool saveSTL = false;
int STL_Frequency = 1;
int reader_SkipFrames = 0;
int numthreads = 4;
int totFrames;
bool sceneIsBackground = false;
bool do_BG_cleanup = true;
bool do_Obj_cleanup = true;
bool do_Obj_tracking = true;

//TODO debug this non-GUI version

/**
 * @brief ain function of ObjSLAM without GUI, currently still under development
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {




    //TODO Debug output
    cout << "**Hello SLAM World!" << endl;

    //Path of the depth image file
    string path = argv[1];
    Vector2i imgSize(640, 480);

    totFrames =atoi( argv[2]);
    std::cout<<"totFrames = "<<totFrames<<std::endl;

    if(argc>3 && atoi(argv[3])>0){
        reader_SkipFrames = atoi(argv[3]);
    }

    if(argc>4){
        saveSTL = (atoi(argv[4])!=0);
        if(saveSTL){
            STL_Frequency = atoi(argv[5]);
        }
        do_BG_cleanup = (atoi(argv[6])!=0);
        do_Obj_cleanup = (atoi(argv[7])!=0);
        do_Obj_tracking = (atoi(argv[8])!=0);
    }
    std::cout<<"BG Cleanup = "<<do_BG_cleanup<<std::endl;
    std::cout<<"Obj Cleanup = "<<do_Obj_cleanup<<std::endl;
    std::cout<<"Obj Tracking = "<<do_Obj_tracking<<std::endl;


//    ObjSLAM::ObjSLAMUI* ui =new ObjSLAM::ObjSLAMUI(imgSize);


    std::shared_ptr<ITMLib::ITMLibSettings> internalSettings = std::make_shared<ITMLib::ITMLibSettings>();
    internalSettings->sceneParams =ITMLib::ITMSceneParams(0.1f, 100, 0.01f, 0.1, 10.0, true);// ITMLib::ITMSceneParams(0.08f, 100, 0.008f, 0.1, 10.0, true);

    std::shared_ptr<ITMLib::ITMLibSettings> internalSettings_obj = std::make_shared<ITMLib::ITMLibSettings>();
    internalSettings_obj->sceneParams = ITMLib::ITMSceneParams(0.1f, 100, 0.01f, 0.1, 10.0, true);
    //(0.1, 10, 0.025, 0.1, 4.0, false); //(0.02f, 100, 0.002f, 0.2f, 3.0f, false);  //(0.2, 4, 0.05, 0.1, 4.0, false);
    //0.1f, 5, 0.01f, 0.1, 6.0, false  0.04f, 100, 0.005f, 0.2f, 5.0f, false
//  float mu, int maxW, float voxelSize, float viewFrustum_min, float viewFrustum_max, bool stopIntegratingAtMaxW

    internalSettings->deviceType = ITMLib::ITMLibSettings::DEVICE_CPU;


//  LPD_Dataset_Reader reader(path, imgSize);
    DatasetReader* reader= nullptr;
    if(path.find("Teddy")!=std::string::npos){
        cout<<"Teddy\n";
        reader = new TeddyReader(path,imgSize);
    }else if(path.find("RealisticRenderingDataset")!=std::string::npos){
        cout<<"RealisticRenderingDataset\n";
        reader = new LPD_Dataset_Reader(path,imgSize);
    }else /*if(path.find("rgbd")!=std::string::npos || path.find("traj")!=std::string::npos)*/{
        cout<<"TUM RGBD\n";
        reader = new TUM_Reader(path,imgSize);
    }


    sceneIsBackground=true;
    ObjSLAMMainEngine* mainEngine =new ObjSLAMMainEngine(internalSettings, internalSettings_obj,std::shared_ptr<DatasetReader>(reader));


    int imgNum = 0;
    if (imgNum <= totFrames) {
        sceneIsBackground = true;
        imgNum = mainEngine->ReadNext();
        mainEngine->TrackFrame();
        mainEngine->UpdateMappingEngine();
        mainEngine->MapFrame();
        for(int i = 0; i<reader_SkipFrames;++i){
            imgNum = mainEngine->ReadNext();
            mainEngine->TrackFrame();
        }


        if(imgNum%10==0) mainEngine->OutputPics();
    }


    delete reader;

    return 0;
}
