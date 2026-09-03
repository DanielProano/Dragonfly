#ifndef CRSF_H
#define CRSF_H

#include <stdbool.h>
#include <stdint.h>

#define CRSF_NUM_CHANNELS 16

void crsf_init(void);
void crsf_task(void);
void crsf_get_channels(int16_t *channels_out);
bool crsf_is_stale(uint32_t timeout_ms);

#endif
