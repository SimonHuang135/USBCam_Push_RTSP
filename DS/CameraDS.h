#ifndef CCAMERA_H
#define CCAMERA_H


#define WIN32_LEAN_AND_MEAN

#include "DllManager.h"
#include "ICameraCaptuer.h"

#include <atlbase.h>
#include "DirectShow/Include/qedit.h"
#include "DirectShow/Include/dshow.h"
#include <windows.h>

#define MYFREEMEDIATYPE(mt)    {if ((mt).cbFormat != 0)        \
                    {CoTaskMemFree((PVOID)(mt).pbFormat);    \
                    (mt).cbFormat = 0;                        \
                    (mt).pbFormat = NULL;                    \
                }                                            \
                if ((mt).pUnk != NULL)                        \
                {                                            \
                    (mt).pUnk->Release();                    \
                    (mt).pUnk = NULL;                        \
                }}                                    

// declare parameters, COM, functions
class CCameraDS : public ICameraCaptuer
{
private:
    unsigned char* m_pImgData; // RGB
    unsigned char* m_pYUVData; // YUV
    bool m_bConnected;
    int m_nWidth;
    int m_nHeight;
    bool m_bLock;
    bool m_bChanged;
    long m_nBufferSize;

    CComPtr<IGraphBuilder> m_pGraph;
    CComPtr<IBaseFilter> m_pDeviceFilter;
    CComPtr<IMediaControl> m_pMediaControl;
    CComPtr<IBaseFilter> m_pSampleGrabberFilter;
    CComPtr<ISampleGrabber> m_pSampleGrabber;
    CComPtr<IPin> m_pGrabberInput;
    CComPtr<IPin> m_pGrabberOutput;
    CComPtr<IPin> m_pCameraOutput;
    CComPtr<IMediaEvent> m_pMediaEvent;
    CComPtr<IBaseFilter> m_pNullFilter;
    CComPtr<IPin> m_pNullInputPin;

    static int m_iRefCnt;

private:
    bool BindFilter(int nCamIDX, IBaseFilter** pFilter);

public:
    // :  要将nCamID构造函数的参数，使得一个CCameraDS对象只管理一个摄像头
    CCameraDS();
    virtual ~CCameraDS();

    //打开摄像头，nCamID指定打开哪个摄像头，取值可以为0,1,2,...
    //bDisplayProperties指示是否自动弹出摄像头属性页
    //nWidth和nHeight设置的摄像头的宽和高，如果摄像头不支持所设定的宽度和高度，则返回false
    bool OpenCamera(int nCamID, int nWidth = 1280, int nHeight = 720); // 352 288

    //关闭摄像头，析构函数会自动调用这个函数
    void CloseCamera();

    //返回摄像头的数目
    //可以不用创建CCameraDS实例，采用int c=CCameraDS::CameraCount();得到结果。
    int CameraCount();

    //根据摄像头的编号返回摄像头的名字
    //nCamID: 摄像头编号
    //sName: 用于存放摄像头名字的数组
    //nBufferSize: sName的大小
    //可以不用创建CCameraDS实例，采用CCameraDS::CameraName();得到结果。
    int CameraName(int nCamID, char* sName, int nBufferSize);

    //返回图像宽度
    int GetWidth() { return m_nWidth; }

    //返回图像高度
    int GetHeight() { return m_nHeight; }

    //抓取一帧，返回的IplImage不可手动释放！
    //返回图像数据的为RGB模式的Top-down(第一个字节为左上角像素)，即IplImage::origin=0(IPL_ORIGIN_TL)
    unsigned  char* QueryFrame();
};

#endif 
