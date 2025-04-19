#include "lap.h"

lap::lap(){

}

lap::lap(int lapNumber, int32_t time, float fuel,float fuelUsed)
{
    
    this->lapNumber=lapNumber;
    this->lapTime=time;
    this->fuelLevel=fuel;
    this->fuelUsed=fuelUsed;
}