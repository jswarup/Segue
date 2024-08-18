// tr_heistcrew.h _____________________________________________________________________________________________________________
#pragma once
  
#include    "mettle/heist//tr_heistcntl.h"  

//--------------------------------------------------------------------------------------------------------------------------------- 
 
class Tr_HeistCrew  :  public Tr_HeistTraits
{    
    //--------------------------------------------------------------------------------------------------------------------------------- 
    typedef Tr_FArr< Tr_Atm< JobId>>           JobIdChunk;

    Scheme                  *m_Scheme;          // scheme for to all queue in the 

    Tr_NAtm< uint32_t>      m_Index;            // Index of this Queue
    Tr_NAtm< uint32_t>      m_SzJobRuns;        // Number of jobs run so far by the Queue
    Tr_NAtm< uint32_t>      m_GrabFailCount;    // Number of times attempt to grab a job from others failed.
    
    Spinlock                m_Spinlock;         // Spinlock for interaction with other Queues

    Tr_Atm< uint32_t>       m_SzQJob;           // Number of jobs ready to run (QJob) for this queue.
    JobIdChunk              m_QJobChunk;        // Chunk for storing QJobs

    Tr_Atm< uint32_t>       m_SzTQJob;          // Number of jobs ready to run (QJob) for this queue, during executions by this Queue
    JobIdChunk              m_TQJobChunk;       // Chunk for storing (QJob)  during executions by this Queue

    JobCache                m_JobCache;         // Local Free job Cache, replenished from global cache 
    std::thread             m_Thread; 
    Tr_NAtm< void *>        m_User;
    Tr_NAtm< JobId>         m_SuccId;

public:

    bool            DoInit( Scheme *scheme, uint32_t index)
    {
        m_Index = index; 
        m_Scheme = scheme; 
        m_JobCache.SetStore( scheme->JobStack());
        m_QJobChunk.DoInit( MaxJob);
        m_TQJobChunk.DoInit( MxFnJob);
        m_GrabFailCount = 0;
        scheme->SetCrew( this);
        return true;
    }  

    JobId          PopJob( void)  
    {   
        Spinlock::Guard     guard( &m_Spinlock); 
        if ( !m_SzQJob.Get())
            return Scheme::NullJob();  
        uint16_t            jobId = m_QJobChunk.Stack( &m_SzQJob).Pop().Get();
        return jobId;
    } 

    void            FlushTempJobs( void)  
    {    
        Spinlock::Guard     guard( &m_Spinlock);  
        uint32_t            szJob = m_SzQJob.Get() +m_SzTQJob.Get();
        //TR_SANITY_ASSERT( szJob < MaxJob)
        auto        jobIdStk = m_QJobChunk.Stack( &m_SzQJob);
        auto        tempIdStk = m_TQJobChunk.Stack( &m_SzTQJob);
        tempIdStk.TransferTo( &jobIdStk);
    }
    
    void            EnqueueJob( JobId jobId)  
    {    
        TR_ERROR_ASSERT( jobId)
        m_Scheme->RaiseSzSchedJob();
        if ( m_SzTQJob.Get())
            FlushTempJobs();
        auto    tempIdStk = m_TQJobChunk.Stack( &m_SzTQJob); 
        //TR_ERROR_MESSAGE( tempIdStk.SzVoid(), ( "Local Job Queue Full"))
        tempIdStk.Push( jobId); 
    }
    
    JobId           AllocJob( void) 
    {
        uint32_t    sz = m_JobCache.ProbeSzFree();         
        //TR_SANITY_ASSERT( sz && ( m_QJobChunk.Size() != m_SzQJob.Get()))         // Too many jobs..
        JobId      jobId = m_JobCache.AllocId();
        m_Scheme->SetSzPred( jobId, 0);
        return jobId;
    } 

template < typename Rogue,  typename... Args>
    auto            Construct( JobId succId,  Rogue  rogue, const Args &... args) 
    {   
        JobId       jobId = AllocJob();
        m_Scheme->FillJob( jobId, rogue, args...); 
        m_Scheme->AssignSucc( jobId, succId);
        return jobId; 
    } 
     
    Tr_HeistCrew( void)
        : m_Index( TR_UINT32_MAX), m_SzJobRuns( 0), m_Scheme(NULL), m_GrabFailCount( 0) 
    {}  

    std::ostream    *OStrm( void) { return m_Scheme->OStrm(); }
    
    Scheme          *Scheme( void) const { return m_Scheme; }
    uint32_t        SzQueue( void) const { return m_Scheme->SzQueue(); } 
    uint32_t        SzHoard( void) const { return 2 * m_Scheme->SzQueue() +1; }
 
    uint32_t        Index( void) const { return m_Index.Get(); }   
    uint32_t        SzQJob( void) const { return m_SzQJob.Get(); }  
 
    uint16_t        SuccId( void) const { return m_SuccId.Get(); }   
    
    void            *CurUserData( void) const { return m_User.Get(); }
    void            SetUserData( JobId jobId, void *userData) { m_Scheme->SetUserData( jobId, userData); }

    JobId           GrabJob( void);
    
    void            Run( JobId jobId) 
    {  
        m_SzJobRuns.Incr();                                         // update the count statistics
        m_GrabFailCount = 0;                  
        //TR_SANITY_ASSERT( !m_Scheme->SzPred( jobId))
        m_User.Set( m_Scheme->UserData( jobId));   
        m_SuccId.Set( m_Scheme->SuccId( jobId));    
        if ( m_SzTQJob.Get())
            FlushTempJobs();
        m_Scheme->DoWork( jobId, m_SuccId.Get(), this);  
        m_User.Set( NULL);  
        if ( m_SuccId.Get()) 
        {
            auto    succPred = m_Scheme->LowerSzPred( m_SuccId.Get()); 
            if ( !succPred)
                EnqueueJob( m_SuccId.Get());
        } 
        m_Scheme->SetSzPred( jobId, TR_UINT16_MAX);  
        if ( m_SzTQJob.Get())
            FlushTempJobs();  
        bool    dynFlg = m_JobCache.DiscardId( jobId);           
    }  
    
    void    DoExecute( void)
    {  
        if ( m_SzTQJob.Get())
            FlushTempJobs();
        m_Scheme->DoStart();  
        while ( m_Scheme->SzSchedJob())
        {
            JobId   jobId = PopJob();
            if ( !jobId)
                jobId = GrabJob();
            if ( !jobId)
                continue;   
            Run( jobId);   
            m_Scheme->LowerSzSchedJob(); 
        }
        //TR_DEBUG_ASSERT( (m_Scheme->DumpLockExchange( TR_UINT32_MAX, m_Index.Get()) != m_Index.Get()) || dbgDotChore( m_Scheme))

        if ( m_Scheme->OStrm()) 
        {
            Spinlock::Guard         guard( m_Scheme->Globlock()); 
            *m_Scheme->OStrm() << "Stoping " << m_Index.Get() << " Job: " << m_SzJobRuns.Get()  <<  '\n' << std::flush; 
        }
        return;
    }  

    void LaunchExecute( void)    
    {
        m_Thread  = std::thread( &Tr_HeistCrew::DoExecute, this);  
    }

    bool            DoJoin( void)
    {
        m_Thread.join();
        return true;
    }
  
}; 

//--------------------------------------------------------------------------------------------------------------------------------- 
// schedule deps after the jobId is run

inline  void   Tr_HeistCntl::FillScheduleAfterJob( JobId schedId, JobId jobId, JobId *deps, uint32_t sz)
{    
    Tr_USeg( 0, sz).Traverse( [&]( uint32_t i) {
        RaiseSzPred( deps[ i]);  
    }); 
    m_WorkFns.SetAt( schedId, [=]( JobId succ, Tr_HeistCrew *queue) {
        Tr_USeg( 0, sz).Traverse( [&]( uint32_t i) {
            uint16_t    sId = deps[ i];
            auto        sPred = queue->Scheme()->LowerSzPred( sId); 
            if ( !sPred)
                queue->EnqueueJob( sId);
        });
    }); 
    AssignSucc( jobId, schedId);
    return;
} 

//--------------------------------------------------------------------------------------------------------------------------------- 
