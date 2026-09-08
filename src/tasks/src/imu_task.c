#include "imu_task.h"
#include "mpu.h"
#include "imu_interrupt.h"
#include "comms_protocol.h"
#include "scheduler.h"

/* Telemetry to the ground station is capped independent of how often the
   sensor itself wakes this task, so raising the sensor's report rate for
   a future control loop doesn't also flood the link. */
#define IMU_TELEM_INTERVAL_MS 100U

static bool imu_ready;
static uint32_t imu_last_telem_tick;

void imu_init(void) {
    imu_interrupt_init();
    imu_ready = mpu_init();
    imu_last_telem_tick = 0;
}

void imu_task(void) {
    for (;;) {
        imu_interrupt_wait();

        if (imu_ready) {
            float ax, ay, az, gx, gy, gz, mx, my, mz;
            bool have_accel, have_gyro, have_magnitude;

            mpu_poll();

            have_accel = mpu_get_acceleration(&ax, &ay, &az);
            have_gyro = mpu_get_gyro(&gx, &gy, &gz);
            have_magnitude = mpu_get_magnitude(&mx, &my, &mz);

            if (have_accel && have_gyro && have_magnitude) {
                uint32_t now = scheduler_get_tick_count();

                if ((now - imu_last_telem_tick) >= IMU_TELEM_INTERVAL_MS) {
                    IMU imu;

                    imu.timestamp = now;
                    imu.acceleration.x = ax;
                    imu.acceleration.y = ay;
                    imu.acceleration.z = az;
                    imu.gyro.x = gx;
                    imu.gyro.y = gy;
                    imu.gyro.z = gz;
                    imu.magnitude.x = mx;
                    imu.magnitude.y = my;
                    imu.magnitude.z = mz;

                    send_telem_imu(&imu);
                    imu_last_telem_tick = now;
                }
            }
        }
    }
}
