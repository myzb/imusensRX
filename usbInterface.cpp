#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(__linux__)
#define OS_LINUX
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>
#elif defined(OS_WINDOWS)
#include <conio.h>
#endif

#include <boost/thread.hpp>

#include "hid.h"
#include "usbInterface.h"
#include "quaternionFilters.h"
#include "utils.h"

#define AHRS

static const int Debug = 0;
static bool hidStop = false;

static char get_keystroke(void);

bool hid_getStatus()
{
    return hidStop;
}

int hid_init()
{
    int ret = 0;

    while(ret <= 0) {
        // C-based example is 16C0:0480:FFAB:0200
        ret = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
        if (ret <= 0) {
            // Arduino-based example is 16C0:0486:FFAB:0200
            ret = rawhid_open(1, 0x16C0, 0x0486, 0xFFAB, 0x0200); // RawHID
            //r = rawhid_open(1, 0x16C0, 0x0476, 0xFFAB, 0x0200); // Everything
            if (ret <= 0) {
                printf("no rawhid device found\n");
                sleep(2);
            }
        }
    }
    printf("found rawhid device\n");

    // wait 10ms
    usleep(10000);
    hidStop = false;
    boost::thread thread(hid_getMsg);

    return 0;
}

int hid_getMsg()
{
    int num;
    data_t rx_data;
    int ts_start, delta_ts;             // Control the hid rx/tx loop speed

    u_int32_t ts1, ts2;                 // Help vars to compute filterspeed
    u_int32_t lastUpdate = 0;           // Used to calculate integration interval
    float delta_t = 0.0f;               // Integration interval for both filter schemes

    // Msg the uC that application is online
    boost::thread thread(hid_sendMsg, rx_data.raw, 64, 100);

    while(!hidStop) {
        ts_start = getTimestamp_usec();

        // Get Raw HID packet, set timeout to 1ms. This caps the loop speed at 1ms
        num = rawhid_recv(0, &rx_data.raw, sizeof(rx_data), 1);

        if (num < 0) {
            printf("\nerror reading, device went offline\n");
            hidStop = true;
            rawhid_close(0);
            break;
        } else if (num > 0) {
            // Pass Usb buffer to programm
#ifdef AHRS
            setQ(rx_data.num_f);
#else
            // Apply SensorFusion
            ts1 = getTimestamp_usec();
            delta_t = updateTime(lastUpdate);
            MadgwickQuaternionUpdate(
            //MahonyQuaternionUpdate(
                    rx_data.num[0], rx_data.num[1], rx_data.num[2],
                    rx_data.num[3], rx_data.num[4], rx_data.num[5],
                    rx_data.num[6], rx_data.num[7], rx_data.num[8],
                    delta_t);
            ts2 = getTimestamp_usec();
            //printf("Fusion time: %d\n", ts2 - ts1);
#endif /* AHRS */

        } else {
            if (Debug) printf("Timeout!\n");
        }

        // Send the msg back
        //boost::thread thread(hid_sendMsg, rx_data.raw, 64, 10);

        delta_ts = getTimestamp_usec() - ts_start;
        if (Debug > 1) printf("delta_rx usec: %d\n", delta_ts);
    }

    printf("Thread terminated!\n");
    return 0;
}

int hid_sendMsg(void *buf, int len, int timeout)
{
    int num = rawhid_send(0, buf, len, timeout);
    if (num > 0) {
        if (Debug) printf("sendMsg at ms: %u\n", getTimestamp_usec()/1000);
    }else if (num < 0) {
        printf("Error Sending\n");
    }
    return 0;
}

int hid_end(int num)
{
    hidStop = true;
    sleep(1);   // give the thread time to stop
    rawhid_close(num);
    return 0;
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

static char get_keystroke(void)
{
    if (_kbhit()) {
        char c = _getch();
        if (c >= 32) return c;
    }
    return 0;
}
