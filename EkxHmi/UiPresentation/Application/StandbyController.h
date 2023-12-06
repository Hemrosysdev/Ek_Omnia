///////////////////////////////////////////////////////////////////////////////
///
/// @file StandbyController.h
///
/// @brief Header file of class StandbyController.
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

#ifndef StandbyController_h
#define StandbyController_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>

class MainStatemachine;
class QQmlEngine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StandbyController
        : public QObject
{
    Q_OBJECT

    Q_PROPERTY( bool  preStandbyDimActive     READ isPreStandbyDimActive   NOTIFY preStandbyDimChanged )
    Q_PROPERTY( bool  standbyActive           READ isStandbyActive         NOTIFY standbyChanged )

public:

    explicit StandbyController( QQmlEngine * pEngine,
                                MainStatemachine * pMainStatemachine );

    virtual ~StandbyController() override;

    void create();

    bool isStandbyActive() const;

    bool isPreStandbyDimActive() const;

    void setStandbyTimeout( const int nMinutes );

signals:

    void preStandbyDimChanged( const bool bPreStandbyDimActive );

    void standbyChanged( const bool bStandbyActive );

    void standbyActivated();

    void standbyDeactivated();

public slots:

    void processTimeoutStandbyTrigger();
    void processTimeoutPreStandbyDimTrigger();

    Q_INVOKABLE void processUserInput();

    Q_INVOKABLE void wakeUp();

    void processStandbyTimeChanged();
    void prelongStandbyTrigger();

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    QTimer m_timerStandbyTrigger;

    QTimer m_timerPreStandbyDimTrigger;

    bool m_bStandbyActive { false };

    bool m_bPreStandbyDimActive { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // StandbyController_h
