/* 

��� D, E, F, H �������� ������������, �� ���� �� ����������� ��������� �����������
��������� �� ��� D ������ �� ������ ����������, �� �� ����������� ��.
��������� �� ���� A, B, C, D �������� ������ � ����� ���� ������������� � ���������� X, Y, Z

�������� ��������� ��������������:

A, B, C, D => X, Y, Z - ������ ����������, ������� RXYZpos
X, Y, Z, D => A, B, C - ��������� ����������, ������� XYZmove
[A,] B, C, D => [A,] R, Z - ������� � �������� ����������, ������� RZpos
[A,] R, Z, D => [A,] B, C - ������� �� �������� ���������, ������� RZmove


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

void testKinematics();
