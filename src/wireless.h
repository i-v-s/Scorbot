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
    bool parseOn;
    int client, size, state;
    char var[32], val[64], * dst;
    void (* onDone)(bool Ok);
    Queue<ESPCMD, 16> cmds;
    const char * expectResult;
    int sendSize;
    const AP * aps, * currentAP;
    void onCWLAP(char * val);
//protected:
    //virtual 
    //virtual void onTX();
    static ESPCMD * onCmd(void * obj, ESPCMD * cmd, ESPCMD * end);
    static char * parseRX(void * obj, char * text, char * end);
public:
    void espInit(const AP * list, const char * ap);
    //void query(const char * query, void (* done)(const char * res));
    void set(const char * var, const char * value, void (* done)(bool Ok));
    static const char * send(void * obj, const char * data, const char * end);
    void echo(bool e, void (* done)(bool Ok));
    void exec(const char * * list);
    void (* onConnect) (int id);
    void (* onDisconnect) (int id);
    ESP(): dst(var), state(0), aps(0), sendSize(0), expectResult(0) { rxOutput.set(&parseRX, this); cmds.output.set(&onCmd, this);};
};

extern ESP esp;
void initESP();
