#include "mx_gpio.h"

typedef struct
{
    TIM_TypeDef * timer;
    int prec; // ������ ����������������
    volatile int pos; // ������� ������� (���� �� � timer)
    int ref; // �������� �������
    int time;
    volatile int rate; // ��������
    int oldPos; // ���������� �������
    void (* forward)();
    void (* reverse)();
    void (* stop)();
} Motor;

typedef struct
{
    float (* getPos)();
    void (* moveTo)(float pos);
    float scale;
    int zero;
    int (* getSwitch)();
} Axis;

typedef struct
{
    Axis * axis;
    float pos;
} Command;

extern Command program[64];
extern Command * cmdPtr;

extern Motor motors[6];
extern Axis axes[7];
extern const char axisNames[];

#define axisA (axes + 0)
#define axisB (axes + 1)
#define axisC (axes + 2)
#define axisD (axes + 3)
#define axisE (axes + 4)
#define axisF (axes + 5)
#define axisH (axes + 6)
#define nextCmd ((Axis *) 1)

int getMotorPos(Motor * motor);
void initEncoder(Motor * motor, TIM_TypeDef * timer);
void initMotor(Motor * motor, void (* forward)(), void (* reverse)(), void (* stop)());

void moveMotor(Motor * motor, int ref);
void motorsOff(void);
void motorsOn(void);

void ctlLoop();
void zero();