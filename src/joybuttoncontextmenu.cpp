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

#include "joybuttoncontextmenu.h"
#include "inputdevice.h"
#include "common.h"
#include "joybutton.h"

#include <QDebug>
#include <QActionGroup>
#include <QWidget>


JoyButtonContextMenu::JoyButtonContextMenu(JoyButton *button, QWidget *parent) :
    QMenu(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->button = button;

    connect(this, SIGNAL(aboutToHide()), this, SLOT(deleteLater()));
}

void JoyButtonContextMenu::buildMenu()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QAction *action = nullptr;

    PadderCommon::inputDaemonMutex.lock();

    action = this->addAction(trUtf8("Toggle"));
    action->setCheckable(true);
    action->setChecked(button->getToggleState());
    connect(action, SIGNAL(triggered()), this, SLOT(switchToggle()));

    action = this->addAction(trUtf8("Turbo"));
    action->setCheckable(true);
    action->setChecked(button->isUsingTurbo());
    connect(action, SIGNAL(triggered()), this, SLOT(switchToggle()));

    this->addSeparator();

    action = this->addAction(trUtf8("Clear"));
    action->setCheckable(false);
    connect(action, SIGNAL(triggered()), this, SLOT(clearButton()));

    this->addSeparator();

    QMenu *setSectionMenu = this->addMenu(trUtf8("Set Select"));

    action = setSectionMenu->addAction(trUtf8("Disabled"));
    if (button->getChangeSetCondition() == JoyButton::SetChangeDisabled)
    {
        action->setCheckable(true);
        action->setChecked(true);
    }
    connect(action, SIGNAL(triggered()), this, SLOT(disableSetMode()));

    setSectionMenu->addSeparator();

    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        QMenu *tempSetMenu = setSectionMenu->addMenu(trUtf8("Set %1").arg(i+1));
        int setSelection = i*3;

        if (i == button->getSetSelection())
        {
            QFont tempFont = tempSetMenu->menuAction()->font();
            tempFont.setBold(true);
            tempSetMenu->menuAction()->setFont(tempFont);
        }

        QActionGroup *tempGroup = new QActionGroup(tempSetMenu);

        action = tempSetMenu->addAction(trUtf8("Set %1 1W").arg(i+1));
        action->setData(QVariant(setSelection + 0));
        action->setCheckable(true);
        if ((button->getSetSelection() == i) &&
            (button->getChangeSetCondition() == JoyButton::SetChangeOneWay))
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        action = tempSetMenu->addAction(trUtf8("Set %1 2W").arg(i+1));
        action->setData(QVariant(setSelection + 1));
        action->setCheckable(true);
        if ((button->getSetSelection() == i) &&
            (button->getChangeSetCondition() == JoyButton::SetChangeTwoWay))
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        action = tempSetMenu->addAction(trUtf8("Set %1 WH").arg(i+1));
        action->setData(QVariant(setSelection + 2));
        action->setCheckable(true);
        if ((button->getSetSelection() == i) &&
            (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld))
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        if (i == button->getParentSet()->getIndex())
        {
            tempSetMenu->setEnabled(false);
        }
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::switchToggle()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    PadderCommon::inputDaemonMutex.lock();
    button->setToggle(!button->getToggleState());
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::switchTurbo()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    PadderCommon::inputDaemonMutex.lock();
    button->setToggle(!button->isUsingTurbo());
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::switchSetMode()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QAction *action = qobject_cast<QAction*>(sender()); // static_cast
    int item = action->data().toInt();
    int setSelection = item / 3;
    int setChangeCondition = item % 3;
    JoyButton::SetChangeCondition temp;

    if (setChangeCondition == 0)
    {
        temp = JoyButton::SetChangeOneWay;
    }
    else if (setChangeCondition == 1)
    {
        temp = JoyButton::SetChangeTwoWay;
    }
    else if (setChangeCondition == 2)
    {
        temp = JoyButton::SetChangeWhileHeld;
    }

    PadderCommon::inputDaemonMutex.lock();
    // First, remove old condition for the button in both sets.
    // After that, make the new assignment.
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    button->setChangeSetSelection(setSelection);
    button->setChangeSetCondition(temp);
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::disableSetMode()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    PadderCommon::inputDaemonMutex.lock();
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::clearButton()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QMetaObject::invokeMethod(button, "clearSlotsEventReset");
}
