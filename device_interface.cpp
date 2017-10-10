/*
 * device_interface.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <tcl.h>

#include "device_interface.h"
#include "DeviceBase.h"
#include "smarttrack.h"

DeviceBase *device = nullptr;

int device_init(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device = new SmartTrack("dummy");
    device->Init();
    return 0;
}

int device_terminate(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device->Stop();
    return 0;
}

int device_close(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    delete device;
    return 0;
}

