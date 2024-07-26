// tr_arr.h _____________________________________________________________________________________________________________
#pragma once
 
#include    "mettle/silo/tr_seg.h"

//---------------------------------------------------------------------------------------------------------------------------------
 
template <   typename TStor, typename TSzType = uint32_t>
class Tr_Arr 
{
public:
    typedef  TSzType    SzType;
    typedef  TStor      Stor;

protected:
    Stor        *m_Arr;
    SzType      m_Sz;

public:
    Tr_Arr( void) : 
        m_Arr( NULL), m_Sz( 0)
    {}

    Tr_Arr( Stor *stor, SzType len) :
        m_Arr( stor), m_Sz( len)
    {}
 
    Tr_Arr( Stor *stor, Stor *end) :
        m_Arr( stor), m_Sz( SzType( end -stor))
    {}

    Tr_Arr( const Tr_Arr &carr) :
        m_Arr( carr.m_Arr), m_Sz( carr.m_Sz)
    {}


    ~Tr_Arr( void) 
    {} 

    void    Clear( void) 
    { 
        this->m_Arr = NULL;
        this->m_Sz = 0;  
    } 
 
    auto    DoSetAll( const Stor &iVal)
    {
        Tr_USeg( 0, this->m_Sz).Traverse( [&]( SzType i) { 
            this->m_Arr[ i] = iVal; 
        });
        return SELF;
    }

    auto    &DoIndicize( SzType b = 0)
    { 
        for ( SzType i = 0; i < Size(); ++i) 
            this->SetAt( i, i +b); 
        return SELF;
    }
    
    SzType          Size( void) const { return m_Sz; }
    Tr_Arr          &SetSize( SzType sz) { m_Sz = sz; return SELF; }  


    const Stor      &At( SzType i) const { return  m_Arr[ i]; }
    void            SetAt( SzType i, const Stor &x) { m_Arr[ i] = x; }  

    auto            operator[]( SzType i) const { return m_Arr[ i]; } 

    const Stor      *PtrAt( SzType k) const { return &m_Arr[ 0] +k; }
    Stor            *PtrAt( SzType k) { return &m_Arr[ 0] +k; }

    const Stor      *Begin( void) const { return &m_Arr[ 0]; }
    const Stor      *End( void) const { return &m_Arr[ 0] + m_Sz; } 
    
    Stor            *Back( void) { return &m_Arr[ 0] + m_Sz -1; }
    const Stor      *Back( void) const { return &m_Arr[ 0] + m_Sz -1; }
    
    const Stor      &First( void) const {  return *this->PtrAt( 0); }
    const Stor      &Last( void) const {  return *this->PtrAt( this->Size() -1); }

    bool            IsNull( void) const { return !this->Size() && !m_Arr; } 
    
    bool            IsValid( void) const { return !!this->Size(); }   
    
    auto            Seg( void) const { return Tr_USeg( 0, this->Size()); } 

    void            Swap( Tr_Arr< Stor, SzType> *pArr) 
    {
        std::swap( m_Arr, pArr->m_Arr);
        std::swap( m_Sz, pArr->m_Sz);
    } 
    
    void        SwapAt( SzType i0, SzType i1)
    {
        std::swap( *this->PtrAt( i0), *this->PtrAt( i1));
    }

    Tr_Arr          LSnip( SzType k)  const  {  return Tr_Arr( m_Arr +k, m_Sz -k); }
    Tr_Arr          RSnip( SzType k)  const  {  return Tr_Arr( m_Arr, m_Sz -k); }

    Tr_Arr          Subset( SzType offBegin, SzType offEnd) const { return Tr_Arr( m_Arr + offBegin, offEnd -offBegin); } 

    Tr_Arr          Skip( SzType k = 1) { return ( m_Sz > k ) ? Tr_Arr( m_Arr +k, m_Sz -k) : Tr_Arr();} 
    
    Tr_Arr          &Advance( SzType k = 1)                                                  //! remove one element from begining
    {
        m_Sz -= k;
        m_Arr += k;
        return SELF;
    }  

template < typename Lambda, typename... Args> 
    auto    &DoApply( const Lambda &lm, const Args &... args)
    { 
        for ( SzType i = 0; i < Size(); ++i) 
            lm( i, &m_Arr[ i], args...);
        return SELF;
    } 

template < typename StorType>
    SzType    DoAssign( const Tr_Arr< StorType, SzType> &arr)
    {  
        SzType    sz = std::min( this->m_Sz, arr.Size());
        Tr_USeg( 0, sz).Traverse(  [&]( SzType i) { 
            this->m_Arr[ i] = Stor( arr.At( i)); 
        }); 
        return sz;
    }

/*  
template < typename Count>
    auto    Stack( Count *pCount) { return Tr_AtmStk< Tr_Arr< Stor, SzType>, Count>( this, pCount); }
*/

template < typename Less, typename... Args>   
    auto   MinMax( const Less &less, const Args &... args) const 
    { 
        Stor    mx = At( 0);
        Stor    mn = At( 0);

        for ( SzType i = 1; i < Size(); ++i)
        {
            auto    x = At( i);;
            if ( less( x, mn, args...))
                mn = x;
            
            if ( less( mx, x, args...))
                mx = x;
        }
        return Tr_Dual< Stor>( mn, mx);
    }
    

template < typename Select, typename... Args>
    SzType  Find( const Select &select, const Args &... args)  
    { 
        for ( SzType i = 0; i < Size(); ++i) 
            if ( select( At( i), args...))
                return i;
        return CY_UINT32_MAX;
    }

template < typename Equal, typename... Args>
    SzType  Find( const Stor &elm, const Equal &equal, const Args &... args)  
    { 
        return Find( equal, elm);
    }

    SzType  Find( const Stor &elm)  
    {
        return Find( elm, [&]( auto a, auto b) { return a == b; }); 
    }

template < typename Less, typename... Args>
    auto    &DoSortAll_Depricate( const Less &less, const Args &... args)                   // sort with comparator as Lambda.
    {
        std::sort( this->PtrAt( 0), this->PtrAt( 0) +this->Size(), [&]( const Stor &a, const Stor &b) {
            return less( a, b, args...);
        });  
        return SELF;
    }
    
template < typename Less, typename... Args>
    auto        &DoStableSort( const Less &lambda, const Args &... args)             // stable sort with comparator as Lambda.
    {
        std::stable_sort( this->PtrAt( 0), this->PtrAt( 0) +this->Size(), [&]( const Stor &a, const Stor &b) {
            return lambda( a, b, args...);
        });  
        return SELF;
    }
    
template < typename Less, typename... Args>
    auto        &DoQSort( const Less &lambda, const Args &... args)                   // sort with comparator as Lambda.
    {
        Tr_USeg( 0, this->Size()).QSort( [&]( SzType i, SzType j) { 
            return lambda( this->At( i), this->At( j), args...);     
        }, [&]( SzType i, SzType j) { 
            SwapAt( i, j);
        });
        return SELF;
    }  

template < typename Less, typename... Args>    
    SzType    LowerBound( SzType it, SzType sz, const Stor &elm, const Less &less, const Args &... args) const
    {                                                                                   // lower_bound index for element elm from a sorted-array within the interval [ it, sz).
        auto        iter = std::lower_bound( this->PtrAt( 0) +it, this->PtrAt( 0) +sz, elm, [&]( const  Stor &a, const Stor &b) {
            return less( a, b, args...);
        });  
        return SzType( iter -this->PtrAt( 0));
    }    
    
template < typename Less, typename... Args>    
    SzType    UpperBound( SzType it, SzType sz, const Stor &elm, const Less &less, const Args &... args) const
    {                                                                                   // upper_bound index for element elm from a sorted-array within the interval [ it, sz).
        auto        iter = std::upper_bound( this->PtrAt( 0) +it, this->PtrAt( 0) +sz, elm, [&]( const  Stor &a, const Stor &b) {
            return less( a, b, args...);
        });  
        return SzType( iter -this->PtrAt( 0));
    } 

template < typename Less, typename... Args>
    SzType    Locate( const Stor &x, const Less &less, const Args &... args) const      // locate the first element with weak-equality [ Comparator fails for Cmp( a, b) and Cmp( b. a) ]
    {
        auto	it = LowerBound( 0, this->Size(), x, less, args...);
		if (( it == this->Size()) || less( x, this->At( it), args...))
			return -1;
		return it; 
    } 
 
template< typename Equiv>
    auto        &UniqueTrim(  const Equiv &eqiv)
    {
        auto it = std::unique( this->PtrAt( 0), this->PtrAt( 0) +this->Size(), eqiv);
        this->SetSize( SzType( it - this->PtrAt( 0)));
        return SELF;
    }

template< typename Lambda, typename... Args>
    auto        &Morph( const Lambda &xf, const Args &... args)
    {
        Tr_USeg( 0, this->Size()).Traverse( [&]( SzType i) { 
            this->SetAt( i, xf( i, this->At( i), args...)); 
        });
        return SELF;
    }

template< typename Arr, typename Lambda, typename... Args>
    auto        &Morph( const Arr &arr, const Lambda &xf, const Args &... args)
    {
        Tr_USeg( 0, this->Size()).Traverse( [&]( SzType i) { 
            this->SetAt( i, xf( i, arr.At( i), args...)); 
        });
        return SELF;
    }

template < typename Lambda, typename... Args>   
    void    Traverse( const Lambda &lambda, Args &&... args) const             // interate and call a lambda for every entry in the array : const 
    { 
        Tr_USeg( 0, this->Size()).Traverse( [&]( SzType i) { 
            lambda( i, this->At( i), args...); 
        }); 
    }    

template < typename Lambda, typename... Args>      
	bool    Validate( const Lambda &lambda, Args... args) const
	{   
        SzType    validFlg = true;
        for ( SzType i = 0; validFlg && ( i < this->Size()); ++i)
		    validFlg = lambda( i, this->At( i), args...);
		return validFlg;
	} 

template  < typename Less, typename... Args>
    bool    SortSanity( const Less &sorter, const Args &... args) const                 // test is array is sorted
    {
        for ( SzType i = 1; i < this->Size(); ++i)
            if ( !sorter( this->at( i -1), this->at( i), args...))
                return false;    
        return true;
    }

    friend std::ostream     &operator<<( std::ostream &ostr, const Tr_Arr &slice)
    {  
        SzType        sz = slice.Size();
        bool            firstFlg = true;
        for ( SzType  i = 0; i < sz; ++i)
        {  
            if ( firstFlg)
                firstFlg = false;
            else
                ostr << ", ";   
            ostr <<  slice.At( i);
        }  
        return ostr;
    }  
    
};

//---------------------------------------------------------------------------------------------------------------------------------
// Fixed Array
template <   typename TStor, typename TSzType = uint32_t>
class Tr_FArr : public Tr_Arr< TStor, TSzType> 
{
public:
    typedef  TSzType                    SzType;
    typedef  TStor                      Stor;
    typedef  Tr_Arr< Stor, SzType>      Base;
 
private:
    Tr_FArr( const Tr_FArr &) = delete;
    SzType      PushBack( const Stor &x)  = delete;

public:
    Tr_FArr( void) 
    {}

    Tr_FArr( SzType len)
    {
        DoInit( len);
    }
 
    Tr_FArr( SzType len, const Stor &iVal) 
    {
        DoInit( len, iVal);
    }
    
template < typename StorType>
    Tr_FArr( const std::initializer_list< StorType> &initList) 
    {    
        DoInit( Tr_Arr< StorType, SzType>( ( StorType *) initList.begin(), SzType( initList.size())));
    }

    Tr_FArr( const Tr_Arr< Stor, SzType> &arr)
    {
        DoInit( arr);
    }

    Tr_FArr( Tr_FArr &&arr) :
        Base( arr)
    {
        arr.m_Arr = NULL;
        arr.m_Sz = 0; 
    }
    
    ~Tr_FArr( void) 
    {
        Clear();    
    }

    Tr_FArr &operator=( Tr_FArr &&arr)
    {
        Clear();
        this->m_Arr = arr.m_Arr;
        arr.m_Arr = NULL;
        this->m_Sz = arr.m_Sz; 
        arr.m_Sz = 0; 
        return SELF;
    }
    
    auto    &Clear( void) 
    {
        if ( !this->m_Arr) 
            return SELF;
        delete [] this->m_Arr;
        Base::Clear();
        return SELF;
    } 

    auto    &DoInit( SzType sz) 
    { 
        if ( sz == this->Size())
            return SELF;
        Clear();
        this->m_Arr = new Stor[ sz];
        this->m_Sz = sz;
        return SELF;
    }
    
    auto    &DoInit( SzType sz, const Stor &iVal)
    {
        DoInit( sz);
        this->DoSetAll( iVal);
        return SELF;
    }

template < typename StorType>
    auto    &DoInit( const Tr_Arr< StorType, SzType> &arr)
    {
        DoInit( arr.Size());
        this->DoAssign( arr);
        return SELF;
    }  

    auto    &DoIndicize( SzType sz, SzType b = 0)
    {
        DoInit( sz);  
        for ( SzType i = 0; i < sz; ++i) 
            this->SetAt( i, i +b); 
        return SELF;
    }

template < typename Less, typename... Args>   
    auto    &DoSortIndicize( uint32_t sz, const Less &less, const Args &... args)
    {
        this->DoIndicize( sz).DoQSort( [&]( auto i, auto j) {
            return less(  i, j, args...);
        });       
        return SELF;;
    }

    auto    &DoInit( const Stor *stors, SzType sz)
    {
        DoInit( sz);
        Tr_USeg( 0, this->m_Sz).Traverse(  [&]( SzType i) { this->m_Arr[ i] = stors[ i]; }); 
        return SELF;
    }

    void        Swap( Tr_FArr< Stor, SzType> *pArr) 
    {
        std::swap( this->m_Arr, pArr->m_Arr);
        std::swap( this->m_Sz, pArr->m_Sz);
        return;
    }  
    
    Stor                    *ObjAt( SzType k) { return this->PtrAt( k); }

    Tr_Arr< Stor, SzType>   Arr( void) { return Tr_Arr< Stor, SzType>( this->Size() ? this->PtrAt( 0) : NULL, this->Size()); } 
    
    // the domain values are relocated to index indicated by remapIds, remaps to -1 are lost.
    uint32_t                DoRemapDom( const Tr_Arr< SzType> &remapIds, bool trimFlg = true)                   
    {  
        uint32_t                    szValid = 0;
        uint32_t                    mapIndMx = 0;
        Tr_FArr< Stor, SzType>      arr;
        arr.DoInit( this->m_Sz);
        Tr_USeg( 0, this->Size()).Traverse( [&]( SzType i) { 
            uint32_t    mapIndex = remapIds.At( i);
            if ( mapIndex == CY_UINT32_MAX)
                return;
            arr.SetAt( mapIndex, this->At(  i));
            mapIndMx = std::max( mapIndex, mapIndMx);
            ++szValid;
        });  
        szValid = mapIndMx +1;
        if ( trimFlg && ( szValid  < arr.Size()))
            arr.SetSize( szValid);
        Swap( &arr); 
        return szValid;
    } 

    // the image values are relocated to image indicated by remapIds, ones that are not remapped are lost.
    void        DoRemapImg( const Tr_Arr< SzType> &mapIds)                    
    {   
        Tr_USeg( 0, this->Size()).Traverse( [&]( SzType i) {  
            SzType    mapIndex = this->At(  i);
            SzType    image = ( mapIndex <  mapIds.Size()) ? mapIds.At( mapIndex) : CY_UINT32_MAX;
            this->SetAt( i, image);
        }); 
        return;
    } 
}; 

//_____________________________________________________________________________________________________________________________
 