/*****************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Fast Search Engine (FSE) memory map description.
 ****************************************************************************/

#ifndef __FSEV2_H__
#define __FSEV2_H__

#include <inttypes.h>

#define FSE2_CHAN_MAX 32

static const uint32_t FSE2_CONTROL_ENA      = (1 << 31);  // 0=disable; 1=enable
static const uint32_t FSE2_CONTROL_ADC      = (1 << 30);  // 0=bin offset; 1=sign/magn
static const uint32_t FSE2_STATE_NXT_DOPLER = (1 << 21);
static const uint32_t FSE2_STATE_PROCESSING = (1 << 20);
static const uint32_t FSE2_STATE_SELCHAN    = (1 << 19);
static const uint32_t FSE2_STATE_WRITING    = (1 << 18);
static const uint32_t FSE2_STATE_WAIT_MS    = (1 << 17);
static const uint32_t FSE2_STATE_IDLE       = (1 << 16);

typedef struct fsev2_chan_fields {
    volatile uint32_t common;//prn, acc_ms, carr_steps, coh_ena
    volatile int32_t carr_nco_f0;
    volatile int32_t carr_nco_dlt;
    volatile int32_t carr_nco_letter;
    volatile uint32_t max;
    volatile uint32_t ind;
    volatile uint32_t noise;
    volatile int32_t dopler;
} fsev2_chan_fields;

typedef struct fsev2_map {
   fsev2_chan_fields chan[FSE2_CHAN_MAX];

   volatile uint32_t hw_id; // msec ram capacity and hw_id
   volatile uint32_t control;
   volatile uint32_t ms_marker;
   volatile uint32_t carr_nco_th;
   volatile uint32_t code_nco_th;
   volatile int32_t carr_nco_if;
   volatile uint32_t code_nco;
   uint32_t reserved;
} fsev2_map;


#endif//__FSEV2_H__
