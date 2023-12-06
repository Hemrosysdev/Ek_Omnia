///////////////////////////////////////////////////////////////////////////////
///
/// @file TimeRecipe.h
///
/// @brief Header file of class TimeRecipe.
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

#ifndef TimeRecipe_h
#define TimeRecipe_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class TimeRecipe
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged )
    //* Whether the recipe has been saved to disk once or is "still new". */
    Q_PROPERTY( bool persisted READ persisted NOTIFY persistedChanged )
    Q_PROPERTY( int recipeBigValue READ recipeBigValue WRITE setRecipeBigValue NOTIFY recipeBigValueChanged )
    Q_PROPERTY( int recipeCommaValue READ recipeCommaValue WRITE setRecipeCommaValue NOTIFY recipeCommaValueChanged )
    Q_PROPERTY( int grindingSize READ grindingSize WRITE setGrindingSize NOTIFY grindingSizeChanged )
    Q_PROPERTY( int portaFilterIndex READ portaFilterIndex WRITE setPortaFilterIndex NOTIFY portaFilterIndexChanged )
    Q_PROPERTY( bool recipeIconOn READ recipeIconOn WRITE setRecipeIconOn NOTIFY recipeIconOnChanged )
    Q_PROPERTY( bool showCoarseness READ showCoarseness WRITE setShowCoarseness NOTIFY showCoarsenessChanged )
    Q_PROPERTY( QString recipeName READ recipeName WRITE setRecipeName NOTIFY recipeNameChanged )
    Q_PROPERTY( bool showRecipeName READ showRecipeName WRITE setShowRecipeName NOTIFY showRecipeNameChanged )

public:

    enum PortaFilter
    {
        PORTAFILTER_UNDEF = 0,
        PORTAFILTER_SINGLE,
        PORTAFILTER_DOUBLE,
        PORTAFILTER_BOTTOMLESS
    };
    Q_ENUM( PortaFilter )

public:

    explicit TimeRecipe( bool      bPersisted,
                         QObject * parent = 0 );
    virtual ~TimeRecipe() override;

    void setRecipeBigValue( const int nRecipeBigValue );

    int recipeBigValue() const;

    void setRecipeCommaValue( const int nRecipeCommaValue );

    int recipeCommaValue() const;

    void setGrindingSize( const int nGrindingSize );

    int grindingSize() const;

    void setPortaFilterIndex( const int nPortaFilterIndex );

    int portaFilterIndex() const;

    void setRecipeIconOn( const bool bRecipeIconOn );

    bool recipeIconOn() const;

    void setShowCoarseness( const bool bShow );

    bool showCoarseness() const;

    void setRecipeName( const QString & strRecipeName );

    QString recipeName() const;

    void setShowRecipeName( bool bShowRecipeName );

    bool showRecipeName() const;

    void setUuid( const QString & strUuid );

    const QString & uuid() const;

    bool persisted() const;

    void setPersisted( bool bPersisted );

signals:

    void recipeBigValueChanged();

    void recipeCommaValueChanged();

    void grindingSizeChanged();

    void dddBigValueChanged();

    void dddCommaValueChanged();

    void dddValueChanged();

    void portaFilterIndexChanged();

    void recipeIconOnChanged();

    void showCoarsenessChanged();

    void recipeNameChanged();

    void showRecipeNameChanged();

    void uuidChanged();

    void persistedChanged();

private:

    QString m_strUuid;
    bool    m_bPersisted { false };
    int     m_nRecipeBigValue { 0 };
    int     m_nRecipeCommaValue { 0 };
    int     m_nDddBigValue { 0 };
    int     m_nDddCommaValue { 0 };
    int     m_nGrindingSize { 0 };
    int     m_nPortaFilterIndex { 0 };
    bool    m_bRecipeIconOn { true };
    bool    m_bShowCoarseness { true };
    QString m_strRecipeName;
    bool    m_bShowRecipeName { false };     // TODO Decide with designers.

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // TimeRecipe_h
