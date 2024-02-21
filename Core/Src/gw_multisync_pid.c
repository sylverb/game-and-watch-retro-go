#include "gw_multisync_pid.h"
#include <odroid_system.h>

// Initialize PID controller
void PID_Init(PIDController *pid, double setpoint, double kp, double ki, double kd, double min_output, double max_output) {
    pid->setpoint = setpoint;
    pid->prev_error = 0.0;
    pid->integral = 0.0;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->min_output = min_output;
    pid->max_output = max_output;
}
//TODO: Clamp integral as well?
// Update PID controller
double PID_Update(PIDController *pid, double point) {
    double error = pid->setpoint - point;
    double derivative = error - pid->prev_error;
    pid->integral += error;

    // Calculate control output
    double output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);

    // Update previous error
    pid->prev_error = error;

    // Clamp output value
    return MIN(MAX(output, pid->min_output), pid->max_output);
}
