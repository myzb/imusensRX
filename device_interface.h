/*
 * device_interface.h
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#ifndef DEVICE_INTERFACE_H_
#define DEVICE_INTERFACE_H_

int device_init(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
int device_terminate(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
int device_close(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

#endif /* DEVICE_INTERFACE_H_ */
