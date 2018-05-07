/*****************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     GNSS Engine memory map description.
 ****************************************************************************/
#ifndef __MAP_GNSSENGINE_H__
#define __MAP_GNSSENGINE_H__

#include <inttypes.h>

static const uint32_t GNSS_FLAG_ACC_READY    = 0x80000000;
static const uint32_t GNSS_FLAG_WAS_COREPOCH = 0x40000000;
static const uint32_t GNSS_FLAG_SYMBSYNC_SET = 0x20000000;
static const uint32_t GNSS_FLAG_PHASECORR    = 0x10000000;

static const uint32_t GNSS_SETTINGS_SIGN_IF  = 0x40000000;

static const uint32_t GNSS_MISC_GPSSF_ENA    = 0x00000001;  // Glonass self-test: 0=disable; 1=enable
static const uint32_t GNSS_MISC_GLOSF_ENA    = 0x00000002;  // GPS self-test
static const uint32_t GNSS_MISC_GPS_MAGN_OFF = 0x00000004;  // 0=binary offset; 1=signed magnitude
static const uint32_t GNSS_MISC_GLO_MAGN_OFF = 0x00000008;  // 0=binary offset; 1=signed magnitude

typedef struct ReadChannel_fields {
    volatile uint32_t PrnEpochCnt;
    volatile uint32_t Flags;
    volatile uint64_t MeasCodeAcc;
    volatile uint64_t MeasCarr;
    volatile int32_t Q;
    volatile int32_t I;
    volatile int32_t QmF;
    volatile int32_t ImF;
    volatile int32_t QpF;
    volatile int32_t IpF;
    volatile int32_t QmT;
    volatile int32_t ImT;
    volatile int32_t QpT;
    volatile int32_t IpT;
} ReadChannel_fields;

typedef struct WriteChannel_fields {
    volatile uint32_t CodeNcoTh;
    volatile uint32_t CarrNcoTh;
    volatile uint32_t CodeNco;
    volatile int32_t  CodeNcoShift;
    volatile int32_t  CarrNcoIF;
    volatile int32_t  CarrNcoF0;
    volatile int32_t  DltCarrNco;
    volatile uint32_t InitPrnG2;
    volatile uint32_t Settings;
    volatile uint32_t DropSyncUseStrob;
    volatile uint32_t AccInterval;
    volatile uint32_t unused[5];
} WriteChannel_fields;

typedef struct GnssChannel_fields {
    union {
        ReadChannel_fields  r;
        WriteChannel_fields w;
    } u;
} GnssChannel_fields;

typedef struct GnssTimer_fields {
    volatile uint32_t rw_MsLength;
    volatile uint32_t r_MsCnt;
    volatile int32_t  rw_tow;
    volatile int32_t  rw_tod;
    volatile uint32_t unused[12];
} GnssTimer_fields;

typedef struct GnssNoise_fields {
    volatile uint32_t rw_Carrier;
    volatile uint32_t rw_CarrierTh;
    volatile uint32_t rw_AccLength;
    volatile uint32_t reserved;
    volatile uint32_t r_GpsAmp;
    volatile uint32_t r_GpsFiltSum;
    volatile uint32_t r_GloAmp;
    volatile uint32_t r_GloFiltSum;
    volatile uint32_t unused[8];
} GnssNoise_fields;

typedef struct GnssMisc_fields {
    volatile uint32_t Date;
    volatile uint32_t GenericChanCfg;
    volatile uint32_t CarrierNcoTh;
    volatile int32_t  CarrierNcoIF;
    volatile uint32_t SfCodeNcoTh;
    volatile uint32_t SfCodeNco;
    volatile uint32_t unused2[9];
    volatile uint32_t InputCfg;     // select selftest, change bin/off to sign/mag
} GnssMisc_fields;

typedef struct GnssEngine_map {
    GnssMisc_fields    misc;
    GnssTimer_fields   tmr;
    GnssNoise_fields   noise;
    GnssChannel_fields chn[256];    // any number.
} GnssEngine_map;

#endif  // __MAP_GNSSENGINE_H__
