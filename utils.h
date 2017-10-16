/*
 * utils.h
 *
 *  Created on: Jun 7, 2017
 *      Author: may
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

int get_micros();
char get_keystroke(void);

class exporter {
public:
int export_data(std::string filename, float *data, int data_len);
};

#endif /* _UTILS_H_ */
