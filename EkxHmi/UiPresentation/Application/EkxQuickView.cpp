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

#include "EkxQuickView.h"
#include "MainStatemachine.h"
#include "StandbyController.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxQuickView::EkxQuickView()
    : QQuickView()
{
    setFlags( Qt::Dialog );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxQuickView::setMainStatemachine(MainStatemachine *pMainStatemachine)
{
    m_pMainStatemachine = pMainStatemachine;
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
void EkxQuickView::keyPressEvent( QKeyEvent *e )
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
                m_pMainStatemachine->dddDriver()->incSimulatedScaleRotation( +1 );
            }
            break;

            case Qt::Key_Minus:
            {
                m_pMainStatemachine->dddDriver()->incSimulatedScaleRotation( -1 );
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
void EkxQuickView::keyReleaseEvent( QKeyEvent *e )
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
