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
    // :  Ҫ��nCamID���캯���Ĳ�����ʹ��һ��CCameraDS����ֻ����һ������ͷ
    CCameraDS();
    virtual ~CCameraDS();

    //������ͷ��nCamIDָ�����ĸ�����ͷ��ȡֵ����Ϊ0,1,2,...
    //bDisplayPropertiesָʾ�Ƿ��Զ���������ͷ����ҳ
    //nWidth��nHeight���õ�����ͷ�Ŀ�͸ߣ��������ͷ��֧�����趨�Ŀ�Ⱥ͸߶ȣ��򷵻�false
    bool OpenCamera(int nCamID, int nWidth = 1280, int nHeight = 720); // 352 288

    //�ر�����ͷ�������������Զ������������
    void CloseCamera();

    //��������ͷ����Ŀ
    //���Բ��ô���CCameraDSʵ��������int c=CCameraDS::CameraCount();�õ������
    int CameraCount();

    //��������ͷ�ı�ŷ�������ͷ������
    //nCamID: ����ͷ���
    //sName: ���ڴ������ͷ���ֵ�����
    //nBufferSize: sName�Ĵ�С
    //���Բ��ô���CCameraDSʵ��������CCameraDS::CameraName();�õ������
    int CameraName(int nCamID, char* sName, int nBufferSize);

    //����ͼ����
    int GetWidth() { return m_nWidth; }

    //����ͼ��߶�
    int GetHeight() { return m_nHeight; }

    //ץȡһ֡�����ص�IplImage�����ֶ��ͷţ�
    //����ͼ�����ݵ�ΪRGBģʽ��Top-down(��һ���ֽ�Ϊ���Ͻ�����)����IplImage::origin=0(IPL_ORIGIN_TL)
    unsigned  char* QueryFrame();
};

#endif 
