#ifndef DSHOT_H
#define DSHOT_H

#include <stdint.h>

typedef enum {
    DSHOT_CH1,
    DSHOT_CH2,
    DSHOT_CH3,
    DSHOT_CH4,
    NUM_DSHOT_CHANNELS
} dshot_channel_t;

#define DSHOT_THROTTLE_MIN 48U
#define DSHOT_THROTTLE_MAX 2047U

void dshot_init(void);
void dshot_set_throttle(dshot_channel_t channel, uint16_t throttle);
void dshot_send(void);

#endif
