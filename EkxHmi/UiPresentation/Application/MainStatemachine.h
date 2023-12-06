///////////////////////////////////////////////////////////////////////////////
///
/// @file MainStatemachine.h
///
/// @brief Header file of class MainStatemachine.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
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

#ifndef MainStatemachine_h
#define MainStatemachine_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QDateTime>

#include "RecipeControl.h"
#include "EspDriver.h"
#include "DddDriver.h"
#include "HmiTempDriver.h"
#include "SettingsStatemachine.h"
#include "SwUpdateController.h"
#include "DeviceInfoCollector.h"
#include "StartStopButtonDriver.h"

class StandbyController;
class DisplayController;
class SqliteInterface;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class MainStatemachine
        : public QObject
{
    Q_OBJECT

    Q_ENUMS( EkxMainstates )

    Q_PROPERTY( MainStatemachine::EkxMainstates ekxMainstate    READ ekxMainstate      WRITE tryActivateEkxMainstate   NOTIFY ekxMainstateChanged )
    Q_PROPERTY( int                             currentRecipe   READ currentRecipe     WRITE setCurrentRecipe          NOTIFY currentRecipeChanged )
    Q_PROPERTY( bool                            isCoffeeRunning    READ isCoffeeRunning    WRITE setIsCoffeeRunning    NOTIFY isCoffeeRunningChanged )
    Q_PROPERTY( bool                            isCoffeeRunAllowed READ isCoffeeRunAllowed WRITE setIsCoffeeRunAllowed NOTIFY isCoffeeRunAllowedChanged )
public:

    enum EkxMainstates
    {
        EKX_UNKNOWN = 0,
        EKX_CLASSIC_MODE,
        EKX_TIME_MODE,
        EKX_LIBRARY_MODE,
        EKX_MENU,
        EKX_ERROR,
        EKX_SWUPDATE,
        EKX_STANDBY,
        EKX_LAST
    };

public:

    explicit MainStatemachine( QQmlEngine* pEnigne,
                               QObject *parent = nullptr );
    virtual ~MainStatemachine() override;

    MainStatemachine::EkxMainstates  ekxMainstate();
    bool tryActivateEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate );
    void setEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate );


    int  currentRecipe();
    void setCurrentRecipe( int newCurrentRecipe );
    Q_INVOKABLE void resetEkxMainstate();
    
    DddDriver*            dddDriver();
    EspDriver*            espDriver();
    SettingsStatemachine* settingsStatemachine();
    StandbyController*    standbyController();
    DeviceInfoCollector*  deviceInfoCollector();
    SqliteInterface*      sqliteInterface();
    RecipeControl*        recipeControl();

    static QString rootPath();

    static bool ensureExistingDirectory( const QString & strFilename );

    bool isMcuIgnored( void ) const;

    bool isCoffeeRunning() const;
    void setIsCoffeeRunning( const bool bRunning );
    bool isCoffeeRunAllowed() const;
    void setIsCoffeeRunAllowed(bool coffeeRunAllowed);

signals:

    void ekxMainstateChanged();
    void currentRecipeChanged();
    void isCoffeeRunningChanged();
    void isCoffeeRunAllowedChanged();

public slots:

    void processStartStopGrind( const StartStopButtonDriver::ButtonState nButtonState );
    void processSwUpdateStart( );
    void processSwUpdateStop( );
    void processStandbyChanged( const bool bStandbyActive );

private:

    bool tryActivateStandbyState();
    bool tryActivateClassicState();
    bool tryActivateTimeModeState();
    bool tryActivateLibraryModeState();
    bool tryActivateMenuState();
    bool tryActivateErrorState();
    bool tryActivateSwUpdateState();

    MainStatemachine::EkxMainstates loadCurrentState();

private slots:

    void saveEkxMainState();

private:

    static int m_nEkxMainstatesQTypeId;
    static int m_nQmlEkxMainstateId;
    
    RecipeControl*                  m_pRecipeControl { nullptr };
    EspDriver*                      m_pEspDriver { nullptr };
    DddDriver*                      m_pDddDriver { nullptr };
    HmiTempDriver*                  m_pHmiTempDriver { nullptr };
    DisplayController*              m_pDisplayController { nullptr };
    SqliteInterface *               m_pSqliteInterface { nullptr };
    StandbyController*              m_pStandbyController { nullptr };
    DeviceInfoCollector*            m_pDeviceInfoCollector { nullptr };
    SwUpdateController*             m_pSwUpdateController { nullptr };
    SettingsSerializer*             m_pSettingsSerializer { nullptr };
    SettingsStatemachine*           m_pSettingsStatemachine { nullptr };

    QString                         m_strRootPath;
    MainStatemachine::EkxMainstates m_nEkxMainstate { EkxMainstates::EKX_STANDBY };
    QTimer                          m_timerSaveEkxMainstate;
    bool                            m_isCoffeeRunning { false };
    bool                            m_isCoffeeRunAllowed { true };
    int                             m_nCurrentRecipe { 0 };
    QDateTime                       m_dtStartGrinding;

    bool                            m_bIgnoreMcu { false };       // TODO: remove this for final release

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // MainStatemachine_h
