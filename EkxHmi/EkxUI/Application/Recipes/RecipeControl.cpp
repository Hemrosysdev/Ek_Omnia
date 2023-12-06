///////////////////////////////////////////////////////////////////////////////
///
/// @file RecipeControl.cpp
///
/// @brief Implementation file of class RecipeControl.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, fsonntag@ultratronik.de
///
/// @date 23.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "RecipeControl.h"

#include <QDebug>
#include <QtQml>
#include <QKeyEvent>

#include <algorithm> // for std::sort, std::max

#include "EkxGlobals.h"
#include "MainStatemachine.h"
#include "EkxSqliteTypes.h"
#include "SqliteInterface.h"
#include "AgsaControl.h"
#include "EspStepperMotorDriver.h"
#include "SettingsSerializer.h"
#include "JsonHelper.h"
#include "EspDriver.h"
#include "TimeRecipe.h"
#include "LibraryRecipe.h"
#include "LibraryGrammage.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define RECIPE_CONFIG_VERSION           "01.05"

#define MAX_TIME_RECIPES                6
#define MIN_TIME_RECIPES                1

#define MAX_LIBRARY_RECIPES             10
#define MIN_LIBRARY_RECIPES             1

#define MAX_LIBRARY_GRAMMAGES           6

#define MAX_DDD_VALUE                   80

#define MAX_GRAMMAGE                    2
#define MIN_GRAMMAGE                    0

#define RECIPE_JSON_NAME                "timeRecipes"
#define RECIPE_JSON_UUID                "uuid"
#define RECIPE_JSON_GRINDING_SIZE       "grindingSize"
#define RECIPE_JSON_BIGVALUE            "timeBig"
#define RECIPE_JSON_COMMAVALUE          "timeComma"
#define RECIPE_JSON_PF_INDEX            "portaFilterIndex"
#define RECIPE_JSONMODEICON             "iconOn"
#define RECIPE_JSON_SHOW_COARSENESS     "showCoarseness"
#define RECIPE_JSON_RECIPE_NAME         "recipeName"
#define RECIPE_JSON_SHOW_RECIPE_NAME    "showRecipeName"

#define LIBRARY_JSON_NAME               "libraryRecipes"
#define LIBRARY_JSON_UUID               "uuid"
#define LIBRARY_JSON_GRINDING_SIZE      "grindingSize"
#define LIBRARY_JSON_BEANNAME           "beanName"
#define LIBRARY_JSON_CALIBRATION_OFFSET "offset"
#define LIBRARY_JSON_CALIBRATION_SLOPE  "slope"

// Inside libraryRecipes
#define GRAMMAGE_JSON_NAME              "grammages"
#define GRAMMAGE_JSON_UUID              "uuid"
#define GRAMMAGE_JSON_ICON              "icon"
#define GRAMMAGE_JSON_GRAMMAGE          "grammage"
#define GRAMMAGE_JSON_DOSING_TIME       "dosingTime"

#define CONFIG_VERSION                  "version"

#define RECIPE_FILENAME                 "/config/EkxRecipes.json"

bool libraryRecipeLessThan( const LibraryRecipe * a,
                            const LibraryRecipe * b )
{
    return a->beanName().toLower() < b->beanName().toLower();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RecipeControl::RecipeControl( QObject * pParent )
    : QObject( pParent )
    , m_theQlpTimeRecipeList( this, &m_theTimeRecipeList )
    , m_theQlpLibraryRecipeList( this, &m_theLibraryRecipeList )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + RECIPE_FILENAME );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RecipeControl::~RecipeControl()
{
    clearRecipes();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    if ( !loadRecipes() )
    {
        factoryReset();
    }

    m_timerStartDelayedAgsa.setSingleShot( true );
    connect( &m_timerStartDelayedAgsa, &QTimer::timeout, this, &RecipeControl::processStartDelayedAgsa );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::clearRecipes()
{
    while ( !m_theTimeRecipeList.isEmpty() )
    {
        m_theTimeRecipeList.takeFirst()->deleteLater();
    }

    while ( !m_theLibraryRecipeList.isEmpty() )
    {
        m_theLibraryRecipeList.takeFirst()->deleteLater();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QList<TimeRecipe *> RecipeControl::timeRecipeList()
{
    return m_theTimeRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QList<LibraryRecipe *> RecipeControl::libraryRecipeList()
{
    return m_theLibraryRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentTimeRecipeCount() const
{
    return m_theTimeRecipeList.size();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentTimeRecipeIndex() const
{
    return m_nCurrentTimeRecipeIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::maxTimeRecipesNum() const
{
    return MAX_TIME_RECIPES;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::getMaxDddValue() const
{
    return MAX_DDD_VALUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString RecipeControl::currentRecipeUuid() const
{
    QString strUuid;

    switch ( m_pMainStatemachine->ekxMainstate() )
    {
        case MainStatemachine::EKX_TIME_MODE:
            strUuid = m_theTimeRecipeList[m_nCurrentTimeRecipeIndex]->uuid();
            break;

        case MainStatemachine::EKX_CLASSIC_MODE:
            strUuid = RECIPE_UUID_CLASSIC;
            break;

        case MainStatemachine::EKX_LIBRARY_MODE:
            strUuid = m_theLibraryRecipeList[m_nCurrentLibraryIndex]->uuid();
            break;

        default:
            // do nothing
            break;
    }

    return strUuid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::startDelayedAgsa()
{
    if ( m_pMainStatemachine->settingsSerializer()->agsaEnabled() )
    {
        m_timerStartDelayedAgsa.start( m_pMainStatemachine->settingsSerializer()->agsaStartDelaySec() * 1000 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::stopDelayedAgsa()
{
    m_timerStartDelayedAgsa.stop();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setAgsaAllowed( bool bAllowed )
{
    if ( m_bAgsaAllowed != bAllowed )
    {
        m_bAgsaAllowed = bAllowed;
        Q_EMIT agsaAllowedChanged();

        if ( !bAllowed )
        {
            stopDelayedAgsa();
            m_pMainStatemachine->agsaControl()->stopAgsaControl();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool RecipeControl::isAgsaAllowed() const
{
    return m_bAgsaAllowed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentGrindingSize() const
{
    switch ( m_pMainStatemachine->ekxMainstate() )
    {
        case MainStatemachine::EkxMainstates::EKX_LIBRARY_MODE:
        {

            if ( const auto * recipe = m_theLibraryRecipeList.value( m_nCurrentLibraryIndex ) )
            {
                return recipe->grindingSize();
            }
            else
            {
                qCritical() << "Current library recipe" << m_nCurrentLibraryIndex << "does not exist";
            }

            break;

        }
        case MainStatemachine::EkxMainstates::EKX_TIME_MODE:
        {

            if ( const auto * recipe = m_theTimeRecipeList.value( m_nCurrentTimeRecipeIndex ) )
            {
                return recipe->grindingSize();
            }
            else
            {
                qCritical() << "Current recipe" << m_nCurrentTimeRecipeIndex << "does not exist";
            }

            break;
        }
        default:
            return -1;

            break;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::processStartDelayedAgsa()
{
    if ( !isAgsaAllowed() )
    {
        return;
    }

    const int nGrindingSize = currentGrindingSize();

    if ( nGrindingSize < 0 )
    {
        return;
    }

    // Check if current time recipe has DDD display enabled.
    if ( m_pMainStatemachine->ekxMainstate() == MainStatemachine::EkxMainstates::EKX_TIME_MODE )
    {
        if ( const auto * recipe = m_theTimeRecipeList.value( m_nCurrentTimeRecipeIndex ) )
        {
            if ( !recipe->showCoarseness() )
            {
                return;
            }
        }
    }

    m_pMainStatemachine->agsaControl()->moveToDddValue( nGrindingSize );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::processMainStateChanged()
{
    stopDelayedAgsa();

#ifdef TARGETBUILD
    if ( m_pMainStatemachine->espDriver()->agsaStepperMotorDriver()->isRunning() )
    {
        m_pMainStatemachine->agsaControl()->stopAgsaControl();
    }
#else
    m_pMainStatemachine->agsaControl()->stopAgsaControl();
#endif

    switch ( m_pMainStatemachine->ekxMainstate() )
    {
        case MainStatemachine::EkxMainstates::EKX_LIBRARY_MODE:
            startDelayedAgsa();
            break;

        case MainStatemachine::EkxMainstates::EKX_TIME_MODE:
            startDelayedAgsa();
            break;

        default:
//            stopDelayedAgsa();
//#ifdef TARGETBUILD
//            if ( m_pMainStatemachine->espDriver()->agsaControl()->isRunning() )
//            {
//                m_pMainStatemachine->espDriver()->agsaControl()->stopAgsaControl();
//            }
//#else
//            m_pMainStatemachine->espDriver()->agsaControl()->stopAgsaControl();
//#endif
            break;
    }

    Q_EMIT currentGrindingSizeChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setCurrentTimeRecipeIndex( const int nCurrentRecipeIndex )
{
    if ( nCurrentRecipeIndex != m_nCurrentTimeRecipeIndex )
    {
        const int nOldGrindingSize = currentGrindingSize();

        if ( nCurrentRecipeIndex >= m_theTimeRecipeList.length() )
        {
            m_nCurrentTimeRecipeIndex = m_theTimeRecipeList.length() - 1;
        }
        else
        {
            m_nCurrentTimeRecipeIndex = nCurrentRecipeIndex;
        }

        emit currentTimeRecipeIndexChanged();

        processMainStateChanged();

        if ( nOldGrindingSize != currentGrindingSize() )
        {
            Q_EMIT currentGrindingSizeChanged();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::maxLibraryRecipesNum() const
{
    return MAX_LIBRARY_RECIPES;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setCurrentLibraryRecipeIndex( int nCurrentLibraryIndex )
{
    if ( nCurrentLibraryIndex != m_nCurrentLibraryIndex )
    {
        const int nOldGrindingSize = currentGrindingSize();

        const int nOldLibraryGrammageCount = currentLibraryGrammageCount();
        const int nOldLibraryGrammageIndex = currentLibraryGrammageIndex();

        m_nCurrentLibraryIndex = nCurrentLibraryIndex;
        Q_EMIT currentLibraryIndexChanged();

        if ( nOldLibraryGrammageCount != currentLibraryGrammageCount() )
        {
            Q_EMIT currentLibraryGrammageCountChanged();
        }

        if ( nOldLibraryGrammageIndex != currentLibraryGrammageIndex() )
        {
            Q_EMIT currentLibraryGrammageIndexChanged();
        }

        processMainStateChanged();

        if ( nOldGrindingSize != currentGrindingSize() )
        {
            Q_EMIT currentGrindingSizeChanged();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentLibraryRecipeIndex() const
{
    return m_nCurrentLibraryIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::maxLibraryGrammageNum() const
{
    return MAX_LIBRARY_GRAMMAGES;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentLibraryGrammageCount() const
{
    if ( const auto * recipe = m_theLibraryRecipeList.value( m_nCurrentLibraryIndex ) )
    {
        return recipe->grammages().size();
    }
    else
    {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setCurrentLibraryGrammageIndex( int nCurrentLibraryGrammageIndex )
{
    if ( m_nCurrentLibraryGrammageIndex == nCurrentLibraryGrammageIndex )
    {
        return;
    }

    const int nOldGrindingSize = currentGrindingSize();

    m_nCurrentLibraryGrammageIndex = nCurrentLibraryGrammageIndex;

    Q_EMIT currentLibraryGrammageIndexChanged();

    processMainStateChanged();

    if ( nOldGrindingSize != currentGrindingSize() )
    {
        Q_EMIT currentGrindingSizeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentLibraryGrammageIndex() const
{
    return std::max( 0, std::min( m_nCurrentLibraryGrammageIndex, currentLibraryGrammageCount() - 1 ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::resetTimeRecipeUuids()
{
    m_theTimeRecipeList[0]->setUuid( RECIPE_UUID_RECIPE1 );
    if ( m_theTimeRecipeList.size() > 1 )
    {
        m_theTimeRecipeList[1]->setUuid( RECIPE_UUID_RECIPE2 );
    }
    if ( m_theTimeRecipeList.size() > 2 )
    {
        m_theTimeRecipeList[2]->setUuid( RECIPE_UUID_RECIPE3 );
    }
    if ( m_theTimeRecipeList.size() > 3 )
    {
        m_theTimeRecipeList[3]->setUuid( RECIPE_UUID_RECIPE4 );
    }
    if ( m_theTimeRecipeList.size() > 4 )
    {
        m_theTimeRecipeList[4]->setUuid( RECIPE_UUID_RECIPE5 );
    }
    if ( m_theTimeRecipeList.size() > 5 )
    {
        m_theTimeRecipeList[5]->setUuid( RECIPE_UUID_RECIPE6 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::resetTimeRecipeNames()
{
    for ( int i = 0; i < m_theTimeRecipeList.size(); ++i ) {
        auto * pRecipe = m_theTimeRecipeList.at( i );
        if ( pRecipe->recipeName().isEmpty() )
        {
            const QString strRecipeName = QStringLiteral( "Recipe %1" ).arg( i + 1 );
            m_theTimeRecipeList.at( i )->setRecipeName( strRecipeName );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

TimeRecipe * RecipeControl::createTimeRecipe() const
{
    if ( m_theTimeRecipeList.length() >= MAX_TIME_RECIPES )
    {
        return nullptr;
    }

    // Returning a parent-less QObject from a Q_INVOAKBLE has the QML gc take care of it.
    TimeRecipe * pRecipe = new TimeRecipe( false /*not persisted*/ );
    // Uuid will be set once it is added to the list.
    pRecipe->setRecipeName( QStringLiteral( "Recipe %1" ).arg( m_theTimeRecipeList.length() + 1 ) );
    return pRecipe;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::addTimeRecipe( TimeRecipe * pRecipe )
{
    if ( !pRecipe )
    {
        return -1;
    }

    if ( m_theTimeRecipeList.length() >= MAX_TIME_RECIPES )
    {
        return -1;
    }

    // Important, we handed it out to the QML gc but now we want it back.
    QQmlEngine::setObjectOwnership( pRecipe, QQmlEngine::CppOwnership );
    pRecipe->setPersisted( true );

    m_theTimeRecipeList.append( pRecipe );

    resetTimeRecipeUuids();
    resetTimeRecipeNames();
    saveRecipes();

    Q_EMIT timeRecipeListChanged();

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_ADD,
                                                      m_theTimeRecipeList.last()->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      pRecipe->recipeName() );

    return m_theTimeRecipeList.indexOf( pRecipe );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::editTimeRecipe( TimeRecipe *    pRecipe,
                                    const int       nRecipeBig,
                                    const int       nRecipeComma,
                                    const int       nGrindingSize,
                                    const int       nPortaFilterIndex,
                                    const bool      bIconOn,
                                    const bool      bShowCoarseness,
                                    const QString & strRecipeName,
                                    const bool      bShowRecipeName )
{
    if ( !pRecipe )
    {
        return;
    }

    const int nRecipeIndex = m_theTimeRecipeList.indexOf( pRecipe );
    if ( nRecipeIndex == -1 )
    {
        return;
    }

    if ( pRecipe->uuid().isEmpty() )
    {
        resetTimeRecipeUuids();
    }

    const int nOldGrindingSize = currentGrindingSize();

    pRecipe->setRecipeBigValue( nRecipeBig );
    pRecipe->setRecipeCommaValue( nRecipeComma );
    pRecipe->setGrindingSize( nGrindingSize );
    pRecipe->setPortaFilterIndex( nPortaFilterIndex );
    pRecipe->setRecipeIconOn( bIconOn );
    pRecipe->setShowCoarseness( bShowCoarseness );
    pRecipe->setRecipeName( strRecipeName );
    pRecipe->setShowRecipeName( bShowRecipeName );

    if ( pRecipe->recipeName().isEmpty() )
    {
        resetTimeRecipeNames();
    }

    Q_EMIT timeRecipeListChanged();

    int nDbPortaFilterIndex = nPortaFilterIndex + 1;
    if ( !bIconOn )
    {
        nDbPortaFilterIndex = 0;
    }

    m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                          pRecipe->recipeName(),
                                                          nDbPortaFilterIndex,
                                                          EkxSqliteTypes::SqliteRecipeMode_TIME );
    QString     strChanges;
    QTextStream stream( &strChanges );
    stream << strRecipeName << "/"
           << nRecipeBig << "." << nRecipeComma << "/"
           << nGrindingSize << "/"
           << nPortaFilterIndex << "/" << bIconOn << "/" << bShowCoarseness
           << strRecipeName << "/" << bShowRecipeName;
    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_CHANGE,
                                                      pRecipe->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      strChanges );

    saveRecipes();

    if ( nOldGrindingSize != currentGrindingSize() )
    {
        Q_EMIT currentGrindingSizeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

Q_INVOKABLE void RecipeControl::deleteTimeRecipe( const int nIndex )
{
    if ( m_theTimeRecipeList.length() > MIN_TIME_RECIPES )
    {
        TimeRecipe * pRecipe = m_theTimeRecipeList.takeAt( nIndex );

        emit timeRecipeListChanged();

        QString strRecipeName = QString( "Recipe %1" ).arg( nIndex + 1 );

        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_DELETE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                          strRecipeName );

        // no effective delete in database, because time mode recipes will never be deleted

        resetTimeRecipeUuids();
        resetTimeRecipeNames();
        saveRecipes();

        pRecipe->deleteLater();

        // if current index falls out of valif range
        if ( nIndex >= m_theTimeRecipeList.size() )
        {
            setCurrentTimeRecipeIndex( m_theTimeRecipeList.size() - 1 );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

LibraryRecipe * RecipeControl::createLibraryRecipe() const
{
    if ( m_theLibraryRecipeList.length() >= MAX_LIBRARY_RECIPES )
    {
        return nullptr;
    }

    // Returning a parent-less QObject from a Q_INVOAKBLE has the QML gc take care of it.
    LibraryRecipe * pRecipe = new LibraryRecipe( false /*not persisted*/ );
    pRecipe->setUuid( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
    pRecipe->setBeanName( QString( "Bean %1" ).arg( m_theLibraryRecipeList.length() ) );
    return pRecipe;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::addLibraryRecipe( LibraryRecipe * pRecipe )
{
    if ( !pRecipe )
    {
        return -1;
    }

    if ( m_theLibraryRecipeList.length() >= MAX_LIBRARY_RECIPES )
    {
        return -1;
    }

    if ( m_theLibraryRecipeList.contains( pRecipe ) )
    {
        qWarning() << "Cannot add recipe" << pRecipe << "as it was already added.";
        return -1;
    }

    // Important, we handed it out to the QML gc but now we want it back.
    QQmlEngine::setObjectOwnership( pRecipe, QQmlEngine::CppOwnership );
    pRecipe->setPersisted( true );

    m_theLibraryRecipeList.append( pRecipe );

    sortLibraryRecipes();

    emit libraryRecipeListChanged();

    saveRecipes();

    m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                          pRecipe->beanName(),
                                                          0,
                                                          EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_ADD,
                                                      m_theLibraryRecipeList.last()->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      pRecipe->beanName() );

    return m_theLibraryRecipeList.indexOf( pRecipe );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::editLibraryRecipe( LibraryRecipe * pRecipe,
                                       const QString & strBeanName,
                                       int             nGrindingSize )
{
    if ( !pRecipe )
    {
        return;
    }

    const int nOldGrindingSize = currentGrindingSize();

    pRecipe->setBeanName( strBeanName );
    pRecipe->setGrindingSize( nGrindingSize );

    sortLibraryRecipes();
    Q_EMIT libraryRecipeListChanged();

    QString     strChanges;
    QTextStream stream( &strChanges );
    stream << strBeanName << "/"
           << nGrindingSize;

    m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                          pRecipe->beanName(),
                                                          0,
                                                          EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_CHANGE,
                                                      pRecipe->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      strChanges );
    saveRecipes();

    processMainStateChanged();

    if ( nOldGrindingSize != currentGrindingSize() )
    {
        Q_EMIT currentGrindingSizeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::calibrateLibraryRecipe( LibraryRecipe * pRecipe,
                                            int             nFirstGrindDurationMs,
                                            int             nFirstGrindAmountMg,
                                            int             nSecondGrindDurationMs,
                                            int             nSecondGrindAmountMg )
{
    if ( !pRecipe )
    {
        return;
    }

    if ( nFirstGrindDurationMs == nSecondGrindDurationMs )
    {
        qWarning() << "Cannot calibrate recipe, grind duration is implausible";
        return;
    }

    // Do some Maths :-) Calculate slope and offset, f(t) = slope * t + offset.

    // slope = (mg2-mg1) / (t2-t1)
    const qreal nSlope = ( ( nSecondGrindAmountMg - nFirstGrindAmountMg ) / qreal( nSecondGrindDurationMs - nFirstGrindDurationMs ) );

    // solve for offset.
    const int nOffset = nFirstGrindAmountMg - ( nFirstGrindDurationMs * nSlope );

    qDebug() << "Calibrated recipe" << pRecipe << "with slope" << nSlope << "offset" << nOffset;
    qDebug() << "  first: duration =" << nFirstGrindDurationMs << "ms, amount =" << nFirstGrindAmountMg << "mg";
    qDebug() << "  second: duration =" << nSecondGrindDurationMs << "ms, amount = " << nSecondGrindAmountMg << "mg";

    pRecipe->setCalibrationSlope( nSlope );
    pRecipe->setCalibrationOffset( nOffset );
    // TODO Q_EMIT libraryRecipeListChanged(); ?

    // FIXME Sqlite transaction

    saveRecipes();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::deleteLibraryRecipe( const int nRecipeIndex )
{
    if ( m_theLibraryRecipeList.length() > MIN_LIBRARY_RECIPES )
    {
        LibraryRecipe * pRecipe = m_theLibraryRecipeList.takeAt( nRecipeIndex );
        emit            libraryRecipeListChanged();

        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_DELETE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                          pRecipe->beanName() );

        // no effective delete in database, because time mode recipes will never be deleted

        saveRecipes();

        pRecipe->deleteLater();

        // if current index falls out of valif range
        if ( nRecipeIndex >= m_theLibraryRecipeList.size() )
        {
            setCurrentLibraryRecipeIndex( m_theLibraryRecipeList.size() - 1 );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::sortLibraryRecipes( void )
{
    if ( !std::is_sorted( m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(), libraryRecipeLessThan ) )
    {
        LibraryRecipe * pRecipe = m_theLibraryRecipeList.at( currentLibraryRecipeIndex() );

        std::sort( m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(), libraryRecipeLessThan );
        //std::sort(m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(),
        //          [](LibraryRecipe* a, LibraryRecipe* b) -> bool
        //{
        //    return( a->beanName().toLower() < b->beanName().toLower() );
        //}
        //);

        setCurrentLibraryRecipeIndex( m_theLibraryRecipeList.indexOf( pRecipe ) );
        emit libraryRecipeListChanged();
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

LibraryGrammage * RecipeControl::createLibraryGrammage( LibraryRecipe * pRecipe ) const
{
    if ( !pRecipe )
    {
        return nullptr;
    }

    if ( pRecipe->grammages().size() >= MAX_LIBRARY_GRAMMAGES )
    {
        return nullptr;
    }

    LibraryGrammage * pGrammage = new LibraryGrammage( false /*not persisted*/ );
    pGrammage->setUuid( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
    return pGrammage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::addLibraryGrammage( LibraryRecipe *   pRecipe,
                                       LibraryGrammage * pGrammage )
{
    if ( !pRecipe
         || !pGrammage )
    {
        return -1;
    }

    auto newGrammages = pRecipe->grammages();
    if ( newGrammages.size() >= MAX_LIBRARY_GRAMMAGES )
    {
        return -1;
    }

    if ( newGrammages.contains( pGrammage ) )
    {
        qWarning() << "Cannot add grammage" << pGrammage << "to recipe" << pRecipe << "as it was already added.";
        return -1;
    }

    // Important, we handed it out to the QML gc but now we want it back.
    QQmlEngine::setObjectOwnership( pGrammage, QQmlEngine::CppOwnership );
    pGrammage->setPersisted( true );

    const int nOldLibraryGrammageCount = currentLibraryGrammageCount();

    newGrammages.append( pGrammage );
    pRecipe->setGrammages( newGrammages );

    saveRecipes();

    if ( nOldLibraryGrammageCount != currentLibraryGrammageCount() )
    {
        Q_EMIT currentLibraryGrammageCountChanged();
    }

    // FIXME SQL stuff
    qCritical() << "TODO" << Q_FUNC_INFO << "sqlite recipe_chage event not implemented yet!";
    /*m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                          pRecipe->beanName(),
                                                          0,
                                                          EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
       m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_ADD,
                                                      m_theLibraryRecipeList.last()->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      pRecipe->beanName() );*/

    return newGrammages.size() - 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::editLibraryGrammage( LibraryGrammage *             pGrammage,
                                         int /*LibraryGrammage::Icon*/ nIcon,
                                         int                           nGrammageMg,
                                         int                           nDosingTimeMs )
{
    if ( !pGrammage )
    {
        return;
    }

    pGrammage->setIcon( static_cast<LibraryGrammage::Icon>( nIcon ) );
    pGrammage->setGrammageMg( nGrammageMg );
    pGrammage->setDosingTimeMs( nDosingTimeMs );

    // TODO should we emit grammageChanged for the recipe?

    // FIXME SQL stuff
    qCritical() << "TODO" << Q_FUNC_INFO << "sqlite recipe_chage event not implemented yet!";
    /*QString strChanges;
       QTextStream  stream( &strChanges );
       stream << strBeanName << "/"
           << nGrindingSize << "/" << nGrammage;

       m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                          pRecipe->beanName(),
                                                          0,
                                                          EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
       m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_CHANGE,
                                                      pRecipe->uuid(),
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      strChanges );*/
    saveRecipes();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::deleteLibraryGrammage( LibraryRecipe * pRecipe,
                                           const int       nGrammageIndex )
{
    if ( !pRecipe )
    {
        return;
    }

    auto newGrammages = pRecipe->grammages();
    if ( nGrammageIndex < 0
         || nGrammageIndex >= newGrammages.size() )
    {
        return;
    }

    newGrammages.takeAt( nGrammageIndex )->deleteLater();
    pRecipe->setGrammages( newGrammages );

    if ( nGrammageIndex >= newGrammages.size() )
    {
        setCurrentLibraryGrammageIndex( newGrammages.size() - 1 );
    }

    Q_EMIT currentLibraryGrammageCountChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<TimeRecipe> RecipeControl::qlpTimeRecipeList() const
{
    return m_theQlpTimeRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<LibraryRecipe> RecipeControl::qlpLibraryRecipeList() const
{
    return m_theQlpLibraryRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::ensureConsistentDbRecipes()
{
    for ( auto pRecipe : m_theTimeRecipeList )
    {
        // ensure, we have consitent data for recipes
        int nDbPortaFilterIndex = pRecipe->portaFilterIndex() + 1;
        if ( !pRecipe->recipeIconOn() )
        {
            nDbPortaFilterIndex = 0;
        }

        m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                              pRecipe->recipeName(),
                                                              nDbPortaFilterIndex,
                                                              EkxSqliteTypes::SqliteRecipeMode_TIME );
    }

    for ( auto pRecipe : m_theLibraryRecipeList )
    {
        // ensure, we have consitent data for recipes
        m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                              pRecipe->beanName(),
                                                              0,
                                                              EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool RecipeControl::saveRecipes() const
{
    bool success = false;

    QJsonArray timeRecipeArray;
    for ( int i = 0; i < m_theTimeRecipeList.count(); i++ )
    {
        QJsonObject recipeJson;

        recipeJson[RECIPE_JSON_UUID]             = m_theTimeRecipeList[i]->uuid();
        recipeJson[RECIPE_JSON_GRINDING_SIZE]    = m_theTimeRecipeList[i]->grindingSize();
        recipeJson[RECIPE_JSON_BIGVALUE]         = m_theTimeRecipeList[i]->recipeBigValue();
        recipeJson[RECIPE_JSON_COMMAVALUE]       = m_theTimeRecipeList[i]->recipeCommaValue();
        recipeJson[RECIPE_JSON_PF_INDEX]         = m_theTimeRecipeList[i]->portaFilterIndex();
        recipeJson[RECIPE_JSONMODEICON]          = m_theTimeRecipeList[i]->recipeIconOn();
        recipeJson[RECIPE_JSON_SHOW_COARSENESS]  = m_theTimeRecipeList[i]->showCoarseness();
        recipeJson[RECIPE_JSON_RECIPE_NAME]      = m_theTimeRecipeList[i]->recipeName();
        recipeJson[RECIPE_JSON_SHOW_RECIPE_NAME] = m_theTimeRecipeList[i]->showRecipeName();

        timeRecipeArray.append( recipeJson );
    }

    QJsonArray libraryRecipeArray;
    for ( int i = 0; i < m_theLibraryRecipeList.count(); i++ )
    {
        const auto * pRecipe = m_theLibraryRecipeList.at( i );
        QJsonObject  recipeJson;

        recipeJson[LIBRARY_JSON_UUID]          = pRecipe->uuid();
        recipeJson[LIBRARY_JSON_GRINDING_SIZE] = pRecipe->grindingSize();
        recipeJson[LIBRARY_JSON_BEANNAME]      = pRecipe->beanName();

        if ( pRecipe->calibrated() )
        {
            recipeJson[LIBRARY_JSON_CALIBRATION_SLOPE]  = pRecipe->calibrationSlope();
            recipeJson[LIBRARY_JSON_CALIBRATION_OFFSET] = pRecipe->calibrationOffset();
        }

        const auto grammages = pRecipe->grammages();
        QJsonArray grammagesJsonArr;

        for ( const auto * pGrammage : grammages ) {
            QJsonObject grammageJson;

            grammageJson[GRAMMAGE_JSON_UUID]        = pGrammage->uuid();
            grammageJson[GRAMMAGE_JSON_ICON]        = static_cast<int>( pGrammage->icon() );
            grammageJson[GRAMMAGE_JSON_GRAMMAGE]    = pGrammage->grammageMg();
            grammageJson[GRAMMAGE_JSON_DOSING_TIME] = pGrammage->dosingTimeMs();

            grammagesJsonArr.append( grammageJson );
        }

        if ( !grammagesJsonArr.isEmpty() )
        {
            recipeJson[GRAMMAGE_JSON_NAME] = grammagesJsonArr;
        }

        libraryRecipeArray.append( recipeJson );
    }

    QJsonObject recipeLists;
    recipeLists[RECIPE_JSON_NAME]  = timeRecipeArray;
    recipeLists[LIBRARY_JSON_NAME] = libraryRecipeArray;
    recipeLists[CONFIG_VERSION]    = RECIPE_CONFIG_VERSION;

    if ( JsonHelper::writeJsonFile( MainStatemachine::rootPath() + RECIPE_FILENAME, recipeLists ) )
    {
        success = true;
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool RecipeControl::loadRecipes()
{
    qInfo() << "RecipeControl::loadRecipes()";

    clearRecipes();

    bool success = false;

    QJsonObject jsonFile;
    if ( !JsonHelper::readJsonFile( MainStatemachine::rootPath() + RECIPE_FILENAME, jsonFile ) )
    {
        qWarning() << "RecipeControl::loadRecipes(): can't load recipes from" << RECIPE_FILENAME;
    }
    else
    {
        QString strVersion = JsonHelper::read( jsonFile, CONFIG_VERSION, QString() );

        if ( strVersion.isEmpty() )
        {
            qCritical() << "RecipeControl::loadRecipes(): can't load recipes from, no valid version";
        }
        else
        {
            {
                QJsonArray timeRecipeJsonArr = jsonFile[RECIPE_JSON_NAME].toArray();

                for ( int i = 0; i < timeRecipeJsonArr.count(); i++ )
                {
                    TimeRecipe * pRecipe = new TimeRecipe( true /*persisted*/, this );
                    pRecipe->setUuid( timeRecipeJsonArr.at( i )[RECIPE_JSON_UUID].toString() );
                    pRecipe->setRecipeBigValue( timeRecipeJsonArr.at( i )[RECIPE_JSON_BIGVALUE].toInt() );
                    pRecipe->setRecipeCommaValue( timeRecipeJsonArr.at( i )[RECIPE_JSON_COMMAVALUE].toInt() );
                    pRecipe->setGrindingSize( timeRecipeJsonArr.at( i )[RECIPE_JSON_GRINDING_SIZE].toInt() );
                    pRecipe->setPortaFilterIndex( timeRecipeJsonArr.at( i )[RECIPE_JSON_PF_INDEX].toInt() );
                    pRecipe->setRecipeIconOn( timeRecipeJsonArr.at( i )[RECIPE_JSONMODEICON].toBool() );
                    pRecipe->setShowCoarseness( timeRecipeJsonArr.at( i )[RECIPE_JSON_SHOW_COARSENESS].toBool() );
                    pRecipe->setRecipeName( timeRecipeJsonArr.at( i )[RECIPE_JSON_RECIPE_NAME].toString() );
                    pRecipe->setShowRecipeName( timeRecipeJsonArr.at( i )[RECIPE_JSON_SHOW_RECIPE_NAME].toBool() );

                    m_theTimeRecipeList.append( pRecipe );
                }
            }

            {
                const QJsonArray libraryRecipeJsonArr = jsonFile[LIBRARY_JSON_NAME].toArray();

                for ( const auto & jsonRecipe : libraryRecipeJsonArr )
                {
                    LibraryRecipe * pRecipe = new LibraryRecipe( true /*persisted*/, this );

                    pRecipe->setUuid( jsonRecipe[LIBRARY_JSON_UUID].toString() );
                    pRecipe->setGrindingSize( jsonRecipe[LIBRARY_JSON_GRINDING_SIZE].toInt() );
                    pRecipe->setBeanName( jsonRecipe[LIBRARY_JSON_BEANNAME].toString() );
                    pRecipe->setCalibrationSlope( jsonRecipe[LIBRARY_JSON_CALIBRATION_SLOPE].toDouble() );
                    pRecipe->setCalibrationOffset( jsonRecipe[LIBRARY_JSON_CALIBRATION_OFFSET].toInt() );

                    const QJsonArray           grammagesJsonArr = jsonRecipe[GRAMMAGE_JSON_NAME].toArray();
                    QVector<LibraryGrammage *> grammages;

                    for ( const auto & jsonGrammage : grammagesJsonArr )
                    {
                        auto * pGrammage = new LibraryGrammage( true /*persisted*/, pRecipe );

                        pGrammage->setUuid( jsonGrammage[GRAMMAGE_JSON_UUID].toString() );
                        pGrammage->setIcon( static_cast<LibraryGrammage::Icon>( jsonGrammage[GRAMMAGE_JSON_ICON].toInt() ) );
                        pGrammage->setGrammageMg( jsonGrammage[GRAMMAGE_JSON_GRAMMAGE].toInt() );
                        pGrammage->setDosingTimeMs( jsonGrammage[GRAMMAGE_JSON_DOSING_TIME].toInt() );

                        grammages.append( pGrammage );
                    }

                    pRecipe->setGrammages( grammages );

                    m_theLibraryRecipeList.append( pRecipe );
                }

                resetTimeRecipeNames();
            }

            success = true;

            ensureConsistentDbRecipes();
        }
    }

    sortLibraryRecipes();
    emit timeRecipeListChanged();
    emit libraryRecipeListChanged();

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::checkAndConvertRecipeConfig()
{
    QString strConfigVersion = "";

    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + RECIPE_FILENAME, jsonFile ) )
    {
        strConfigVersion = JsonHelper::read( jsonFile, CONFIG_VERSION, QString() );
    }

    if ( strConfigVersion != RECIPE_CONFIG_VERSION )
    {
        factoryReset();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::factoryReset()
{
    qInfo() << "RecipeControl::factoryReset()";

    clearRecipes();
    setUpFactoryTimeRecipeList();
    setUpFactoryLibraryRecipeList();
    saveRecipes();

    ensureConsistentDbRecipes();

    // Bugfix: Internal Qt Object does not recognize changes in the grammage value. So whole object has to be triggered.
    setCurrentLibraryRecipeIndex( 1 );
    setCurrentLibraryRecipeIndex( 0 );

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_FACTORY_RESET );

    processMainStateChanged();
    Q_EMIT currentGrindingSizeChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setUpFactoryTimeRecipeList()
{
    TimeRecipe * pRecipe = new TimeRecipe( true /*persisted*/, this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE1 );
    pRecipe->setRecipeBigValue( 19 );
    pRecipe->setRecipeCommaValue( 4 );
    pRecipe->setGrindingSize( 194 );
    pRecipe->setPortaFilterIndex( 0 );
    pRecipe->setRecipeIconOn( true );
    pRecipe->setShowCoarseness( true );

    m_theTimeRecipeList.append( pRecipe );

    pRecipe = new TimeRecipe( true, this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE2 );
    pRecipe->setRecipeBigValue( 15 );
    pRecipe->setRecipeCommaValue( 9 );
    pRecipe->setGrindingSize( 159 );
    pRecipe->setPortaFilterIndex( 1 );
    pRecipe->setRecipeIconOn( true );

    m_theTimeRecipeList.append( pRecipe );

    pRecipe = new TimeRecipe( true, this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE3 );
    pRecipe->setRecipeBigValue( 8 );
    pRecipe->setRecipeCommaValue( 1 );
    pRecipe->setGrindingSize( 81 );
    pRecipe->setPortaFilterIndex( 2 );
    pRecipe->setRecipeIconOn( true );

    m_theTimeRecipeList.append( pRecipe );

    // TODO Provide some defaults after discussing with designers.
    resetTimeRecipeNames();

    emit timeRecipeListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setUpFactoryLibraryRecipeList()
{
    LibraryRecipe * recipe = new LibraryRecipe( true /*persisted*/, this );
    recipe->setUuid( RECIPE_UUID_ELPUENTE );
    recipe->setBeanName( "El Puente" );
    recipe->setGrindingSize( 568 );

    LibraryGrammage * grammage = new LibraryGrammage( true /*persisted*/, recipe );
    grammage->setGrammageMg( 39000 );
    grammage->setIcon( LibraryGrammage::Cup );
    recipe->addGrammage( grammage );

    grammage = new LibraryGrammage( true, recipe );
    grammage->setGrammageMg( 35200 );
    grammage->setIcon( LibraryGrammage::SingleFilter );
    recipe->addGrammage( grammage );

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( true, this );
    recipe->setUuid( RECIPE_UUID_BURUNDIBWAYI );
    recipe->setBeanName( "Burundi Bwayi" );
    recipe->setGrindingSize( 568 );

    grammage = new LibraryGrammage( true /*persisted*/, recipe );
    grammage->setGrammageMg( 39000 );
    grammage->setIcon( LibraryGrammage::Cup );
    recipe->addGrammage( grammage );

    grammage = new LibraryGrammage( true, recipe );
    grammage->setGrammageMg( 35200 );
    grammage->setIcon( LibraryGrammage::SingleFilter );
    recipe->addGrammage( grammage );

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( true, this );
    recipe->setUuid( RECIPE_UUID_MISCHGOLD );
    recipe->setBeanName( "Mischgold" );
    recipe->setGrindingSize( 193 );

    grammage = new LibraryGrammage( true /*persisted*/, recipe );
    grammage->setGrammageMg( 39000 );
    grammage->setIcon( LibraryGrammage::Cup );
    recipe->addGrammage( grammage );

    grammage = new LibraryGrammage( true, recipe );
    grammage->setGrammageMg( 35200 );
    grammage->setIcon( LibraryGrammage::SingleFilter );
    recipe->addGrammage( grammage );

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( true, this );
    recipe->setUuid( RECIPE_UUID_LADALIA );
    recipe->setBeanName( "La Dalia" );
    recipe->setGrindingSize( 193 );

    m_theLibraryRecipeList.append( recipe );

    emit libraryRecipeListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

