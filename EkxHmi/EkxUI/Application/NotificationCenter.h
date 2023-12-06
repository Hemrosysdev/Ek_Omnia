///////////////////////////////////////////////////////////////////////////////
///
/// @file NotificationCenter.h
///
/// @brief Header file of class NotificationCenter.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 11.04.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef NotificationCenter_h
#define NotificationCenter_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QList>
#include <QQmlListProperty>

#include "EkxSqliteTypes.h"

class MainStatemachine;
class Notification;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class NotificationCenter : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QQmlListProperty<Notification> notificationList READ qlpNotificationList NOTIFY notificationListChanged )
    Q_PROPERTY( bool anyActive READ isAnyActive NOTIFY anyActiveChanged )
    Q_PROPERTY( bool anyNotAcknowledged READ isAnyNotAcknowledged NOTIFY anyNotAcknowledgedChanged )
    Q_PROPERTY( Notification * activeNotification READ activeNotification NOTIFY anyActiveChanged )
    Q_PROPERTY( Notification * notAcknowledgedNotification READ notAcknowledgedNotification NOTIFY anyNotAcknowledgedChanged )
    Q_PROPERTY( QString notAcknowlegdedImage READ notAcknowledgedImage NOTIFY anyNotAcknowledgedChanged )
    Q_PROPERTY( QString modeIndicatorImage READ modeIndicatorImage NOTIFY anyNotAcknowledgedChanged )
    Q_PROPERTY( QString menuImage READ menuImage NOTIFY anyNotAcknowledgedChanged )

public:

    explicit NotificationCenter();
    ~NotificationCenter() override;

    bool suppressNotifications() const;

    void setSuppressNotifications( bool bSuppress );

    void create( MainStatemachine * const pMainStatemachine );

    void reset();

    bool isEmpty() const;

    bool isAnyActive() const;

    int activeNotifications() const;

    Notification * activeNotification() const;

    bool isAnyNotAcknowledged() const;

    Notification * notAcknowledgedNotification() const;

    QString notAcknowledgedImage() const;

    QString modeIndicatorImage() const;

    QString menuImage() const;

    void raiseNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void removeNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void raiseRecoverAction( const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void refreshActiveNotification();

    void refreshNotAcknowledgedNotification();

    void updateNotification( const bool                                   bActivate,
                             const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void raisePopNextRecoverActionStep();

private:

    void addNotification( Notification * const pNotification );

    QQmlListProperty<Notification> qlpNotificationList() const;

signals:

    void anyActiveChanged();

    void anyNotAcknowledgedChanged();

    void notificationListChanged();

    void recoverAction( const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void popNextRecoverActionStep();

public slots:

    void processDddFineFailure( const bool bFailed );

    void processDddCourseFailure( const bool bFailed );

    void processAgsaFailure();

    void processEspInterfaceFailure( const bool bConnected );

    void processMcuInterfaceFailure( const bool bMcuAlive );

    void processAgsaFaultPinActiveFailure( const bool bActive );

    void processPduFaultPinActiveFailure( const bool bActive );

private:

    MainStatemachine *             m_pMainStatemachine { nullptr };

    QList<Notification *>          m_theNotificationList;

    QQmlListProperty<Notification> m_theQlpNotificationList;

    Notification *                 m_pActiveNotification { nullptr };

    Notification *                 m_pNotAcknowledgedNotification { nullptr };

    bool                           m_bCreated { false };

    bool                           m_bSuppressNotifications { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // NotificationCenter_h
