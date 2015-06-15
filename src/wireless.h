#include <string.h>

class UART
{
private:
    char rxData[64];
    char * volatile rxPos;
    char txData[64], * volatile txSrc, * volatile txDst, * nextOutSrc, * txStop;
    USART_TypeDef * USART;
    DMA_TypeDef * DMA;
    DMA_Channel_TypeDef * DMA_RX, * DMA_TX;
protected:
    virtual void onRX(const char * text, const char * end) = 0;
    virtual void onTX() = 0;
    inline void outEnable() { txStop = 0; if(!(DMA_TX->CCR & DMA_CCR_EN)) txComplete(); };
    inline void outDisable() { if(!txStop) txStop = txDst;}
    int uartSend(const char * text, int size);
    inline int uartSend(const char * text) { return uartSend(text, strlen(text));}
public:
    UART();
    void uartInit(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMA_RX, DMA_Channel_TypeDef * DMA_TX);
    void txComplete();
    void rxNotEmpty();
};

struct AP { const char * name, * pwd;};

class ESP: public UART
{
private:
    char state;
    bool parseOn;
    int client, size;
    char var[32], val[64], * dst;
    void (* onDone)(bool Ok);
    const char * * outList;
    int sendSize;
    const AP * aps, * currentAP;
    void onCWLAP(char * val);
protected:
    virtual void onRX(const char * text, const char * end);
    virtual void onTX();
public:
    void espInit(const AP * list, const char * ap);
    //void query(const char * query, void (* done)(const char * res));
    void set(const char * var, const char * value, void (* done)(bool Ok));
    static char * send(void * obj, char * data, int size);
    void echo(bool e, void (* done)(bool Ok));
    void exec(const char * * list);
    void (* onConnect) (int id);
    void (* onDisconnect) (int id);
    ESP();
};

extern ESP esp;
void initESP();
