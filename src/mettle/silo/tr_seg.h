// cy_seg.h ---------------------------------------------------------------------------------------------------------------------
#pragma once
 
template < typename X>
struct  Tr_Dual : public std::pair< X, X> 
{
    typedef std::pair< X, X>    Base;

    Tr_Dual( const X &d1 = X(), const X &d2 = X())
        : Base( d1, d2)
    {}
 
    Tr_Dual( const Base &data)
        : Base( data)
    {}
    
    Tr_Dual( Base &&data)
        : Base( data)
    {}   
};


//---------------------------------------------------------------------------------------------------------------------------------
// Interpret Pair as Seg, begin and size.

template < typename X>
struct  Tr_Seg : public std::pair< X, X> 
{    
    typedef std::pair< X, X>    Base;

    Tr_Seg( const X &d1 = X( -1), const X &sz = X( 0))
        : Base( d1, sz -1)
    {}
 
    Tr_Seg( const Base &data)
        : Base( data)
    {}
    
    Tr_Seg( Base &&data)
        : Base( data)
    {} 

    auto        Size( void) const { return this->second +1; } 
    auto        First( void) const { return this->first; } 
    auto        Mid( void) const { return this->First() +this->Size()/2; } 
    auto        Last( void) const { return this->First() +this->Size() -1; }

    auto        SnapAt( const X &val) const 
    { 
        Tr_Seg   f( this->First(), val -this->First()); 
        Tr_Seg   s( val, this->First() +this->Size() -val);
        return Tr_Dual< Tr_Seg>( f, s);
    }

    bool        Within( const X &val) const 
    {
        if ( val < this->First())
            return false;
        return ( val - this->First()) < this->Size();
    }

template < typename Lambda, typename... Args>   
    void        Traverse( const Lambda &lambda, Args &&... args) const 
    { 
        for ( uint32_t i = 0; i < this->second; ++i) 
            lambda(  i +this->first, args...); 
        return;
    }  

template < typename Lambda, typename... Args>   
    void        RevTraverse( const Lambda &lambda, Args &&... args) const 
    { 
        for ( uint32_t i = 0; i < this->second; ++i) 
            lambda(  this->second -i -1 +this->first, args...); 
        return;
    }  

template < typename LessAt> 
    auto        LowerBound( const LessAt &lessAt, const X &x)
    { 
        auto    l = this->First();
        auto    h = this->First() +this->Size();  
        while ( l < h) 
        {
            X   mid =  (l + h)/2;
            if ( lessAt( mid, x))
                l = mid + 1;
            else
                h = mid;
        }
        return l;
    }

template < typename LessAt> 
    auto    UpperBound( const X &x, const LessAt &lessAt)
    { 
        auto    l = this->First();
        auto    h = this->First() +this->Size();  
        while ( l < h) 
        {
            auto    mid =  (l + h)/2;
            if ( lessAt( x, mid))
                h = mid;
            else
                l = mid + 1;
        }
        return l;
    }

template < typename LessAt, typename SwapAt> 
    struct SortOps
    { 
        LessAt  m_LessAt;
        SwapAt  m_SwapAt;

        SortOps( const LessAt &lessAt, const SwapAt &swapAt) :
            m_LessAt( lessAt), m_SwapAt( swapAt)
        {}
        
        auto    QSortPartition( X  i, X  j, X  piv) const
        { 
            while ( true)
            {  
                while ( !( m_LessAt( piv, i)) && (i < piv)) 
                    i++;
                while ( !( m_LessAt( j, piv)) && ( j > piv))
                    j--;
                if ( i == piv && j == piv)
                    return piv;
                m_SwapAt( i, j);
                if ( i == piv)
                    piv = j;
                else if ( j == piv)
                    piv = i;
            }
            TR_ERROR_ASSERT( false)
        }     

template < typename Segs>
        void    QSort( Segs *pSegs) const
        {    
            while ( pSegs->size())
            {
                auto        *pIntvl = &pSegs->back();
                auto        pivotIndex = ( pIntvl->First() + pIntvl->Size())/2;
                auto        p = QSortPartition( pIntvl->First(), pIntvl->Size(), pivotIndex);
                auto        curIntvl = *pIntvl;
                if ( p > curIntvl.First()) 
                {
                    pIntvl->SetSize( p);
                    pIntvl = NULL;
                }    
                if ( ++p < curIntvl.Size())
                {
                    if ( pIntvl)
                    {
                        pIntvl->SetFirst( p);
                        pIntvl = NULL;
                    } else
                        pSegs->emplace_back( Tr_Dual< X>( p, curIntvl.Size()));
                }
                if ( pIntvl)
                    pSegs->pop_back();
            } 
        }  
    }; 

template < typename LessAt, typename SwapAt> 
    uint16_t    QSort( const LessAt &lessAt, const SwapAt &swapAt, uint16_t succId = 0, class Tr_ChoreQueue *queue = NULL) 
    {   
        if ( !this->Size())
            return succId;

        std::vector< Tr_Dual< X>>   pendingSegs;
        pendingSegs.reserve( 1024);
        pendingSegs.emplace_back( Tr_Seg< X>( this->First(), this->Last())); 
        SortOps< LessAt, SwapAt>( lessAt, swapAt).QSort( &pendingSegs);
        return 0;
    } 
};

//---------------------------------------------------------------------------------------------------------------------------------

typedef  Tr_Seg< uint32_t>     Tr_USeg;

//---------------------------------------------------------------------------------------------------------------------------------
