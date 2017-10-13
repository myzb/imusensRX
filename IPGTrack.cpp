#include <stdio.h>
#include <stdint.h>

#include <tcl.h>
#include <boost/chrono.hpp>
#include <glm/glm.hpp>
//#include <glm/gtx/quaternion.hpp>
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

int IPGTrack::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret)
{
    // Convert the quaternion to matrix (account for origin displacement)
    glm::mat4 q = glm::toMat4(_quat_r * glm::inverse(_quat_o));

    // Append matrix colums as elements to tcl_ret object
    const float *mvm = (const float*)glm::value_ptr(q);
    for (int i = 0; i < 16; i++) {
        Tcl_ListObjAppendElement(interp, tcl_ret, Tcl_NewDoubleObj(mvm[i]));
    }
    return 0;
}

int IPGTrack::Init()
{
    int ret = 0;

    printf("start\n"); fflush(stdout);

    while(ret <= 0) {
        // C-based example is 16C0:0480:FFAB:0200
        ret = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
        if (ret <= 0) {
            // Arduino-based example is 16C0:0486:FFAB:0200
            ret = rawhid_open(1, 0x16C0, 0x0486, 0xFFAB, 0x0200); // RawHID
            if (ret <= 0) {
                printf("no rawhid device found\n");
                boost::this_thread::sleep_for(boost::chrono::seconds(2));
            }
        }
    }
    printf("found rawhid device\n");

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

    // Msg the uC that application is online
    data_t tx_data = { 0 }, rx_data = { 0 };
    boost::thread(&IPGTrack::Send, this, &tx_data, 64, 100);

    while(_running) {

        // Get Raw HID packet, set timeout to 1ms. This caps the loop speed at 1ms
        ret = rawhid_recv(0, &rx_data.raw, sizeof(rx_data), 1);

        if (ret < 0) {
            printf("\nerror reading, device went offline\n");
            _running = false;
            rawhid_close(1);
            break;
        } else if (ret > 0) {
            // Pass Usb buffer to programm
            SetQuat(rx_data);
        } else {
            if (Debug) printf("Timeout!\n");
            timeout++;
        }

        // On 'Spacebar': set the new reference origin
        if (get_keystroke() == 32) {
            if (Debug) printf("\nReseting Camera\n");
            _quat_o = _quat_r;
        }

        if (get_micros() - ts >= 5000000) {
            printf("timeout/s = %.1f\n", timeout/5.0f);
            ts = get_micros();
            timeout = 0;
        }

        // Send the msg back
        //boost::thread thread(Send, rx_data.raw, 64, 10);
    }

    printf("Thread terminated!\n");
}

void IPGTrack::Send(void *buf, int len, int timeout)
{
    int ret = rawhid_send(0, buf, len, timeout);
    if (ret > 0) {
        if (Debug) printf("sendMsg at ms: %u\n", get_micros()/1000);
    }else if (ret < 0) {
        printf("Error Sending\n");
    }
}

int IPGTrack::Terminate()
{
    _running = false;
    _rxThread->join(); // join the thread and wait for it to return
    rawhid_close(1);
    return 0;
}
