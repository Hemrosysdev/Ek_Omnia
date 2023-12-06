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
#if QT_VERSION < QT_VERSION_CHECK( 5, 15, 0 )
#include <QVariant>
#endif

#include "LibraryGrammage.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class LibraryRecipe : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged )
    //* Whether the recipe has been saved to disk once or is "still new". */
    Q_PROPERTY( bool persisted READ persisted NOTIFY persistedChanged )
    Q_PROPERTY( int grindingSize READ grindingSize WRITE setGrindingSize NOTIFY grindingSizeChanged )
    Q_PROPERTY( QString beanName READ beanName WRITE setBeanName NOTIFY beanNameChanged )
    Q_PROPERTY( qreal calibrationSlope READ calibrationSlope WRITE setCalibrationSlope NOTIFY calibrationSlopeChanged )
    Q_PROPERTY( int calibrationOffset READ calibrationOffset WRITE setCalibrationOffset NOTIFY calibrationOffsetChanged )
    Q_PROPERTY( bool calibrated READ calibrated NOTIFY calibratedChanged STORED false )

#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    Q_PROPERTY( LibraryGrammage::List grammages READ grammages NOTIFY grammagesChanged )
#else
    Q_PROPERTY( QVariantList grammages READ grammagesVariant NOTIFY grammagesChanged )
#endif

public:

    explicit LibraryRecipe( bool      bPersisted,
                            QObject * parent = nullptr );
    ~LibraryRecipe() override;

    QString uuid() const;

    void setUuid( const QString & strUuid );

    Q_SIGNAL void uuidChanged();

    bool persisted() const;

    void setPersisted( bool bPersisted );

    Q_SIGNAL void persistedChanged();

    int grindingSize() const;

    void setGrindingSize( const int nGrindingSize );

    Q_SIGNAL void grindingSizeChanged();

    QString beanName() const;

    void setBeanName( const QString & strBeanName );

    Q_SIGNAL void beanNameChanged();

    qreal calibrationSlope() const;

    void setCalibrationSlope( qreal nCalibrationSlope );

    Q_SIGNAL void calibrationSlopeChanged();

    int calibrationOffset() const;

    void setCalibrationOffset( int nCalibrationOffset );

    Q_SIGNAL void calibrationOffsetChanged();

    bool calibrated() const;

    Q_SIGNAL void calibratedChanged();

    LibraryGrammage::List grammages() const;

#if QT_VERSION < QT_VERSION_CHECK( 5, 15, 0 )
    QVariantList grammagesVariant() const;

#endif
    void setGrammages( const LibraryGrammage::List & grammages );

    Q_SIGNAL void grammagesChanged();

    void addGrammage( LibraryGrammage * pGrammage );

private:

    QString               m_strUuid;
    bool                  m_bPersisted { false };
    int                   m_nGrindingSize { 0 };
    QString               m_strBeanName;
    qreal                 m_nCalibrationSlope { 0.0 };
    int                   m_nCalibrationOffset { 0 };

    LibraryGrammage::List m_grammages;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // LibraryRecipe_h
