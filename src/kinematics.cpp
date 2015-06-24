#include "controller.h"
#include "kinematics.h"
#include <math.h>
#include "host_io.h"
#include <stdio.h>

#define R0  15 // смещение от центра платформы до плеча
#define Z0 363 // 208: высота нижнего кра€ платформы + 140: рассто€ние от нижнего кра€ до плеча + 15: фанера
#define L1 221   // –асто€ние от плеча до локт€
#define L2 221   // от локт€ до кисти
#define L3 140   // от кисти до центра между захватами
#define PI180 0.017453292519943295769
#define PI2 1.570796326794897

// ѕр€ма€ кинематика

void BCDtoRZ(RXYZ * d, float B, float C, float D)
{
    B *= PI180; C *= PI180; D *= PI180;
    d->R = R0 + L1 * sin(B) + L2 * cos(C) + L3 * sin(D);
    d->Z = Z0 + L1 * cos(B) - L2 * sin(C) - L3 * cos(D);
}

void ABCDtoRXYZ(RXYZ * d, float A, float B, float C, float D)
{
    BCDtoRZ(d, B, C, D);
    A *= PI180;
    d->X = d->R * cos(A);
    d->Y = d->R * sin(A);
}

// ќбратна€ кинематика

bool RZtoBC(ABC * abc, float R, float Z, float D)
{
    R -= R0 + L3 * sin(D);
    Z -= Z0 - L3 * cos(D);
    float L = sqrt(R * R + Z * Z);
    if(L > L1 + L2) return false;
    float a = atan2(Z, R);
#if L1 == L2
    float t = acos(L / (L1 + L2));
    float B = (PI2 - a - t) / PI180;
    float C = (t - a) / PI180;
#else
    float sq = sqrt(L1 * L1 - L2 * L2) / L;
    float B = (PI2 - a - acos((L + sq) / (2.0 * L1))) / PI180;
    float C = (a - acos((L - sq) / (2.0 * L2))) / PI180;
#endif
    abc->B = B;
    abc->C = C;
    return true;
}

bool XYZtoABC(ABC * abc, float X, float Y, float Z, float D)
{
    float R = sqrt(X * X + Y * Y);
    if(!RZtoBC(abc, R, Z, D)) return false;
    abc->A = atan2(Y, X) / PI180;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void RZpos(RXYZ * d)
{
    BCDtoRZ(d, axisB->getPos(), axisC->getPos(), axisD->getPos());
}

void RZref(RXYZ * d)
{
    BCDtoRZ(d, axisB->getRef(), axisC->getRef(), axisD->getRef());
}

void RXYZpos(RXYZ * d)
{
    RZpos(d);
    float A = axisA->getPos() * PI180;
    d->X = d->R * cos(A);
    d->Y = d->R * sin(A);
}

void RXYZref(RXYZ * d)
{
    RZref(d);
    float A = axisA->getRef() * PI180;
    d->X = d->R * cos(A);
    d->Y = d->R * sin(A);
}

bool RZmoveTo(float R, float Z)
{
    ABC abc;
    if(!RZtoBC(&abc, R, Z, axisD->getRef() * PI180)) return false;
    axisB->moveTo(abc.B);
    axisC->moveTo(abc.C);
    return true;
}

bool XYZmoveTo(float X, float Y, float Z)
{
    ABC abc;
    if(!XYZtoABC(&abc, X, Y, Z, axisD->getRef() * PI180)) return false;
    axisA->moveTo(abc.A);
    axisB->moveTo(abc.B);
    axisC->moveTo(abc.C);
    return true;
}

bool testRZmoveTo()
{
    out.log("RZmoveTo ");
    for(float D = -90; D < 90; D += 20.0)
    for(float B = -50; B < 120; B += 10.0)
    for(float C = B - 90; C < B + 90; C += 10.0)
    {
        axisB->moveTo(B);
        axisC->moveTo(C);
        axisD->moveTo(D);
        RXYZ d;
        RZref(&d);
        RZmoveTo(d.R, d.Z);
        float br = axisB->getRef();
        float cr = axisC->getRef();
        if(fabs(br - B) > 0.1) 
        { 
            out.log("B mismatch"); 
            return false;
        }
        if(fabs(cr - C) > 0.1) 
        { 
            out.log("C mismatch"); 
            return false;
        }
    }    
    return true;
}

bool testXYZtoABC()
{
    out.log("XYZtoABC ");
    for(float D =  -30; D < 190; D += 20.0)
    {
        axisD->moveTo(D);

        for(float X = -400; X < 400; X += 70.0)
        for(float Y = -400; Y < 400; Y += 70.0)
        for(float Z = -400; Z < 400; Z += 70.0)
        {
            ABC abc;
            RXYZ d;
            if(!XYZtoABC(&abc, X, Y, Z, D * PI180)) continue;
            ABCDtoRXYZ(&d, abc.A, abc.B, abc.C, D);
            if(fabs(X - d.X) > 0.5)
            {
                out.log("X mismatch"); 
                return false;
            }
            if(fabs(Y - d.Y) > 0.5)
            {
                out.log("Y mismatch"); 
                return false;
            }        
            if(fabs(Z - d.Z) > 0.5)
            {
                out.log("Z mismatch"); 
                return false;
            }        
        }
    }
    return true;
}

bool testXYZmoveTo()
{
    out.log("XYZmoveTo ");
    for(float D =  180; D < 190; D += 10.0)
    {
        axisD->moveTo(D);

        for(float X = -400; X < 400; X += 50.0)
        for(float Y = -400; Y < 400; Y += 50.0)
        for(float Z = -400; Z < 400; Z += 50.0)
        {
            if(!XYZmoveTo(X, Y, Z)) continue;
            RXYZ d;
            RXYZref(&d);
            if(fabs(X - d.X) > 0.5)
            {
                out.log("X mismatch"); 
                return false;
            }
            if(fabs(Y - d.Y) > 0.5)
            {
                out.log("Y mismatch"); 
                return false;
            }        
            if(fabs(Z - d.Z) > 0.5)
            {
                out.log("Z mismatch"); 
                return false;
            }        
        }
    }
    return true;
}

void testKinematics()
{
    out.log("\nKinematics test: ");
    testXYZtoABC();
    testRZmoveTo();
    testXYZmoveTo();
    
    out.log("ok ");
}