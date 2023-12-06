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

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DisplayController : public QObject
{
    Q_OBJECT

    //Q_PROPERTY( int brightnessPercent READ brightnessPercent WRITE setBrightnessPercent NOTIFY brightnessPercentChanged )
    Q_PROPERTY( int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged )
    Q_PROPERTY( int minBrightness READ minBrightness CONSTANT )
    Q_PROPERTY( int maxBrightness READ maxBrightness CONSTANT )

public:

    static constexpr int c_nMinBrightness    = 1;
    static constexpr int c_nMaxBrightness    = 7;
    static constexpr int c_nDimmedBrightness = 6;

public:

    explicit DisplayController( QObject * pParent = nullptr );
    ~DisplayController() override;

    // QML property setters and getters:
    int brightness() const;

    // non-QML methods:
    bool brightnessEnable( void ) const;

    int minBrightness() const;

    int maxBrightness() const;

public slots:

    // QML property setters and getters:
    void setBrightness( const int brightness );

    // non-QML methods:
    void setBrightnessEnable( const bool brightnessEnable );

    void setBrightnessDisable( const bool brightnessDisable );

    void setDimmedBrightnessEnable( const bool enable );

signals:

    void brightnessChanged();

    void brightnessEnableChanged();

    void dimmedBrightnessEnableChanged();

private:

    void updateBrightness();

    static void applyBrightnessSystemValue( int brightnessSystemvalue );

private:

    int  m_nBrightness { c_nMaxBrightness };
    bool m_brightnessEnable { true };
    bool m_dimmedBrightnessEnable { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DisplayController_h

