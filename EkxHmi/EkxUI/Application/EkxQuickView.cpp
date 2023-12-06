///////////////////////////////////////////////////////////////////////////////
///
/// @file EkxQuickView.cpp
///
/// @brief main application entry point of EkxQuickView.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.01.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EspDcHallMotorDriver.h"  // position of header intended because of mingw header fault on Windows

#include "EkxQuickView.h"

#include <QFileInfo>

#include "MainStatemachine.h"
#include "StandbyController.h"
#include "DddCouple.h"
#include "EspDriver.h"

#define PDU_INSTALLED_FILE  "/pduInstalled"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxQuickView::EkxQuickView()
    : QQuickView()
    , m_strPduInstalledFile( MainStatemachine::rootPath() + PDU_INSTALLED_FILE )
{
    setFlags( Qt::Dialog );
    setColor( Qt::black );

    MainStatemachine::ensureExistingDirectory( m_strPduInstalledFile );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxQuickView::setMainStatemachine( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

#ifdef SIMULATION_BUILD
    m_pMainStatemachine->espDriver()->pduDcHallMotorDriver()->setMotorTestOk( QFileInfo::exists( m_strPduInstalledFile ) );
    m_pMainStatemachine->espDriver()->pduDcHallMotorDriver()->setValid( true );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EkxQuickView::event( QEvent * e )
{
    if ( m_pMainStatemachine )
    {
        // TouchEnd will be raised on target system (touch driven)
        // instead on Linux host systems mouse press events will be raised
        if ( e->type() == QEvent::TouchEnd
             || e->type() == QEvent::MouseButtonPress )
        {
            m_pMainStatemachine->processUserInput();
            m_pMainStatemachine->standbyController()->processUserInput();
        }
    }

    return QQuickView::event( e );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD
void EkxQuickView::keyPressEvent( QKeyEvent * e )
{
    if ( m_pMainStatemachine )
    {
        switch ( e->key() )
        {
            case Qt::Key_F10:

            {
                m_pMainStatemachine->espDriver()->startStopButtonDriver()->setValid( true );
                m_pMainStatemachine->espDriver()->startStopButtonDriver()->setPressed( true );
            }
            break;

            case Qt::Key_Plus:
            {
                m_pMainStatemachine->dddCouple()->incSimulatedScaleRotation( +1 );
            }
            break;

            case Qt::Key_Minus:
            {
                m_pMainStatemachine->dddCouple()->incSimulatedScaleRotation( -1 );
            }
            break;

            case Qt::Key_P:
            {
                m_pMainStatemachine->espDriver()->pduDcHallMotorDriver()->setMotorTestOk( !m_pMainStatemachine->espDriver()->pduDcHallMotorDriver()->isMotorTestOk() );

                if ( m_pMainStatemachine->espDriver()->pduDcHallMotorDriver()->isMotorTestOk() )
                {
                    QFile( m_strPduInstalledFile ).open( QIODevice::WriteOnly );
                }
                else
                {
                    QFile::remove( m_strPduInstalledFile );
                }
            }
            break;
        }
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD
void EkxQuickView::keyReleaseEvent( QKeyEvent * e )
{
    if ( m_pMainStatemachine )
    {
        if ( e->key() == Qt::Key_F10 )
        {
            m_pMainStatemachine->espDriver()->startStopButtonDriver()->setValid( true );
            m_pMainStatemachine->espDriver()->startStopButtonDriver()->setPressed( false );
        }
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
