// tr_console.cpp -----------------------------------------------------------------------------------------------------------------

#include "mettle/cove/tr_includes.h"
#include "mettle/strand/tr_atomic.h"
#include "mettle/silo/tr_arr.h"

//---------------------------------------------------------------------------------------------------------------------------------

int main( int argc, char *argv[])
{    
    if ( argc < 2)
    {
        std::cout << TR_VERSION << "\n"; 
        return -1;
    } 
    int     ret = 0;
    Tr_Arr< double>     test;

    if ( ret < 0)
        std::cout << "No Applet Matched\n"; 
    return ret;
}

//---------------------------------------------------------------------------------------------------------------------------------
