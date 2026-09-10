#include "esc.h"
#include "dshot.h"
#include <stdbool.h>

#define DSHOT_THROTTLE_RANGE (DSHOT_THROTTLE_MAX - DSHOT_THROTTLE_MIN)

static bool esc_armed;

void esc_init(void) {
    dshot_init();
    esc_armed = false;
}

void esc_arm(void) {
    esc_armed = true;
}

void esc_disarm(void) {
    esc_armed = false;
    esc_stop_all();
}

void esc_set_throttle(dshot_channel_t channel, uint8_t percent) {
    if (percent > 100U) {
        percent = 100U;
    }

    if (!esc_armed) {
        dshot_set_throttle(channel, 0U);
        return;
    }

    uint16_t value = (uint16_t) (DSHOT_THROTTLE_MIN + ((uint32_t) percent * DSHOT_THROTTLE_RANGE) / 100U);
    dshot_set_throttle(channel, value);
}

void esc_stop_all(void) {
    for (dshot_channel_t channel = DSHOT_CH1; channel < NUM_DSHOT_CHANNELS; channel++) {
        dshot_set_throttle(channel, 0U);
    }
}

void esc_send(void) {
    dshot_send();
}
