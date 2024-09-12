#ifndef __PID_H
#define __PID_H

typedef struct
{
    float Kp; // 比例系数
    float Ki; // 积分系数
    float Kd; // 微分系数

    float integral;     // 积分值
    float max_integral; // 积分限幅

    float last_error; // 上次误差值

    float output;     // 输出值
    float max_output; // 输出限幅
} PID;

extern PID pid_voltage;

void PID_SetParam(PID *pid, float Kp, float Ki, float Kd, float max_integral, float max_output);
void PID_Init();
float PID_Calculate(PID *pid, float error);

#endif // __PID_H