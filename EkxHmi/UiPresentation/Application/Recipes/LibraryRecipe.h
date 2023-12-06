///////////////////////////////////////////////////////////////////////////////
///
/// @file LibraryRecipe.h
///
/// @brief Header file of class LibraryRecipe.
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

#ifndef LibraryRecipe_h
#define LibraryRecipe_h

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

class LibraryRecipe : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString       uuid                       READ uuid                    WRITE setUuid                     NOTIFY uuidChanged )
    Q_PROPERTY( int           dddBigValue                READ dddBigValue             WRITE setDddBigValue              NOTIFY dddBigValueChanged )
    Q_PROPERTY( int           dddCommaValue              READ dddCommaValue           WRITE setDddCommaValue            NOTIFY dddCommaValueChanged )
    Q_PROPERTY( QString       beanName                   READ beanName                WRITE setBeanName                 NOTIFY beanNameChanged )
    Q_PROPERTY( int           grammage1Big               READ grammage1Big            WRITE setGrammage1Big             NOTIFY grammage1BigChanged )
    Q_PROPERTY( int           grammage2Big               READ grammage2Big            WRITE setGrammage2Big             NOTIFY grammage2BigChanged )
    Q_PROPERTY( int           grammage1Comma             READ grammage1Comma          WRITE setGrammage1Comma           NOTIFY grammage1CommaChanged )
    Q_PROPERTY( int           grammage2Comma             READ grammage2Comma          WRITE setGrammage2Comma           NOTIFY grammage2CommaChanged )
    Q_PROPERTY( int           grammage1Icon              READ grammage1Icon           WRITE setGrammage1Icon            NOTIFY grammage1IconChanged )
    Q_PROPERTY( int           grammage2Icon              READ grammage2Icon           WRITE setGrammage2Icon            NOTIFY grammage2IconChanged )
    Q_PROPERTY( int           grammageCount              READ grammageCount           WRITE setGrammageCount            NOTIFY grammageCountChanged )
    Q_PROPERTY( bool          showGrammage               READ showGrammage            WRITE setShowGrammage             NOTIFY showGrammageChanged )

public:

    explicit LibraryRecipe( QObject *parent = 0 );
    virtual ~LibraryRecipe() override;

    void setDddBigValue( const int nDddBigValue );
    int dddBigValue() const;

    void setDddCommaValue( const int nDddCommaValue );
    int dddCommaValue() const;

    void setBeanName( const QString & strBeanName );
    const QString & beanName() const;

    int grammage1Big() const;
    void setGrammage1Big( const int nGrammage1Big );

    int grammage2Big() const;
    void setGrammage2Big( const int nGrammage2Big );

    int grammage1Comma() const;
    void setGrammage1Comma( const int nGrammage1Comma );

    int grammage2Comma() const;
    void setGrammage2Comma( const int nGrammage2Comma );

    int grammage1Icon() const;
    void setGrammage1Icon( const int nGrammage1Icon );

    int grammage2Icon() const;
    void setGrammage2Icon( const int nGrammage2Icon );

    int grammageCount() const;
    void setGrammageCount( const int nGrammageCount );

    bool showGrammage() const;
    void setShowGrammage( const bool bShowGrammage );

    void setUuid( const QString & strUuid );
    const QString & uuid( ) const;

signals:

    void dddBigValueChanged();
    void dddCommaValueChanged();
    void beanNameChanged();
    void grammage1BigChanged();
    void grammage2BigChanged();
    void grammage1CommaChanged();
    void grammage2CommaChanged();
    void grammage1IconChanged();
    void grammage2IconChanged();
    void grammageCountChanged();
    void showGrammageChanged();
    void uuidChanged();

private:

    QString  m_strUuid;
    int      m_nDddBigValue { 0 };
    int      m_nDddCommaValue { 0 };
    QString  m_strBeanName;
    int      m_nGrammage1Big { 0 };
    int      m_nGrammage2Big { 0 };
    int      m_nGrammage1Comma { 0 };
    int      m_nGrammage2Comma { 0 };
    int      m_nGrammage1Icon { 0 };
    int      m_nGrammage2Icon { 0 };
    int      m_nGrammageCount { 0 };
    bool     m_bShowGrammage { true };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // LibraryRecipe_h
