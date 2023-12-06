///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppLevel3Msg.h
///
/// @brief Header file of class UpppLevel3Msg.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef UpppLevel3Msg_H
#define UpppLevel3Msg_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "UpppCoreMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppLevel3Msg : public UpppCoreMsg
{
    Q_OBJECT

public:

    UpppLevel3Msg( QObject * parent  = nullptr );
    UpppLevel3Msg( const UpppLevel3Msg & theRight );

    virtual ~UpppLevel3Msg();

    UpppLevel3Msg & operator=( const UpppLevel3Msg & theRight );

    int timeout( void ) const;
    void setTimeout( const int nTimeout );

    bool repeatSend( void ) const;
    void setRepeatSend( const bool bRepeat );

    int retryCnt() const;
    void setRetryCnt( const int nRetryCnt );
    void incRetryCnt();

signals:

public slots:

private:

    int m_nTimeout { 500 };

    bool m_bRepeatSend { true };

    int m_nRetryCnt { 0 };
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppLevel3Msg_H
