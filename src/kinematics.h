/* 

Оси D, E, F, H являются независимыми, то есть не управляются инверсной кинематикой
Положение по оси D влияет на расчёт кинематики, но не управляется ей.
Положения по осям A, B, C, D являются углами и могут быть преобразованы в координаты X, Y, Z

Возможны следующие преобразования:

A, B, C, D => X, Y, Z - прямая кинематика, функция RXYZpos
X, Y, Z, D => A, B, C - инверсная кинематика, функция XYZmove
[A,] B, C, D => [A,] R, Z - перевод в полярные координаты, функция RZpos
[A,] R, Z, D => [A,] B, C - перевод из полярных координат, функция RZmove


*/

struct RXYZ
{
    float R, X, Y, Z;
};

struct ABC
{
    float A, B, C;
};

bool RZtoBC(ABC * abc, float R, float Z, float D);
void RZpos(RXYZ * d);
void RXYZpos(RXYZ * d);
void RZref(RXYZ * d);
void RXYZref(RXYZ * d);

bool RZmoveTo(float R, float Z);
bool XYZmoveTo(float X, float Y, float Z);
