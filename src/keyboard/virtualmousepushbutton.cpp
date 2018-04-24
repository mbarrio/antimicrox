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

#include "virtualmousepushbutton.h"

#include <QDebug>

VirtualMousePushButton::VirtualMousePushButton(QString displayText, int code, JoyButtonSlot::JoySlotInputAction mode, QWidget *parent) :
    QPushButton(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((mode == JoyButtonSlot::JoyMouseButton) || (mode == JoyButtonSlot::JoyMouseMovement))
    {
        this->setText(displayText);

        if (mode == JoyButtonSlot::JoyMouseMovement)
        {
            switch (code)
            {
                case JoyButtonSlot::MouseUp:
                case JoyButtonSlot::MouseDown:
                case JoyButtonSlot::MouseLeft:
                case JoyButtonSlot::MouseRight:
                {
                    this->code = code;
                    break;
                }
                default:
                {
                    this->code = 0;
                    break;
                }
            }
        }
        else
        {
            this->code = code;
        }
        this->mode = mode;
    }
    else
    {
        this->setText(trUtf8("INVALID"));
        this->code = 0;
        this->mode = JoyButtonSlot::JoyMouseButton;
    }

    connect(this, SIGNAL(clicked()), this, SLOT(createTempSlot()));
}

int VirtualMousePushButton::getMouseCode() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return code;
}

JoyButtonSlot::JoySlotInputAction VirtualMousePushButton::getMouseMode() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return mode;
}

void VirtualMousePushButton::createTempSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButtonSlot *tempslot = new JoyButtonSlot(this->code, this->mode, this);
    emit mouseSlotCreated(tempslot);
}
