#include "controller.h"

struct Axis
{
    TIM_TypeDef * timer;
    int pos;
} axes[6];

void initAxis(int axis, TIM_TypeDef * timer)
{
    axes[axis].timer = timer;
    timer->SMCR = 3;
    timer->CCER = 0;
    timer->ARR = 0xFFFF;
    timer->CR1 = 1; // CEN
}

int getEncPos(int axis)
{
    struct Axis * a = axes + axis;
    TIM_TypeDef * tim = a->timer;
    if(tim) return tim->CNT;
    return a->pos;
}
