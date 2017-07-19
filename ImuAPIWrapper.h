// OvrAPIWrapper.h

#ifndef _IMUAPIWRAPPER_H_
#define _IMUAPIWRAPPER_H_

extern "C" {
    int Imusensor_Init(Tcl_Interp* interp);
    int Imusensor_Unload(Tcl_Interp* interp, int flags);
}
#endif /* _IMUAPIWRAPPER_H_ */
