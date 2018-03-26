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

#include "joycontrolstickpushbutton.h"
#include "joycontrolstickcontextmenu.h"
#include "joycontrolstick.h"

#include <QDebug>

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->stick = stick;

    refreshLabel();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(refreshLabel()));
}

JoyControlStick* JoyControlStickPushButton::getStick()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return stick;
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyControlStickPushButton::generateLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (!stick->getStickName().isEmpty() && displayNames)
    {
        temp.append(stick->getPartialName(false, true));
    }
    else
    {
        temp.append(stick->getPartialName(false));
    }

    qDebug() << "Name of joy control stick push button: " << temp;
    return temp;
}

void JoyControlStickPushButton::disableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    disconnect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyControlStickPushButton::enableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

void JoyControlStickPushButton::showContextMenu(const QPoint &point)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QPoint globalPos = this->mapToGlobal(point);
    JoyControlStickContextMenu *contextMenu = new JoyControlStickContextMenu(stick, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyControlStickPushButton::tryFlash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (stick->getCurrentDirection() != JoyControlStick::StickCentered)
    {
        flash();
    }
}
