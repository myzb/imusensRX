/*
 * smarttrack.h
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#ifndef SMARTTRACK_H
#define SMARTTRACK_H

#include <tcl.h>
#include <string>
#include <boost/thread/thread.hpp>
#include <glm/mat4x4.hpp>
#include <DTrackSDK.hpp>

class SmartTrack : public DeviceBase {

private:
    bool _running = false;
    bool _export  = false;
    bool _start   = false;

    DTrackSDK*         _dt = nullptr;
    DTrack_Body_Type_d _body = { 0 };
    boost::thread*     _rxThread = nullptr;
    boost::mutex       _rxdata;

    glm::vec3 _pos_o = glm::vec3(), _pos_r = glm::vec3();
    glm::mat3 _dcm_o = glm::mat3(), _dcm_r = glm::mat3();
    glm::mat4 view, proj;

    struct stinfo_t {
        std::string src_ip, dst_ip, ch_num;
        int src_port, dst_port;
    } _st;

    void TaskLoop();

public:
    SmartTrack();
    SmartTrack(std::string &name);

    int Init();
    int Terminate();
    int GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret);
    void Export(bool state);
    void ResetCamera();
};

#endif /* _SMARTTRACK_H_ */
