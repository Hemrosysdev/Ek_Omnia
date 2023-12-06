///////////////////////////////////////////////////////////////////////////////
///
/// @file NotificationCenter.cpp
///
/// @brief main application entry point of NotificationCenter.
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

#include "NotificationCenter.h"

#include <QDebug>

#include "MainStatemachine.h"
#include "Notification.h"
#include "AgsaControl.h"
#include "SqliteInterface.h"
#include "SettingsSerializer.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NotificationCenter::NotificationCenter()
    : QObject()
    , m_theQlpNotificationList( this, &m_theNotificationList )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NotificationCenter::~NotificationCenter()
{
    reset();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NotificationCenter::suppressNotifications() const
{
    return m_bSuppressNotifications;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::setSuppressNotifications( bool bSuppress )
{
    m_bSuppressNotifications = bSuppress;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::create( MainStatemachine * const pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

#ifdef TEST
    for ( int i = 0; i < 50; i++ )
    {
        raiseNotification( static_cast<EkxSqliteTypes::SqliteErrorTypes>( i ) );
    }
#endif

    m_bCreated = true;

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::reset()
{
    QList<Notification *> tempList = m_theNotificationList;

    m_theNotificationList.clear();

    emit notificationListChanged();
    emit anyActiveChanged();
    emit anyNotAcknowledgedChanged();

    while ( !tempList.isEmpty() )
    {
        tempList.takeFirst()->deleteLater();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NotificationCenter::isEmpty() const
{
    return m_theNotificationList.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NotificationCenter::isAnyActive() const
{
    return m_pActiveNotification != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int NotificationCenter::activeNotifications() const
{
    int nActive = 0;

    for ( int i = 0; i < m_theNotificationList.size(); i++ )
    {
        if ( m_theNotificationList[i]->isActive() )
        {
            nActive++;
        }
    }

    return nActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

Notification * NotificationCenter::activeNotification() const
{
    return m_pActiveNotification;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NotificationCenter::isAnyNotAcknowledged() const
{
    return m_pNotAcknowledgedNotification != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

Notification * NotificationCenter::notAcknowledgedNotification() const
{
    return m_pNotAcknowledgedNotification;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString NotificationCenter::notAcknowledgedImage() const
{
    QString strImage;

    if ( m_pNotAcknowledgedNotification )
    {
        strImage = m_pNotAcknowledgedNotification->image();
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString NotificationCenter::modeIndicatorImage() const
{
    QString strImage;

    if ( m_pNotAcknowledgedNotification )
    {
        strImage = m_pNotAcknowledgedNotification->modeIndicatorImage();
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString NotificationCenter::menuImage() const
{
    QString strImage;

    if ( m_pNotAcknowledgedNotification )
    {
        strImage = m_pNotAcknowledgedNotification->menuImage();
    }

    return strImage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::raiseNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationId )
{
    if ( m_bSuppressNotifications )
    {
        qDebug() << "NotificationCenter::raiseNotification()" << nNotificationId << "was suppressed";
    }
    else
    {
        addNotification( new Notification( this, nNotificationId ) );
    }

    m_pMainStatemachine->sqliteInterface()->raiseNotification( nNotificationId, "" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::removeNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationId )
{
    bool bFound = false;
    for ( int i = m_theNotificationList.size() - 1; i >= 0; i-- )
    {
        if ( m_theNotificationList[i]->typeId() == nNotificationId )
        {
            qInfo() << "NotificationCenter::removeNotification()" << nNotificationId;
            Notification * pNotification = m_theNotificationList.takeAt( i );
            emit           notificationListChanged();

            pNotification->deleteLater();
            bFound = true;
        }
    }

    refreshActiveNotification();
    refreshNotAcknowledgedNotification();

    if ( bFound )
    {
        m_pMainStatemachine->sqliteInterface()->removeNotification( nNotificationId, "" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::raiseRecoverAction( const EkxSqliteTypes::SqliteNotificationType nNotificationId )
{
    emit recoverAction( nNotificationId );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::addNotification( Notification * const pNotification )
{
    if ( !m_bCreated )
    {
        delete pNotification;
        qCritical() << "NotificationCenter::addNotification() fatal, center not created";
    }
    else if ( pNotification )
    {
        // insert into current list according to priority
        int i;
        for ( i = 0; i < m_theNotificationList.size(); i++ )
        {
            if ( pNotification->classId() >= m_theNotificationList[i]->classId() )
            {
                m_theNotificationList.insert( i, pNotification );
                emit notificationListChanged();
                break;
            }
        }

        // not inserted, add it at end
        if ( i == m_theNotificationList.size() )
        {
            m_theNotificationList.push_back( pNotification );
            emit notificationListChanged();
        }

        refreshActiveNotification();
        refreshNotAcknowledgedNotification();
    }
    else
    {
        qCritical() << "NotificationCenter::addNotification() nullptr given";
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::refreshActiveNotification()
{
    Notification * pActiveNotification = nullptr;

    // try to find first active notification
    for ( int i = 0; i < m_theNotificationList.size(); i++ )
    {
        if ( m_theNotificationList[i]->isActive() )
        {
            pActiveNotification = m_theNotificationList[i];
            break;
        }
    }

    if ( m_pActiveNotification != pActiveNotification )
    {
        m_pActiveNotification = pActiveNotification;
        emit anyActiveChanged();

        if ( m_pActiveNotification )
        {
            m_pActiveNotification->updateRecoverFunction();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::refreshNotAcknowledgedNotification()
{
    Notification * pNotAcknowledgedNotification = nullptr;

    // try to find first active notification
    for ( int i = 0; i < m_theNotificationList.size(); i++ )
    {
        if ( !m_theNotificationList[i]->isAcknowledged() )
        {
            pNotAcknowledgedNotification = m_theNotificationList[i];
            break;
        }
    }

    if ( m_pNotAcknowledgedNotification != pNotAcknowledgedNotification )
    {
        m_pNotAcknowledgedNotification = pNotAcknowledgedNotification;
        emit anyNotAcknowledgedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::updateNotification( const bool                                   bActivate,
                                             const EkxSqliteTypes::SqliteNotificationType nNotificationId )
{
    if ( bActivate )
    {
        raiseNotification( nNotificationId );
    }
    else
    {
        removeNotification( nNotificationId );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::raisePopNextRecoverActionStep()
{
    emit popNextRecoverActionStep();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<Notification> NotificationCenter::qlpNotificationList() const
{
    return m_theQlpNotificationList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processDddFineFailure( const bool bFailed )
{
    updateNotification( bFailed,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DDD2_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processDddCourseFailure( const bool bFailed )
{
    updateNotification( bFailed,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DDD1_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processAgsaFailure()
{
    if ( m_pMainStatemachine->settingsSerializer()->agsaEnabled() )
    {
        updateNotification( m_pMainStatemachine->agsaControl()->isFailedNoDdd(),
                            EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_AGSA_FAILED_NO_DDD );
        updateNotification( m_pMainStatemachine->agsaControl()->isFailedBlockage(),
                            EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_AGSA_FAILED_BLOCKAGE );
        updateNotification( m_pMainStatemachine->agsaControl()->isFailedTimeout(),
                            EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_AGSA_FAILED_TIMEOUT );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processEspInterfaceFailure( const bool bConnected )
{
    updateNotification( !bConnected,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_ESP_INTERFACE_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processMcuInterfaceFailure( const bool bMcuAlive )
{
    updateNotification( !bMcuAlive,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_MCU_INTERFACE_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processAgsaFaultPinActiveFailure( const bool bActive )
{
    updateNotification( bActive,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_AGSA_FAULT_PIN_ACTIVE );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NotificationCenter::processPduFaultPinActiveFailure( const bool bActive )
{
    updateNotification( bActive,
                        EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_PDU_FAULT_PIN_ACTIVE );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
