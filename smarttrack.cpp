/*
 * smarttrack.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <iostream>
#include <complex>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <boost/circular_buffer.hpp>
#include <DTrackSDK.hpp>

// Include project headers
#include "DeviceBase.h"
#include "smarttrack.h"


static const int debug = 0;

// Configuration Flags
#define ST_DEBUG
#define NO_OGL
#define STORE_DATA

SmartTrack::SmartTrack(std::string &name) : DeviceBase(name) {
    _dt = nullptr;
    _udprx_thread = nullptr;
    _run = false;
}

void* SmartTrack::udprx_func(DTrackSDK* _dt)
{
    if (debug)
        printf("Executing %s::Thread \n", __FUNCTION__); fflush(stdout);

    while (_run) {
#ifndef ST_DEBUG
        // Poll UDP port for new data
        if (_dt->receive() && (debug == 2))
            printf("Received something\n"); fflush(stdout);


        // Don't start copying new data until a body is found
        if (_dt->getNumBody() == 0) {
            continue;
        } else if (_dt->getNumBody() != 1) {
            printf("Found %d bodies! tracking _body id %d only\n",
                _dt->getNumBody(), _dt->getBody(0)->id);
            fflush(stdout);
        }

        static bool print = true;       // print once flag
        static int ts = 0;              // timestamp when CalTimeDelta got called

        // Quality of -1 means we lost track of the body. Out of track body has rot/loc of "0"
        if (_dt->getBody(0)->quality >= 0) {

            // Mutex lock
            _rxdata.lock();

            // Save ts to .t_input and delta (to old ts) to .d_rx_loop
            ts = TimeUtils::CalcTimeDelta(ts, &frame_new.t_input, &frame_new.d_rx_loop);

            frame_new.num = _dt->getFrameCounter();                      // Frame number
            frame_new.t_capture  = (int)(_dt->getTimeStamp() * 1000.0f); // IR-flash fire time [ms]
            _body = *_dt->getBody(0);                                     // get tracking data

#ifdef STORE_DATA
            FileUtils::ExportRotData(&_body.rot[0], 9);
            FileUtils::ExportLocData(&_body.loc[0], 3);
#endif /*STORE_DATA */

            // Mutex unlock
            _rxdata.unlock();

            print = true;                           // If we lose track msg will be printed once
        } else if (debug && print) {
            printf("Lost track of _body id %d\n", _dt->getBody(0)->id); fflush(stdout);
            print = false;                      // Silence until next lose of track
        }
#else
        boost::this_thread::sleep(boost::posix_time::milliseconds(16)); // Simulate polling delay
        _rxdata.lock();
//        TimeUtils::GetTimeSinceMidnight(&frame_new.t_input);    // The time this frame arrived
//        frame_new.num = _dt->getMessageFrameNr();                // The frame number
//        frame_new.t_capture  = (int)_dt->getTimeStamp();  // The time the IR flash fired
        // _body = 0
        // Mutex unlock
        _rxdata.unlock();
#endif /* ST_DEBUG */
    }
    if (debug)
        printf("%s::Thread terminated\n", __FUNCTION__); fflush(stdout);
    return 0;
}

int SmartTrack::Init(Tcl_Interp* interp, Tcl_Obj* tcl_ret, char* pSrcIpAddr,
                     char* pDestIpAddr, char* pChNum)
{
    printf("Executing %s \n", __FUNCTION__); fflush(stdout);

    int err_code = 0;
    _run = false;        // ends the udp_rx thread loop

    // Extract the src/dest IP address and port from input string
    short divPos;
    std::string ipAddr = pSrcIpAddr;
    divPos = ipAddr.find_first_of(':');
    *(pSrcIpAddr + divPos) = '\0';                  // 'Break' the string at pos of ':'
    char* pSrcPort = pSrcIpAddr + divPos + 1;

    ipAddr = pDestIpAddr;
    divPos = ipAddr.find_first_of(':');
    *(pDestIpAddr + divPos) = '\0';
    char* pDestPort = pDestIpAddr + divPos + 1;

    // Store the com/port configurations
    st = { pSrcIpAddr,
           pDestIpAddr,
           pChNum,
           atoi(pSrcPort),
           atoi(pDestPort),
    };

    if (debug) {
        printf("SmartTrack IP address '%s', port '%d'.\n", st.pSrcIpAddr, st.srcPort);
        printf("Data rx IP address '%s', port '%d'.\n", st.pDestIpAddr, st.destPort);
        fflush(stdout);
    }
#ifndef ST_DEBUG
    // Init library
    if (st.srcPort != 50105)
        printf("WARNING: DTrack2 port must be 50105\n");

    _dt = new DTrackSDK(st.pSrcIpAddr,           /* IP address of ARTtrack */
                       st.srcPort,              /* Port number of ARTtrackÂŽ*/
                       st.destPort,             /* Data rx port number to rx data from ART-Track */
                       DTrackSDK::SYS_DTRACK_2, /* ARTtrack type */
                       32768,                   /* UDP rx buffer size (in bytes) */
                       20000,                   /* Timout to rx data (in us) */
                       5000000);                /* Timeout for std ARTtrack replies (in us) */

    if (!_dt->isLocalDataPortValid()) {
        if (debug)
        printf("DTrack init error\n"); fflush(stdout);
        delete _dt;
        return -2;
    }

    // Forcestop any running measurements
    _dt->stopMeasurement();

    // Configure SmartTrack
    // Set channel, protocol, destination IP (localhost IP address) and the portNum
    std::stringstream dtrack2Cmd;
    dtrack2Cmd << "dtrack2 set output net " << st.pChNum << " udp " << st.pDestIpAddr
               << " " << st.destPort;
    if (debug)
        printf("Sending command string: %s\n", dtrack2Cmd.str()); fflush(stdout);

    err_code = _dt->sendDTrack2Command(dtrack2Cmd.str(), NULL);

    // Clear the sstream
    dtrack2Cmd.str("");
    dtrack2Cmd.clear();

    if (err_code < 0) {
        printf("Error setting destination IP address for SmartTrack data.\n"); fflush(stdout);
        delete _dt;
        return -2;
    }

    // Set the data to be transfer on channel (chNum) and activate the channel
    dtrack2Cmd << "dtrack2 set output active " << st.pChNum << " " << "all" << " " << "yes";
    if (debug)
        printf("Sending command string: %s\n", dtrack2Cmd.str()); fflush(stdout);

    err_code = _dt->sendDTrack2Command(dtrack2Cmd.str(), NULL);

    // Clear the sstream
    dtrack2Cmd.str("");
    dtrack2Cmd.clear();

    if (err_code < 0) {
        printf("Error setting params and active output channel.\n"); fflush(stdout);
        delete _dt;
        return -2;
    }

    if (debug)
        printf("Connected to SmartTrack and listening for data on port '%d'.\n", _dt->getDataPort());

    if(_dt->startMeasurement()) {
        // set the udp_rx thread loop flag
        run = true;
        if (debug)
            printf("Started measurement\n"); fflush(stdout);
    } else {
        if (debug) {
            printf("Error starting measurement."
                   "Make sure SmartTrack is not currently being accessed by another instance.\n");
            fflush(stdout);
        }
        delete _dt;
        return -2;
    }
#else
    // Init library
    _dt = new DTrackSDK(st.srcPort);
    //  st.srcPort,              /* Port number of Smattrack */
    //  st.destPort,             /* Data rx port number to rx data from ART-Track */
    //  DTrackSDK::SYS_DTRACK_2, /* ARTtrack type */
    //  32768,                   /* UDP rx buffer size (in bytes) */
    //  20000,                   /* Timout to rx data (in us) */
    //  5000000);                /* Timeout for std ARTtrack replies (in us) */
    _run = true;
#endif /* ST_DEBUG */

    // Set the initial value to 0
    memset(_body.loc,0,3);
    memset(_body.rot,0,9);

    // Create and start the thread function
    _udprx_thread = new boost::thread(&SmartTrack::udprx_func, this, SmartTrack::_dt);

    return 0;
}

int SmartTrack::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret)
{
    if (debug == 2)
        printf("Executing %s \n", __FUNCTION__); fflush(stdout);

    // Mutex lock
    _rxdata.lock();

    DTrack_Body_Type_d* pBody = &_body;

#ifndef TW_DEBUG
    // Copy and transform the ART data into OVR math representation
    glm::vec3 position = glm::vec3((float)pBody->loc[0], (float)pBody->loc[1], (float)pBody->loc[2]);

    glm::mat3 rotation = glm::mat3(
        (float)pBody->rot[0], (float)pBody->rot[3], (float)pBody->rot[6],
        (float)pBody->rot[1], (float)pBody->rot[4], (float)pBody->rot[7],
        (float)pBody->rot[2], (float)pBody->rot[5], (float)pBody->rot[8]);
#else
    // dummy position
    Vector3f position = Vector3f(0.0f, 0.0f, 0.0f);

    // dummy rotation
    float phi = PI / 60, theta = 0, psi = 0;

    Matrix3f rotation = Matrix3f(cosf(phi)*cosf(theta), -sinf(phi)*cosf(theta), sinf(theta),
        sinf(phi)*cosf(psi) + cosf(phi)*sinf(theta)*sinf(psi), cosf(phi)*cosf(psi) - sinf(phi)*sinf(theta)*sinf(psi), -cosf(theta)*sinf(psi),
        sinf(phi)*sinf(psi) - cosf(phi)*sinf(theta)*cosf(psi), cosf(phi)*sinf(psi) + sinf(phi)*sinf(theta)*cosf(psi), cosf(theta)*cosf(psi));
#endif /* TW_DEBUG */

    // Mutex unlock
    _rxdata.unlock();

    // Convert mm to m
    position = glm::vec3(0.001f) * position;

    // Remap the position to openGL coordinates
    position = glm::vec3(-position.y, position.z, -position.x);

    // Convert the 3x3 rotation matrix into a quaternion
    glm::quat q = glm::toQuat(rotation);

    // Remap the quaternion to OpenGL coordinates
    glm::quat  orientation = glm::quat(-q.x, q.z, -q.x, q.w);
#if 0
    // Transpose the matrices to return elements columnwise
    float* pView = (float*)glm::value_ptr(view);

    // Append matrix elements to tcl return list
    for (unsigned int i = 0; i < 16; i++)
        Tcl_ListObjAppendElement(interp, tcl_ret, Tcl_NewDoubleObj(pView[i]));
#endif
    return 0;
}

int SmartTrack::Terminate()
{
    if (debug)
        printf("Executing %s \n", __FUNCTION__); fflush(stdout);

    // clean up:
    if (_dt != nullptr) {

#ifndef ST_DEBUG
        // Bring the thread to a controlled end
        run = !_dt->stopMeasurement();
#else
        _run = 0;
#endif /* ST_DEBUG */

        // Deactivate data output channel (chNum)
        std::stringstream dtrack2Cmd;
        dtrack2Cmd << "dtrack2 set output active " << st.pChNum << " " << "all" << " " << "no";
        if (debug)
            printf("Sending command string: %s\n", dtrack2Cmd.str()); fflush(stdout);

        if (_dt->sendDTrack2Command(dtrack2Cmd.str(), NULL) < 0) {
            printf("Error deactivating output channel: %s.\n", st.pChNum);
            fflush(stdout);
        }

        // Clear the sstream
        dtrack2Cmd.str("");
        dtrack2Cmd.clear();

        // join and wait for the thread to return
        _udprx_thread->join();

        delete _udprx_thread;
        delete _dt;
        _dt = nullptr;
        _udprx_thread = nullptr;

    }
    return 0;
}
