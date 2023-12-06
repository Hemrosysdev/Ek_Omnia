///////////////////////////////////////////////////////////////////////////////
///
/// @file GpioOut.h
///
/// @brief Header file of class GpioOut.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 20.09.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef GpioOut_h
#define GpioOut_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "SystemDeviceFile.h"

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class GpioOut : public SystemDeviceFile
{
    Q_OBJECT

public:

    explicit GpioOut ( QObject * pParent = nullptr );
    explicit GpioOut( const QString & strSystemDeviceFile,
                      const bool      bInitLevel,
                      QObject *       pParent = nullptr );

    ~GpioOut() override;

    void setLow();

    void setHigh();

    bool level() const;

    bool isLow() const;

    bool isHigh() const;

private:

    bool m_bLevel { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // GpioOut_h
