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

Operator ops[] = 
{
    {   add, '+', 5},
    {commit, ';', 0},
    {0}
};

Statement statements[] =
{
    {   if_st, "if"},
    {0}
};

char name[32], * nm = name;

int stack[128], * sp = stack;
int prog[128], * ip = prog;
#define labelMode 0
#define numberMode 1

char mode = 0;

char * compile(const char * code, const char * end)
{
    while(code < end)
    {
        unsigned char c = *(code++);
        if(c >= 'A' && c <= 'Z') c -= 'A' - 'a'; 
        if(c >= 'a' && c <= 'z')
        {
            *(nm++) = c;
        }
        
        if(c >= 'a'
        else if(nm != name)
        {
            
            
        }
        
    }
    return 0;
}
