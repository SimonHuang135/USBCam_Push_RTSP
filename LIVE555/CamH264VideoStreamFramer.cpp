
// CamH264VideoStreamFramer *********************************************************
//���ϻ�ȡ��������ͷ��Ƶ������x264���룬��Ϊ��׼H264Դ
#include "ICameraCaptuer.h"		//Camera YUV Data
#include "H264EndWrapper.h"		//H264 Encode
#include "H264DecWrapper.h"		//H264 Decode
#include "FramedSource.hh"
#include "H264VideoStreamFramer.hh"
#include "GroupsockHelper.hh"
//#include "opencv2/opencv.hpp"

ICameraCaptuer* CamH264VideoStreamFramer::m_pCamera = NULL;

#undef _TEST_OUTPUT_264  //�������H264�ļ�
#undef _TEST_OUTPUT_YUV  //�������YUV�ļ�
#undef _TEST_DISPLAY     //������ʾ

#if defined(_TEST_DISPLAY) || defined(_TEST_OUTPUT_YUV)
#define _TEST_DECODE
#endif

#if defined(_TEST_OUTPUT_264)
FILE* f264;
#endif

#if defined(_TEST_OUTPUT_YUV)
FILE* fyuv;
#endif

#if defined(_TEST_DISPLAY)
#include "cv.h"
#include "highgui.h"
#include "convert.h"
IplImage* g_IplImage = NULL;
#endif

CamH264VideoStreamFramer::CamH264VideoStreamFramer(UsageEnvironment& env,
    FramedSource* inputSource, H264EncWrapper* pH264Enc, H264DecWrapper* pH264Dec)
    :H264VideoStreamFramer(env, inputSource, False, False),
    m_pNalArray(NULL), m_iCurNalNum(0), m_iCurNal(0), m_iCurFrame(0),
    m_pH264Enc(pH264Enc), m_pH264Dec(pH264Dec)
{ // stream fps (encoded fps)
    fFrameRate = 25.0; // We assume a frame rate of 25 fps, unless we learn otherwise (from parsing a Sequence Parameter Set NAL unit)

}

CamH264VideoStreamFramer::~CamH264VideoStreamFramer()
{
    m_pCamera->CloseCamera();

    m_pH264Enc->Destroy();
    delete m_pH264Enc;
    m_pH264Enc = NULL;

    m_pH264Dec->Destroy();
    delete m_pH264Dec;
    m_pH264Dec = NULL;

#if defined(_TEST_OUTPUT_264)
    fclose(f264);
#endif

#if defined(_TEST_OUTPUT_YUV)
    fclose(fyuv);
#endif

#if defined(_TEST_DISPLAY)
    cvDestroyWindow("TestRTSPServer");
    cvReleaseImage(&g_IplImage);
#endif
}

const int VIDEO_WIDTH = 1280, VIDEO_HEIGHT = 720;

CamH264VideoStreamFramer* CamH264VideoStreamFramer::createNew(
    UsageEnvironment& env,
    FramedSource* inputSource)
{
#if defined(_TEST_OUTPUT_264)
    f264 = fopen("C:/Users/Administrator/Desktop/TestRTSPServer.264", "wb");
#endif

#if defined(_TEST_OUTPUT_YUV)
    fyuv = fopen("TestRTSPServer.yuv", "wb");
#endif

    // ��ȡCamera����
    if (NULL == m_pCamera && NULL == (m_pCamera = CamCaptuerMgr::GetCamCaptuer()))
    {
        env.setResultMsg("Create camera instance error");
        return NULL;
    }

    // ��Camera
    if (!m_pCamera->OpenCamera(0, VIDEO_WIDTH, VIDEO_HEIGHT))
    {
        env.setResultMsg("Can not open camera");
        return NULL;
    }

    // ��ʼ��H264 Encode
    H264EncWrapper* pH264Enc = new H264EncWrapper;
    if (pH264Enc->Initialize(VIDEO_WIDTH, VIDEO_HEIGHT, 480, 25) < 0)
    {
        env.setResultMsg("Initialize x264 encoder error");
        return NULL;
    }

    // ��ʼ��H264 Decode��ֻ�����ڱ�����ʾ
    H264DecWrapper* pH264Dec = new H264DecWrapper;
    if (pH264Dec->Initialize() < 0)
    {
        env.setResultMsg("Initialize H.264 decoder error");
        return NULL;
    }

#if defined(_TEST_DISPLAY)
    // ��ҪOpenCV��֧�֣����ڲ�ͬ�汾OpenCVд����һ��
    RGBYUVConvert::InitConvertTable();
    //OpenCV
    cvNamedWindow("TestRTSPServer");
    g_IplImage = cvCreateImage(cvSize(VIDEO_WIDTH, VIDEO_HEIGHT), IPL_DEPTH_8U, 3);
    if (NULL == g_IplImage)
    {
        printf(ERR, "Initialize OpenCV error.");
        return NULL;
    }
#endif

    CamH264VideoStreamFramer* fr;
    fr = new CamH264VideoStreamFramer(env, inputSource, pH264Enc, pH264Dec);
    return fr;
}

Boolean CamH264VideoStreamFramer::currentNALUnitEndsAccessUnit()
{
    if (m_iCurNal >= m_iCurNalNum)
    {
        m_iCurFrame++;
        return True;
    }
    else
    {
        return False;
    }
}

// live555ÿһ�ε���doGetNextFrame()ʱ���Ȳ鿴�Ƿ�����һ����Ƶ֡��NAL Unitû��
// ������ɣ����û�У��������ͣ����û�У��ȴ�����ͷ��ȡһ��YUV������֡��x264
// ����YUV��һ��NAL Unit���飬�ٿ�ʼ����NAL���飬һ�η���һ��NAL Unit
void CamH264VideoStreamFramer::doGetNextFrame()
{
    TNAL* pNal = NULL;
    unsigned char* pOrgImg;
    // �����Ƿ���δ�������NAL Unit
    if ((m_pNalArray != NULL) && (m_iCurNal < m_iCurNalNum))
    {
        pNal = &m_pNalArray[m_iCurNal];
    }
    else
    {
        // ���NAL Unit����
        m_pH264Enc->CleanNAL(m_pNalArray, m_iCurNalNum);
        m_iCurNal = 0;

        // ��ȡ������Ƶ֡,YUV
        pOrgImg = m_pCamera->QueryFrame();
        gettimeofday(&fPresentationTime, NULL);

        // H264 Encode
        m_pH264Enc->Encode(pOrgImg, m_pNalArray, m_iCurNalNum);
        pNal = &m_pNalArray[m_iCurNal];
        //printf("Frame[%d], Nal[%d:%d]: size = %d\n", m_iCurFrame, m_iCurNalNum, m_iCurNal, pNal->size);
        //printf(INF, "Frame[%d], Nal[%d:%d]: size = %d", m_iCurFrame, m_iCurNalNum, m_iCurNal, pNal->size);
    }
    m_iCurNal++;

#if defined(_TEST_DECODE)
    static const int YUV_IMG_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2;
    static unsigned char yuv[YUV_IMG_SIZE] = { 0 };
    static int iDecodedFrame = 0;
    int iYuvSize = 0;
    bool bGetFrame = true;
    int iDecodedLen = 0;
    unsigned char* nal_ptr = pNal->data;
    int len = pNal->size;

    while (len > 0)
    {
        iDecodedLen = m_pH264Dec->Decode(pNal->data, pNal->size, yuv, iYuvSize, bGetFrame);
        if (bGetFrame)
        {
#if defined(_TEST_DISPLAY)
            RGBYUVConvert::ConvertYUV2RGB(yuv, (unsigned char*)g_IplImage->imageData, VIDEO_WIDTH, VIDEO_HEIGHT);
            cvFlip(g_IplImage, NULL, 1);
            cvShowImage("TestRTSPServer", g_IplImage);
            cvWaitKey(5);
#endif       

#if defined(_TEST_OUTPUT_YUV)
            fwrite(yuv, 1, iYuvSize, fyuv);
#endif       

            printf(INF, "Success to decode one frame[%d]", iDecodedFrame);
            iDecodedFrame++;
        }
        len -= iDecodedLen;
        nal_ptr += iDecodedLen;
    }
#endif

#if defined(_TEST_OUTPUT_264)
    fwrite(pNal->data, 1, pNal->size, f264);
#endif

    unsigned char* realData = pNal->data;
    unsigned int realLen = pNal->size;

    if (realLen < fMaxSize)
    {
        memcpy(fTo, realData, realLen);
    }
    else
    {
        memcpy(fTo, realData, fMaxSize);
        fNumTruncatedBytes = realLen - fMaxSize;
    }

    fDurationInMicroseconds = 20000; // 200000
    //printf(INF, "fPresentationTime = %d.%d", fPresentationTime.tv_sec, fPresentationTime.tv_usec);
    //printf("fPresentationTime = %d.%d\n", fPresentationTime.tv_sec, fPresentationTime.tv_usec);
    fFrameSize = realLen;
    afterGetting(this);
}

// H264LiveVideoServerMediaSubsession *********************************************************
H264LiveVideoServerMediaSubsession*
H264LiveVideoServerMediaSubsession::createNew(UsageEnvironment& env,
    Boolean reuseFirstSource) {
    return new H264LiveVideoServerMediaSubsession(env, reuseFirstSource);
}

H264LiveVideoServerMediaSubsession
::H264LiveVideoServerMediaSubsession(UsageEnvironment& env,
    Boolean reuseFirstSource)
    : OnDemandServerMediaSubsession(env, reuseFirstSource) {
}

H264LiveVideoServerMediaSubsession::~H264LiveVideoServerMediaSubsession() {
}

FramedSource* H264LiveVideoServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
    estBitrate = 480;
    // Create the video source:
    return CamH264VideoStreamFramer::createNew(envir(), NULL);
}

RTPSink* H264LiveVideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* /*inputSource*/) {
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, 480);
}

char const* H264LiveVideoServerMediaSubsession::sdpLines()
{
    return fSDPLines =
        "m=video 0 RTP/AVP 96\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "b=AS:480\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 packetization-mode=1;profile-level-id=000000;sprop-parameter-sets=H264\r\n"
        "a=control:track1\r\n";
}
