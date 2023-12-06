///////////////////////////////////////////////////////////////////////////////
///
/// @file Notification.cpp
///
/// @brief main application entry point of Notification.
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

#include "Notification.h"

#include <QDebug>

#include "MainStatemachine.h"
#include "NotificationCenter.h"

#define LABEL_RECOVER  "Try Recover"
#define LABEL_CANCEL   "Cancel"
#define LABEL_RETRY    "Retry"
#define LABEL_ACK      "Acknowledge"
#define LABEL_CONFIRM  "Confirm"
#define LABEL_INFO     "Info"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

Notification::Notification( NotificationCenter * const                   pNotificationCenter,
                            const EkxSqliteTypes::SqliteNotificationType nNotificationId )
    : QObject()
    , m_pNotificationCenter( pNotificationCenter )
    , m_nTypeId( nNotificationId )
{
    m_timestamp = QDateTime::currentDateTime();

    qInfo() << "Notification::Notification: create ID" << nNotificationId;

    if ( static_cast<int>( nNotificationId ) < 1
         || static_cast<int>( nNotificationId ) >= EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_LAST )
    {
        qCritical() << "Notification created with unknown ID " << nNotificationId;
        m_bActive = false;
    }
    else
    {
        m_nClassId          = EkxSqliteTypes::m_theNotifications[nNotificationId].m_nClassId;
        m_strDescription    = EkxSqliteTypes::m_theNotifications[nNotificationId].m_strDescription;
        m_strShortText      = EkxSqliteTypes::m_theNotifications[nNotificationId].m_strShortInfo;
        m_strLongText       = EkxSqliteTypes::m_theNotifications[nNotificationId].m_strLongInfo;
        m_bHasRecoverAction = EkxSqliteTypes::m_theNotifications[nNotificationId].m_bRecoverAction;
    }

    m_strConfirmLabel = LABEL_ACK;
    if ( m_bHasRecoverAction )
    {
        m_strCancelLabel = LABEL_RECOVER;
    }
    else
    {
        m_strCancelLabel = LABEL_CANCEL;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

Notification::~Notification()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxSqliteTypes::SqliteNotificationType Notification::typeId() const
{
    return m_nTypeId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxSqliteTypes::SqliteNotificationClass Notification::classId() const
{
    return m_nClassId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString Notification::image() const
{
    QString strImage;

    switch ( m_nClassId )
    {
        default:
        case EkxSqliteTypes::SqliteNotificationClass::None:
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Info:
            strImage = "qrc:/Icons/Notification-72px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Warning:
            strImage = "qrc:/Icons/Warning-72px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Error:
            strImage = "qrc:/Icons/Error-72px.png";
            break;
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString Notification::modeIndicatorImage() const
{
    QString strImage;

    switch ( m_nClassId )
    {
        default:
        case EkxSqliteTypes::SqliteNotificationClass::None:
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Info:
            strImage = "qrc:/Icons/Notification-16px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Warning:
            strImage = "qrc:/Icons/Warning-16px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Error:
            strImage = "qrc:/Icons/Error-16px.png";
            break;
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString Notification::menuImage() const
{
    QString strImage;

    switch ( m_nClassId )
    {
        default:
        case EkxSqliteTypes::SqliteNotificationClass::None:
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Info:
            strImage = "qrc:/Icons/Notification-56px+Border.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Warning:
            strImage = "qrc:/Icons/Warning-56px+Border.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Error:
            strImage = "qrc:/Icons/Error-56px+Border.png";
            break;
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString Notification::listEntryImage() const
{
    QString strImage;

    switch ( m_nClassId )
    {
        default:
        case EkxSqliteTypes::SqliteNotificationClass::None:
            strImage = "";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Info:
            strImage = "qrc:/Icons/Notification-56px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Warning:
            strImage = "qrc:/Icons/Warning-56px.png";
            break;

        case EkxSqliteTypes::SqliteNotificationClass::Error:
            strImage = "qrc:/Icons/Error-56px.png";
            break;
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & Notification::shortText() const
{
    return m_strShortText;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & Notification::longText() const
{
    return m_strLongText;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & Notification::description() const
{
    return m_strDescription;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void Notification::setCancelLabel( const QString & strLabel )
{
    if ( m_strCancelLabel != strLabel )
    {
        m_strCancelLabel = strLabel;
        emit cancelLabelChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & Notification::cancelLabel() const
{
    return m_strCancelLabel;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & Notification::confirmLabel() const
{
    return m_strConfirmLabel;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString Notification::timestamp() const
{
    return m_timestamp.toString( "yyyy-MM-dd hh:mm" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool Notification::isActive() const
{
    return m_bActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool Notification::isAcknowledged() const
{
    return m_bAcknowledged;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool Notification::hasRecoverAction() const
{
    return m_bHasRecoverAction;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void Notification::activateCancel()
{
    if ( m_bHasRecoverAction )
    {
        activateAcknowledge();
        m_pNotificationCenter->raiseRecoverAction( m_nTypeId );
    }
    else
    {
        m_bActive = false;
        emit activeChanged();
        emit cancelActivated();
        m_pNotificationCenter->refreshActiveNotification();
        m_pNotificationCenter->raisePopNextRecoverActionStep();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void Notification::activateAcknowledge()
{
    m_bAcknowledged = true;
    m_bActive       = false;
    emit activeChanged();
    emit acknowledgeActivated();
    emit acknowledgedChanged();
    m_pNotificationCenter->refreshActiveNotification();
    m_pNotificationCenter->refreshNotAcknowledgedNotification();
    m_pNotificationCenter->raisePopNextRecoverActionStep();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void Notification::updateRecoverFunction()
{
    //if ( m_pNotificationCenter->activeNotifications() > 1 )
    //{
    //    setCancelLabel( LABEL_CANCEL );
    //    m_bHasRecoverAction = false;
    //}
    //else
    if ( m_bHasRecoverAction )
    {
        setCancelLabel( LABEL_RECOVER );
        m_bHasRecoverAction = true;
    }
    else
    {
        setCancelLabel( LABEL_CANCEL );
        m_bHasRecoverAction = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
