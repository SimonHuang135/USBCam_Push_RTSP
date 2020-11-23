#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// True：The client that is started later always starts playing from the position that the first client has already played
// False：Every client plays video files from the beginning
Boolean reuseFirstSource = False;

// This function prints relevant information
static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName);

int main(int argc, char** argv) {

    // 1. Create a task scheduler and initialize the use environment
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();

    // 2. Create an interactive environment
    env = BasicUsageEnvironment::createNew(*scheduler);
    // The following is the code for permission control. After setting, the client without permission cannot connect
    UserAuthenticationDatabase* authDB = NULL;

    authDB = new UserAuthenticationDatabase;
    authDB->addUserRecord("admin", "Suntek123"); // replace these with real strings  


    // 3. Create an RTSP server and start monitoring the connection of the module client. 
    // Note that if the port here is not the default port 554, you must specify the port number when accessing the URL:
    RTSPServer* rtspServer = RTSPServer::createNew(*env, 554, authDB);
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    char const* descriptionString = "Session streamed by \"h264LiveMediaServer\"";

    // A H.264 video elementary stream:
    {
        char const* streamName = NULL; // Stream name, media name

    // 4. Create a media session 
        // When the client orders, the stream name streamName must be input to tell the RTSP server which stream is ordering.
        // Media session manages session-related information such as session description, session duration, stream name, etc.
        // When running h264LiveMediaServer through the IDE, live555 pushes the video or audio in the project working directory. The working directory is the directory at the same level as *.vcxproj,
        //第二个参数:媒体名、三:媒体信息、四:媒体描述
        ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName, streamName, descriptionString);

    //5. Add 264 sub-session. The file name here is the name of the file that is actually opened.
        sms->addSubsession(H264LiveVideoServerMediaSubsession::createNew(*env, reuseFirstSource));
    //6. Add session for rtspserver, but streamName = NULL
        rtspServer->addServerMediaSession(sms);
        announceStream(rtspServer, sms, streamName);
    }

    // Trying to create an HTTP server for the RTSP-over-HTTP channel.
    if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
        *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
    }
    else {
        *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
    }

   // Entering the event loop, the read event of the socket and the delayed sending of the media file are all completed in this loop.
    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName) {
    char* url = rtspServer->rtspURL(sms);
    UsageEnvironment& env = rtspServer->envir();
    env << "\nstreamName: " << streamName << "\n";
    env << "Play using the URL \"" << url << "\"\n";
    delete[] url;
}
