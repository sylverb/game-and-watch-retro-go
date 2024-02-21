#ifndef _GW_MULTISYNC_PID_H_
#define _GW_MULTISYNC_PID_H_

// PID controller structure
typedef struct {
    double setpoint;    // Desired setpoint
    double prev_error;  // Previous error
    double integral;    // Integral of error
    double kp;          // Proportional gain
    double ki;          // Integral gain
    double kd;          // Derivative gain
    double min_output;  // Min output value
    double max_output;  // Max output value
} PIDController;

// Initialize PID controller
void PID_Init(PIDController *pid, double setpoint, double kp, double ki, double kd, double min_output, double max_output);

// Update PID controller
double PID_Update(PIDController *pid, double process_variable);

#endif
