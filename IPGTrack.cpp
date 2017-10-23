/*
 * IPGTrack.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <iostream>
#include <stdint.h>
#include <boost/chrono.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DeviceBase.h"
#include "hid.h"
#include "utils.h"
#include "IPGTrack.h"

static const int Debug = 0;

IPGTrack::IPGTrack () : DeviceBase() { }
IPGTrack::IPGTrack (std::string &cmd) : DeviceBase(cmd) { }

bool IPGTrack::Running()
{
    return _running;
}

void IPGTrack::Export(bool state)
{
    _export = state;
}

int IPGTrack::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret)
{
    // Convert the quaternion to matrix (and apply correction for ref orientation)
    glm::mat3 rot = glm::toMat3(_quat_r * glm::inverse(_quat_o));

    glm::vec3 finalUp        = rot * glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 finalForward   = rot * glm::vec3(0.0f, 0.0f, -1.0f);

    // Position of left/right eye camera relative to origin
    glm::vec3 pos            = glm::vec3(0.0f, 0.0f, 0.0f);

    // Build modelview matrix
    glm::mat4 view = glm::lookAtRH(pos, finalForward, finalUp);

    // Print the current modelview matrix
    if (Debug > 1) std::cout << glm::to_string(rot) << std::endl;

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
    // TODO: Note on 'w': Multiplying w by (-1) rotates the cube according to the sensor.
    // For the actual implementation we want to use w*(+1) as we will be rotating the camera.
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
    boost::thread(&IPGTrack::Send, this, &tx_data, 64, 100);

    while(_running) {

        // Get Raw HID packet, set timeout to 1ms. This caps the loop speed at 1ms
        ret = rawhid_recv(0, &rx_data.raw, sizeof(rx_data), 1);

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

        // On 'Spacebar': set the new reference origin
        if (get_keystroke() == 32) {
            if (Debug) std::cout << __func__ << ": Resetting camera" << std::endl;
            _quat_o = _quat_r;
        }

        if (get_micros() - ts >= 5000000) {
            std::cout << __func__ << ": Timeout/s = " << timeout/5.0f << std::endl;
            ts = get_micros();
            timeout = 0;
        }

        if (_export) {
            static exporter data;
            // Convert the quaternion to matrix (account for origin displacement)
            glm::quat q =_quat_r * glm::inverse(_quat_o);
            data.export_data("ipgtrack_data.txt", glm::value_ptr(q), 4);
        }

        // Send the msg back
        //boost::thread thread(Send, rx_data.raw, 64, 10);
    }

    std::cout << __func__ << ": Thread terminated!" << std::endl;
}

void IPGTrack::Send(void *buf, int len, int timeout)
{
    int ret = rawhid_send(0, buf, len, timeout);
    if (ret > 0) {
        if (Debug)
            std::cout << __func__ << ": Sending Msg at ms: " << get_micros()/1000.0f << std::endl;
    }else if (ret < 0) {
        std::cout << __func__ << ": Error sending!" << std::endl;
    }
}

int IPGTrack::Terminate()
{
    _running = false;
    _rxThread->join(); // join the thread and wait for it to return
    rawhid_close(1);
    return 0;
}
