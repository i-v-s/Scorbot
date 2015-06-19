#include "uart.h"

struct AP { const char * name, * pwd;};

struct ESPCMD
{
    const char * cmd, * result;
    ESPCMD(const char * cmd, const char * result = "OK"): cmd(cmd), result(result) {};
    ESPCMD(): cmd(0), result(0) {};
};

class ESP: public UART
{
private:
    bool busy;
    int client, size, state, timeOut;
    char var[32], val[64], * dst;
    void (* onDone)(bool Ok);
    Queue<ESPCMD, 16> cmds;
    const char * expectResult;
    int sendSize;
    const AP * aps, * currentAP;
    void onCWLAP(char * val);
    static ESPCMD * onCmd(void * obj, ESPCMD * cmd, ESPCMD * end);
    static char * parseRX(void * obj, char * text, char * end);
    unsigned int connects, ready; // Соединения и готовые к работе
    static void tickHandler(void * obj);
public:
    void espInit(const AP * list, const char * ap);
    //void query(const char * query, void (* done)(const char * res));
    void set(const char * var, const char * value, void (* done)(bool Ok));
    static const char * send(void * obj, const char * data, const char * end);
    void echo(bool e, void (* done)(bool Ok));
    void exec(const char * * list);
    ESP(): dst(var), state(0), aps(0), sendSize(0), expectResult(0), connects(0), ready(0), busy(false), timeOut(1000)
    { 
        rxOutput.set(&parseRX, this); cmds.output.set(&onCmd, this);
    };
};

extern ESP esp;
void initESP();
