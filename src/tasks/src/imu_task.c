#include "imu_task.h"
#include "mpu.h"
#include "imu_interrupt.h"
#include "comms_protocol.h"
#include "scheduler.h"
#include "i2c.h"

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

            i2c_lock();
            mpu_poll();
            i2c_unlock();

            have_accel = mpu_get_acceleration(&ax, &ay, &az);
            have_gyro = mpu_get_gyro(&gx, &gy, &gz);
            have_magnitude = mpu_get_magnitude(&mx, &my, &mz);

            if (have_accel && have_gyro && have_magnitude) {
                uint32_t now = scheduler_get_tick_count();

                if ((now - imu_last_telem_tick) >= IMU_TELEM_INTERVAL_MS) {
                    imu imu_data;

                    imu_data.timestamp = now;
                    imu_data.acceleration.x = ax;
                    imu_data.acceleration.y = ay;
                    imu_data.acceleration.z = az;
                    imu_data.gyro.x = gx;
                    imu_data.gyro.y = gy;
                    imu_data.gyro.z = gz;
                    imu_data.magnitude.x = mx;
                    imu_data.magnitude.y = my;
                    imu_data.magnitude.z = mz;

                    send_telem_imu(&imu_data);
                    imu_last_telem_tick = now;
                }
            }
        }
    }
}
