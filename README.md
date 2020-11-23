# USBCam_Push_RTSP

Use Direct Show lib to get camera frame as YUV continuously, then encode to H264 flow by using x264 lib, then decode to Nal unit and push RTSP by Live555 lib.

ps: code of CamH264VideoStreamFramer.cpp in LIVE555 can be appended to RTSPServer.cpp of liveMedia in Live555 lib
