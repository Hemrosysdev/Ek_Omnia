///////////////////////////////////////////////////////////////////////////////
///
/// @file EkxQuickView.h
///
/// @brief Header file of class EkxQuickView.
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

#ifndef EkxQuickView_h
#define EkxQuickView_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QQuickView>

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EkxQuickView : public QQuickView
{
    Q_OBJECT

public:

    explicit EkxQuickView();

    void setMainStatemachine( MainStatemachine * pMainStatemachine );

    void applyBrightnessSystemValue( int brightnessSystemvalue );

    static QString rootPath();

protected:

    bool event( QEvent * e ) override;

#ifndef TARGETBUILD
    void keyPressEvent( QKeyEvent *e ) override;

    void keyReleaseEvent( QKeyEvent *e ) override;
#endif

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    double  m_dSimulatedRotation { 0.0 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EkxQuickView_h
