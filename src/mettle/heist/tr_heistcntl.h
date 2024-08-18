// tr_heistcntl.h _____________________________________________________________________________________________________________
#pragma once
   
 
#include "mettle/silo/tr_freestack.h" 

class   Tr_HeistCntl;
class   Tr_HeistCrew;  
class   Tr_HeistRaid;  

//--------------------------------------------------------------------------------------------------------------------------------- 
// Tr_HeistTraits is the definition class. It stores all the information that is common to all sessions and job.

struct Tr_HeistTraits
{
    typedef  uint16_t       JobId;                                  // we should be fine with 16K possible threads, if more are needed 
                                                                    // better change the user algorithm design. For limited number of CPUs higher granular job are a drag
    enum {
        MxThread        = 64,                                       // Max number of threads 
        MxFnJob         = 64,                                       // Max number of Jobs a Job can enqueue.                        
        MaxJob          = TR_UINT16_MAX, 
    }; 

    typedef void                                Work( void);
    typedef std::function< Work>                WorkFn;
    typedef Tr_FreeStack< uint16_t, MaxJob>     JobStack;
    typedef Tr_FreeCache< 256, JobStack>        JobCache;
    typedef Tr_HeistCntl                        Scheme; 
    typedef Tr_Spinlock                         Spinlock;
 
    constexpr static JobId      NullJob( void) { return JobId( 0); }  
    
    struct  CoNote
    {
        uint16_t    m_SzSkip;
        uint16_t    m_SzYield;
        uint16_t    m_SzSleep; 
        
        CoNote( void)  : 
            m_SzSkip( 4), m_SzYield( 1024), m_SzSleep( 2048)
        {}
    }; 

    static  auto  *CoNoteS( void) 
    {
        static CoNote  s_CoNote;
        return &s_CoNote;
    }

    static uint32_t     SzLogicalCPU( void) 
    {
#if defined(TR_WINDOWS)
        uint32_t    count = 0;
        uint64_t    processAffinityMask = 1;
        uint64_t    systemAffinityMask = 1;

        GetProcessAffinityMask( GetCurrentProcess(), &processAffinityMask,  &systemAffinityMask);
 
        while (processAffinityMask > 0) 
        {
            if (processAffinityMask & 1) 
                count++;
            processAffinityMask >>= 1;
        }
        return count; 
#else
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
    } 
};

//--------------------------------------------------------------------------------------------------------------------------------- 
// Struct to store common data and provide common services

class Tr_HeistCntl  : public Tr_HeistTraits
{
protected:
    Tr_Atm< uint32_t>               m_StartCount;           // Count of Processing Queue started, used for startup and shutdown 
    Tr_Atm< uint32_t>               m_SzSchedJob;           // Count of cumulative scheduled jobs in Works and Queues

    Tr_Atm< uint32_t>               m_SzQueue; 
    JobStack                        m_JobStore;            // common free-pool for jobs.
    std::ostream                    *m_OStrmPtr;            // common log stream
    Tr_FArr< WorkFn>                m_WorkFns;
    Tr_FArr< Tr_Atm< JobId>>        m_SuccIds;              // the Id of job to run after this    
    Tr_FArr< Tr_Atm< uint16_t>>     m_SzPreds;              // number of jobs still to run before this job can be taken up
                                                            // the scheduler checks the Succ after this job completes and 
                                                            // queues it it the Succ does not have any Pred left to run. ie, m_SzPred == 0  
    Tr_FArr< Tr_Atm< void *>>       m_UserDatas;            // userData 
    Tr_Atm< uint32_t>               m_DumpLock;   
    Spinlock                        m_Globlock;  
    
    static inline thread_local Tr_HeistCrew    *s_CurCrew = NULL;

public:
    Tr_HeistCntl( void)
        : m_StartCount( 0), m_SzSchedJob( 0), m_SzQueue( TR_UINT32_MAX),   m_OStrmPtr( NULL), m_DumpLock( TR_UINT32_MAX)
    { 
        m_WorkFns.DoInit( MaxJob);
        m_SuccIds.DoInit( MaxJob);
        m_SzPreds.DoInit( MaxJob, TR_UINT16_MAX);
        m_UserDatas.DoInit( MaxJob, NULL);
    } 
    
    ~Tr_HeistCntl( void)
    {}
    
    
    static Tr_HeistCrew   *Crew( void) { return s_CurCrew; }
    static void           SetCrew( Tr_HeistCrew *cur) { s_CurCrew = cur; }

    std::ostream    *OStrm( void) { return m_OStrmPtr; }
    void            SetOStrm( std::ostream *pOStrm) { m_OStrmPtr = pOStrm; }  

    uint32_t        SzQueue( void) const { return m_SzQueue.Get(); } 

    JobStack        *JobStack( void) { return &m_JobStore; }  

    uint16_t        SuccId( JobId jobId) const { return m_SuccIds.At( jobId).Get(); }
    void            SetSuccId( JobId jobId, uint16_t succId) { m_SuccIds.SetAt( jobId, succId); }

    uint16_t        SzPred( JobId jobId) const { return m_SzPreds.At( jobId).Get(); }
    void            SetSzPred( JobId jobId, uint16_t szPred) { m_SzPreds.SetAt( jobId, szPred); }
    
    auto            RaiseSzPred( JobId jobId) 
    {  
        //TR_SANITY_ASSERT( SzPred( jobId) != TR_UINT16_MAX)
        return m_SzPreds.PtrAt( jobId)->Incr(); 
    }

    auto            LowerSzPred( JobId jobId) 
    { 
        //TR_SANITY_ASSERT( SzPred( jobId) != TR_UINT16_MAX)
        return m_SzPreds.PtrAt( jobId)->Decr(); 
    } 
    
    auto            RaiseSzSchedJob( void)  {   return m_SzSchedJob.Incr(); }
    auto            LowerSzSchedJob( void)  {   return m_SzSchedJob.Decr(); }
    auto            SzSchedJob( void)  {   return m_SzSchedJob.Get(); }
    
    void            *UserData( JobId jobId) const { return m_UserDatas.At( jobId).Get(); }
    void            SetUserData( JobId jobId, void *userData) { m_UserDatas.SetAt( jobId, userData); }

    void            DoWork( JobId jobId)
    {
        m_WorkFns[ jobId]();  
    }

    
    auto            *Globlock( void) { return &m_Globlock; }

    uint32_t        DumpLockExchange( uint32_t expectedValue, uint32_t newValue) 
    { 
        return m_DumpLock.Exchange( expectedValue, newValue); 
    } 

    void            DoStart( void)
    {
        m_StartCount.Incr();
        while ( m_StartCount.Get() != SzQueue())
            std::this_thread::yield(); 
    }

    void            AllocStaticJobs( JobId *jobIds, uint32_t sz)   
    { 
        Tr_Arr< JobId>      jArr( jobIds, sz);
        Tr_NAtm<uint32_t>   tSz( 0);
        auto                jStk = jArr.Stack( &tSz);
        m_JobStore.AllocBulk( &jStk); 
        Tr_USeg( 0, tSz.Get()).Traverse( [&]( uint32_t k) { 
            SetSuccId( jArr[ k], 0);
            SetSzPred( jArr[ k], 0);
        });
        return;
    }
    
    void     FillJob( JobId jobId,  const WorkFn &work)
    {
        m_WorkFns.SetAt( jobId, work);
    }

    void    FillScheduleAfterJob( JobId schedId, JobId jobId, JobId *deps, uint32_t n);

    void    AssignSucc( JobId jobId, JobId succId)
    { 
        if ( succId)
            RaiseSzPred( succId); 
        SetSuccId( jobId, succId);
    }

    void    DumpDot( std::ostream &strm)
    {
        Tr_USeg( 0, m_WorkFns.Size()).Traverse( [&]( uint16_t k) { 
            uint16_t    szPred = m_SzPreds.At( k).Get();
            if ( !m_WorkFns[ k] || ( szPred == TR_UINT16_MAX))
                return;
            strm << 'C' << k <<  " [ shape=polygon sides=4 color=" << ("midnightblue") << " label= <<FONT> ";
            strm << k  << " <BR/> "  << szPred << "<BR/>" <<  "</FONT>>];\n";  
            strm << 'C' << k << " -> " << 'C' << m_SuccIds[ k].Get() << " [color=darkcyan label=<<FONT> " << "</FONT>>] ; \n" ;	
        });
        return;
    }

    friend  bool  dbgDotChore( Scheme *scheme)
    {  
        std::ofstream       dotfile( "chore.dot"); 
        scheme->DumpDot( dotfile);   
        return true;
    }
};
 
//--------------------------------------------------------------------------------------------------------------------------------- 
