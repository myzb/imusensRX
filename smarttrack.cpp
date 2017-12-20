/*
 * smarttrack.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <iostream>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <DTrackSDK.hpp>

#include "utils.h"
#include "DeviceBase.h"
#include "smarttrack.h"

static const int Debug = 0;

SmartTrack::SmartTrack() : DeviceBase()
{
    // TODO: these will be passed from TCL
    std::string dst_addr = "10.0.1.15:50105";
    std::string src_addr = "10.0.8.5:50105";
    std::string ch_num   = "ch03";

    // Extract the src/dest IP address and port from input string
    short separator = src_addr.find_first_of(':');
    std::string src_port = src_addr.substr(separator+1, src_addr.npos);
    std::string src_ip   = src_addr.substr(0,separator);

    separator = dst_addr.find_first_of(':');
    std::string dst_port = dst_addr.substr(separator+1, dst_addr.npos);
    std::string dst_ip   = dst_addr.substr(0,separator);

    // Store the com/port configurations
    _st = { src_ip, dst_ip, ch_num, std::stoi(src_port, nullptr), std::stoi(dst_port, nullptr) };
}

SmartTrack::SmartTrack(std::string &name) : DeviceBase(name) { }

void SmartTrack::Export(bool state)
{
    _export = state;
}

void SmartTrack::ResetCamera()
{
    if (Debug) std::cout << __func__ << ": Resetting camera" << std::endl;
    _dcm_o = _dcm_r;
}

void SmartTrack::TaskLoop()
{
    if (Debug) std::cout << "Executing " << __func__ << std::endl;

    bool silence_output = false; // debug variable

    while (_running) {
        // Poll UDP port for new data
        if (_dt->receive() && (Debug == 2))
            std::cout << "Received UDP package" << std::endl;

        // Don't start copying new data until a body is found
        if (_dt->getNumBody() == 0) {
            continue;
        } else if ((_dt->getNumBody() > 0) && (Debug == 2)) {
            std::cout << "Found " << _dt->getNumBody() << " bodies! tracking _body id "
                      << _dt->getBody(0)->id << std::endl;
        }

        // Quality -1 = lost track of body
        if (_dt->getBody(0)->quality >= 0) {

            if (Debug == 2) std::cout << "Reading..." << std::endl;

            _body = *_dt->getBody(0); // Get tracking data

            // Mutex lock
            _rxdata.lock();

            std::copy(_body.rot, _body.rot+9, glm::value_ptr(_dcm_r)); // DCM
            std::copy(_body.loc, _body.loc+3, glm::value_ptr(_pos_r)); // Position

            if (_export) {
                static exporter data("smarttrack_data.txt");
#if 0

                data.export_data(glm::value_ptr(_dcm_r), 9);
#else
                glm::mat3 dcm = _dcm_r * glm::inverse(_dcm_o);
                data.export_data(glm::value_ptr(dcm), 9);
#endif
            }

            // Mutex unlock
            _rxdata.unlock();

            silence_output = false;

        } else if (Debug && !silence_output) {
            std::cout << "Lost track of _body id "<<  _dt->getBody(0)->id
                      << " quality = " << _dt->getBody(0)->quality << std::endl;
            silence_output = true;
        }
    }

    if (Debug) std::cout << __func__ <<"::Thread terminated" << std::endl;

    return;
}

int SmartTrack::Init()
{
    std::cout << "Executing " << __func__ << std::endl;

    if (Debug) {
        std::cout << "Src  IP address: " << _st.src_ip << " port " << _st.src_port << std::endl;
        std::cout << "Dest IP address: " << _st.dst_ip << " port " << _st.dst_port << std::endl;
    }

    // Init library
    if (_st.src_port != 50105)
        std::cout << "WARNING: DTrack2 port must be 50105" << std::endl;

    _dt = new DTrackSDK(_st.src_ip,             /* IP address of Strack */
                       _st.src_port,            /* Port number of Strack */
                       _st.dst_port,            /* Data rx port number to rx data from STrack */
                       DTrackSDK::SYS_DTRACK_2, /* ARTtrack type */
                       32768,                   /* UDP rx buffer size (in bytes) */
                       20000,                   /* Timout to rx data (in us) */
                       5000000);                /* Timeout for std STrack replies (in us) */

    if (!_dt->isLocalDataPortValid()) {
        if (Debug) std::cout << "DTrack init error" << std::endl;
        delete _dt;
        return -2;
    }

    // Forcestop any running measurements
    _dt->stopMeasurement();

    // Configure SmartTrack
    // Set channel, protocol, destination IP (localhost IP address) and the portNum
    std::stringstream dtrack2_cmd;
    dtrack2_cmd << "dtrack2 set output net " << _st.ch_num << " udp " << _st.dst_ip
                << " " << _st.dst_port;

    if (Debug) std::cout << "Sending command string: " << dtrack2_cmd.str() << std::endl;

    int err_code = _dt->sendDTrack2Command(dtrack2_cmd.str(), NULL);

    // Clear the sstream
    dtrack2_cmd.str("");
    dtrack2_cmd.clear();

    if (err_code < 0) {
        std::cout << "Error, Smarttrack answered with error code: " << err_code << std::endl;
        delete _dt;
        return -2;
    }

    // Set the data to be transfer on channel (chNum) and activate the channel
    dtrack2_cmd << "dtrack2 set output active " << _st.ch_num << " " << "all" << " " << "yes";
    if (Debug)
        std::cout << "Sending command string: " << dtrack2_cmd.str() << std::endl;

    err_code = _dt->sendDTrack2Command(dtrack2_cmd.str(), NULL);

    // Clear the sstream
    dtrack2_cmd.str("");
    dtrack2_cmd.clear();

    if (err_code < 0) {
        std::cout << "Error setting params and active output channel." << std::endl;
        delete _dt;
        return -2;
    }

    if (Debug)
        std::cout << "Connected to SmartTrack. Listening for data on port " << _dt->getDataPort()
                  << std::endl;

    if(_dt->startMeasurement()) {
        // set the udp_rx thread loop flag
        _running = true;
        if (Debug)
            std::cout << "Started measurement" << std::endl;
    } else {
        if (Debug) {
            std::cout << "Error starting measurement.\n"
                         "Make sure SmartTrack is not currently being accessed by another instance"
                      << std::endl;
        }
        delete _dt;
        return -2;
    }

    // Create and start the thread function
    _rxThread = new boost::thread(boost::bind( &SmartTrack::TaskLoop, this));

    return 0;
}

int SmartTrack::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret)
{
    if (Debug == 2)
        std::cout << "Executing " << __func__ << std::endl;

    // Mutex lock
    _rxdata.lock();

    // Substract origin from current data
    glm::mat3 rot = _dcm_r * glm::inverse(_dcm_o);
    glm::vec3 pos = _pos_r - _pos_o;

    // Mutex unlock
    _rxdata.unlock();

    glm::vec3 finalUp      = rot * glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 finalForward = rot * glm::vec3(0.0f, 0.0f, -1.0f);

    // Build modelview matrix
    glm::mat4 view = glm::lookAtRH(pos, finalForward, finalUp);

    // Print the current modelview matrix
    if (Debug > 1) std::cout << glm::to_string(rot) << std::endl;

    // return an identity matrix
    if (!_start) view = glm::mat4();

    // Append matrix colums as elements to tcl_ret object
    const float *mvm = (const float*)glm::value_ptr(view);
    for (int i = 0; i < 16; i++) {
        Tcl_ListObjAppendElement(interp, tcl_ret, Tcl_NewDoubleObj(mvm[i]));
    }

    return 0;
}

int SmartTrack::Terminate()
{
    if (Debug)
        std::cout << "Executing " << __func__ << std::endl;

    // Clean up:
    if (_dt != nullptr) {

        // Bring the thread to a controlled end
        _running = !_dt->stopMeasurement();

        // Deactivate data output channel (ch_num)
        std::stringstream dtrack2_cmd;
        dtrack2_cmd << "dtrack2 set output active " << _st.ch_num << " " << "all" << " " << "no";
        if (Debug)
            std::cout << "Sending command string: " << dtrack2_cmd.str() << std::endl;

        if (_dt->sendDTrack2Command(dtrack2_cmd.str(), NULL) < 0) {
            std::cout << "Error deactivating output channel: " << _st.ch_num << std::endl;
        }

        // Clear the sstream
        dtrack2_cmd.str("");
        dtrack2_cmd.clear();

        // Join and wait for the thread to return
        _rxThread->join();

        delete _rxThread;
        delete _dt;
        _dt       = nullptr;
        _rxThread = nullptr;
    }

    return 0;
}
