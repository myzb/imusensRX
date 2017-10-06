/*
 * utils.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: may
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "utils.h"
#include "quaternionFilters.h"

// Calculate the time the last update took for use in the quaternion filters
float updateTime(uint32_t &lastUpdate)
{
    struct timeval tv;
    uint32_t now;
    float deltat;

    gettimeofday(&tv, NULL);
    //printf("\n%ld.%06ld\n", tv.tv_sec, tv.tv_usec);

    now = 1000000 * tv.tv_sec + tv.tv_usec;

    // Set integration time by time elapsed since last filter update
    deltat = ((now - lastUpdate) / 1000000.0f);
    lastUpdate = now;

    return deltat;
}

void getEulers()
{
	float yaw, pitch, roll;

    yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ() *
                  *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1) * *(getQ()+1)
                  - *(getQ()+2) * *(getQ()+2) - *(getQ()+3) * *(getQ()+3));
    pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() *
                  *(getQ()+2)));
    roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2) *
                  *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1) * *(getQ()+1)
                  - *(getQ()+2) * *(getQ()+2) + *(getQ()+3) * *(getQ()+3));

    pitch *= RAD_TO_DEG;
    yaw   *= RAD_TO_DEG;
    yaw   += 2.23f; 				// Declination at work is 2.23 deg on 2017-06-06
    if(yaw < 0) yaw   += 360.0f; 	// Ensure yaw stays between 0 and 360
    roll  *= RAD_TO_DEG;

    printf("Yaw\t\t Pitch\t\t Roll\n");
    printf("%f\t %f\t %f\n", yaw, pitch, roll);
}

int getTimestamp_usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (1000000 * tv.tv_sec + tv.tv_usec);
}

