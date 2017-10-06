/*
 * utils.h
 *
 *  Created on: Jun 7, 2017
 *      Author: may
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#define RAD_TO_DEG 57.295779513082320876798154814105f
#define DEG_TO_RAD 0.017453292519943295769236907684886f

// RawHID transfers floats as bytes
typedef union {
  float num_f[16];
  uint32_t num_d[16];
  uint8_t raw[64];
} data_t;


float updateTime(uint32_t &lastUpdate);

void getEulers();

int getTimestamp_usec();

#endif /* _UTILS_H_ */
