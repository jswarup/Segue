// tr_console.cpp -----------------------------------------------------------------------------------------------------------------

#include "mettle/cove/tr_includes.h"
#include "mettle/cove/tr_assist.h"
#include "mettle/strand/tr_atomic.h"
#include "mettle/silo/tr_arr.h"
#include "mettle/silo/tr_freestack.h"

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

int miscTest( int argc, char *argv[])
{
    Tr_FreeStack< uint32_t, TR_UINT16_MAX>    freeStack;
    Tr_FreeCache< 256, Tr_FreeStack< uint32_t, TR_UINT16_MAX> >   freeCache;

    freeCache.SetStore( &freeStack);

    Tr_FArr< uint32_t>     testArr( 1000);

    Tr_Do::Loop( testArr.Size(), [&]( uint32_t k) {
        testArr.SetAt( k, freeCache.AllocId());
    });
    Tr_Do::Loop( testArr.Size(), [&]( uint32_t k) {
        freeCache.DiscardId( testArr.At( k) );
    });
    return 0;
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
