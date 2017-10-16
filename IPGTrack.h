/*
 * IPGTrack.h
 *
 *  Created on: Jun 8, 2017
 *      Author: may
 */

#ifndef _IPGTRACK_H_
#define _IPGTRACK_H_

#include <string>
#include <tcl.h>
#include <boost/thread/thread.hpp>
#include <glm/gtx/quaternion.hpp>

// RawHID transfers floats as bytes
typedef union data {
  float num_f[16];
  uint32_t num_d[16];
  uint8_t raw[64];
} data_t;

class IPGTrack : public DeviceBase {

private:
    bool _running = false;
    bool _export = false;
    boost::thread *_rxThread;
    glm::quat _quat_o = glm::quat(), _quat_r = glm::quat();

    void SetQuat(data_t &rx_data);
    void TaskLoop();
    void Send(void *buf, int len, int timeout);

public:
    IPGTrack();
    IPGTrack(std::string &cmd);

    int Init();
    int Terminate();
    int GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret);
    bool Running();

    void Export(bool state);
};

#endif /* _IPGTRACK_H_ */
