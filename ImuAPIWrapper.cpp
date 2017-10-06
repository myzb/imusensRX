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

#include "quaternionFilters.h"
#include "usbInterface.h"
#include "ImuAPIWrapper.h"

int get_mvmatrix(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    static bool flag = 1;
    // get the rotation quaternion received via USB. getQ() returns a pointer to the global 'q[0]'
    glm::quat qOri = glm::quat(*(getQ()+1)*(+1),    /* OGL_x =  IMU_x */
                               *(getQ()+3)*(+1),    /* OGL_y =  IMU_z */
                               *(getQ()+2)*(-1),    /* OGL_z = -IMU_y */
                               *(getQ()+0)*(-1));   /* OGL_w =  IMU_w */

    // TODO: Note on 'w': Multiplying w by (-1) rotates the cube according to the sensor.
    // For the actual implementation we want to use w*(+1) as we will be rotating the camera.

    // Default Initializations

    if (flag) {
        hid_init();
        flag = 0;
    }

    // FIXME: Workaround to set undefined variable Pgm(Debug)
    Tcl_SetVar2Ex(interp, "Pgm", "Debug", Tcl_NewIntObj(0), 0);

    // Convert the quaternion to matrix
    glm::mat4 mv = glm::toMat4(qOri);

    // Allocate new tcl_ret object
    Tcl_Obj* tcl_ret = Tcl_NewListObj(0, NULL);

    // Append matrix colums as elements to tcl_ret object
    const float *pMV = (const float*)glm::value_ptr(mv);
    for (int i = 0; i < 16; ++i) {
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

    // Return
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}
