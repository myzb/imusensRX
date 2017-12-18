/*
 * DeviceBase.h
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#ifndef _DEVICEBASE_H_
#define _DEVICEBASE_H_

#include <string>
#include <tcl.h>

class DeviceBase {
private:
    std::string _device;
public:
    DeviceBase();
    DeviceBase(std::string& cmd);
    virtual ~DeviceBase();

    virtual int Init();
    virtual int Terminate();
    virtual int GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret);
    virtual int PostProcc();

    virtual void ResetCamera();
    virtual void Export(bool state);
};

#endif /* _DEVICEBASE_H_ */
