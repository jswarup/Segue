// tr_includes.h -------------------------------------------------------------------------------------------------------------------

#include        <array>
#include        <vector>
#include        <iostream>

#include        <thread>
#include        <atomic>
#include        <mutex>
#include        <algorithm> 

//---------------------------------------------------------------------------------------------------------------------------------

#define SELF        (*this)


#define     TR_UINT8_MAX        uint8_t( -1)
#define     TR_UINT16_MAX       uint16_t( -1)
#define     TR_UINT32_MAX       uint32_t( -1)
#define     TR_UINT64_MAX       uint64_t( -1)
#define     TR_UINT32_SUBMAX    ( uint32_t( -1) -1)
        
#define     TR_INT8_MIN         int8_t( 0x80)    
#define     TR_INT8_MAX         int8_t( 0x7F)
         
#define     TR_INT16_MIN        int16_t( 0x8000)
#define     TR_INT16_MAX        int16_t( 0x7FFF) 
         
#define     TR_INT32_MIN        int32_t( 0x80000000)
#define     TR_INT32_MAX        int32_t( 0x7FFFFFFF)
        
#define     TR_INT64_MIN        int64_t( 0x8000000000000000)
#define     TR_INT64_MAX        int64_t( 0x7FFFFFFFFFFFFFFF)

#define     TR_DOUBLE_MAX       std::numeric_limits<double>::max()

#define     TR_LOWMASK( x)	    (( uint64_t (0x1) << ( x)) -1)


#ifdef TR_WINDOWS
#define TR_POPEN            _popen 
#define TR_PCLOSE           _pclose
#else

#define TR_POPEN            popen
#define TR_PCLOSE           pclose
#endif
#define TR_ERROR_ASSERT( X)     



//---------------------------------------------------------------------------------------------------------------------------------
