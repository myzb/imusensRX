/*
 * DeviceBase.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#include <tcl.h>

#include "DeviceBase.h"

DeviceBase::DeviceBase(char* device) { this->_device = device; }
DeviceBase::~DeviceBase() { this->_device = "null"; }

int DeviceBase::Init() { return 0; }
int DeviceBase::Stop() { return 0; }
