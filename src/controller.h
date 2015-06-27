#include "mx_gpio.h"

#define LEFT_MAX 10000000
#define LEFT_PREC 100

typedef struct
{
//private:
    volatile int pos; // ������� ������� (���� �� � timer)
    int oldPos; // ���������� ������� (������ ��� ������� ��������)
    TIM_TypeDef * timer;
//public:
    int prec; // ������ ����������������
    int ref; // �������� �������
    int rDelta; // �������� ������� ����� ����� � ������ ��������
    int time, state;
    volatile int rate; // ��������
    void (* forward)();
    void (* reverse)();
    void (* stop)();
    int control(int l);
    inline int getPos() { return timer ? (int16_t)timer->CNT : pos;}
    inline int left() { return (ref - getPos()) * rDelta;}
    void moveTo(int to);
} Motor;

typedef struct
{
    float (* getPos)();
    float (* getRef)();
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
extern Command * volatile cmdPtr;
extern int doCount;

extern Motor motors[6];
extern Axis axes[11];
extern const char axisNames[];
extern volatile int ticks;

#define axisA (axes + 0)
#define axisB (axes + 1)
#define axisC (axes + 2)
#define axisD (axes + 3)
#define axisE (axes + 4)
#define axisF (axes + 5)
#define axisH (axes + 6)
#define axisR (axes + 7)
#define axisX (axes + 8)
#define axisY (axes + 9)
#define axisZ (axes + 10)
#define ptpCmd ((Axis *) 1)
#define linCmd ((Axis *) 2)
#define endCmd ((Axis *) 3)

void initEncoder(Motor * motor, TIM_TypeDef * timer);
void initMotor(Motor * motor, void (* forward)(), void (* reverse)(), void (* stop)());
void setMotorPos(Motor * motor, int pos);

#ifdef __cplusplus
 extern "C" {
#endif
void addTickHandler(void (* h) (void * obj), void * obj = 0);
#ifdef __cplusplus
 }
#endif

void moveMotor(Motor * motor, int ref);
void motorsOff(void);
void motorsOn(void);

void ctlLoop();
char zero(char noZero);
bool pushCommand(int type, Command * cmd);
void allStop();
