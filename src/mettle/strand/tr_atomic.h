// tr_atomic.h _____________________________________________________________________________________________________________
#pragma once
 

//--------------------------------------------------------------------------------------------------------------------------------- 
// memory_order_relaxed : Each location has a total modification order and  same thread on the same memory location are not reordered with respect to the modification order 

template < typename Stor>
class Tr_Atm
{ 
    std::atomic< Stor>          m_Value;
  
public:
    Tr_Atm( const Stor &value = Stor()) 
        :  m_Value( value)
    {} 
  
    Tr_Atm( const Tr_Atm &atm) 
        :  m_Value( Stor( atm.m_Value))
    {} 
     
    Stor    Get( void) const 
    {  
        return Stor( m_Value.load( std::memory_order_acquire)); // On x86_64 every load has aqcuire semantic, so load with acquire semantic is same as one with relaxed semantic 
    }  
    
    Stor    SetOR( const Stor &t)
    {  
        m_Value.store( t, std::memory_order_release);           // x86_64 the costs are same as for simple read store.
        return t;
    }    
     
    Stor    SetSC( const Stor &t)
    {  
        m_Value.store( t, std::memory_order_seq_cst);           // as of now,we have only one memory_order_seq_cst call, after every task run.
        return t;
    } 

    Tr_Atm     &operator=( const Tr_Atm &t)
    {
        SetOR( t.Get());
        return *this;        
    }
    
    auto            Incr( void)  { return ++m_Value;  } 
    auto            Incr( const Stor &t)  { return m_Value += t;  } 

    auto            Decr( void)  { return --m_Value;  }
    auto            Decr( const Stor &t)  { return m_Value -= t;  } 

 
    Tr_Atm     &operator+=( const Stor &t) 
    {
        m_Value += t;
        return *this;
    }
    
    bool            operator<( const Tr_Atm &atm) const  { return Get() < atm.Get(); }
    bool            operator==( const Tr_Atm &atm) const  { return Get() == atm.Get(); }

    Stor    Diff( const Tr_Atm &t) 
    {
        return m_Value -t.m_Value;
    }

    Stor    Exchange( Stor expectedValue, const Stor &newValue)
    {
        Stor    storedValue = expectedValue;                    // expectedValue is what caller had assumed to be StoredValue to compute the newValue
        do {                                                    // Keep trying until storedValue in Atomic is no longer expectedValue
            if (  storedValue != expectedValue)
                return storedValue;                             // someone else changed the content of storedValue; We might be OK or need to recompute the newValue
        } while( !m_Value.compare_exchange_weak( storedValue, newValue));
        return newValue;                                        // desired value is written to memory
    }
};
  
//--------------------------------------------------------------------------------------------------------------------------------- 
 
template < typename Stor>
class Tr_NAtm
{ 
    Stor          m_Value;
  
public:
    Tr_NAtm( const Stor &value = Stor()) 
        :  m_Value( value)
    {} 
  
    Tr_NAtm( const Tr_NAtm &atm) 
        :  m_Value( Stor( atm.m_Value))
    {} 
     
    Stor        Get( void) const  {   return  m_Value;  }  
    
    Stor        Set( const Stor &t) {   m_Value = t; return t; }    
     
    Stor        SetSC( const Stor &t) { return Set( t); } 
 
    auto        Incr( void)  { return ++m_Value;  } 
    auto        Incr( const Stor &t)  { return m_Value += t;  } 

    auto        Decr( void)  { return --m_Value;  }
    auto        Decr( const Stor &t)  { return m_Value -= t;  } 

    Stor        Diff( const Tr_NAtm &t)  { return m_Value -t.m_Value;}

    Tr_NAtm     &operator+=( const Stor &t) 
    {
        m_Value += t;
        return *this;
    }
     
    Stor        Exchange( const Stor &oldValue, const Stor &newValue) { return Set( newValue); }
};

//--------------------------------------------------------------------------------------------------------------------------------- 
 
class Tr_SpinMutex
{     
    std::atomic<bool>    m_Flag; 

public:
    Tr_SpinMutex( void)  
        : m_Flag( false) 
    {} 

    void Lock( void)
    { 
        for ( uint32_t k = 1; m_Flag.exchange( true, std::memory_order_relaxed); ++k)
        {}
        /*    Tr_USeg( 0, ( k % 8) +1).Traverse( [&]( auto ) {
                _mm_pause();
            });
        */
    }

    void Unlock( void)
    { 
        m_Flag.store( false, std::memory_order_release); 
    }

    struct Guard
    {
        Tr_SpinMutex     *m_Lock;

        Guard( Tr_SpinMutex *lck)
            : m_Lock( lck)
        {
            m_Lock->Lock();
        }

        ~Guard()
        {
            m_Lock->Unlock();
        }                
    };  

    static auto *Universal( void)
    {
        static  Tr_SpinMutex s_Universal;
        return &s_Universal;
    }

template < typename Lambda, typename... Args>
    auto    Apply( const Lambda &lambda, const Args &... args)
    {
        Guard    guard( this);
        return lambda( args...);
    }
};


//--------------------------------------------------------------------------------------------------------------------------------- 
 
class Tr_StdMutex
{     
    std::mutex   m_Mutex; 

public:
    Tr_StdMutex( void)   
    {} 
 
    struct Guard : public std::lock_guard<std::mutex>
    { 

        Guard( Tr_StdMutex *lck)
            : std::lock_guard<std::mutex>( lck->m_Mutex)
        {}

        ~Guard()
        {}                
    };  

    static auto *Universal( void)
    {
        static  Tr_StdMutex s_Universal;
        return &s_Universal;
    }

template < typename Lambda, typename... Args>
    auto    Apply( const Lambda &lambda, const Args &... args)
    {
        Guard    guard( this);
        return lambda( args...);
    }
};

typedef Tr_SpinMutex Tr_Spinlock;

//--------------------------------------------------------------------------------------------------------------------------------- 
