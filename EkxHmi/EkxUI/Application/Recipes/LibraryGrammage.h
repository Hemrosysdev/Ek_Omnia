///////////////////////////////////////////////////////////////////////////////
///
/// @file LibraryGrammage.h
///
/// @brief Header file of class LibraryGrammage.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Kai Uwe Broulik, kai.uwe.broulik@basyskom.com
///
/// @date 2023-02-17
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef LIBRARYGRAMMAGE_H
#define LIBRARYGRAMMAGE_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>
#include <QVector>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class LibraryRecipe;

class LibraryGrammage : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged )
    //* Whether the grammage has been saved to disk once or is "still new". */
    Q_PROPERTY( bool persisted READ persisted NOTIFY persistedChanged )

    Q_PROPERTY( Icon icon READ icon WRITE setIcon NOTIFY iconChanged )
    Q_PROPERTY( int grammageMg READ grammageMg WRITE setGrammageMg NOTIFY grammageMgChanged )
    Q_PROPERTY( int dosingTimeMs READ dosingTimeMs WRITE setDosingTimeMs NOTIFY dosingTimeMsChanged )

public:

    explicit LibraryGrammage( bool      bPersisted,
                              QObject * parent = nullptr );
    ~LibraryGrammage() override;

    using List = QVector<LibraryGrammage *>;

    enum Icon
    {
        Cup,
        SingleFilter,
        DoubleFilter
    };
    Q_ENUM( Icon )

    QString uuid() const;

    void setUuid( const QString & strUuid );

    Q_SIGNAL void uuidChanged();

    bool persisted() const;

    void setPersisted( bool bPersisted );

    Q_SIGNAL void persistedChanged();

    Icon icon() const;

    void setIcon( Icon nIcon );

    Q_SIGNAL void iconChanged();

    int grammageMg() const;

    void setGrammageMg( int nGrammageMg );

    Q_SIGNAL void grammageMgChanged();

    int dosingTimeMs() const;

    void setDosingTimeMs( int nDosingTimeMs );

    Q_SIGNAL void dosingTimeMsChanged();

private:

    QString m_strUuid;
    bool    m_bPersisted { false };
    Icon    m_nIcon { Cup };
    int     m_nGrammageMg { 0 };
    int     m_nDosingTimeMs { 0 };

};

Q_DECLARE_METATYPE( LibraryGrammage::List )
Q_DECLARE_METATYPE( LibraryGrammage::Icon )

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // LIBRARYGRAMMAGE_H
