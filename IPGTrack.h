/*
 * usbInterface.h
 *
 *  Created on: Jun 8, 2017
 *      Author: may
 */

#ifndef _USBINTERFACE_H_
#define _USBINTERFACE_H_

#include <string>
#include <glm/gtx/quaternion.hpp>
#include <boost/thread/thread.hpp>
#include <tcl.h>

// RawHID transfers floats as bytes
typedef union data {
  float num_f[16];
  uint32_t num_d[16];
  uint8_t raw[64];
} data_t;

class IPGTrack : public DeviceBase {

private:
    bool _running = false;
    boost::thread *_rxThread;
    glm::quat _quat_o = glm::quat(), _quat_r = glm::quat();

    void SetQuat(data_t &rx_data);
    void TaskLoop();

public:
    IPGTrack();
    IPGTrack(std::string &cmd);

    int Terminate();
    void Send(void *buf, int len, int timeout);
    bool Running();

    int Init();
    int GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret);
};

#endif /* _USBINTERFACE_H_ */
