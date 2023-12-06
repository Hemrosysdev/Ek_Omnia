///////////////////////////////////////////////////////////////////////////////
///
/// @file DisplayController.cpp
///
/// @brief Class encapsulates the display control as a QtObject
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Xaver Pilz, Forschung & Entwicklung, xpilz@ultratronik.de
///
/// @date 04.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef DisplayController_h
#define DisplayController_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class QQmlEngine;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DisplayController : public QObject
{
    Q_OBJECT

    Q_PROPERTY( int brightnessPercent READ brightnessPercent WRITE setBrightnessPercent NOTIFY brightnessPercentChanged )

public:

    explicit DisplayController( QQmlEngine* engine,
                                MainStatemachine * pMainStatemachine );
    virtual ~DisplayController( ) override;

    // QML property setters and getters:
    int brightnessPercent() const;

    // non-QML methods:
    bool brightnessEnable(void) const;

public slots:

    // QML property setters and getters:
    void setBrightnessPercent( const int brightnessPercent );

    // non-QML methods:
    void setBrightnessEnable( const bool brightnessEnable );
    void setBrightnessDisable( const bool brightnessDisable );

    void setDimmedBrightnessEnable( const bool enable );

signals:

    void brightnessPercentChanged();
    void brightnessEnableChanged();
    void dimmedBrightnessEnableChanged();

private:

    void updateBrightness();

    static int  brightnessPercentageToSystemValue( int brightnessPercentage );
    static void applyBrightnessSystemValue( int brightnessSystemvalue );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    int          m_brightnessPercent { 100 };
    bool         m_brightnessEnable { true };
    bool         m_dimmedBrightnessEnable { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DisplayController_h

