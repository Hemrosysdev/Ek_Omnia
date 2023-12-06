///////////////////////////////////////////////////////////////////////////////
///
/// @file AcyclicNotification.h
///
/// @brief Header file of class AcyclicNotification.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 09.09.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef AcyclicNotification_h
#define AcyclicNotification_h

#include "EkxSqliteTypes.h"

class NotificationCenter;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AcyclicNotification
{

public:

    AcyclicNotification();
    virtual ~AcyclicNotification();

    void create( NotificationCenter * const                   pNotificationCenter,
                 const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    bool isActive() const;

    void activate();

    void deactivate();

    void setActive( const bool bActive );

private:

    NotificationCenter *                   m_pNotificationCenter { nullptr };

    EkxSqliteTypes::SqliteNotificationType m_nNotificationId { EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_LAST };

    bool                                   m_bActive { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // AcyclicNotification_h
