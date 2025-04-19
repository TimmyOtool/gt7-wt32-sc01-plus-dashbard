#ifndef LAP_H
#define LAP_H

#include <inttypes.h>

class lap
{
public:
    int lapNumber;
    int32_t lapTime;
    float fuelLevel;
    float fuelUsed;

    lap();
    lap(int lapNumber, int32_t time, float fuel,float fuelUsed);
};
#endif
