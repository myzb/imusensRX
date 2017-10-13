/*
 * DeviceBase.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include "DeviceBase.h"

DeviceBase::DeviceBase() { }
DeviceBase::DeviceBase(std::string& cmd) { _device = cmd; }
DeviceBase::~DeviceBase() { _device = "null"; }

int DeviceBase::Init() { return 0; }
int DeviceBase::Terminate() { return 0; }
int DeviceBase::GetMVMatrix(Tcl_Interp* interp, Tcl_Obj* tcl_ret) { return 0; }
