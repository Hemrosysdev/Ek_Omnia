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

class MainStatemachine;
class TimeRecipe;
class LibraryRecipe;
class LibraryGrammage;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class RecipeControl
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QQmlListProperty<TimeRecipe>    installedRecipeList READ qlpTimeRecipeList NOTIFY timeRecipeListChanged )
    Q_PROPERTY( QQmlListProperty<LibraryRecipe> installedLibraryList READ qlpLibraryRecipeList NOTIFY libraryRecipeListChanged )
    Q_PROPERTY( int maxTimeRecipesNum READ maxTimeRecipesNum CONSTANT )
    Q_PROPERTY( int currentTimeRecipeCount READ currentTimeRecipeCount NOTIFY timeRecipeListChanged )
    Q_PROPERTY( int currentTimeRecipeIndex WRITE setCurrentTimeRecipeIndex READ currentTimeRecipeIndex NOTIFY currentTimeRecipeIndexChanged )
    Q_PROPERTY( int maxLibraryRecipesNum READ maxLibraryRecipesNum CONSTANT )
    Q_PROPERTY( int currentLibraryRecipeIndex WRITE setCurrentLibraryRecipeIndex READ currentLibraryRecipeIndex NOTIFY currentLibraryIndexChanged )
    Q_PROPERTY( int maxLibraryGrammageNum READ maxLibraryGrammageNum CONSTANT )
    Q_PROPERTY( int currentLibraryGrammageCount READ currentLibraryGrammageCount NOTIFY currentLibraryGrammageCountChanged )
    Q_PROPERTY( int currentLibraryGrammageIndex WRITE setCurrentLibraryGrammageIndex READ currentLibraryGrammageIndex NOTIFY currentLibraryGrammageIndexChanged )
    Q_PROPERTY( bool agsaAllowed WRITE setAgsaAllowed READ isAgsaAllowed NOTIFY agsaAllowedChanged )
    Q_PROPERTY( int currentGrindingSize READ currentGrindingSize NOTIFY currentGrindingSizeChanged )

public:

    explicit RecipeControl( QObject * pParent = nullptr );
    ~RecipeControl() override;

    void create( MainStatemachine * pMainStatemachine );

    void clearRecipes();

    QList<TimeRecipe *> timeRecipeList();

    QList<LibraryRecipe *> libraryRecipeList();

    int maxTimeRecipesNum() const;

    int currentTimeRecipeCount() const;

    void setCurrentTimeRecipeIndex( int nCurrentRecipeIndex );

    int currentTimeRecipeIndex() const;

    int maxLibraryRecipesNum() const;

    void setCurrentLibraryRecipeIndex( int nCurrentLibraryIndex );

    int currentLibraryRecipeIndex() const;

    int maxLibraryGrammageNum() const;

    int currentLibraryGrammageCount() const;

    void setCurrentLibraryGrammageIndex( const int nCurrentLibraryGrammageIndex );

    int currentLibraryGrammageIndex() const;

    Q_INVOKABLE TimeRecipe * createTimeRecipe() const;

    Q_INVOKABLE int addTimeRecipe( TimeRecipe * pRecipe );

    Q_INVOKABLE void editTimeRecipe( TimeRecipe *    pRecipe,
                                     const int       nRecipeBig,
                                     const int       nRecipeComma,
                                     const int       nGrindingSize,
                                     const int       nPortaFilterIndex,
                                     const bool      bIconOn,
                                     const bool      bShowCoarseness,
                                     const QString & strRecipeName,
                                     const bool      bShowRecipeName );

    Q_INVOKABLE void deleteTimeRecipe( const int nIndex );

    Q_INVOKABLE LibraryRecipe * createLibraryRecipe() const;

    Q_INVOKABLE int addLibraryRecipe( LibraryRecipe * pRecipe );

    Q_INVOKABLE void editLibraryRecipe( LibraryRecipe * pRecipe,
                                        const QString & strBeanName,
                                        int             nGrindingSize );

    Q_INVOKABLE void calibrateLibraryRecipe( LibraryRecipe * pRecipe,
                                             int             nFirstGrindDurationMs,
                                             int             nFirstGrindAmountMg,
                                             int             nSecondGrindDurationMs,
                                             int             nSecondGrindAmountMg );

    Q_INVOKABLE void deleteLibraryRecipe( const int nRecipeIndex );

    void sortLibraryRecipes( void );

    Q_INVOKABLE LibraryGrammage * createLibraryGrammage( LibraryRecipe * pRecipe ) const;

    Q_INVOKABLE int addLibraryGrammage( LibraryRecipe *   pRecipe,
                                        LibraryGrammage * pGrammage );

    Q_INVOKABLE void editLibraryGrammage( LibraryGrammage *             pGrammage,
                                          int /*LibraryGrammage::Icon*/ nIcon,
                                          int                           nGrammageMg,
                                          int                           nDosingTimeMs );

    Q_INVOKABLE void deleteLibraryGrammage( LibraryRecipe * pRecipe,
                                            int             nGrammageIndex );

    Q_INVOKABLE bool saveRecipes() const;

    bool loadRecipes();

    void checkAndConvertRecipeConfig();

    Q_INVOKABLE void factoryReset();

    Q_INVOKABLE int getMaxDddValue() const;

    QString currentRecipeUuid() const;

    Q_INVOKABLE void startDelayedAgsa();

    Q_INVOKABLE void stopDelayedAgsa();

    void setAgsaAllowed( bool bAllowed );

    bool isAgsaAllowed() const;

    int currentGrindingSize() const;

public slots:

    void processMainStateChanged();

    void processStartDelayedAgsa();

protected:

    QQmlListProperty<TimeRecipe> qlpTimeRecipeList() const;

    QQmlListProperty<LibraryRecipe> qlpLibraryRecipeList() const;

signals:

    void timeRecipeListChanged();

    void libraryRecipeListChanged();

    void currentTimeRecipeIndexChanged();

    void currentLibraryIndexChanged();

    void currentLibraryGrammageCountChanged();

    void currentLibraryGrammageIndexChanged();

    void agsaAllowedChanged();

    void currentGrindingSizeChanged();

private:

    void ensureConsistentDbRecipes();

    void resetTimeRecipeUuids();

    void resetTimeRecipeNames();

    void setUpRecipeInterface();

    void setUpFactoryTimeRecipeList();

    void setUpFactoryLibraryRecipeList();

private:

    MainStatemachine *              m_pMainStatemachine { nullptr };

    QList<TimeRecipe *>             m_theTimeRecipeList;
    QQmlListProperty<TimeRecipe>    m_theQlpTimeRecipeList;

    QList<LibraryRecipe *>          m_theLibraryRecipeList;
    QQmlListProperty<LibraryRecipe> m_theQlpLibraryRecipeList;

    int                             m_nCurrentTimeRecipeIndex { 0 };
    int                             m_nCurrentLibraryIndex { 0 };
    int                             m_nCurrentLibraryGrammageIndex { 0 };

    QTimer                          m_timerStartDelayedAgsa;

    bool                            m_bAgsaAllowed { true };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // RecipeControl_h
