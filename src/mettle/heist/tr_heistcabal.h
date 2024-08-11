// tr_heistcabal.h _____________________________________________________________________________________________________________
#pragma once
  
#include    "mettle/heist//tr_heistcrew.h"  

//--------------------------------------------------------------------------------------------------------------------------------- 

class Tr_HeistCabal :  public Tr_HeistCntl
{    
    uint32_t                            m_MxQueue;
    uint32_t                            m_LastGrab;
    Tr_HeistCrew                       m_Queue; 
    Tr_FArr< Tr_HeistCrew *>           m_ChoreQueues; 
     
public:
    Tr_HeistCabal( uint32_t mxQueue) 
        : m_MxQueue( mxQueue), m_LastGrab( 0)
    {
        m_ChoreQueues.DoInit( m_MxQueue +1, NULL);
    } 

    ~Tr_HeistCabal( void)
    {
        for ( uint32_t i = 0; i < m_MxQueue; ++i)
            if ( m_ChoreQueues[ i])
                delete m_ChoreQueues[ i];
    }

    Tr_HeistCrew  *CurQueue( void) { return &m_Queue; }    

    bool    DoInit( void)
    {
        JobStack()->SetupLockMark();
        
        bool    flg = true; 
        for ( uint32_t i = 0; flg && ( i < m_ChoreQueues.Size()); ++i)
        {
            m_ChoreQueues.SetAt(  i, ( i < m_MxQueue) ? new Tr_HeistCrew() : &m_Queue);
            flg = m_ChoreQueues.At( i)->DoInit( this, i);
        }
        if ( !flg)
            return false;
        m_SzQueue = m_ChoreQueues.Size(); 
        return flg;
    } 
     
    JobId   GrabJob( void)   
    { 
        for ( uint32_t i = 0; i < SzQueue(); ++i, ++m_LastGrab)
        {
            Tr_HeistCrew  *queue = m_ChoreQueues.At( m_LastGrab % SzQueue()); 
            if ( queue->SzQJob())
                return queue->PopJob();
        }
        return NullJob();
    }

    // Scheduler entry point on main thread
    bool    DoLaunch( void)                                             
    {  
        if ( OStrm())
            *OStrm() << "Launch..." << '\n'<< std::flush;               
        
        for ( uint32_t i = 0; i < m_MxQueue; ++i)
             m_ChoreQueues.At( i)->LaunchExecute();                     // Launch  DoExecute for each thread
        m_Queue.DoExecute();                                            // Launch  DoExecute for main thread
        for ( uint32_t i = 0; ( i < m_MxQueue); ++i)
            m_ChoreQueues.At( i)->DoJoin();

        if ( OStrm()) 
            *OStrm() << "Over..." << '\n' << std::flush;   
        return true;
    }
}; 

//_____________________________________________________________________________________________________________________________

inline Tr_HeistCrew::JobId          Tr_HeistCrew::GrabJob( void) 
{ 
     CoNote      *coNote = CoNoteS(); 
    if ( m_GrabFailCount.Incr() > 4)
    { 
        if ( m_GrabFailCount.Get() < 4096) 
            std::this_thread::yield();
        else
        {  
            auto        duration = std::chrono::microseconds( m_GrabFailCount.Get()/16);
            std::this_thread::sleep_for( duration); 
        }
    } 
    _mm_pause();
    return static_cast < Tr_HeistCabal*>( m_Scheme)->GrabJob();
}
  
//--------------------------------------------------------------------------------------------------------------------------------- 
