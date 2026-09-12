#include "health_task.h"
#include "barometer.h"
#include "comms_protocol.h"
#include "scheduler.h"
#include "i2c.h"

#define HEALTH_POLL_INTERVAL_MS 100U

static bool health_barometer_ready;

void health_init(void) {
    health_barometer_ready = barometer_init();
}

void health_task(void) {
    for (;;) {
        barometer barometer_data;
        bool have_barometer;

        i2c_lock();
        have_barometer = health_barometer_ready && barometer_poll(&barometer_data);
        i2c_unlock();

        if (have_barometer) {
            send_telem_barometer(&barometer_data);
        }

        task_delay(HEALTH_POLL_INTERVAL_MS);
    }
}
