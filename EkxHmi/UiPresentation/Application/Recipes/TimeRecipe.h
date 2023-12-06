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

    Q_PROPERTY( QString       uuid                       READ uuid                    WRITE setUuid                     NOTIFY uuidChanged )
    Q_PROPERTY( int           recipeBigValue             READ recipeBigValue          WRITE setRecipeBigValue           NOTIFY recipeBigValueChanged )
    Q_PROPERTY( int           recipeCommaValue           READ recipeCommaValue        WRITE setRecipeCommaValue         NOTIFY recipeCommaValueChanged )
    Q_PROPERTY( int           dddBigValue                READ dddBigValue             WRITE setDddBigValue              NOTIFY dddBigValueChanged )
    Q_PROPERTY( int           dddCommaValue              READ dddCommaValue           WRITE setDddCommaValue            NOTIFY dddCommaValueChanged )
    Q_PROPERTY( int           portaFilterIndex           READ portaFilterIndex        WRITE setPortaFilterIndex         NOTIFY portaFilterIndexChanged )
    Q_PROPERTY( bool          recipeIconOn               READ recipeIconOn            WRITE setRecipeIconOn             NOTIFY recipeIconOnChanged )
    Q_PROPERTY( bool          showCoarseness             READ showCoarseness          WRITE setShowCoarseness           NOTIFY showCoarsenessChanged )

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

    explicit TimeRecipe(QObject *parent = 0);
    virtual ~TimeRecipe() override;

    void setRecipeBigValue( const int nRecipeBigValue );
    int  recipeBigValue() const;

    void setRecipeCommaValue( const int nRecipeCommaValue );
    int  recipeCommaValue() const;

    void setDddBigValue( const int nDddBigValue );
    int  dddBigValue() const;

    void setDddCommaValue( const int nDddCommaValue );
    int  dddCommaValue() const ;

    void setPortaFilterIndex( const int nPortaFilterIndex );
    int  portaFilterIndex() const;

    void setRecipeIconOn( const bool bRecipeIconOn );
    bool recipeIconOn() const;

    void setShowCoarseness( const bool bShow );
    bool showCoarseness() const;

    void setUuid( const QString & strUuid );
    const QString & uuid( ) const;

signals:

    void recipeBigValueChanged();
    void recipeCommaValueChanged();
    void dddBigValueChanged();
    void dddCommaValueChanged();
    void portaFilterIndexChanged();
    void recipeIconOnChanged();
    void showCoarsenessChanged();
    void uuidChanged();

private:

    QString     m_strUuid;
    int         m_nRecipeBigValue { 0 };
    int         m_nRecipeCommaValue { 0 };
    int         m_nDddBigValue { 0 };
    int         m_nDddCommaValue { 0 };
    int         m_nPortaFilterIndex { 0 };
    bool        m_bRecipeIconOn { true };
    bool        m_bShowCoarseness { true };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // TimeRecipe_h
