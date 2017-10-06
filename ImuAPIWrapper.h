// OvrAPIWrapper.h

#ifndef _IMUAPIWRAPPER_H_
#define _IMUAPIWRAPPER_H_

#if defined(OS_WINDOWS) && defined(BUILD_DLL)
#define SENSOR_DLL __declspec(dllexport)
#elif defined(OS_WINDOWS)
#define SENSOR_DLL __declspec(dllimport)
#else
#define SENSOR_DLL
#endif /* OS_WINDOWS */

extern "C" {
SENSOR_DLL int Imusensor_Init(Tcl_Interp* interp);
SENSOR_DLL int Imusensor_Unload(Tcl_Interp* interp, int flags);
}
#endif /* _IMUAPIWRAPPER_H_ */
