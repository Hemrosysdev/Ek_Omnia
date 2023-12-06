///////////////////////////////////////////////////////////////////////////////
///
/// @file UartDriver.cpp
///
/// @brief main application entry point of UartDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "UartDriver.h"

#include <QDebug>

#include "EkxGlobals.h"
#include "McuDriver.h"
#include "EspUartIn.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UartDriver::UartDriver( MainStatemachine * pMainStatemachine,
                        QObject *parent )
    : QObject( parent )
    , m_pMainStatemachine( pMainStatemachine )
    , m_pMcuDriver( new McuDriver() )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UartDriver::~UartDriver()
{
    disconnectMcuInterface();

    delete m_pMcuDriver;
    m_pMcuDriver = nullptr;

    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UartDriver::connectMcuInterface( EspUartIn *pUartMcuIn )
{
    if ( m_pMcuUartIn != pUartMcuIn )
    {
        disconnectMcuInterface();
        m_pMcuUartIn = pUartMcuIn;

        if ( m_pMcuUartIn )
        {
            connect( m_pMcuUartIn, &EspDataInterface::dataChanged, this, &UartDriver::processMcuDataChanged );
            connect( m_pMcuUartIn, &EspDataInterface::validChanged, this, &UartDriver::processMcuValidChanged );

            setMcuUartInValid( m_pMcuUartIn->isValid() );

            processMcuDataChanged( m_pMcuUartIn );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UartDriver::disconnectMcuInterface()
{
    setMcuUartInValid( false );

    m_pMcuDriver->close();

    m_pMcuUartIn = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UartDriver::mcuUartInValid() const
{
    return m_bMcuUartInValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UartDriver::setMcuUartInValid( const bool bMcuUartInValid )
{
    if ( bMcuUartInValid != m_bMcuUartInValid )
    {
        m_bMcuUartInValid = bMcuUartInValid;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UartDriver::processMcuDataChanged( EspDataInterface * pData )
{
    EspUartIn * pMcuUartIn = dynamic_cast<EspUartIn*>( pData );

    if ( pMcuUartIn )
    {
        if ( pMcuUartIn->isValid() )
        {
            if ( m_pMcuDriver->open( pMcuUartIn->vcomName() ) )
            {
                qInfo() << "processMcuDataChanged() MCU UART port opened" << pMcuUartIn->vcomName();
            }
        }
        else
        {
            m_pMcuDriver->close();
            qInfo() << "processMcuDataChanged() MCU UART port closed";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UartDriver::processMcuValidChanged( EspDataInterface * pData )
{
    EspUartIn * pMcuUartIn = dynamic_cast<EspUartIn*>( pData );

    if ( pMcuUartIn )
    {
        setMcuUartInValid( pMcuUartIn->isValid() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuDriver* UartDriver::mcuDriver()
{
    return m_pMcuDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
