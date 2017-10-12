/*
 * ImuAPIWrapper.cpp
 *
 *  Created on: July 13, 2017
 *      Author: may
 */

#include <tcl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ImuAPIWrapper.h"
#include "device_interface.h"
#include "quaternionFilters.h"
#include "usbInterface.h"

int get_mvmatrix(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    // Default Initializations
    static bool flag = 1;
    if (flag) {
        hid_init();
        flag = 0;
    }

    // The rotation to the actual origin
    static glm::quat qOri_ref;

    // get the rotation quaternion received via USB. getQ() returns a pointer to the global 'q[0]'
    glm::quat qOri = glm::quat(*(getQ()+0)*(+1),    /* CM_w = +IMU_w */
                               *(getQ()+1)*(+1),    /* CM_x = +IMU_x */
                               *(getQ()+2)*(-1),    /* CM_y = -IMU_y */
                               *(getQ()+3)*(-1));   /* CM_z = -IMU_z */

    // TODO: Note on 'w': Multiplying w by (-1) rotates the cube according to the sensor.
    // For the actual implementation we want to use w*(+1) as we will be rotating the camera.

    // Get the new reference origin
    if (get_keystroke() == 32) {
        printf("\nReseting Camera\n");
        qOri_ref = qOri;
    }

    // Convert the quaternion to matrix (account for origin displacement)
    glm::mat4 ori = glm::toMat4(qOri * glm::inverse(qOri_ref));

    // FIXME: Workaround to set undefined variable Pgm(Debug)
    Tcl_SetVar2Ex(interp, "Pgm", "Debug", Tcl_NewIntObj(0), 0);

    // Allocate new tcl_ret object
    Tcl_Obj* tcl_ret = Tcl_NewListObj(0, NULL);

    // Append matrix colums as elements to tcl_ret object
    const float *pMV = (const float*)glm::value_ptr(ori);
    for (int i = 0; i < 16; i++) {
        Tcl_ListObjAppendElement(interp, tcl_ret, Tcl_NewDoubleObj(pMV[i]));
    }

    // Return the complete tcl list
    Tcl_SetObjResult(interp, tcl_ret);
    return TCL_OK;
}

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
    Tcl_CreateObjCommand(interp, "imu_get_mvmatrix", get_mvmatrix,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    Tcl_CreateObjCommand(interp, "device_init", device_init,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_terminate", device_terminate,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(interp, "device_close", device_close,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

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
    Tcl_DeleteCommand(interp, "device_init");
    Tcl_DeleteCommand(interp, "device_terminate");

    // Return
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}
