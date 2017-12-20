/*
 * utils.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: may
 */

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <chrono>

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>
#elif defined(OS_WINDOWS)
#include <conio.h>
#endif

#include "utils.h"

exporter::exporter(std::string filename)
{
    _myfile = std::ofstream(filename, std::ios::trunc | std::ios::out);
}

int exporter::export_data(float *data, int data_len)
{
    if (_myfile.is_open()) {
        _myfile << get_micros();
        for (int i = 0; i < data_len; i++)
             _myfile << "\t" << data[i];

        _myfile << std::endl;
    } else {
        std::cout << "Unable to open file\n";
    }
    return 0;
}

int get_micros()
{
    static int flag = 0;
    static std::chrono::system_clock::time_point midnight;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // Get midnight time for today only once
    if (!flag) {
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm *date = std::localtime(&tnow);
        date->tm_hour = 0;
        date->tm_min = 0;
        date->tm_sec = 0;
        midnight = std::chrono::system_clock::from_time_t(std::mktime(date));

        flag = 1;
    }

    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - midnight);
    return diff.count();
}

#if defined(OS_LINUX) || defined(OS_MACOSX)
// Linux (POSIX) implementation of _kbhit().
// Morgan McGuire, morgan@cs.brown.edu
static int _kbhit() {
    static const int STDIN = 0;
    static int initialized = 0;
    int bytesWaiting;

    if (!initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
static char _getch(void) {
    char c;
    if (fread(&c, 1, 1, stdin) < 1) return 0;
    return c;
}
#endif

char get_keystroke(void)
{
    if (_kbhit()) {
        char c = _getch();
        if (c >= 32) return c;
    }
    return 0;
}
