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

template <typename Left, typename Right, typename LeftMule, typename RightMule>
struct Tr_SeqMule;

template <typename Left, typename Right, typename LeftMule, typename RightMule>
struct Tr_ParMule;

struct Tr_JobMule;

//---------------------------------------------------------------------------------------------------------------------------------

struct Tr_MuleBase
{  
    enum Type {
        UnkTy,
        JobTy,
        SeqTy,
        ParTy,
    };
    static constexpr const uint32_t                     Type = Tr_MuleBase::UnkTy;
    typedef std::function< void( void)>                 WorkFn;
};

//---------------------------------------------------------------------------------------------------------------------------------

template < typename TMule>
struct Tr_Mule : public Tr_MuleBase
{ 
    typedef  TMule         Mule;    
    uint32_t               m_Ind;
    
public:
	Tr_Mule( void)  
	{}  
};

//---------------------------------------------------------------------------------------------------------------------------------

template <typename Left, typename Right, typename LeftMule = typename Left::Mule, typename RightMule = typename Right::Mule>
struct Tr_SeqMule : public  Tr_Mule< Tr_SeqMule< Left, Right, LeftMule, RightMule> >
{
    typedef  Tr_SeqMule< Left, Right, LeftMule, RightMule>      Mule;  
    
    LeftMule          m_Left;
    RightMule         m_Right;

    static constexpr const uint32_t             Type = Tr_MuleBase::SeqTy;
    static constexpr const uint32_t             SzJob = LeftMule::SzJob +RightMule::SzJob;
    
    Tr_SeqMule( const Left &left, const Right &right)
        : m_Left( left),  m_Right( right)
    {} 
    
template < typename Lambda, typename... Args>
    void    PlayChild( const Lambda &lambda, const Args &... args)
    {
    }
};

//---------------------------------------------------------------------------------------------------------------------------------

template <typename Left, typename Right, typename LeftMule = typename Left::Mule, typename RightMule = typename Right::Mule>
struct Tr_ParMule : public  Tr_Mule< Tr_ParMule< Left, Right, LeftMule, RightMule> >
{
    typedef  Tr_ParMule< Left, Right, LeftMule, RightMule>      Mule;   
    
    static constexpr const uint32_t             Type = Tr_MuleBase::ParTy;  
    static constexpr const uint32_t             SzJob = []() {
        uint32_t     sz = LeftMule::SzJob +RightMule::SzJob +1;
        if ( LeftMule::Type == Tr_MuleBase::Type )
            --sz; 
        if ( RightMule::Type == Tr_MuleBase::Type )
            --sz; 
        return sz;
    }();
    

    LeftMule          m_Left;
    RightMule         m_Right;
    
    Tr_ParMule( const Left &left, const Right &right)
        : m_Left( left),  m_Right( right)
    {} 

template < typename Lambda, typename... Args>
    void    PlayChild( const Lambda &lambda, const Args &... args) 
    {
    }
    
};
 
//---------------------------------------------------------------------------------------------------------------------------------
 

struct Tr_JobMule : public  Tr_Mule< Tr_JobMule>
{
    static constexpr const uint32_t  SzJob = 1;
    static constexpr const uint32_t                     Type = Tr_MuleBase::UnkTy;

    typedef  Tr_JobMule     Mule;   
    WorkFn                  m_WorkFn;

    Tr_JobMule( const WorkFn &workFn) : 
        m_WorkFn( workFn)
    {}  
    
template < typename Lambda, typename... Args>
    void    PlayChild( const Lambda &lambda, const Args &... args)
    {
    }
};

//---------------------------------------------------------------------------------------------------------------------------------

template < typename Left, typename Right, typename RightMule = typename Right::Mule, typename LeftMule = typename Left::Mule>
inline auto    operator>>( const Left &left, const Right &right) 
{
    return Tr_SeqMule< LeftMule, RightMule>( left, right);
} 

template < typename Right, typename RightMule = typename Right::Mule>
inline auto    operator>>( const typename RightMule::WorkFn &workFn, const Right &right) 
{
    return Tr_SeqMule< Tr_JobMule, RightMule>( Tr_JobMule( workFn), right);
}

template < typename Left, typename LeftMule = typename Left::Mule>
inline auto    operator>>( const Left &left, const  typename LeftMule::WorkFn &workFn) 
{
    return Tr_SeqMule< LeftMule, Tr_JobMule>( left, Tr_JobMule( workFn));
}

inline auto     operator>>( const Tr_JobMule::WorkFn &w1, const Tr_JobMule::WorkFn &w2)
{
    return Tr_SeqMule< Tr_JobMule, Tr_JobMule>( Tr_JobMule( w1), Tr_JobMule( w2));
}  

template < typename Left, typename Right, typename RightMule = typename Right::Mule, typename LeftMule = typename Left::Mule>
inline auto    operator||( const Left &left, const Right &right) 
{
    return Tr_ParMule< LeftMule, RightMule>( left, right);
} 

template < typename Right, typename RightMule = typename Right::Mule>
inline auto    operator||( const typename RightMule::WorkFn &workFn, const Right &r) 
{
    return Tr_ParMule< Tr_JobMule, RightMule>( Tr_JobMule( workFn), r);
}

template < typename Left, typename LeftMule = typename Left::Mule>
inline auto    operator||( const Left &l, const  typename LeftMule::WorkFn &workFn) 
{
    return Tr_ParMule<  LeftMule, Tr_JobMule>( l, Tr_JobMule( workFn));
}

inline auto     operator||( const Tr_JobMule::WorkFn &w1, const Tr_JobMule::WorkFn &w2)
{
    return Tr_ParMule< Tr_JobMule, Tr_JobMule>( Tr_JobMule( w1), Tr_JobMule( w2));
} 

//---------------------------------------------------------------------------------------------------------------------------------

auto    SortBench( void)
{
    Tr_FArr< double>     *testArr = new Tr_FArr< double>( 191);

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
    auto  t3 = [=]( void) {
        Tr_Do::Loop( testArr->Size(), [&]( uint32_t k) {
            std::cout << testArr->At( k) << " ";
        });
        std::cout << "\n";
    };

    auto    t4 = [=]( void) {
        delete testArr;
    };
    
    auto            test = t1  >> ( ( t2 || t1 || t4 ) >> t4) >> t1 >> t4;
    uint32_t        s = decltype( test)::SzJob;
    test.PlayChild( [&]( auto *node, auto *child) {
        std::cout << "A" << node << "->A" << child << ";\n";
    });     
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
        auto            tCalls = SortBench();
        auto            w1 = std::get< 1>( tCalls);
        auto            w2 = std::get< 2>( tCalls);
        Tr_HeistCrew    *crew = Tr_HeistCntl::Crew();
        uint16_t        jobId = crew->SuccId();
        jobId = crew->Construct( jobId, std::get< 3>( tCalls));
        jobId = crew->Construct( jobId, std::get< 2>( tCalls));
        jobId = crew->Construct( jobId, std::get< 1>( tCalls));
        jobId = crew->Construct( jobId, std::get< 0>( tCalls)); 
        crew->EnqueueJob( jobId);
    });
    scheduler.CurQueue()->EnqueueJob( jobId);
    scheduler.DoLaunch();
    return 0;

}


//---------------------------------------------------------------------------------------------------------------------------------
