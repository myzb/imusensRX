#ifndef _QUATERNIONFILTERS_H_
#define _QUATERNIONFILTERS_H_

void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy,
                              float gz, float mx, float my, float mz,
                              float deltat);
void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy,
                            float gz, float mx, float my, float mz,
                            float deltat);
const float * getQ();

void setQ(float *q_in);

#endif // _QUATERNIONFILTERS_H_
