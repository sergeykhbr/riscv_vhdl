//****************************************************************************
// Entity:      GNSS channels firmware
// Contact:     chief@gnss-sensor.com
// Description:	Virtual Firmware interface for any implementation
//****************************************************************************

#ifndef __IFACE_H__
#define __IFACE_H__

class IFace {
public:
    IFace(const char *name) : face_name_(name) {}
    virtual const char *getFaceName() { return face_name_; }
private:
    const char *face_name_;
};

#endif//__IFACE_H__
