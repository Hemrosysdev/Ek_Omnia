///////////////////////////////////////////////////////////////////////////////
///
/// @file RecipeControl.h
///
/// @brief Header file of class RecipeControl.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 21.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef RecipeControl_h
#define RecipeControl_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QQmlListProperty>
#include <QTimer>

#include "TimeRecipe.h"
#include "LibraryRecipe.h"

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class RecipeControl
        : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QQmlListProperty<TimeRecipe>    installedRecipeList                                           READ qlpTimeRecipeList          NOTIFY timeRecipeListChanged )
    Q_PROPERTY( QQmlListProperty<LibraryRecipe> installedLibraryList                                          READ qlpLibraryRecipeList       NOTIFY libraryRecipeListChanged )
    Q_PROPERTY( int                             currentTimeRecipeCount                                        READ currentTimeRecipeCount     NOTIFY currentTimeRecipeCountChanged )
    Q_PROPERTY( int                             currentTimeRecipeIndex     WRITE setCurrentTimeRecipeIndex    READ currentTimeRecipeIndex     NOTIFY currentTimeRecipeIndexChanged )
    Q_PROPERTY( int                             currentLibraryRecipeIndex  WRITE setCurrentLibraryRecipeIndex READ currentLibraryRecipeIndex  NOTIFY currentLibraryIndexChanged )

public:

    explicit RecipeControl( QQmlEngine* pEngine,
                            MainStatemachine * pMainStatemachine );
    virtual ~RecipeControl() override;

    void create();
    void clearRecipes();

    QList<TimeRecipe*> timeRecipeList();

    QList<LibraryRecipe*> libraryRecipeList();

    Q_INVOKABLE int currentTimeRecipeCount();
    void setCurrentRecipeCount( const int nCurrentRecipeCount );

    void setCurrentTimeRecipeIndex(const int nCurrentRecipeIndex );
    int currentTimeRecipeIndex() const;

    void setCurrentLibraryRecipeIndex(int nCurrentLibraryIndex );
    int currentLibraryRecipeIndex() const;

    Q_INVOKABLE void addTimeRecipe();
    Q_INVOKABLE void editTimeRecipe( const int nRecipeBig,
                                     const int nRecipeComma,
                                     const int nDddBig,
                                     const int nDddComma,
                                     const int nPortaFilterIndex,
                                     const bool bIconOn,
                                     const bool bShowCoarseness,
                                     const int nRecipeIndex);
    Q_INVOKABLE void deleteTimeRecipe( const int nIndex );

    Q_INVOKABLE void addLibraryRecipe();
    Q_INVOKABLE void editLibraryRecipe( const int nDddBig,
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
                                        const int nRecipeIndex );
    Q_INVOKABLE void deleteLibraryRecipe( const int nRecipeIndex );
    void sortLibraryRecipes( void );
    Q_INVOKABLE void addLibraryGrammage( const int nRecipeIndex );
    Q_INVOKABLE void deleteLibraryGrammage( const int nRecipeIndex,
                                            const int nGrammageIndex );

    Q_INVOKABLE bool saveRecipes() const;
    bool loadRecipes();

    void checkAndConvertRecipeConfig();

    Q_INVOKABLE void factoryReset();

    Q_INVOKABLE int maxTimeRecipesNum() const;

    Q_INVOKABLE int getMaxDddValue() const;

    QString currentRecipeUuid() const;

protected:

    QQmlListProperty<TimeRecipe> qlpTimeRecipeList() const;
    QQmlListProperty<LibraryRecipe> qlpLibraryRecipeList() const;

signals:

    void timeRecipeListChanged();
    void libraryRecipeListChanged();
    void currentTimeRecipeCountChanged();
    void currentTimeRecipeIndexChanged();
    void currentLibraryIndexChanged();

private:

    void ensureConsistentDbRecipes();
    void resetTimeRecipeUuids();
    void setUpRecipeInterface();
    void setUpFactoryTimeRecipeList();
    void setUpFactoryLibraryRecipeList();

private:

    static int                      m_nTimeRecipeTypeId;
    static int                      m_nLibraryRecipeTypeId;

    MainStatemachine *              m_pMainStatemachine { nullptr };

    QList<TimeRecipe*>              m_theTimeRecipeList;
    QQmlListProperty<TimeRecipe>    m_theQlpTimeRecipeList;

    QList<LibraryRecipe*>           m_theLibraryRecipeList;
    QQmlListProperty<LibraryRecipe> m_theQlpLibraryRecipeList;

    int                             m_nCurrentTimeRecipeCount { 3 };
    int                             m_nCurrentTimeRecipeIndex { 0 };
    int                             m_nCurrentLibraryIndex { 0 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // RecipeControl_h
