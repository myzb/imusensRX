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

    // Smarttrack variables
    DTrackSDK*          _dt;
    DTrack_Body_Type_d  _body;               // buffer holding pos/rot data

    struct stConfig_t {
        char *pSrcIpAddr, *pDestIpAddr, *pChNum;
        int  srcPort, destPort;
    } st;

    // OpenGL variables
    glm::mat4 view, proj; // Current view and proj matrices

    // Thread variables
    bool                _run;
    boost::thread*      _udprx_thread;
    boost::mutex        _rxdata;

    void* udprx_func(DTrackSDK* dt);   // Thread function
public:
    // Public functions
    SmartTrack(std::string &name);

    int Init(Tcl_Interp* interp, Tcl_Obj* tcl_ret, char* pSrcIpAddr, char* pDestIpAddr, char* pChNum);
    int GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret);
    int Terminate();
};

#endif /* _SMARTTRACK_H_ */
