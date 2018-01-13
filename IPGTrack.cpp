/*
 * IPGTrack.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <iostream>
#include <stdint.h>
#include <boost/chrono.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DeviceBase.h"
#include "hid.h"
#include "utils.h"
#include "IPGTrack.h"

static const int Debug = 1;

IPGTrack::IPGTrack () : DeviceBase() { }
IPGTrack::IPGTrack (std::string &cmd) : DeviceBase(cmd) { }

void IPGTrack::ResetCamera()
{
    if (Debug) std::cout << __func__ << ": Resetting camera" << std::endl;

    // Reset the camera and start output to IPGMovie
    _quat_o = _quat_r;
    _start = true;
}

void IPGTrack::Export(bool state)
{
    _export = state;
}

int IPGTrack::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret)
{
    // Convert the quaternion to matrix (and substact 'fixed' sensor -> head mounting orientation)
    glm::mat3 rot = glm::toMat3(_quat_r * glm::conjugate(_quat_o));

    // CM_up = rot*OGL_up (+y), CM_fwd = rot*OGL_fwd (-z)
    glm::vec3 finalUp        = rot * glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 finalForward   = rot * glm::vec3(0.0f, 0.0f, -1.0f);

    // Position of left/right eye camera relative to origin
    glm::vec3 pos            = glm::vec3(0.0f, 0.0f, 0.0f);

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

int IPGTrack::Init()
{
    int ret = 0;

    while(ret <= 0) {
        // C-based example is 16C0:0480:FFAB:0200
        ret = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
        if (ret <= 0) {
            // Arduino-based example is 16C0:0486:FFAB:0200
            ret = rawhid_open(1, 0x16C0, 0x0486, 0xFFAB, 0x0200); // RawHID
            if (ret <= 0) {
                std::cout << __func__ << ": No device found" << std::endl;
                boost::this_thread::sleep_for(boost::chrono::seconds(2));
            }
        }
    }

    std::cout << __func__ << ": Found device, starting ..." << std::endl;

    // wait 10ms
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
    _running = true;
    _rxThread = new boost::thread(boost::bind(&IPGTrack::TaskLoop, this));
    return 0;
}

void IPGTrack::SetQuat(data_t &rx_data)
{
    // Remap the NED quat to CM coordinates
    // +w rotates the sensor frame, -w rotates the camera
    _quat_r = glm::quat(rx_data.num_f[0]*(+1.0f),    /* CM_w = +NED_w */
                        rx_data.num_f[1]*(+1.0f),    /* CM_x = +NED_x */
                        rx_data.num_f[2]*(-1.0f),    /* CM_y = -NED_y */
                        rx_data.num_f[3]*(-1.0f));   /* CM_z = -NED_z */
}

void IPGTrack::TaskLoop()
{
    int ret;
    uint32_t ts = 0, timeout = 0;
    data_t tx_data = { 0 }, rx_data = { 0 };

    // Msg the uC 'anything' to signalise application is online
    boost::thread(rawhid_send, 0, &tx_data.raw, 64, 100);

    while (_running) {

        // Get Raw HID packet, set timeout to 1ms. This caps the loop speed at 1ms
        ret = rawhid_recv(0, &rx_data.raw, sizeof(rx_data), 5);

        if (ret < 0) {
            std::cout << __func__ << ": Error reading, device went offline" << std::endl;
            _running = false;
            rawhid_close(1);
            break;
        } else if (ret > 0) {
            // Pass Usb buffer to programm
            SetQuat(rx_data);
        } else {
            if (Debug) std::cout << __func__ << ": Timeout!" << std::endl;
            timeout++;
        }

        if (get_micros() - ts >= 5000000) {
            std::cout << __func__ << ": Timeout/s = " << timeout/5.0f << std::endl;
            ts = get_micros();
            timeout = 0;
        }

        if (_export) {
            // Export all the received data
            static exporter data("/home/matt/ipgtrack_data.txt");
#if 1
            std::vector<float> ipgtrack;
            glm::quat q =_quat_r * glm::inverse(_quat_o);
            ipgtrack.insert(ipgtrack.begin(), glm::value_ptr(q), glm::value_ptr(q)+4);
            ipgtrack.insert(ipgtrack.end(),rx_data.num_f, rx_data.num_f+16);
            data.export_data(ipgtrack.data(), ipgtrack.size());
#else
            data.export_data(rx_data.num_f, 16);
#endif
        }

        // Send the msg back
        //boost::thread thread(Send, rx_data.raw, 64, 10);
    }

    std::cout << __func__ << ": Thread terminated!" << std::endl;
}

int IPGTrack::Send(char keycode)
{
    if (Debug) std::cout << __func__ << ": Entered" << std::endl;
    static data_t tx_data = { 0 };

    switch (keycode) {
    // (a)ll on
    case 97:
        tx_data.raw[0] = 0x00;
        std::cout << __func__ << ": All on" << std::endl;
        break;
    // (c)orrection only
    case 99:
        tx_data.raw[0] = 0x01;
        std::cout << __func__ << ": Prediction only" << std::endl;
        break;
    // (p)rediction only
    case 112:
        tx_data.raw[0] = 0x02;
        std::cout << __func__ << ": Correction only" << std::endl;
        break;
    // (x) all off
    case 120:
        tx_data.raw[0] = 0x03;
        std::cout << __func__ << ": All off" << std::endl;
        break;
    // Unknown key
    default:
        return -1;
    }
    // Send the bitfield to uC
    if (Debug) std::cout << __func__ << ": Sending " << std::bitset<8>(tx_data.raw[0]) << std::endl;
    boost::thread(rawhid_send, 0, &tx_data, 64, 100);
    return 0;
}

int IPGTrack::Terminate()
{
    _running = false;
    _rxThread->join(); // join the thread and wait for it to return
    _rxThread = nullptr;
    rawhid_close(1);
    return 0;
}
