// tr_array.h _____________________________________________________________________________________________________________
#pragma once
  
  
//---------------------------------------------------------------------------------------------------------------------------------
// Static Array
template <  typename TStor, uint32_t Sz>
class Tr_Array : public std::array< TStor,  Sz>
{
public:
    typedef  uint32_t   SzType;
    typedef  TStor      Stor;

    Tr_Array( void)
    {}

template < typename StorType>
    Tr_Array( const std::initializer_list< StorType> &initList) 
    {    
        static_assert( uint32_t( initList.size()) == Sz); 
        for ( SzType i = 0; i < Sz; ++i)
           this->at( i) = Stor( *( initList.begin() +i));
    }

    Tr_Array(  const Tr_Array &vec)
        : std::array< Stor, Sz>( vec)
    {}

    Tr_Array(  Tr_Array &&vec)
        : std::array< Stor, Sz>( vec)
    {}  

    void        SetAll( const Stor &iVal) 
    { 
        Cy_Do::Loop< Sz>( [&]( uint32_t k) {
            SetAll( k, iVal);
        });
    }

    auto        &DoAssign( const Cy_Arr< Stor, SzType> &arr)
    { 
        CY_ERROR_ASSERT( Size() == arr.Size());
        Cy_Do::Loop< Sz>( [&]( uint32_t k) {
            SetAll( k, arr.At( 0));
        }); 
        return SELF;
    }

    SzType      Size( void) const { return SzType( this->size());}  
    const Stor  &At( SzType k)  const { return *this->PtrAt( k); }
    const Stor  &SetAt( SzType ind, const Stor &d) { return this->at( ind) = d; }
    const Stor  &operator[]( SzType i) const { return At( i); }  

    const Stor  *PtrAt( SzType k) const { return &this->at( 0) +k; }
    Stor        *PtrAt( SzType k) { return &this->at( 0) +k; }

    const Stor  *Begin( void) const { return this->PtrAt( 0); }  
    const Stor  *End( void) const { return this->PtrAt( 0) +this->Size(); }
    
    Stor        ObjAt( SzType k)   const { return *this->PtrAt( k); }

    Tr_Array    &operator=( const Tr_Array &vec) { *( std::array< Stor, Sz> *) this = vec; return SELF; }

    Cy_Arr< Stor, SzType> Arr( void) { return Cy_Arr< Stor, SzType>( &SELF[ 0], Size()); }

};

//---------------------------------------------------------------------------------------------------------------------------------

