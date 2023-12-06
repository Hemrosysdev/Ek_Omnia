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
#include <QTimer>
#include <QElapsedTimer>
#include <QList>
#include <QPair>
#include <QSharedPointer>

#include "StartStopButtonDriver.h"
#include "AcyclicNotification.h"

class StandbyController;
class DisplayController;
class SqliteInterface;
class NotificationCenter;
class AgsaControl;
class DddCouple;
class QQmlEngine;
class SettingsStatemachine;
class RecipeControl;
class EspDriver;
class SwUpdateController;
class DeviceInfoCollector;
class SettingsSerializer;

namespace SystemIo
{
class DddDriver;
class AdcDmaDemuxer;
class CpuMp157Driver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class MainStatemachine
    : public QObject
{
    Q_OBJECT

    Q_ENUMS( EkxMainstates )

    Q_PROPERTY( int maxGrindDuration READ maxGrindDuration CONSTANT )
    Q_PROPERTY( MainStatemachine::EkxMainstates ekxMainstate READ ekxMainstate WRITE tryActivateEkxMainstate NOTIFY ekxMainstateChanged )
    Q_PROPERTY( bool grindRunning READ isGrindRunning WRITE setGrindRunning NOTIFY grindRunningChanged )
    Q_PROPERTY( bool grindingAllowed READ isGrindingAllowed WRITE setGrindingAllowed NOTIFY grindingAllowedChanged )
    Q_PROPERTY( bool systemSetupOpen READ isSystemSetupOpen WRITE setSystemSetupOpen NOTIFY systemSetupOpenChanged )

public:

    enum EkxMainstates
    {
        EKX_UNKNOWN = 0,
        EKX_CLASSIC_MODE,
        EKX_TIME_MODE,
        EKX_LIBRARY_MODE,
        EKX_SETTINGS_MODE,
        EKX_ERROR,
        EKX_SWUPDATE,
        EKX_STANDBY,

        EKX_LAST
    };

private:

    enum AdcDemuxerPosition
    {
        Ddd2,
        Ddd1,

        LastOrDemuxerSize
    };

public:

    explicit MainStatemachine( QQmlEngine * pEnigne,
                               QObject *    parent = nullptr );
    virtual ~MainStatemachine() override;

    int maxGrindDuration() const;

    MainStatemachine::EkxMainstates ekxMainstate();

    bool tryActivateEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate );

    void setEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate );

    Q_INVOKABLE void doFactoryReset();

    Q_INVOKABLE void resetEkxMainstate();

    DddCouple * dddCouple();

    EspDriver * espDriver();

    SettingsSerializer * settingsSerializer();

    SettingsStatemachine * settingsStatemachine();

    StandbyController * standbyController();

    DeviceInfoCollector * deviceInfoCollector();

    SqliteInterface * sqliteInterface();

    RecipeControl * recipeControl();

    NotificationCenter * notificationCenter();

    DisplayController * displayController();

    SwUpdateController * swUpdateController();

    AgsaControl * agsaControl( void );

    SystemIo::DddDriver * dddFineDriver();

    SystemIo::DddDriver * dddCoarseDriver();

    static QString rootPath();

    static bool ensureExistingDirectory( const QString & strFilename );

    bool isMcuIgnored( void ) const;

    bool isGrindRunning() const;

    void setGrindRunning( const bool bRunning );

    bool isGrindingAllowed() const;

    void setGrindingAllowed( const bool bAllowed );

    bool isSystemSetupOpen() const;

    void setSystemSetupOpen( bool bOpen );

    void setSerialPort( const QString & strSerialPort );

signals:

    void ekxMainstateChanged();

    void grindRunningChanged();

    void grindingAllowedChanged();

    void systemSetupOpenChanged();

    void pduTutorialRequested();

public slots:

    void processUserInput();

    void processStartStopButton( const StartStopButtonDriver::ButtonState nButtonState );

    void processTimeoutDelayedMotorStop();

    void processTimeoutAutoLogout();

    void processSwUpdateStart();

    void processSwUpdateStop();

    void processTimeRecipeIndexChanged();

    void processLibraryRecipeIndexChanged();

    void processLibraryGrammageIndexChanged();

    void processStandbyChanged( const bool bStandbyActive );

    void processMaxGrindDuration();

    void processSettingsModeExitTimeout();

    void checkPduStatus();

    void processRecoverAction( const EkxSqliteTypes::SqliteNotificationType nNotificationId );

    void processNextRecoverActionStep();

private:

    void prepareContextProperties( QQmlEngine * const pEngine );

    void prepareComponents();

    void prepareConnections();

    bool tryActivateStandbyState();

    bool tryActivateClassicState();

    bool tryActivateTimeModeState();

    bool tryActivateLibraryModeState();

    bool tryActivateMenuState();

    bool tryActivateErrorState();

    bool tryActivateSwUpdateState();

    MainStatemachine::EkxMainstates loadAndActivateFileState();

    void checkStartSystemTime();

    void checkGrindingDiscsStatus();

private slots:

    void saveEkxMainState();

    void startOrStopAutoLogoutTimer();

private:

    void pushRecoverActionStep( const EkxSqliteTypes::SqliteNotificationType nNotification,
                                const int                                    nStep );

private:

    NotificationCenter *                                       m_pNotificationCenter { nullptr };
    SqliteInterface *                                          m_pSqliteInterface { nullptr };
    RecipeControl *                                            m_pRecipeControl { nullptr };
    EspDriver *                                                m_pEspDriver { nullptr };
    DddCouple *                                                m_pDddCouple { nullptr };
    SystemIo::DddDriver *                                      m_pDddCoarseDriver { nullptr };
    SystemIo::DddDriver *                                      m_pDddFineDriver { nullptr };
    DisplayController *                                        m_pDisplayController { nullptr };
    StandbyController *                                        m_pStandbyController { nullptr };
    DeviceInfoCollector *                                      m_pDeviceInfoCollector { nullptr };
    SwUpdateController *                                       m_pSwUpdateController { nullptr };
    SettingsSerializer *                                       m_pSettingsSerializer { nullptr };
    SettingsStatemachine *                                     m_pSettingsStatemachine { nullptr };
    AgsaControl *                                              m_pAgsaControl { nullptr };
    QSharedPointer<SystemIo::AdcDmaDemuxer>                    m_pAdcDmaDemuxer;
    QSharedPointer<SystemIo::CpuMp157Driver>                   m_pCpuMp157Driver;

    QString                                                    m_strRootPath;
    MainStatemachine::EkxMainstates                            m_nEkxMainstate { EkxMainstates::EKX_SWUPDATE };
    QTimer                                                     m_timerSaveEkxMainstate;
    bool                                                       m_bGrindRunning { false };
    bool                                                       m_bGrindingAllowed { true };
    bool                                                       m_bSystemSetupOpen { false };

    int                                                        m_nCurrentTimeRecipe { 0 };
    int                                                        m_nCurrentLibraryRecipe { 0 };
    int                                                        m_nCurrentLibraryGrammage { 0 };
    QElapsedTimer                                              m_grindingTimer;

    bool                                                       m_bIgnoreMcu { false }; // TODO: remove this for final release

    QTimer                                                     m_timerMaxGrindDuration;

    bool                                                       m_bStartUp { true };

    AcyclicNotification                                        m_acycNotificationStateFileCorrupted;

    AcyclicNotification                                        m_acycNotificationStateFileWrite;

    QTimer                                                     m_timerOneMinute;

    QTimer                                                     m_timerStandbyCounter;

    QTimer                                                     m_timerDelayedMotorStop;

    QTimer                                                     m_timerAutoLogout;

    QList<QPair<EkxSqliteTypes::SqliteNotificationType, int> > m_theRecoverActions;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // MainStatemachine_h
