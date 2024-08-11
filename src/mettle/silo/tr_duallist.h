// tr_duallist.h ---------------------------------------------------------------------------------------------------------------------
#pragma once
 
#include    "mettle/silo/tr_array.h" 

//__________________________________________________________________________________________________________________________

template <  typename Stor>
struct Tr_DualList  
{   
    Tr_FArr< Tr_Dual< Stor>>  m_DLinks;
    
public:
    Tr_DualList( void)
    {}
    
    static constexpr Stor    NIL( void) { return  0; }
     
    Stor        Prev( Stor dlink) const { return m_DLinks[ dlink].First(); }
    Stor        Next( Stor dlink) const { return m_DLinks[ dlink].Second(); }

    void        SetPrev( Stor dlink, Stor d) { m_DLinks.PtrAt( dlink)->SetFirst( d); }
    void        SetNext( Stor dlink, Stor d) { m_DLinks.PtrAt( dlink)->SetSecond( d); }
    
    void        Init( Stor dl)
    {
        SetPrev( dl, dl);
        SetNext( dl, dl);
    }

    void        DoInit( uint32_t szLeaves)
    {
        m_DLinks.DoInit( szLeaves); 
        Tr_USeg( 0, szLeaves).Traverse( [&]( uint32_t k) {
            Init( k);
        });
    } 

    bool        IsSingle( Stor dl) const
    {
        return ( dl == Prev( dl)) && ( dl == Next( dl));
    }
 
    bool        IsDual( Stor dl) const
    {
        return  ( Next( dl) == Prev( dl)) && ( dl !=  Next( dl));
    }

    bool        IsTrivial( Stor dl) const
    {
        return ( Next( dl) == Prev( dl));
    } 
    
    // Append s2 after s1
    void    ConnectAfter( const Stor &s1, const Stor &s2)
    { 
        Stor    oldNx1 = Next( s1);
        Stor    oldPr2 = Prev( s2);
        if ( oldPr2!= TR_UINT32_MAX)
            SetNext( oldPr2, oldNx1);
        if ( oldNx1!= TR_UINT32_MAX)
            SetPrev( oldNx1, oldPr2);
        SetPrev( s2, s1);
        SetNext( s1, s2); 
        return;
    }

    void    Reverse(  Stor head)
    {   
        Stor    dlink = head;
        do {
            SELF[ dlink].Swap();
            dlink = Prev( dlink);
        } while ( dlink != head);
        return;
    }

    bool    IsSingleton( Stor ind)
    {    
        bool    res = Prev( ind) == ind;
        TR_SANITY_ASSERT( !res || ( Prev( ind) ==  Next( ind)))
        return  res;
    }

    bool    IsDual( Stor ind)
    {    
        bool    res = ind ==  Next( Next( ind)); 
        TR_SANITY_ASSERT( !res || (( Prev( ind) == Next( ind)) && ( Prev( ind) != ind)))
        return  res;
    }
 
template <typename Lambda, typename... Args> 
    void    Traverse( const Lambda &lambda,  const Args&... args) const
    {
        std::vector<bool>   flgs( m_DLinks.size(), false);
        for ( uint32_t i = 0; i < flgs.size(); ++i)
        {
            if ( flgs[ i])
                continue;
            if ( IsTrivial( i))
                continue;
            Stor    sv = i;
            while ( sv < flgs.size()) {
                auto    svSave = sv;
                flgs[ sv] = true;
                sv = Next( sv);
                lambda( svSave, ( sv == i), args...);
                if ( sv == i)
                    break;
            }  
        }
        return;
    }

    void    Dump( std::ostream &ostr)
    {
        Traverse( [&]( Stor sv, bool endFlg) {
            ostr << sv << ( endFlg ? '\n' : ' ');
        }); 
        return;
    }
};
