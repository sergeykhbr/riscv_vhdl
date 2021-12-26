#pragma once

#include <inttypes.h>


void init_qspi();

uint8_t sd_get_byte();
void sd_set_byte(uint8_t v);
void sd_read_block(uint8_t *buf, int sz);
int sd_start_reading(uint64_t addr);
int sd_stop_reading();
