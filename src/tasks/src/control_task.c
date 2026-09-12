#include "control_task.h"
#include "comms_protocol.h"
#include "scheduler.h"
#include "gpio.h"
#include "uart.h"
#include "crsf.h"
#include "attitude.h"
#include "motor_mix.h"
#include "mpu.h"
#include "esc.h"
#include <stdbool.h>
#include <string.h>

#define CONTROL_LOOP_INTERVAL_MS  10U
#define HEARTBEAT_INTERVAL_MS     500U
#define CRSF_FAILSAFE_TIMEOUT_MS  300U

/* Assumes a standard AETR channel order (Aileron/roll, Elevator/pitch,
   Throttle, Rudder/yaw) with a 2-position arm switch on AUX1 - verify
   against your transmitter's actual channel output order. */
#define CH_ROLL     0
#define CH_PITCH    1
#define CH_THROTTLE 2
#define CH_YAW      3
#define CH_ARM      4

/* CRSF's standard 11-bit channel endpoints. */
#define CRSF_RAW_MIN 172.0f
#define CRSF_RAW_MID 992.0f
#define CRSF_RAW_MAX 1811.0f

#define CRSF_ARM_THRESHOLD    1500
#define THROTTLE_ARM_MAX_RAW  300

#define MAX_ANGLE_DEG    30.0f
#define MAX_YAW_RATE_DPS 180.0f

/* Impact: normal flight, even aggressive acro, never approaches this -
   it only fires on an actual collision. Idle timeout: catches "armed
   and sitting still" (landed, forgotten, or crashed with throttle
   already at zero) without touching gyro/orientation, so it can't
   confuse a calm hover for a crash. */
#define IMPACT_ACCEL_THRESHOLD_MPS2 60.0f
#define IDLE_THROTTLE_MAX_PERCENT   3U
#define IDLE_DISARM_TIMEOUT_MS      3000U

static uint8_t control_flight_state;
static uint8_t control_flight_mode;
static uint32_t control_last_heartbeat_tick;
static bool idle_timer_running;
static uint32_t idle_since_tick;

static float crsf_to_bipolar(int16_t raw) {
    float value = ((float) raw - CRSF_RAW_MID) / (CRSF_RAW_MAX - CRSF_RAW_MID);

    if (value > 1.0f) {
        value = 1.0f;
    } else if (value < -1.0f) {
        value = -1.0f;
    }

    return value;
}

static uint8_t crsf_to_throttle_percent(int16_t raw) {
    float value = ((float) raw - CRSF_RAW_MIN) / (CRSF_RAW_MAX - CRSF_RAW_MIN) * 100.0f;

    if (value < 0.0f) {
        value = 0.0f;
    } else if (value > 100.0f) {
        value = 100.0f;
    }

    return (uint8_t) value;
}

static void control_force_disarm(const char *reason) {
    control_flight_state = FLIGHT_DISARMED;
    esc_disarm();
    attitude_reset();
    idle_timer_running = false;
    send_flight_state(control_flight_state);
    send_log_string(reason);
}

static bool detect_impact(float ax, float ay, float az) {
    float magnitude_sq = (ax * ax) + (ay * ay) + (az * az);

    return magnitude_sq > (IMPACT_ACCEL_THRESHOLD_MPS2 * IMPACT_ACCEL_THRESHOLD_MPS2);
}

/* Requires throttle low at the moment of arming (standard safety gate)
   and immediately disarms on either a switch flip or a stale RC link
   (failsafe) - link loss must cut motors regardless of switch position. */
static void control_update_arming(const int16_t *channels) {
    bool switch_high = channels[CH_ARM] > CRSF_ARM_THRESHOLD;
    bool throttle_low = channels[CH_THROTTLE] < THROTTLE_ARM_MAX_RAW;
    bool link_ok = !crsf_is_stale(CRSF_FAILSAFE_TIMEOUT_MS);

    if (control_flight_state == FLIGHT_ARMED) {
        if (!link_ok) {
            control_force_disarm("rc link lost");
        } else if (!switch_high) {
            control_force_disarm("disarm switch");
        }
    } else if (switch_high && throttle_low && link_ok) {
        control_flight_state = FLIGHT_ARMED;
        esc_arm();
        send_flight_state(control_flight_state);
    }
}

/* Only meaningful while armed - checks conditions that are never valid
   during intentional flight, so they can't fight acro/freestyle maneuvers. */
static void control_check_safety_shutdown(uint8_t throttle_percent, uint32_t now) {
    float ax, ay, az;

    if (throttle_percent <= IDLE_THROTTLE_MAX_PERCENT) {
        if (!idle_timer_running) {
            idle_timer_running = true;
            idle_since_tick = now;
        } else if ((now - idle_since_tick) >= IDLE_DISARM_TIMEOUT_MS) {
            control_force_disarm("idle timeout");
            return;
        }
    } else {
        idle_timer_running = false;
    }

    if (mpu_get_acceleration(&ax, &ay, &az) && detect_impact(ax, ay, az)) {
        control_force_disarm("impact detected");
    }
}

void control_init(void) {
    control_flight_state = FLIGHT_DISARMED;
    control_flight_mode = FLIGHT_MANUAL;
    control_last_heartbeat_tick = 0;
    idle_timer_running = false;

    gpio_init_pc13();
    attitude_init();
    esc_init();
}

void control_task(void) {
    for (;;) {
        int16_t channels[CRSF_NUM_CHANNELS];
        uint32_t now = scheduler_get_tick_count();

        if ((now - control_last_heartbeat_tick) >= HEARTBEAT_INTERVAL_MS) {
            if (!uart_any_byte_seen()) {
                gpio_toggle_pc13();
            }
            control_last_heartbeat_tick = now;
        }

        crsf_get_channels(channels);
        control_update_arming(channels);

        if (control_flight_state == FLIGHT_ARMED) {
            control_check_safety_shutdown(crsf_to_throttle_percent(channels[CH_THROTTLE]), now);
        }

        if (control_flight_state == FLIGHT_ARMED) {
            attitude_state_t state = { 0 };
            bool have_quat = mpu_get_quaternion(&state.quat_w, &state.quat_x, &state.quat_y, &state.quat_z);
            bool have_gyro = mpu_get_gyro(&state.gyro_roll_dps, &state.gyro_pitch_dps, &state.gyro_yaw_dps);

            if (have_quat && have_gyro) {
                float roll_setpoint = crsf_to_bipolar(channels[CH_ROLL]) * MAX_ANGLE_DEG;
                float pitch_setpoint = crsf_to_bipolar(channels[CH_PITCH]) * MAX_ANGLE_DEG;
                float yaw_rate_setpoint = crsf_to_bipolar(channels[CH_YAW]) * MAX_YAW_RATE_DPS;
                uint8_t throttle_percent = crsf_to_throttle_percent(channels[CH_THROTTLE]);
                float dt = (float) CONTROL_LOOP_INTERVAL_MS / 1000.0f;
                attitude_output_t correction = attitude_update(
                    true, roll_setpoint, pitch_setpoint, yaw_rate_setpoint, &state, dt);
                motor_outputs_t motors = motor_mix_compute(
                    throttle_percent, correction.roll, correction.pitch, correction.yaw);

                esc_set_throttle(DSHOT_CH1, motors.motors[0]);
                esc_set_throttle(DSHOT_CH2, motors.motors[1]);
                esc_set_throttle(DSHOT_CH3, motors.motors[2]);
                esc_set_throttle(DSHOT_CH4, motors.motors[3]);
            } else {
                esc_stop_all();
            }
        } else {
            esc_stop_all();
        }

        esc_send();

        task_delay(CONTROL_LOOP_INTERVAL_MS);
    }
}

void control_enqueue_command(const frame *frame) {
    /* Temporary diagnostic: rapid-flicker the heartbeat LED so a
       command's arrival at the STM32 is visible independent of
       whether its response makes it back over the link. */
    for (int i = 0; i < 6; i++) {
        gpio_toggle_pc13();
        task_delay(50);
    }

    send_ack(frame->sequence);

    char *msg = "STM32 Printed";
    send_esp32_oled_print(msg, strlen(msg));
}

uint8_t control_get_flight_state(void) {
    return control_flight_state;
}

uint8_t control_get_flight_mode(void) {
    return control_flight_mode;
}
