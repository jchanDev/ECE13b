#ifndef __STM32L476G_DISCOVERY_MOTOR_H
#define __STM32L476G_DISCOVERY_MOTOR_H

#include "stm32l476xx.h"

void Motor_Init(void);
void rotate(void);
void setDire(int8_t direction);

int turnLeft(double endX, double endY, double currentX, double currentY);
int turnRight(double endX, double endY, double currentX, double currentY);

int lessCandyRoutine(int stage, double currentX, double currentY);
int moreCandyRoutine(int stage, double currentX, double currentY);

#endif /* __STM32L476G_DISCOVERY_MOTOR_H */
