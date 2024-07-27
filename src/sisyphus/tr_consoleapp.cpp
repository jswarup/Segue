// tr_console.cpp -----------------------------------------------------------------------------------------------------------------

#include "mettle/cove/tr_includes.h"
#include "mettle/cove/tr_assist.h"
#include "mettle/strand/tr_atomic.h"
#include "mettle/silo/tr_arr.h"

//---------------------------------------------------------------------------------------------------------------------------------

int main( int argc, char *argv[])
{    
    if ( false && ( argc < 2))
    {
        std::cout << TR_VERSION << "\n"; 
        return -1;
    } 
    int                 ret = 0;
    if ( ret < 0)
        std::cout << "No Applet Matched\n"; 
    return ret;
}

//---------------------------------------------------------------------------------------------------------------------------------

int sortTest( int argc, char *argv[])
{
    Tr_FArr< double>     testArr( 91);

    Tr_Do::Loop( testArr.Size(), [&]( uint32_t k) {
        testArr.SetAt( k, std::rand());
    });

    Tr_Seg( 0U, testArr.Size()).QSort( [&]( uint32_t i1, uint32_t i2) {
        return testArr.At( i1)  < testArr.At( i2);
    },
    [&]( uint32_t i1, uint32_t i2) {
        return testArr.SwapAt( i1, i2);
    }); 

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
