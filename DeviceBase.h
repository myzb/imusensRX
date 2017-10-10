/*
 * DeviceBase.h
 *
 *  Created on: Oct 10, 2017
 *      Author: may
 */

#ifndef _DEVICEBASE_H_
#define _DEVICEBASE_H_

class DeviceBase {
private:
    char* _device;

public:
    DeviceBase(char *device);
    virtual ~DeviceBase();

    virtual int Init();
    virtual int Stop();
};

#endif /* _DEVICEBASE_H_ */
