///////////////////////////////////////////////////////////////////////////////
///
/// @file Notification.h
///
/// @brief Header file of class Notification.
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

#ifndef Notification_h
#define Notification_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QDateTime>

#include "EkxSqliteTypes.h"

class NotificationCenter;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class Notification : public QObject
{
    Q_OBJECT

    Q_PROPERTY( int classId READ classId CONSTANT )
    Q_PROPERTY( int typeId READ typeId CONSTANT )
    Q_PROPERTY( QString image READ image CONSTANT )
    Q_PROPERTY( QString modeIndicatorImage READ modeIndicatorImage CONSTANT )
    Q_PROPERTY( QString menuImage READ menuImage CONSTANT )
    Q_PROPERTY( QString listEntryImage READ listEntryImage CONSTANT )
    Q_PROPERTY( QString shortText READ shortText CONSTANT )
    Q_PROPERTY( QString longText READ longText CONSTANT )
    Q_PROPERTY( QString description READ description CONSTANT )
    Q_PROPERTY( QString cancelLabel READ cancelLabel NOTIFY cancelLabelChanged )
    Q_PROPERTY( QString confirmLabel READ confirmLabel CONSTANT )
    Q_PROPERTY( QString timestamp READ timestamp CONSTANT )
    Q_PROPERTY( bool active READ isActive NOTIFY activeChanged )
    Q_PROPERTY( bool acknowledged READ isAcknowledged NOTIFY acknowledgedChanged )
    Q_PROPERTY( bool recoverAction READ hasRecoverAction CONSTANT )

public:

    explicit Notification( NotificationCenter * const                   pNotificationCenter,
                           const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    ~Notification() override;

    EkxSqliteTypes::SqliteNotificationType typeId() const;

    EkxSqliteTypes::SqliteNotificationClass classId() const;

    QString image() const;

    QString modeIndicatorImage() const;

    QString menuImage() const;

    QString listEntryImage() const;

    const QString & shortText() const;

    const QString & longText() const;

    const QString & description() const;

    void setCancelLabel( const QString & strLabel );

    const QString & cancelLabel() const;

    const QString & confirmLabel() const;

    QString timestamp() const;

    bool isActive() const;

    bool isAcknowledged() const;

    bool hasRecoverAction() const;

    Q_INVOKABLE void activateCancel();

    Q_INVOKABLE void activateAcknowledge();

    void updateRecoverFunction();

signals:

    void cancelActivated();

    void cancelLabelChanged();

    void acknowledgeActivated();

    void acknowledgedChanged();

    void activeChanged();

private:

    NotificationCenter *                    m_pNotificationCenter { nullptr };

    EkxSqliteTypes::SqliteNotificationType  m_nTypeId { EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_UNKNOWN };

    EkxSqliteTypes::SqliteNotificationClass m_nClassId { EkxSqliteTypes::SqliteNotificationClass::None };

    QString                                 m_strShortText;

    QString                                 m_strLongText;

    QString                                 m_strDescription;

    QString                                 m_strCancelLabel;

    QString                                 m_strConfirmLabel;

    bool                                    m_bAcknowledged { false };

    bool                                    m_bActive { true };

    bool                                    m_bHasRecoverAction { false };

    QDateTime                               m_timestamp;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // Notification_h
