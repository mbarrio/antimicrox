/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dpadpushbuttongroup.h"
#include "joydpad.h"
#include "joydpadbuttonwidget.h"
#include "dpadpushbutton.h"
#include "buttoneditdialog.h"
#include "dpadeditdialog.h"
#include "inputdevice.h"

#include <QHash>
#include <QWidget>
#include <QDebug>

DPadPushButtonGroup::DPadPushButtonGroup(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->dpad = dpad;
    this->displayNames = displayNames;

    generateButtons();
    changeButtonLayout();

    connect(dpad, SIGNAL(joyModeChanged()), this, SLOT(changeButtonLayout()));
}

void DPadPushButtonGroup::generateButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

    JoyDPadButton *button = nullptr;
    JoyDPadButtonWidget *pushbutton = nullptr;

    button = buttons->value(JoyDPadButton::DpadLeftUp);
    upLeftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));

    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 0);


    button = buttons->value(JoyDPadButton::DpadUp);
    upButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 1);

    button = buttons->value(JoyDPadButton::DpadRightUp);
    upRightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 2);

    button = buttons->value(JoyDPadButton::DpadLeft);
    leftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = leftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 0);

    dpadWidget = new DPadPushButton(dpad, displayNames, parentWidget());
    dpadWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
    connect(dpadWidget, SIGNAL(clicked()), this, SLOT(showDPadDialog()));
    addWidget(dpadWidget, 1, 1);

    button = buttons->value(JoyDPadButton::DpadRight);
    rightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = rightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 2);

    button = buttons->value(JoyDPadButton::DpadLeftDown);
    downLeftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 0);

    button = buttons->value(JoyDPadButton::DpadDown);
    downButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 1);

    button = buttons->value(JoyDPadButton::DpadRightDown);
    downRightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 2);
}

void DPadPushButtonGroup::changeButtonLayout()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((dpad->getJoyMode() == JoyDPad::StandardMode) ||
        (dpad->getJoyMode() == JoyDPad::EightWayMode) ||
        (dpad->getJoyMode() == JoyDPad::FourWayCardinal))
    {
        upButton->setVisible(true);
        downButton->setVisible(true);
        leftButton->setVisible(true);
        rightButton->setVisible(true);
    }
    else
    {
        upButton->setVisible(false);
        downButton->setVisible(false);
        leftButton->setVisible(false);
        rightButton->setVisible(false);
    }

    if ((dpad->getJoyMode() == JoyDPad::EightWayMode) ||
        (dpad->getJoyMode() == JoyDPad::FourWayDiagonal))
    {
        upLeftButton->setVisible(true);
        upRightButton->setVisible(true);
        downLeftButton->setVisible(true);
        downRightButton->setVisible(true);
    }
    else
    {
        upLeftButton->setVisible(false);
        upRightButton->setVisible(false);
        downLeftButton->setVisible(false);
        downRightButton->setVisible(false);
    }
}

void DPadPushButtonGroup::propogateSlotsChanged()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    emit buttonSlotChanged();
}

JoyDPad* DPadPushButtonGroup::getDPad() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return dpad;
}

void DPadPushButtonGroup::openDPadButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButtonWidget *buttonWidget = qobject_cast<JoyButtonWidget*>(sender()); // static_cast
    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, dpad->getParentSet()->getInputDevice(), parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::showDPadDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    DPadEditDialog *dialog = new DPadEditDialog(dpad, parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::toggleNameDisplay()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    displayNames = !displayNames;

    upButton->toggleNameDisplay();
    downButton->toggleNameDisplay();
    leftButton->toggleNameDisplay();
    rightButton->toggleNameDisplay();

    upLeftButton->toggleNameDisplay();
    upRightButton->toggleNameDisplay();
    downLeftButton->toggleNameDisplay();
    downRightButton->toggleNameDisplay();

    dpadWidget->toggleNameDisplay();
}
