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

#include "gamecontrollermappingdialoghelper.h"
#include "inputdevice.h"

#include <QDebug>

GameControllerMappingDialogHelper::GameControllerMappingDialogHelper(InputDevice *device,
                                                                     QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->device = device;
}


void GameControllerMappingDialogHelper::raiseDeadZones()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    device->setRawAxisDeadZone(InputDevice::RAISEDDEADZONE);
    device->getActiveSetJoystick()->raiseAxesDeadZones();
}

void GameControllerMappingDialogHelper::raiseDeadZones(int deadZone)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    device->getActiveSetJoystick()->raiseAxesDeadZones(deadZone);
    device->setRawAxisDeadZone(deadZone);
}

void GameControllerMappingDialogHelper::setupDeadZones()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    device->getActiveSetJoystick()->setIgnoreEventState(true);
    device->getActiveSetJoystick()->release();

    device->getActiveSetJoystick()->currentAxesDeadZones(&originalAxesDeadZones);
    device->getActiveSetJoystick()->raiseAxesDeadZones();

    device->setRawAxisDeadZone(InputDevice::RAISEDDEADZONE);
}

void GameControllerMappingDialogHelper::restoreDeviceDeadZones()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    device->getActiveSetJoystick()->setIgnoreEventState(false);
    device->getActiveSetJoystick()->release();
    device->getActiveSetJoystick()->setAxesDeadZones(&originalAxesDeadZones);

    device->setRawAxisDeadZone(InputDevice::RAISEDDEADZONE);
}
