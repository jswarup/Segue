// tr_freestack.h ____________________________________________________________________________________________________________ 
#pragma once 

#include    "mettle/silo/tr_array.h" 
#include    "mettle/strand/tr_atomic.h"
#include    "mettle/strand/tr_atmutil.h"

//_____________________________________________________________________________________________________________________________
//! \class Tr_FreeStack implements a stack of DStor objects for concurrent access. With Mx be the Max number of objects to be inserted..

template <  typename DStor, uint32_t Mx>
struct Tr_FreeStack  
{       
    typedef DStor                   Stor;   

    Tr_Spinlock                     m_Lock;                                     //! SpinLock to be used for concurrent access 
    uint32_t                        m_LockedMark;                               //! Marker to ensure that a certain initial allocations are not freed.
    Tr_Atm< uint32_t>               m_StkSz;                                    //! Curent Number of entries in the Stack
    Tr_FArr< Stor>                  m_Store;                                    //! Fixed size Array of size Mx 
    
    Tr_FreeStack( void) 
        : m_LockedMark( TR_UINT32_MAX)
    {
        m_Store.DoIndicize( Mx);                                                //! Mx number of integer-sequnce 
        m_StkSz.SetOR( Mx);                                                     //! All are free to be begin
    } 

    bool        IsLocked( DStor id) const { return id > m_LockedMark; }         //! Lock-Mark

    void        SetupLockMark( void) { m_LockedMark = m_StkSz.Get() -1; }       //! Set Lock-Mark

template < typename CacheStore>
    uint32_t        AllocBulk( CacheStore *cacheStore)                              //! allocate in bulk : caller needs to SpinLock if concurrent
    {
        return m_Store.Stack( &m_StkSz).TransferTo( cacheStore);                //! Transfer to cacheStore until it is all full
    } 

template < typename CacheStore>
    void        DiscardBulk( CacheStore *cacheStore, uint32_t maxMov)           //! discard in bulk : caller needs to SpinLock if concurrent
    {
        auto    stk = m_Store.Stack( &m_StkSz);
        cacheStore->TransferTo( &stk, maxMov);                                  //! Transfer out max of maxMov number of entries from cacheStore 
    }
};

//_____________________________________________________________________________________________________________________________
//! \class Tr_FreeCache is private free-store for each thread. These service all storage locally from private free cache, 
//! Instances draws on shared FreeStack to replenish entriesin bulk or shed in bulk when private free cache is full


template < uint32_t CacheSz, typename FreeStack>
struct Tr_FreeCache
{  
    typedef  typename FreeStack::Stor           Stor; 
    typedef  typename Tr_Spinlock::Guard        Guard;
     
    
    FreeStack                   *m_FreeStore;                                   //! reference to glabal FreeStack
    Tr_Atm< uint32_t>           m_StkSz;                                        //! Current number of entries in local Stack
    Tr_FArr< Stor>              m_CacheStore;    

    Tr_FreeCache( void)
       : m_FreeStore( NULL)
    {
        m_CacheStore.DoInit( CacheSz);
    } 
    
    
    FreeStack       *GetStore( void) const { return m_FreeStore; }
    void            SetStore( FreeStack *store) { m_FreeStore = store; }

    uint32_t        SzFree( void)  const { return  m_StkSz.Get(); }
        
    uint32_t        Replenish( void)
    {
        Guard      guard( &m_FreeStore->m_Lock); 
        auto       cacheStk = m_CacheStore.Stack( &m_StkSz);
        return m_FreeStore->AllocBulk( &cacheStk); 
    }

    uint32_t        ProbeSzFree( uint32_t szExpect = 1)                         //! The number of free entries in Cache, if none try fetch.
    { 
        if ( szExpect > m_StkSz.Get())                                          
            Replenish();
        return  m_StkSz.Get();
    } 
     
    Stor            AllocId( void)                                              //! return pointer to free location
    {   
        if ( !m_StkSz.Get())                                                           //! replenish if needed
            Replenish();

        //TR_ERROR_MESSAGE( m_StkSz.Get(), ( "Too many jobs"))
        return m_StkSz.Get()  ? m_CacheStore.At(  m_StkSz.Decr()) : Stor( -1);
    } 
  
    bool    DiscardId( Stor id)                                                   //! discard object at Id
    {  
        if ( m_FreeStore->IsLocked( id))                                        //! ignore free calls to locked Id ( assert ??)
            return false;
        if ( m_StkSz.Get() == m_CacheStore.Size())
        {
            Guard   guard( &m_FreeStore->m_Lock);                               //! setup an SpinLock guard and return to buffer to  Store if overflow
            auto    stk = m_CacheStore.Stack( &m_StkSz);
            m_FreeStore->DiscardBulk( &stk, m_CacheStore.Size()/2); 
        }
        m_CacheStore.Stack( &m_StkSz).Push( id);
        return true;
    }
    
    bool    Discard( void *x) 
    {
        //memset( x, 0xCC, Store::ObjSz);
        return DiscardId( m_FreeStore->MapId( x)); 
    }   
}; 

//_____________________________________________________________________________________________________________________________
