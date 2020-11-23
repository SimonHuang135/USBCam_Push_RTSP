#include "ICameraCaptuer.h"
#include "CameraDS.h"

ICameraCaptuer* CamCaptuerMgr::GetCamCaptuer()
{
    return new CCameraDS;
}

// not used yet
//void CamCaptuerMgr::Destory(ICameraCaptuer* pCamCaptuer) 
//{ 
//    delete  pCamCaptuer; 
//    pCamCaptuer = 0;
//}


