/*
 * device_interface.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <stdlib.h>
#include <boost/thread/once.hpp>

#include "device_interface.h"
#include "utils.h"
#include "DeviceBase.h"
#include "IPGTrack.h"
#include "smarttrack.h"

DeviceBase *device = nullptr;

boost::once_flag once = BOOST_ONCE_INIT;

void hack_ipgmovie_terminate() {
    device->Export(false);
    device->Terminate();
}

void hack_ipgmovie_init() {
    atexit(hack_ipgmovie_terminate);
    device = new IPGTrack();
    device->Init();
    device->Export(true);
}
int device_get_mvmatrix(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    // FIXME: Remove once IPGMovie is ready
    // Make sure to call init function only once
    boost::call_once(hack_ipgmovie_init, once);

    // FIXME: Remove once IPGMovie is ready
    // Workaround to set undefined variable Pgm(Debug)
    Tcl_SetVar2Ex(interp, "Pgm", "Debug", Tcl_NewIntObj(0), 0);

    // Allocate new tcl_ret object
    Tcl_Obj* tcl_ret = Tcl_NewListObj(0, NULL);

    device->GetMVMatrix(interp, tcl_ret);

    // Return the complete tcl list
    Tcl_SetObjResult(interp, tcl_ret);
    return TCL_OK;
}

int device_postproc(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device->PostProcc();
    return TCL_OK;
}

int device_init(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    // TODO: switch-case to initialise desired device
    device = new IPGTrack();
    device->Init();
    return TCL_OK;
}

int device_terminate(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device->Terminate();
    delete device;
    return TCL_OK;
}

int device_close(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device->Terminate();
    delete device;
    return TCL_OK;
}

