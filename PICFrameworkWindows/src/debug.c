#include "debug.h"
#include <p18cxxx.h>

void sendValsToPins(char ra1, char ra2, char ra3){
    if(ra1)
        RA1 = ra1;
    if(ra2)
        RA2 = ra2;
    if(ra3)
        RA3 = ra3;
}