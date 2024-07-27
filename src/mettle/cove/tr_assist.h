// tr_assist.h ----------------------------------------------------------------------------------------------------------------------

#include    <array>

//---------------------------------------------------------------------------------------------------------------------------------

struct Tr_Do
{    
template < uint32_t  Sz, typename fake = void>
    struct LoopAssist : public Tr_Do::LoopAssist< Sz-1>
    {
        typedef LoopAssist< Sz-1, fake>    Base; 

    template < typename Lambda, typename... Args>
        static void    Apply( const Lambda &lambda,  const Args&... args)  
        {
            lambda( Sz -1, args...); 
            Base::Apply( lambda, args...);
        }
    };
        
template < typename fake>
    struct  LoopAssist< 1, fake>
    {     
    template < typename Lambda, typename... Args>
        static void    Apply( const Lambda &lambda,  const Args&... args)  
        {
            lambda( 0, args...); 
        }
    };

    //  For number of iterations known at compile-time, the loop can be unrolled.
template < uint32_t Sz, typename Lambda, typename... Args>
    static void    Loop( const Lambda &lambda,  const Args&... args)  
    {
        LoopAssist< Sz>::Apply(  lambda, args...);
    }

    //  For number of iterations known only at run-time
template < typename Lambda, typename... Args>
    static void    Loop( uint32_t sz, const Lambda &lambda,  const Args&... args)  
    {
        for ( uint32_t i = 0; i < sz; ++i)
            lambda( i, args...); 
    }

template < typename Lambda, typename... Args>
    static uint32_t     ShEx( const std::string &str, const Lambda &lambda,  const Args&... args)  
    { 
        std::array< char, 1024>     buf; 

        FILE            *pipe = TR_POPEN( str.c_str(), "r");
        if ( !pipe) 
            return TR_UINT32_MAX;
        
        uint32_t    szRead = 0;
        try {
            for ( uint32_t    szByte = 0; ( szByte = ( uint32_t) fread( &buf[ 0], sizeof( buf[ 0]), sizeof( buf), pipe)) != 0; szRead += szByte) 
                if ( !lambda( &buf[ 0], szByte))
                    break; 
        } catch (...) 
        {
            TR_PCLOSE( pipe);
            throw;
        }  
        return szRead;
    }

    static uint32_t     ShEx( const std::string &str, std::ostream &ostrm) 
    {
        return ShEx(   str, [&]( char *buf, uint32_t sz)  { 
            ostrm.write( buf, sz);
            return true;
        }); 
    }
};

//---------------------------------------------------------------------------------------------------------------------------------
