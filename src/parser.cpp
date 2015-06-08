// ����� ������
// ���� ���������:
// ���: A B C D E F H X Y Z, ����� �� 16 ����
// �����: 123.56
// ������: 213A 14.7B
// ������: 12x 59y 14z
// ������� ����� ��������� ����� ���������� ����
// ������������� abcd345
// �������: ptp stop lin � �.�.
// ������� ����� ������ ���������
// ��������: A + B * C � �.�.
// ������: (2 + 3)

/* 
�������:
        
    ptp 213a 15b;
     ^      ^
   ������� ������

����������� ������ �� ������
    �������:
    ���������           ���
    2x 3y           [2.0] [3.0] [(1 << X)|(1 << Y)] ��������� � �����: [(1 << X)|(1 << Y)] [2.0] [3.0]
    28              [28.0] [0]                      ��������� � �����: [0] [28.0]
    a + b           [LOAD|a] [LOAD|b] [+]
    c = 2 * b       [0] [2.0] [LOAD|b] [+] [WRITE|c]
    5x 7y / 2       [(1<<X)(1<<Y)] [5] [7] [0] [2.0] [/]
    if(a) {b; c;}   [a] [IF | offset] [b] [c]

������ �������/��������
    16 ��� - ����� ���� ��� ������ ����������
    8 ���  - ������ �������/�������� ��� ������ ������������� ������
    1 ���  - ������� � ����������, ����� ������ � ����

������ ��:
    
    for(int * pc = program; *pc;)
    {
        int c = *(pc++);
        if(c < 0)
        {
            int size = (c >> 16) & 0xF;
            sp++;
            memcpy(sp, pc, size);
            sp += size;
            pc += size;
            *sp = c;
        }
        else
        {
            cmds[(c >> 16) & 0xFF](c & 0xFFFF);
        }
    switch(*(sp--))
    {
    case '+': sp[-1] += *sp; sp--;
    }

������ �������:
    '5x 7y / 2' => [5] [7] [(1<<X)(1<<Y)] [2.0] [0] [/]
                                                 ^
                                                 SP
    int * div(int * sp)
    {
        if(*(sp--) != 0) {send('����� ������ ������ �� ������'); return 0;}
        float d = 1.0 / *(float *)(sp--);
        int mask = *sp;
        for(float * p = (float *)sp - 1; mask; mask >>= 1)
            if(mask & 1) *(p--) *= d;
        return sp;
    }

*/

#include "host_io.h"

typedef struct
{
    void (* exec) ();
    int sign;
    char priority;
} Operator;

typedef struct
{
    void (* exec) ();
    const char * name;
} Statement;

/*Operator ops[] = 
{
    {   add, '+', 5},
    {commit, ';', 0},
    {0}
};

Statement statements[] =
{
    {   if_st, "if"},
    {0}
};*/


int stack[128], * sp = stack;
int prog[128], * ip = prog;
#define spaceMode 0
#define nameMode 1
#define digitMode 2
#define signMode 3

struct Parser
{
    void pushName(const char * name);
    void pushNumber(const char * text, char post);
    void pushSign(const char * sign);
} parser;

void Parser::pushName(const char * name)
{
    
    
}

void Parser::pushNumber(const char * text, char post)
{
    
}

void Parser::pushSign(const char * sign)
{
    
    
}


struct Lexer
{
    char mode;
    char buf[32], * bufp;
    char post;
    void compile(const char * code, const char * end);
    inline void onSign(char s);
    void init(Parser * parser);
    Parser * parser;
} lexer;

inline void Lexer::onSign(char s)
{
    if(s == ';' || s == ',' || s == ')' || s == '}')
    {
        if(bufp > buf) {*bufp = 0; parser->pushSign(buf); bufp = buf;}
        int t = (unsigned char) s;
        parser->pushSign((const char *)&t);
    }
    else
        *(bufp++) = s;
}

void Lexer::init(Parser * parser)
{
    bufp = buf;
    post = 0;
    mode = spaceMode;
    this->parser = parser;
}

void Lexer::compile(const char * code, const char * end)
{
    while(code < end)
    {
        unsigned char c = *(code++);
        if(c >= 'A' && c <= 'Z') c -= 'A' - 'a';
        switch(mode)
        {
        case spaceMode:
            if(c >= 'a' && c <= 'z') {mode = nameMode; break;}
            if(c >= '0' && c <= '9') {mode = digitMode; break;}
            if(c <= ' ') continue;
            mode = signMode; onSign(c); continue;
        case nameMode:
            if((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) break;
            *bufp = 0; parser->pushName(buf); bufp = buf;
            if(c <= ' ') mode = spaceMode;
            else {mode = signMode; onSign(c);}
            continue;
        case digitMode:
            if((c >= '0' && c <= '9') || c == '.') break;
            if(c >= 'a' && c <= 'z')
            {
                if(!post) post = c;
                else sendText("\nToo long axis name");
                continue;
            }
            *bufp = 0;
            parser->pushNumber(buf, post);
            bufp = buf;
            post = 0;
            if(c <= ' ') mode = spaceMode;
            else {mode = signMode; onSign(c);}
            continue;
        case signMode:
            if(c >= 'a' && c <= 'z') mode = nameMode;
            else if(c >= '0' && c < '9') mode = digitMode;
            else if(c <= ' ') mode = spaceMode;
            else { onSign(c); continue;}
            if(mode != signMode && bufp > buf) { *bufp = 0; parser->pushSign(buf); bufp = buf;}
            if(mode == spaceMode) continue;
            break;
        }
        *(bufp++) = c;
    }
}
extern "C" {
const char * parse(const char * data, const char * end)
{
    lexer.init(&parser);
    lexer.compile(data, end);
    return 0;
}
}