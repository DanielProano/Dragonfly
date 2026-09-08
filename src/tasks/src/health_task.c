#include "health_task.h"
#include "barometer.h"
#include "comms_protocol.h"
#include "scheduler.h"

#define HEALTH_POLL_INTERVAL_MS 100U

static bool health_barometer_ready;

void health_init(void) {
    health_barometer_ready = barometer_init();
}

void health_task(void) {
    for (;;) {
        BAROMETER barometer;

        if (health_barometer_ready && barometer_poll(&barometer)) {
            send_telem_barometer(&barometer);
        }

        task_delay(HEALTH_POLL_INTERVAL_MS);
    }
}
