#include "pid.h"

PID pid_voltage;

void PID_SetParam(PID *pid, float Kp, float Ki, float Kd, float max_integral, float max_output)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->integral = 0;
    pid->max_integral = max_integral;

    pid->last_error = 0;

    pid->output = 0;
    pid->max_output = max_output;
}

void PID_Init()
{
    PID_SetParam(&pid_voltage, 0.3f, 0.01f, 0, 10.0f, 0.2f);
}

float PID_Calculate(PID *pid, float error)
{
    // 积分累积
    pid->integral += error;
    // 积分限幅
    if(pid->integral > pid->max_integral) pid->integral = pid->max_integral;
    else if(pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    // 计算输出
    pid->output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * (error - pid->last_error);
    // 记录误差
    pid->last_error = error;
    // 输出限幅
    if(pid->output > pid->max_output) pid->output = pid->max_output;
    else if(pid->output < -pid->max_output) pid->output = -pid->max_output;
    // 输出
    return pid->output;
}