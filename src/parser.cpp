// Новый парсер
// Виды выражений:
// Оси: A B C D E F H X Y Z, итого до 16 штук
// Число: 123.56
// Вектор: 213A 14.7B
// Вектор: 12x 59y 14z
// Вектора могут содержать любое количество осей
// Идентификатор abcd345
// Команды: ptp stop lin и т.п.
// Команды имеют низший приоритет
// Операции: A + B * C и т.п.
// Скобки: (2 + 3)

/* 
Примеры:
        
    ptp 213a 15b;
     ^      ^
   команда вектор

Виртуальная машина со стеком
    Примеры:
    выражение           код
    2x 3y           [2.0] [3.0] [(1 << X)|(1 << Y)] результат в стеке: [(1 << X)|(1 << Y)] [2.0] [3.0]
    28              [28.0] [0]                      результат в стеке: [0] [28.0]
    a + b           [LOAD|a] [LOAD|b] [+]
    c = 2 * b       [0] [2.0] [LOAD|b] [+] [WRITE|c]
    5x 7y / 2       [(1<<X)(1<<Y)] [5] [7] [0] [2.0] [/]
    if(a) {b; c;}   [a] [IF | offset] [b] [c]

Формат команды/операнда
    16 бит - маска осей или индекс переменной
    8 бит  - индекс команды/операции или размер сопутствующих данных
    1 бит  - команда к выполнению, иначе данные в стек

Работа ВМ:
    
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

Пример деления:
    '5x 7y / 2' => [5] [7] [(1<<X)(1<<Y)] [2.0] [0] [/]
                                                 ^
                                                 SP
    int * div(int * sp)
    {
        if(*(sp--) != 0) {send('Можно делить только на скаляр'); return 0;}
        float d = 1.0 / *(float *)(sp--);
        int mask = *sp;
        for(float * p = (float *)sp - 1; mask; mask >>= 1)
            if(mask & 1) *(p--) *= d;
        return sp;
    }

*/

#include <stdlib.h>
#include <stdio.h>
#include "host_io.h"
#include "controller.h"
    
struct Operator
{
    int sign;
    char priority;
    
    void (* exec) ();
};

typedef struct
{
    void (* exec) ();
    const char * name;
} Statement;

Operator ops[] = 
{
    {'+', 5},
    {';', 0},
    {0}
};

/*Statement statements[] =
{
    {   if_st, "if"},
    {0}
};*/

int axisIdx(char a)
{
    for(const char * p = axisNames; *p; p++)
    {
        if(*p == a) return p - axisNames;
    }
    return -1;
}

struct Parser
{
    int stack[128], * sp;
    int prog[128], * ip;
    void pushName(const char * name);
    void pushNumber(const char * text, char post);
    void pushSign(const char * sign);
    void pushOp(Operator * o);
    Parser(void);
} parser;

Parser::Parser(void): ip(prog)
{
    sp = stack - 1;
}

void Parser::pushName(const char * name)
{
    
    
}

void Parser::pushOp(Operator * o)
{
    
    
    
}


void Parser::pushNumber(const char * text, char post)
{
    int mask = 0;
    if(post)
    {
        int axis = axisIdx(post);
        if(axis >= 0) mask = 1 << axis;
        else
        {
            char buf[20];
            sprintf(buf, "\nUnknown axis %c ", post);
            sendText(buf);
        }
    }
    float num = atof(text);
    if(sp >= stack && *sp >= 0) // В стеке сверху значение - надо присоединить
    {
        
        
    }
    else // иначе просто пишем
    {
        *(float *)(++sp) = num;
        *(++sp) = mask | 0x10000;
    }
}

void Parser::pushSign(const char * sign)
{
    while(char c = *sign)
    {
        int cc = *(short *)sign;
        Operator * o = ops;
        for(; o->sign; o++)
        {
            if(o->sign == c) break;
            if(o->sign == cc) { sign++; break;}
        }
        if(o->sign) pushOp(o);
        else         
        {
            char buf[20];
            sprintf(buf, "\nUnknown symbol %c ", c);
            sendText(buf);
        }
        sign++;
    }
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
    if(s == ';' || s == ',' || s == ')' || s == '}') // Некоторые операции должны выполняться сразу
    {
        if(bufp > buf) {*bufp = 0; parser->pushSign(buf); bufp = buf;}
        int t = (unsigned char) s;
        parser->pushSign((const char *)&t);
    }
    else
        *(bufp++) = s;
}


#define spaceMode 0
#define nameMode 1
#define digitMode 2
#define signMode 3

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