/*
 * usbInterface.h
 *
 *  Created on: Jun 8, 2017
 *      Author: may
 */

#ifndef _USBINTERFACE_H_
#define _USBINTERFACE_H_

int hid_init();
void hid_end();
int hid_getMsg();
int hid_sendMsg(void *buf, int len, int timeout);
bool hid_getStatus();

char get_keystroke(void);

#endif /* _USBINTERFACE_H_ */
