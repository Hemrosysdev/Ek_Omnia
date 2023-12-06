///////////////////////////////////////////////////////////////////////////////
///
/// @file UartDriver.h
///
/// @brief Header file of class UartDriver.
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

#ifndef UartDriver_h
#define UartDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class McuDriver;
class MainStatemachine;
class EspDataInterface;
class EspUartIn;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UartDriver : public QObject
{
    Q_OBJECT

public:

    explicit UartDriver( MainStatemachine * pMainStatemachine,
                         QObject *parent = 0 );
    virtual ~UartDriver( ) override;

    void connectMcuInterface( EspUartIn * pUartMcuIn );
    void disconnectMcuInterface();

    bool mcuUartInValid() const;

    McuDriver * mcuDriver();

public slots:

    void processMcuDataChanged( EspDataInterface * pData );
    void processMcuValidChanged( EspDataInterface * pData );

private:

    void setMcuUartInValid( const bool bMcuUartInValid );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };
    EspUartIn*         m_pMcuUartIn { nullptr };
    bool               m_bMcuUartInValid { false };
    McuDriver*         m_pMcuDriver { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UartDriver_h
