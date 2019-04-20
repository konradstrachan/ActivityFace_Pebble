#include <pebble.h>
#include "utilities.h"

int GetRandomNumber( int nMin, int nMax )
{
    return (rand() % (nMax+1-nMin))+nMin; 
}