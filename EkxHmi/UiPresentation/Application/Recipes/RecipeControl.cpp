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
#include "EkxGlobals.h"
#include <QtQml>
#include <QQmlEngine>
#include <QQmlContext>
#include <QKeyEvent>

#include <algorithm> // for std::sort
#include "MainStatemachine.h"
#include "EkxSqliteTypes.h"
#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define RECIPE_CONFIG_VERSION           "01.04"

#define MAX_TIME_RECIPES                     6
#define MIN_TIME_RECIPES                     1

#define MAX_LIBRARY_RECIPES             10
#define MIN_LIBRARY_RECIPES             1

#define MAX_DDD_VALUE                   80

#define MAX_GRAMMAGE                    2
#define MIN_GRAMMAGE                    0

#define RECIPE_JSON_NAME                "TimeRecipes"
#define RECIPE_JSON_UUID                "Uuid"
#define RECIPE_JSON_BIG_DDD             "DddBig"
#define RECIPE_JSON_COMMA_DDD           "DddComma"
#define RECIPE_JSON_BIGVALUE            "TimeBig"
#define RECIPE_JSON_COMMAVALUE          "TimeComma"
#define RECIPE_JSON_PF_INDEX            "PortaFilterIndex"
#define RECIPE_JSONMODEICON             "IconOn"
#define RECIPE_JSON_SHOW_COARSENESS     "ShowCoarseness"

#define LIBRARY_JSON_NAME               "LibraryRecipes"
#define LIBRARY_JSON_UUID               "Uuid"
#define LIBRARY_JSON_BIG_DDD            "DddBigLibrary"
#define LIBRARY_JSON_COMMA_DDD          "DddCommaLibrary"
#define LIBRARY_JSON_BEANNAME           "BeanName"
#define LIBRARY_JSON_GRAM1BIG           "Gram1Big"
#define LIBRARY_JSON_GRAM2BIG           "Gram2Big"
#define LIBRARY_JSON_GRAM1COMMA         "Gram1Comma"
#define LIBRARY_JSON_GRAM2COMMA         "Gram2Comma"
#define LIBRARY_JSON_GRAM1ICON          "Gram1Icon"
#define LIBRARY_JSON_GRAM2ICON          "Gram2Icon"
#define LIBRARY_JSON_GRAMCOUNT          "GramCount"
#define LIBRARY_JSON_SHOWGRAM           "ShowGram"

#define CONFIG_VERSION                  "Version"

#define RECIPE_FILENAME                 "/config/EkxRecipes.json"

int RecipeControl::m_nTimeRecipeTypeId   = qmlRegisterUncreatableType<TimeRecipe>( "RecipeControl",
                                                                                   1,
                                                                                   0,
                                                                                   "TimeRecipe",
                                                                                   "unexpected error" );

int RecipeControl::m_nLibraryRecipeTypeId  = qmlRegisterUncreatableType<LibraryRecipe>( "RecipeControl",
                                                                                        1,
                                                                                        0,
                                                                                        "LibraryRecipe",
                                                                                        "unexpected error" );
bool libraryRecipeLessThan(const LibraryRecipe* a, const LibraryRecipe* b)
{
    return( a->beanName().toLower() < b->beanName().toLower() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RecipeControl::RecipeControl( QQmlEngine* pEngine,
                              MainStatemachine *pMainStatemachine )
    : QObject( pMainStatemachine )
    , m_pMainStatemachine( pMainStatemachine )
    , m_theQlpTimeRecipeList( this, m_theTimeRecipeList )
    , m_theQlpLibraryRecipeList( this, m_theLibraryRecipeList )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + RECIPE_FILENAME );

    pEngine->rootContext()->setContextProperty( "recipeControl" , this );
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

void RecipeControl::create()
{
    if ( !loadRecipes() )
    {
        factoryReset();
    }
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

QList<TimeRecipe*> RecipeControl::timeRecipeList()
{
    return m_theTimeRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QList<LibraryRecipe*> RecipeControl::libraryRecipeList()
{
    return m_theLibraryRecipeList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int RecipeControl::currentTimeRecipeCount()
{
    return m_nCurrentTimeRecipeCount;
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

void RecipeControl::setCurrentTimeRecipeIndex( const int nCurrentRecipeIndex )
{
    if ( nCurrentRecipeIndex != m_nCurrentTimeRecipeIndex )
    {
        if( nCurrentRecipeIndex >= m_theTimeRecipeList.length() )
        {
            m_nCurrentTimeRecipeIndex = m_theTimeRecipeList.length() - 1;
        }
        else
        {
            m_nCurrentTimeRecipeIndex = nCurrentRecipeIndex;
        }

        emit currentTimeRecipeIndexChanged();
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

void RecipeControl::setCurrentLibraryRecipeIndex( int nCurrentLibraryIndex )
{
    if ( nCurrentLibraryIndex != m_nCurrentLibraryIndex )
    {
        m_nCurrentLibraryIndex = nCurrentLibraryIndex;

        emit currentLibraryIndexChanged();
    }
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

void RecipeControl::addTimeRecipe()
{
    if ( m_theTimeRecipeList.length() < MAX_TIME_RECIPES )
    {
        TimeRecipe* pRecipe = new TimeRecipe( this );

        m_theTimeRecipeList.append( pRecipe );

        resetTimeRecipeUuids();
        saveRecipes();

        setCurrentRecipeCount( m_theTimeRecipeList.length() );
        emit timeRecipeListChanged();

        QString  strRecipeName = QString( "Recipe %1" ).arg( m_theTimeRecipeList.length() );
        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_ADD,
                                                          m_theTimeRecipeList.last()->uuid(),
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          strRecipeName );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::editTimeRecipe( const int nRecipeBig,
                                    const int nRecipeComma,
                                    const int nDddBig,
                                    const int nDddComma,
                                    const int nPortaFilterIndex,
                                    const bool bIconOn,
                                    const bool bShowCoarseness,
                                    const int nRecipeIndex)
{
    if ( nRecipeIndex < m_theTimeRecipeList.size() )
    {
        if ( m_theTimeRecipeList[nRecipeIndex]->uuid().isEmpty() )
        {
            resetTimeRecipeUuids();
        }
    }

    if ( nRecipeIndex < MAX_TIME_RECIPES )
    {
        TimeRecipe * pRecipe = m_theTimeRecipeList[nRecipeIndex];

        pRecipe->setRecipeBigValue( nRecipeBig );
        pRecipe->setRecipeCommaValue(nRecipeComma );
        pRecipe->setDddBigValue(nDddBig );
        pRecipe->setDddCommaValue( nDddComma );
        pRecipe->setPortaFilterIndex( nPortaFilterIndex );
        pRecipe->setRecipeIconOn( bIconOn);
        pRecipe->setShowCoarseness( bShowCoarseness );

        //m_theTimeRecipeList.replace( recipeIndex, pRecipe );
        //setCurrentRecipeCount( m_theTimeRecipeList.length() );

        emit timeRecipeListChanged();

        int nDbPortaFilterIndex = nPortaFilterIndex + 1;
        if ( !bIconOn )
            nDbPortaFilterIndex = 0;

        QString  strRecipeName = QString( "Recipe %1" ).arg( nRecipeIndex + 1 );
        m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                              strRecipeName,
                                                              nDbPortaFilterIndex,
                                                              EkxSqliteTypes::SqliteRecipeMode_TIME );
        QString strChanges;
        QTextStream  stream( &strChanges );
        stream << strRecipeName << "/"
               << nRecipeBig << "." << nRecipeComma << "/"
               << nDddBig << "." << nDddComma << "/"
               << nPortaFilterIndex << "/" << bIconOn << "/" << bShowCoarseness;
        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_CHANGE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          strChanges );

        saveRecipes();
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
        setCurrentRecipeCount( m_theTimeRecipeList.length() );
        emit timeRecipeListChanged();

        QString  strRecipeName = QString( "Recipe %1" ).arg( nIndex + 1 );

        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_DELETE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          strRecipeName );

        // no effective delete in database, because time mode recipes will never be deleted

        resetTimeRecipeUuids();
        saveRecipes();

        pRecipe->deleteLater();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::addLibraryRecipe()
{
    if ( m_theLibraryRecipeList.length() < MAX_LIBRARY_RECIPES )
    {
        LibraryRecipe* pRecipe = new LibraryRecipe( this );
        pRecipe->setUuid( QUuid::createUuid().toString(QUuid::WithoutBraces) );
        pRecipe->setBeanName( QString( "Bean %1" ).arg( m_theLibraryRecipeList.length() ) );

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
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          pRecipe->beanName() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::editLibraryRecipe( const int nDddBig,
                                       const int nDddComma,
                                       const QString & strBeanName,
                                       const int nGrammage1Big,
                                       const int nGrammage1Comma,
                                       const int nGrammage1Icon,
                                       const int nGrammage2Big,
                                       const int nGrammage2Comma,
                                       const int nGrammage2Icon,
                                       const int nGrammageCount,
                                       const bool bShowGrammage,
                                       const int nRecipeIndex )
{
    if ( nRecipeIndex < m_theLibraryRecipeList.size()
         && !m_theLibraryRecipeList[nRecipeIndex]->uuid().isEmpty() )
    {
        LibraryRecipe * pRecipe = m_theLibraryRecipeList[nRecipeIndex];

        pRecipe->setBeanName( strBeanName );
        pRecipe->setDddBigValue( nDddBig );
        pRecipe->setDddCommaValue( nDddComma );
        pRecipe->setGrammage1Big( nGrammage1Big );
        pRecipe->setGrammage1Comma( nGrammage1Comma );
        pRecipe->setGrammage2Big( nGrammage2Big );
        pRecipe->setGrammage2Comma( nGrammage2Comma );
        pRecipe->setGrammage1Icon( nGrammage1Icon );
        pRecipe->setGrammage2Icon( nGrammage2Icon );
        pRecipe->setGrammageCount( nGrammageCount );
        pRecipe->setShowGrammage( bShowGrammage );

        sortLibraryRecipes();
        emit libraryRecipeListChanged();

        QString strChanges;
        QTextStream  stream( &strChanges );
        stream << pRecipe->beanName() << "/"
               << nDddBig << "." << nDddComma << "/"
               << nGrammageCount << "/" << bShowGrammage << "/"
               << nGrammage1Big << "." << nGrammage1Comma << "/"
               << nGrammage2Big << "." << nGrammage2Comma << "/"
               << nGrammage1Icon << "/" << nGrammage2Icon << "/"
               << bShowGrammage;

        m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                              pRecipe->beanName(),
                                                              0,
                                                              EkxSqliteTypes::SqliteRecipeMode_LIBRARY );
        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_CHANGE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          strChanges );
        saveRecipes();
    }
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
        emit libraryRecipeListChanged();

        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_RECIPE_DELETE,
                                                          pRecipe->uuid(),
                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                          pRecipe->beanName() );

        // no effective delete in database, because time mode recipes will never be deleted

        saveRecipes();

        pRecipe->deleteLater();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::sortLibraryRecipes( void )
{
    if( !std::is_sorted(m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(), libraryRecipeLessThan) )
    {
        LibraryRecipe * currentRecipe = m_theLibraryRecipeList.at( currentLibraryRecipeIndex() );

        std::sort(m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(), libraryRecipeLessThan);
        //std::sort(m_theLibraryRecipeList.begin(), m_theLibraryRecipeList.end(),
        //          [](LibraryRecipe* a, LibraryRecipe* b) -> bool
        //{
        //    return( a->beanName().toLower() < b->beanName().toLower() );
        //}
        //);

        setCurrentLibraryRecipeIndex( m_theLibraryRecipeList.indexOf( currentRecipe ) );
        emit libraryRecipeListChanged();
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::addLibraryGrammage( const int nRecipeIndex )
{
    int nCurrentGrammageCount= m_theLibraryRecipeList[nRecipeIndex]->grammageCount();
    if( nCurrentGrammageCount < MAX_GRAMMAGE  )
    {
        nCurrentGrammageCount++;
        m_theLibraryRecipeList[nRecipeIndex]->setGrammageCount( nCurrentGrammageCount );
    }
}


///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::deleteLibraryGrammage( const int nRecipeIndex,
                                           const int nGrammageIndex )
{
    int nCurrentGrammageCount = m_theLibraryRecipeList[nRecipeIndex]->grammageCount();
    if( nCurrentGrammageCount > MIN_GRAMMAGE  )
    {

        if( nCurrentGrammageCount == 1 )
        {
            m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Big( 0 );
            m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Comma( 0 );
            m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Icon( 0 );
            m_theLibraryRecipeList[nRecipeIndex]->setShowGrammage( false );
        }

        else if( nCurrentGrammageCount == 2 )
        {
            if( nGrammageIndex == 0 )
            {
                m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Big( m_theLibraryRecipeList[nRecipeIndex]->grammage2Big() );
                m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Comma(m_theLibraryRecipeList[nRecipeIndex]->grammage2Comma());
                m_theLibraryRecipeList[nRecipeIndex]->setGrammage1Icon(m_theLibraryRecipeList[nRecipeIndex]->grammage2Icon());
            }

            m_theLibraryRecipeList[nRecipeIndex]->setGrammage2Big( 0 );
            m_theLibraryRecipeList[nRecipeIndex]->setGrammage2Comma( 0 );
            m_theLibraryRecipeList[nRecipeIndex]->setGrammage2Icon( 0 );
        }

        nCurrentGrammageCount--;
        m_theLibraryRecipeList[nRecipeIndex]->setGrammageCount( nCurrentGrammageCount );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void  RecipeControl::setCurrentRecipeCount(const int nCurrentRecipeCount )
{
    if ( nCurrentRecipeCount != m_nCurrentTimeRecipeCount )
    {
        m_nCurrentTimeRecipeCount = nCurrentRecipeCount;

        emit currentTimeRecipeCountChanged();
    }
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
    int i = 0;
    for ( auto pRecipe : m_theTimeRecipeList )
    {
        // ensure, we have consitent data for recipes
        int nDbPortaFilterIndex = pRecipe->portaFilterIndex() + 1;
        if ( !pRecipe->recipeIconOn() )
            nDbPortaFilterIndex = 0;

        QString  strRecipeName = QString( "Recipe %1" ).arg( ++i );
        m_pMainStatemachine->sqliteInterface()->changeRecipe( pRecipe->uuid(),
                                                              strRecipeName,
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

    QFile file( MainStatemachine::rootPath() + RECIPE_FILENAME );
    if ( file.open( QIODevice::ReadWrite ) )
    {
        file.resize(0);
        QJsonObject recipeLists;
        QJsonArray timeRecipeArray;
        QJsonArray libraryRecipeArray;

        for ( int i = 0; i < m_theTimeRecipeList.count(); i++ )
        {
            QJsonObject recipeJson;

            recipeJson[RECIPE_JSON_UUID]            = m_theTimeRecipeList[i]->uuid();
            recipeJson[RECIPE_JSON_BIG_DDD]         = m_theTimeRecipeList[i]->dddBigValue();
            recipeJson[RECIPE_JSON_COMMA_DDD]       = m_theTimeRecipeList[i]->dddCommaValue();
            recipeJson[RECIPE_JSON_BIGVALUE]        = m_theTimeRecipeList[i]->recipeBigValue();
            recipeJson[RECIPE_JSON_COMMAVALUE]      = m_theTimeRecipeList[i]->recipeCommaValue();
            recipeJson[RECIPE_JSON_PF_INDEX]        = m_theTimeRecipeList[i]->portaFilterIndex();
            recipeJson[RECIPE_JSONMODEICON]         = m_theTimeRecipeList[i]->recipeIconOn();
            recipeJson[RECIPE_JSON_SHOW_COARSENESS] = m_theTimeRecipeList[i]->showCoarseness();

            timeRecipeArray.append(recipeJson);
        }

        for ( int i = 0; i < m_theLibraryRecipeList.count(); i++ )
        {
            QJsonObject recipeJson;

            recipeJson[LIBRARY_JSON_UUID]        = m_theLibraryRecipeList[i]->uuid();
            recipeJson[LIBRARY_JSON_BIG_DDD]     = m_theLibraryRecipeList[i]->dddBigValue();
            recipeJson[LIBRARY_JSON_COMMA_DDD]   = m_theLibraryRecipeList[i]->dddCommaValue();
            recipeJson[LIBRARY_JSON_BEANNAME]    = m_theLibraryRecipeList[i]->beanName();

            recipeJson[LIBRARY_JSON_GRAM1BIG]   = m_theLibraryRecipeList[i]->grammage1Big();
            recipeJson[LIBRARY_JSON_GRAM2BIG]   = m_theLibraryRecipeList[i]->grammage2Big();
            recipeJson[LIBRARY_JSON_GRAM1COMMA] = m_theLibraryRecipeList[i]->grammage1Comma();
            recipeJson[LIBRARY_JSON_GRAM2COMMA] = m_theLibraryRecipeList[i]->grammage2Comma();
            recipeJson[LIBRARY_JSON_GRAM1ICON]  = m_theLibraryRecipeList[i]->grammage1Icon();
            recipeJson[LIBRARY_JSON_GRAM2ICON]  = m_theLibraryRecipeList[i]->grammage2Icon();
            recipeJson[LIBRARY_JSON_GRAMCOUNT]  = m_theLibraryRecipeList[i]->grammageCount();
            recipeJson[LIBRARY_JSON_SHOWGRAM]   = m_theLibraryRecipeList[i]->showGrammage();

            libraryRecipeArray.append(recipeJson);
        }

        recipeLists[RECIPE_JSON_NAME]  = timeRecipeArray;
        recipeLists[LIBRARY_JSON_NAME] = libraryRecipeArray;
        recipeLists[CONFIG_VERSION]    = RECIPE_CONFIG_VERSION;

        QJsonDocument jsonDoc( recipeLists );

        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

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
    clearRecipes();

    bool success = false;

    QFile file( MainStatemachine::rootPath() + QString(RECIPE_FILENAME) );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        {
            QJsonArray timeRecipeJsonArr  = obj[RECIPE_JSON_NAME].toArray();

            for ( int i = 0; i < timeRecipeJsonArr.count(); i++ )
            {
                TimeRecipe* pRecipe = new TimeRecipe( this );
                pRecipe->setUuid( timeRecipeJsonArr.at(i)[RECIPE_JSON_UUID].toString() );
                pRecipe->setRecipeBigValue(  timeRecipeJsonArr.at(i)[RECIPE_JSON_BIGVALUE].toInt() );
                pRecipe->setRecipeCommaValue( timeRecipeJsonArr.at(i)[RECIPE_JSON_COMMAVALUE].toInt() );
                pRecipe->setDddBigValue( timeRecipeJsonArr.at(i)[RECIPE_JSON_BIG_DDD].toInt() );
                pRecipe->setDddCommaValue( timeRecipeJsonArr.at(i)[RECIPE_JSON_COMMA_DDD].toInt() );
                pRecipe->setPortaFilterIndex(timeRecipeJsonArr.at(i)[RECIPE_JSON_PF_INDEX].toInt());
                pRecipe->setRecipeIconOn(timeRecipeJsonArr.at(i)[RECIPE_JSONMODEICON].toBool());
                pRecipe->setShowCoarseness(timeRecipeJsonArr.at(i)[RECIPE_JSON_SHOW_COARSENESS].toBool());

                m_theTimeRecipeList.append( pRecipe );
            }
        }

        {
            QJsonArray libraryRecipeJsonArr  = obj[LIBRARY_JSON_NAME].toArray();

            for ( int i = 0; i < libraryRecipeJsonArr.count(); i++ )
            {
                LibraryRecipe* pRecipe = new LibraryRecipe( this );

                pRecipe->setUuid( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_UUID].toString() );
                pRecipe->setDddBigValue( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_BIG_DDD].toInt() );
                pRecipe->setDddCommaValue( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_COMMA_DDD].toInt() );
                pRecipe->setBeanName( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_BEANNAME].toString() );

                pRecipe->setGrammage1Big( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM1BIG].toInt() );
                pRecipe->setGrammage2Big( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM2BIG].toInt() );
                pRecipe->setGrammage1Comma( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM1COMMA].toInt() );
                pRecipe->setGrammage2Comma( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM2COMMA].toInt() );
                pRecipe->setGrammage1Icon( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM1ICON].toInt() );
                pRecipe->setGrammage2Icon( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAM2ICON].toInt() );
                pRecipe->setGrammageCount( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_GRAMCOUNT].toInt() );
                pRecipe->setShowGrammage( libraryRecipeJsonArr.at(i)[LIBRARY_JSON_SHOWGRAM].toBool() );

                m_theLibraryRecipeList.append( pRecipe );
            }
        }

        success = true;

        ensureConsistentDbRecipes();
    }
    else
    {
        qWarning() << "RecipeControl::loadRecipes(): can't load recipes from" << file.fileName();
    }

    setCurrentRecipeCount( m_theTimeRecipeList.length() );

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
    QString currentConfigVersion = "";

    QFile file( MainStatemachine::rootPath() + QString(RECIPE_FILENAME) );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        currentConfigVersion = obj[CONFIG_VERSION].toString();
    }

    if ( currentConfigVersion != RECIPE_CONFIG_VERSION )
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
    clearRecipes();
    setUpFactoryTimeRecipeList();
    setUpFactoryLibraryRecipeList();
    saveRecipes();

    ensureConsistentDbRecipes();

    // Bugfix: Internal Qt Object does not recognize changes in the grammage value. So whole object has to be triggered.
    setCurrentLibraryRecipeIndex(1);
    setCurrentLibraryRecipeIndex(0);

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_FACTORY_RESET );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setUpFactoryTimeRecipeList()
{
    TimeRecipe* pRecipe = new TimeRecipe( this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE1 );
    pRecipe->setRecipeBigValue( 19 );
    pRecipe->setRecipeCommaValue( 4 );
    pRecipe->setDddBigValue(77);
    pRecipe->setDddCommaValue(3);
    pRecipe->setPortaFilterIndex(0);
    pRecipe->setRecipeIconOn(true);
    pRecipe->setShowCoarseness(true);

    m_theTimeRecipeList.append( pRecipe );

    pRecipe = new TimeRecipe( this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE2 );
    pRecipe->setRecipeBigValue( 15 );
    pRecipe->setRecipeCommaValue( 9 );
    pRecipe->setDddBigValue(45);
    pRecipe->setDddCommaValue(6);
    pRecipe->setPortaFilterIndex(1);
    pRecipe->setRecipeIconOn(true);

    m_theTimeRecipeList.append( pRecipe );

    pRecipe = new TimeRecipe( this );
    pRecipe->setUuid( RECIPE_UUID_RECIPE3 );
    pRecipe->setRecipeBigValue( 8 );
    pRecipe->setRecipeCommaValue(1 );
    pRecipe->setDddBigValue(33);
    pRecipe->setDddCommaValue(3);
    pRecipe->setPortaFilterIndex(2);
    pRecipe->setRecipeIconOn(true);

    m_theTimeRecipeList.append( pRecipe );

    setCurrentRecipeCount( m_theTimeRecipeList.length() );
    emit timeRecipeListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void RecipeControl::setUpFactoryLibraryRecipeList()
{
    LibraryRecipe* recipe = new LibraryRecipe( this );
    recipe->setUuid( RECIPE_UUID_ELPUENTE );
    recipe->setBeanName("Tiger");
    recipe->setDddBigValue(56);
    recipe->setDddCommaValue(8);
    recipe->setGrammage1Big(39);
    recipe->setGrammage1Comma(0);
    recipe->setGrammage2Big(35);
    recipe->setGrammage2Comma(2);
    recipe->setGrammage1Icon(0);
    recipe->setGrammage2Icon(1);
    recipe->setGrammageCount(2);
    recipe->setShowGrammage(true);

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( this );
    recipe->setUuid( RECIPE_UUID_BURUNDIBWAYI );
    recipe->setBeanName("Big Lion");
    recipe->setDddBigValue(56);
    recipe->setDddCommaValue(8);
    recipe->setGrammage1Big(39);
    recipe->setGrammage1Comma(0);
    recipe->setGrammage2Big(35);
    recipe->setGrammage2Comma(2);
    recipe->setGrammage1Icon(0);
    recipe->setGrammage2Icon(1);
    recipe->setGrammageCount(2);
    recipe->setShowGrammage(true);

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( this );
    recipe->setUuid( RECIPE_UUID_MISCHGOLD );
    recipe->setBeanName("Snickers");
    recipe->setDddBigValue(19);
    recipe->setDddCommaValue(3);
    recipe->setGrammage1Big(39);
    recipe->setGrammage1Comma(0);
    recipe->setGrammage2Big(35);
    recipe->setGrammage2Comma(2);
    recipe->setGrammage1Icon(0);
    recipe->setGrammage2Icon(1);
    recipe->setGrammageCount(2);
    recipe->setShowGrammage(true);

    m_theLibraryRecipeList.append( recipe );

    recipe = new LibraryRecipe( this );
    recipe->setUuid( RECIPE_UUID_LADALIA );
    recipe->setBeanName("Coyote");
    recipe->setDddBigValue(19);
    recipe->setDddCommaValue(3);
    recipe->setGrammage1Big(0);
    recipe->setGrammage1Comma(0);
    recipe->setGrammage2Big(0);
    recipe->setGrammage2Comma(0);
    recipe->setGrammage1Icon(0);
    recipe->setGrammage2Icon(0);
    recipe->setGrammageCount(0);
    recipe->setShowGrammage(true);

    m_theLibraryRecipeList.append( recipe );

    emit libraryRecipeListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

