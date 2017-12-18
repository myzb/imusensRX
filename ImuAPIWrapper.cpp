/*
 * ImuAPIWrapper.cpp
 *
 *  Created on: July 13, 2017
 *      Author: may
 */

#include <tcl.h>

#include "ImuAPIWrapper.h"
#include "device_interface.h"

// Imusensor_Init: Defines the entry for the application
SENSOR_DLL int Imusensor_Init(Tcl_Interp *interp)
{
    // Link with the stubs library to make the extension as portable as possible
    if (Tcl_InitStubs(interp, "8.6", 0) == NULL) {
        printf("%s: failed\n", __FUNCTION__);
        fflush(stdout);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(1));
        return TCL_ERROR;
    }
    // Declare which package and version is provided by this C code
    if (Tcl_PkgProvide(interp, "imusensor", "1.0") != TCL_OK) {
        printf("%s: failed\n", __FUNCTION__);
        fflush(stdout);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(1));
        return TCL_ERROR;
    }

    // Register C functions to be wrapped
    Tcl_CreateObjCommand(interp, "imu_get_mvmatrix", device_get_mvmatrix,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#if 0
    Tcl_CreateObjCommand(interp, "device_cam_reset", device_cam_reset,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_postproc", device_postproc,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_init", device_init,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_terminate", device_terminate,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_close", device_close,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif
    // Return
    printf("%s: succeeded\n", __FUNCTION__);
    fflush(stdout);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}

// Imusensor_Unload: Unload the lib
SENSOR_DLL int Imusensor_Unload(Tcl_Interp *interp, int flags)
{
    // Delete registered procedures
    printf("Unloading lib ...\n"); fflush(stdout);

    Tcl_DeleteCommand(interp, "imu_get_mvmatrix");
#if 0
    Tcl_DeleteCommand(interp, "device_cam_reset");
    Tcl_DeleteCommand(interp, "device_postproc");
    Tcl_DeleteCommand(interp, "device_init");
    Tcl_DeleteCommand(interp, "device_terminate");
    Tcl_DeleteCommand(interp, "device_close");
#endif
    // Return
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}
