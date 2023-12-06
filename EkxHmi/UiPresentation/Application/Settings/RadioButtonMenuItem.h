///////////////////////////////////////////////////////////////////////////////
///
/// @file RadioButtonMenu.h
///
/// @brief Header file of class RadioButtonMenu.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Xaver Pilz, Forschung & Entwicklung, xpilz@ultratronik.de
///
/// @date 25.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef RADIO_BUTTON_MENU_ITEM_H
#define RADIO_BUTTON_MENU_ITEM_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class RadioButtonMenuItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key         READ key         NOTIFY keyChanged)
    Q_PROPERTY(QString value       READ value       NOTIFY valueChanged)
    Q_PROPERTY(QString isChoosable READ isChoosable NOTIFY isChoosableChanged)

public:

    RadioButtonMenuItem();
    RadioButtonMenuItem(QString value, QObject *parent = nullptr);

    QString key();
    void setKey(QString key);

    QString value();
    void setValue(QString value);

    bool isChoosable();
    void setChoosable(bool choosable);

signals:

    void keyChanged();
    void valueChanged();
    void isChoosableChanged();

private:

    QString m_key {""};
    QString m_value;
    bool    m_isChoosable {true};
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // RADIO_BUTTON_MENU_ITEM_H
