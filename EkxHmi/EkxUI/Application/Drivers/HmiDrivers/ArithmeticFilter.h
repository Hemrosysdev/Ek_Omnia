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

#include <assert.h>

namespace UxEco
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

template<typename type>
class ArithmeticFilter
{

public:

    ArithmeticFilter()
    {
    };

    virtual ~ArithmeticFilter()
    {
        delete [] m_pDataBuffer;
        m_pDataBuffer = nullptr;
    };

    void init( unsigned int unElementNum,
               const type   initValue )
    {
        m_unElementNum = unElementNum;

        delete [] m_pDataBuffer;
        m_pDataBuffer = new type[m_unElementNum];

        reset( initValue );
    }

    void reset( const type initValue )
    {
        m_sum = 0;
        for ( unsigned int i = 0; i < m_unElementNum; i++ )
        {
            m_pDataBuffer[i] = initValue;
            m_sum           += initValue;
        }
        m_value       = initValue;
        m_unNextIndex = 0;
    };

    type append( const type value )
    {
        if ( m_unElementNum )
        {
            m_sum                       -= m_pDataBuffer[m_unNextIndex];
            m_sum                       += value;
            m_pDataBuffer[m_unNextIndex] = value;
            m_unNextIndex                = ( m_unNextIndex + 1 ) % m_unElementNum;

            m_value = m_sum / static_cast<type>( m_unElementNum );
        }

        return m_value;
    };

    type value() const
    {
        return m_value;
    };

    unsigned int elementNum() const
    {
        return m_unElementNum;
    };

private:

    unsigned int m_unElementNum { 0 };

    type *       m_pDataBuffer { nullptr };

    type         m_value = 0;

    type         m_sum = 0;

    unsigned int m_unNextIndex = 0;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace UxEco

#endif // ArithmeticFilter_h
