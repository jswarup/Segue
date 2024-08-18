// tr_console.cpp -----------------------------------------------------------------------------------------------------------------

#include "mettle/cove/tr_includes.h"
#include "mettle/cove/tr_assist.h"
#include "mettle/heist/tr_atomic.h"
#include "mettle/silo/tr_arr.h"
#include "mettle/silo/tr_freestack.h"
#include "mettle/heist/tr_heistraid.h"

//---------------------------------------------------------------------------------------------------------------------------------

int main( int argc, char *argv[])
{    
    if ( false && ( argc < 2))
    {
        std::cout << TR_VERSION << "\n"; 
        return -1;
    } 
    int                 ret = 0;
    extern int heistTest( int argc, char *argv[]);
    heistTest( argc, argv);
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

auto    SortBench( void)
{
    Tr_FArr< double>     *testArr = new Tr_FArr< double>( 91);

    auto    t1 = [=]( void) {
        Tr_Do::Loop( testArr->Size(), [&]( uint32_t k) {
            testArr->SetAt( k, std::rand());
        });
    };

    auto    t2 = [=]( void) {
        Tr_Seg( 0U, testArr->Size()).QSort( [&]( uint32_t i1, uint32_t i2) {
            return testArr->At( i1)  < testArr->At( i2);
        },
        [&]( uint32_t i1, uint32_t i2) {
            return testArr->SwapAt( i1, i2);
        }); 
    };
    auto    t3 = [=]( void) {
        Tr_Do::Loop( testArr->Size(), [&]( uint32_t k) {
            std::cout << testArr->At( k) << " ";
        });
        std::cout << "\n";
    };

    auto    t4 = [=]( void) {
        delete testArr;
    };

    return std::make_tuple( t1, t2, t3, t4);;
}

//---------------------------------------------------------------------------------------------------------------------------------

int sortTest( int argc, char *argv[])
{
    auto    tCalls = SortBench();
    std::get< 0>( tCalls)();
    std::get< 1>( tCalls)();
    std::get< 2>( tCalls)();
    std::get< 3>( tCalls)();
    return 0;
}
 
//---------------------------------------------------------------------------------------------------------------------------------

int  heistTest( int argc, char *argv[])
{
    
    Tr_HeistRaid       scheduler( 4);
    scheduler.SetOStrm( &std::cout);
    scheduler.DoInit(); 

    auto                *queue = scheduler.CurQueue();
    uint16_t            jobId = scheduler.NullJob(); 
    
    jobId =  queue->Construct( jobId, [=]( void) { 
        auto    tCalls = SortBench();
        Tr_HeistCrew *crew = Tr_HeistCntl::Crew();
        uint16_t jobId = crew->SuccId();
        jobId = crew->Construct( jobId, [=]( void) { 
            std::get< 3>( tCalls)();
        });
        jobId = crew->Construct( jobId, [=]( void) { 
            std::get< 2>( tCalls)();
        });
        jobId = crew->Construct( jobId, [=]( void) { 
            std::get< 1>( tCalls)();
        });
        jobId = crew->Construct( jobId, [=]( void) { 
            std::get< 0>( tCalls)();
        });
        crew->EnqueueJob( jobId);
    });
    scheduler.CurQueue()->EnqueueJob( jobId);
    scheduler.DoLaunch();
    return 0;

}


//---------------------------------------------------------------------------------------------------------------------------------
