/*
 * device_interface.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <stdlib.h>
#if defined(OS_WINDOWS)
#include <windows.h>
#endif
#include <boost/thread/once.hpp>

#include "device_interface.h"
#include "utils.h"
#include "DeviceBase.h"
#include "IPGTrack.h"
#include "smarttrack.h"

DeviceBase *device = nullptr;
#ifdef ST_COMPARE
DeviceBase *st = nullptr;
#endif /* ST_COMPARE */

boost::once_flag once = BOOST_ONCE_INIT;

void hack_ipgmovie_terminate() {
    device->Export(false);
    device->Terminate();

#ifdef ST_COMPARE
    st->Export(false);
    st->Terminate();
#endif /* ST_COMPARE */
}

void hack_ipgmovie_init(Tcl_Interp *interp) {
    atexit(hack_ipgmovie_terminate);

    // Workaround to set undefined variable Pgm($Debug)
    Tcl_SetVar2Ex(interp, "Pgm", "Debug", Tcl_NewIntObj(0), 0);

    device = new IPGTrack();
    device->Init();
    device->Export(true);

#ifdef ST_COMPARE
    st = new SmartTrack();
    st->Init();
    st->Export(true);
#endif /* ST_COMPARE */
}

int device_get_mvmatrix(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    // FIXME: Remove once IPGMovie implements this
    // Make sure to call init function only once
    boost::call_once(boost::bind(&hack_ipgmovie_init, interp), once);

    // Allocate new tcl_ret object
    Tcl_Obj* tcl_ret = Tcl_NewListObj(0, NULL);

    // FIXME: Remove once IPGMovie implements this
    // On 'Spacebar': set the new camera origin
#if defined(OS_WINDOWS)
    if (GetAsyncKeyState(VK_SPACE)) {
#else
    char key = get_keystroke();
    if (key == 32) {
#endif
    device->ResetCamera();
#ifdef ST_COMPARE
        st->ResetCamera();
#endif
    }

    // On 'key': send a message to the device
#if !defined(OS_WINDOWS)
    if (key > 32) device->Send(key);
#endif

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

int device_cam_reset(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    device->ResetCamera();
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

