#pragma once

#include <inttypes.h>

/*
 * Satellite System unique identificator type. Avoid using enum to simplify 
 * bitfields and masks implementation.
 */
typedef uint16_t ESystem;

#define SYS_GPS          0
#define SYS_GLO          1
#define SYS_SBAS         2
#define SYS_GAL          3
#define SYS_QZSS         4
#define SYS_TOTAL        5

static const char * const SYS_NAME[] = {"GPS", "GLO", "SBAS", "GAL", "QZSS"};


#define LIGHT_SPEED       299792458.0          // Light speed [meters/sec]
#define LIGHT_SPEED_MS    (0.001*LIGHT_SPEED)    // Light speed [meters/msec]

#define GPS_L1_CARR       1575420000.0     //1575420000.0;  [Hz]
#define GPS_L2_CARR       1227600000
#define GPS_L5_CARR       1176450000

#define GLO_L1_CARR       1602000000.0
#define GLO_L1_LETTER     562500.0
#define GLO_L2_CARR       1246000000
#define GLO_L2_LETTER     437500
#define GLO_LET_TOTAL     15
#define GLO_SF_DOPLER     100

#define CONST_PI           3.14159265358979323846
#define CONST_PI_2         1.57079632679489661923


