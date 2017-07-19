/*
 * usbInterface.h
 *
 *  Created on: Jun 8, 2017
 *      Author: may
 */

#ifndef _USBINTERFACE_H_
#define _USBINTERFACE_H_

int hid_init();
int hid_end(int num);
int hid_getMsg();
int hid_sendMsg(void *buf, int len, int timeout);
bool hid_getStatus();

#endif /* _USBINTERFACE_H_ */
