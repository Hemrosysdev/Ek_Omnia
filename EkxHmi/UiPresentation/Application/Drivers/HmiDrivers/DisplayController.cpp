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

#include "DisplayController.h"

#include <math.h>

#include <QDebug>
#include <QFile>
#include <QQmlContext>
#include <QQmlEngine>

#include "MainStatemachine.h"

#define BRIGHTNESS_DEVICEFILE_PATH     "/sys/class/backlight/backlight/brightness"

/*======================================================================*/
DisplayController::DisplayController( QQmlEngine *engine,
                                      MainStatemachine * pMainStatemachine )
    : QObject(pMainStatemachine)
    , m_pMainStatemachine( pMainStatemachine )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + BRIGHTNESS_DEVICEFILE_PATH );

    engine->rootContext()->setContextProperty("displayController" , this);
    updateBrightness();
}

/*======================================================================*/
DisplayController::~DisplayController()
{

}

/*======================================================================*/
int DisplayController::brightnessPercent() const
{
    return(m_brightnessPercent);
}

/*======================================================================*/
void DisplayController::setBrightnessPercent( const int brightnessPercent)
{
    int newBrightnessPercent = brightnessPercent;

    if(newBrightnessPercent > 100)
        newBrightnessPercent = 100;
    if(newBrightnessPercent < 0)
        newBrightnessPercent = 0;

    if(m_brightnessPercent != newBrightnessPercent)
    {
        m_brightnessPercent = newBrightnessPercent;
        emit brightnessPercentChanged();
        updateBrightness();
    }
}

/*======================================================================*/
bool DisplayController::brightnessEnable(void) const
{
    return(m_brightnessEnable);
}

/*======================================================================*/
void DisplayController::setBrightnessEnable( const bool brightnessEnable )
{
    if ( m_brightnessEnable != brightnessEnable )
    {
        m_brightnessEnable = brightnessEnable;
        emit brightnessEnableChanged();
        updateBrightness();
    }
}

/*======================================================================*/
void DisplayController::setBrightnessDisable( const bool brightnessDisable )
{
    this->setBrightnessEnable( !brightnessDisable );
}

/*======================================================================*/
void DisplayController::setDimmedBrightnessEnable(const bool enable)
{
    if ( m_dimmedBrightnessEnable != enable )
    {
        m_dimmedBrightnessEnable = enable;
        emit dimmedBrightnessEnableChanged();
        updateBrightness();
    }
}

/*======================================================================*/
void DisplayController::updateBrightness()
{
    if ( !m_brightnessEnable )
    {
        applyBrightnessSystemValue( 0 );
    }
    else if ( m_dimmedBrightnessEnable )
    {
        int dimmedBrightnessPercent = m_brightnessPercent * 2 / 10;   // usually 20%
        dimmedBrightnessPercent = std::max( 10, dimmedBrightnessPercent );  // but never less than 10%
        applyBrightnessSystemValue( brightnessPercentageToSystemValue( dimmedBrightnessPercent ) );
    }
    else
    {
        applyBrightnessSystemValue( brightnessPercentageToSystemValue( m_brightnessPercent ) );
    }
}

/*======================================================================*/
int DisplayController::brightnessPercentageToSystemValue( int brightnessPercentage )
{
    // temporary workaround:brightness mapping is usable now but not good yet.
    brightnessPercentage = (60.0/sqrt(100.0)) * sqrt(brightnessPercentage) + 40.0 + 0.5; // 0.5 for int rounding

    int systemBrightness = qRound( 10.0 + ( (245.0 * static_cast<double>(brightnessPercentage)) / 100.0 ) );
    return(systemBrightness);
}

/*======================================================================*/
void DisplayController::applyBrightnessSystemValue( int brightnessSystemvalue )
{
    QString fileName = MainStatemachine::rootPath() + BRIGHTNESS_DEVICEFILE_PATH;
    QFile file(fileName);

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream out(&file);
        out << brightnessSystemvalue;
        file.flush();
        file.close();
    }
    else
    {
        qDebug() << "ERROR: DisplayController::applyBrightness(" << brightnessSystemvalue << ") cannot open file " << fileName;
    }
}

