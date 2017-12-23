/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Sound source interface with wav-format suppport.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ISOUND_H__
#define __DEBUGGER_COMMON_CORESERVICES_ISOUND_H__

#include <iface.h>
#include <attribute.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_SOUND = "ISound";

static const int SOUND_CHANNELS_MAX = 2;

struct SoundSampleType {
    int chan[SOUND_CHANNELS_MAX];
};

class ISound : public IFace {
 public:
    ISound() : IFace(IFACE_SOUND) {
        soundListeners_.make_list(0);
    }

    /** Update listeners with configurable bit rate 44.1 kHz for an example */
    virtual void registerSoundListener(IFace *listener) {
        AttributeType item;
        item.make_iface(listener);
        soundListeners_.add_to_list(&item);
    }

    virtual void unregisterSoundListener(IFace *listener) {
        for (unsigned i = 0; i < soundListeners_.size(); i++) {
            if (listener == soundListeners_[i].to_iface()) {
                soundListeners_.remove_from_list(i);
                return;
            }
        }
    }

    virtual double getFreqDetectedHz() = 0;

 protected:
    AttributeType soundListeners_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ISOUND_H__
