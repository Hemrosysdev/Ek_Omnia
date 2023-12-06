///////////////////////////////////////////////////////////////////////////////
///
/// @file ArithmeticFilter.h
///
/// @brief Header file of template class ArithmeticFilter.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 08.09.2022
///
///////////////////////////////////////////////////////////////////////////////

#ifndef ArithmeticFilter_h
#define ArithmeticFilter_h
#include <QDebug>
///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

template<typename type, unsigned int unElementNum>
class ArithmeticFilter
{
    static_assert( unElementNum > 1, "Minimum 2 elements required" );

public:

    ArithmeticFilter()
    {
        reset( 0 );
    };

    ArithmeticFilter( const type initValue )
    {
        reset( initValue );
    };

    virtual ~ArithmeticFilter()
    {
    };

    void reset( const type initValue )
    {
        m_sum = 0;
        for ( unsigned int i = 0; i < unElementNum; i++ )
        {
            m_dataBuffer[i] = initValue;
            m_sum          += initValue;
        }
        m_value       = initValue;
        m_unNextIndex = 0;
    };

    type append( const type value )
    {
        m_sum                      -= m_dataBuffer[m_unNextIndex];
        m_sum                      += value;
        m_dataBuffer[m_unNextIndex] = value;
        m_unNextIndex               = ( m_unNextIndex + 1 ) % unElementNum;

        m_value = m_sum / static_cast<type>( unElementNum );

        return m_value;
    };

    type value() const
    {
        return m_value;
    };

private:

    type         m_dataBuffer[unElementNum];

    type         m_value = 0;

    type         m_sum = 0;

    unsigned int m_unNextIndex = 0;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // ArithmeticFilter_h
