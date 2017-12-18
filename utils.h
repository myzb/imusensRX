/*
 * utils.h
 *
 *  Created on: Jun 7, 2017
 *      Author: may
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <fstream>

int get_micros();
char get_keystroke(void);

// The exporter class
class exporter {
private:
    std::ofstream _myfile;

public:
    exporter(std::string filename);
    int export_data(float *data, int data_len);
};

#endif /* _UTILS_H_ */
