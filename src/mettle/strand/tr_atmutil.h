// tr_atmutil.h _____________________________________________________________________________________________________________
#pragma once 

//--------------------------------------------------------------------------------------------------------------------------------- 

template < typename Arr, typename Count>  
class Tr_ArrStk   
{ 
    typedef typename Arr::Stor       Stor;
    typedef typename Arr::SzType     SzType;
    
    Arr     *m_Slice;
    Count   *m_PCount;
    
public:
    Tr_ArrStk( Arr  *slice, Count   *pCount)
        : m_Slice( slice), m_PCount( pCount)
    {}
    
    SzType      Size( void) const { return m_PCount->Get(); }
    const Stor  &At( SzType i) const { return  m_Slice->At( i); }
    void        SetAt( SzType i, const Stor &x) { m_Slice->SetAt( i, x); }

    SzType      SzVoid( void)  const { return  m_Slice->Size() -this->Size(); } 


    const Stor  &Top( void)  { return this->At( m_PCount->Get() -1); }
    const Stor  &Pop( void)  { return this->At( m_PCount->Decr()); }
    SzType      Push( const Stor &x)  {  this->SetAt( m_PCount->Get(), x); return m_PCount->Incr() -1; }
    
    SzType      DecrFill( SzType sz = 1)  {  return m_PCount->Decr( sz); } 
    SzType      IncrFill( SzType sz = 1)  {  return m_PCount->Incr( sz); } 

     
template < typename Stk>
    SzType        TransferTo( Stk *arr, SzType maxMov = -1)                                               // transfer to input array
    {
        SzType    szCacheVoid = arr->SzVoid();                                                          // space in incoming Array
        SzType    szAlloc =  szCacheVoid < m_PCount->Get() ? szCacheVoid : m_PCount->Get();             // Qty to be moved.  

        if ( szAlloc > maxMov)
            szAlloc = maxMov;
        SzType    fInd = m_PCount->Get() -szAlloc;                                                      // Index in source Array
        SzType    tInd = arr->Size();                                                                   // Index in dest Array
        for ( SzType i = 0; i < szAlloc; ++i)
            arr->SetAt( tInd +i, this->At( fInd +i)); 
        arr->IncrFill(  szAlloc);                                                                          // increment the filled amount in input-array
        m_PCount->Decr( szAlloc);                                                                          // reduce the SzFill by number transferred
        return szAlloc;
    } 
};


//--------------------------------------------------------------------------------------------------------------------------------- 

class Tr_Portal
{
    std::mutex              m_mutex;                    // mutex for thread synchronization
    std::condition_variable m_cond;                     // Condition variable for signaling
  
public:
template < typename Storage, typename Lambda, typename... Args>
    auto    Store( Storage *storage, const Lambda &push, const Args &... args)
    {
        std::unique_lock<std::mutex>    lock(m_mutex);  // Acquire lock
        auto      res = push( storage, args...);
        m_cond.notify_one();                            // Notify one waiting thread         
        return res;
    }
  
template < typename Storage, typename Test, typename Lambda, typename... Args>
    auto    Retrieve( Storage *storage, const Lambda &test, const Lambda &pop, const Args &... args)
    {
        std::unique_lock<std::mutex> lock( m_mutex);    // acquire lock
        m_cond.wait( lock, test);                       // wait until test success
        return pop( storage, args...);                  // retrieve item                
    }
};

//--------------------------------------------------------------------------------------------------------------------------------- 

#define CW_FNTRACE( Y)   { Cy_Spinlock::Universal()->Apply( [ &]( const char *fnName) {  std::cout <<  fnName << Cy_Flux::ToFormat##Y << "\n"; } , __func__); }

//--------------------------------------------------------------------------------------------------------------------------------- 
