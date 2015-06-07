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
