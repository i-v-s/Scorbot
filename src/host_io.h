
class Circular
{
protected:
    char data[256];
    char * volatile src, * volatile dst, * volatile newDst;
    //char outState = 1;
    char * (* receiver)(void * obj, char * start, int size);
    void * receiverObj;
public:
    inline int length() {char * d = dst; return (d - src) & (sizeof(data) - 1);}
    inline void setReceiver(char * (* rcv)(void * obj, char * start, int size), void * obj = 0) {receiver = rcv; receiverObj = obj;};
    Circular(): src(data), dst(data), newDst(data), receiver(0) {};
    void log(const char * text);
    void pop();
};

extern Circular out;

void onConfigured();
void enableOut();
const char * testOut();
