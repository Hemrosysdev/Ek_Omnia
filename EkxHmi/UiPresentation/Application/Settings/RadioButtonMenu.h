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

#ifndef RADIO_BUTTON_MENU_H
#define RADIO_BUTTON_MENU_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QList>
#include <QQmlListProperty>

#include "RadioButtonMenuItem.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class RadioButtonMenu : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int                                   selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QQmlListProperty<RadioButtonMenuItem> menuItems     READ qlpMenuItems                         NOTIFY menuItemsChanged)

public:
    explicit RadioButtonMenu(QObject *parent = nullptr);



    int selectedIndex(void);
    void setSelectedIndex(int index);

    QQmlListProperty<RadioButtonMenuItem> qlpMenuItems();

    QList<RadioButtonMenuItem*> menuItems;

signals:

    void menuItemsChanged();
    void selectedIndexChanged();

private:

    int m_selectedIndex;

    static int                  cb_menuItemsCount(QQmlListProperty<RadioButtonMenuItem>* qlp);
    static RadioButtonMenuItem* cb_menuItemsAt(QQmlListProperty<RadioButtonMenuItem>* qlp, int idx);

    static int radioButtonMenu_typeId;
    static int radioButtonMenuItem_typeId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // RADIO_BUTTON_MENU_H
