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

#include "setjoystick.h"
#include "inputdevice.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joybutton.h"
#include "vdpad.h"
#include "joybuttontypes/joycontrolstickbutton.h"

#include <QDebug>
#include <QHashIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtAlgorithms>


const int SetJoystick::MAXNAMELENGTH = 30;
const int SetJoystick::RAISEDDEADZONE = 20000;

SetJoystick::SetJoystick(InputDevice *device, int index, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->device = device;
    this->index = index;

    reset();
}

SetJoystick::SetJoystick(InputDevice *device, int index, bool runreset, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->device = device;
    this->index = index;
    if (runreset)
    {
        reset();
    }
}

SetJoystick::~SetJoystick()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deleteSticks();
    deleteVDpads();
    deleteButtons();
    deleteAxes();
    deleteHats();

    removeAllBtnFromQueue();
}

JoyButton* SetJoystick::getJoyButton(int index) const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return buttons.value(index);
}

JoyAxis* SetJoystick::getJoyAxis(int index) const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return axes.value(index);
}

JoyDPad* SetJoystick::getJoyDPad(int index) const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return hats.value(index);
}

VDPad* SetJoystick::getVDPad(int index) const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return vdpads.value(index);
}

JoyControlStick* SetJoystick::getJoyStick(int index) const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return sticks.value(index);
}

void SetJoystick::refreshButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deleteButtons();

    for (int i=0; i < device->getNumberRawButtons(); i++)
    {
        JoyButton *button = new JoyButton (i, index, this, this);
        buttons.insert(i, button);
        enableButtonConnections(button);
    }
}

void SetJoystick::refreshAxes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deleteAxes();

    InputDevice *device = getInputDevice();
    for (int i=0; i < device->getNumberRawAxes(); i++)
    {
        JoyAxis *axis = new JoyAxis(i, index, this, this);
        axes.insert(i, axis);

        if (device->hasCalibrationThrottle(i))
        {
            JoyAxis::ThrottleTypes throttle = device->getCalibrationThrottle(i);
            axis->setInitialThrottle(throttle);
        }

        enableAxisConnections(axis);
    }
}

void SetJoystick::refreshHats()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deleteHats();

    for (int i=0; i < device->getNumberRawHats(); i++)
    {
        JoyDPad *dpad = new JoyDPad(i, index, this, this);
        hats.insert(i, dpad);
        enableHatConnections(dpad);
    }
}

void SetJoystick::deleteButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            delete button;
            button = nullptr;
        }
    }

    buttons.clear();
}


void SetJoystick::deleteAxes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyAxis*> iter(axes);
    while (iter.hasNext())
    {
        JoyAxis *axis = iter.next().value();
        if (axis)
        {
            delete axis;
            axis = nullptr;
        }
    }

    axes.clear();
}

void SetJoystick::deleteSticks()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyControlStick*> iter(sticks);
    while (iter.hasNext())
    {
        JoyControlStick *stick = iter.next().value();
        if (stick)
        {
            delete stick;
            stick = nullptr;
        }
    }

    sticks.clear();
}

void SetJoystick::deleteVDpads()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, VDPad*> iter(vdpads);
    while (iter.hasNext())
    {
        VDPad *dpad = iter.next().value();
        if (dpad)
        {
            delete dpad;
            dpad = nullptr;
        }
    }

    vdpads.clear();
}

void SetJoystick::deleteHats()
{

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyDPad*> iter(hats);
    while (iter.hasNext())
    {
        JoyDPad *dpad = iter.next().value();
        if (dpad)
        {
            delete dpad;
            dpad = nullptr;
        }
    }

    hats.clear();
}

int SetJoystick::getNumberButtons() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return buttons.count();
}

int SetJoystick::getNumberAxes() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return axes.count();
}

int SetJoystick::getNumberHats() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return hats.count();
}

int SetJoystick::getNumberSticks() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return sticks.size();
}

int SetJoystick::getNumberVDPads() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return vdpads.size();
}

void SetJoystick::reset()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deleteSticks();
    deleteVDpads();
    refreshAxes();
    refreshButtons();
    refreshHats();
    name = QString();
}

void SetJoystick::propogateSetChange(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    emit setChangeActivated(index);
}

void SetJoystick::propogateSetButtonAssociation(int button, int newset, int mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (newset != index)
    {
        emit setAssignmentButtonChanged(button, index, newset, mode);
    }
}

void SetJoystick::propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (newset != index)
    {
        emit setAssignmentAxisChanged(button, axis, index, newset, mode);
    }
}

void SetJoystick::propogateSetStickButtonAssociation(int button, int stick, int newset, int mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (newset != index)
    {
        emit setAssignmentStickChanged(button, stick, index, newset, mode);
    }
}

void SetJoystick::propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (newset != index)
    {
        emit setAssignmentDPadChanged(button, dpad, index, newset, mode);
    }
}

void SetJoystick::propogateSetVDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (newset != index)
    {
        emit setAssignmentVDPadChanged(button, dpad, index, newset, mode);
    }
}

/**
 * @brief Perform a release of all elements of a set. Stick and vdpad
 *     releases will be handled by the associated button or axis.
 */
void SetJoystick::release()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyAxis*> iterAxes(axes);
    while (iterAxes.hasNext())
    {
        JoyAxis *axis = iterAxes.next().value();
        axis->clearPendingEvent();
        axis->joyEvent(axis->getCurrentThrottledDeadValue(), true);
        axis->eventReset();

    }

    QHashIterator<int, JoyDPad*> iterDPads(hats);
    while (iterDPads.hasNext())
    {
        JoyDPad *dpad = iterDPads.next().value();
        dpad->clearPendingEvent();
        dpad->joyEvent(0, true);
        dpad->eventReset();
    }

    QHashIterator<int, JoyButton*> iterButtons(buttons);
    while (iterButtons.hasNext())
    {
        JoyButton *button = iterButtons.next().value();
        button->clearPendingEvent();
        button->joyEvent(false, true);
        button->eventReset();
    }
}

void SetJoystick::readConfig(QXmlStreamReader *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (xml->isStartElement() && (xml->name() == "set"))
    {
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "set")))
        {
            if ((xml->name() == "button") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = getJoyButton(index-1);
                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = getJoyAxis(index-1);
                if (axis)
                {
                    axis->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = getJoyDPad(index-1);
                if (dpad)
                {
                    dpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = getJoyStick(stickIndex);
                    if (stick)
                    {
                        stick->readConfig(xml);
                    }
                    else
                    {
                        xml->skipCurrentElement();
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "vdpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = getVDPad(index-1);
                if (vdpad)
                {
                    vdpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (!temptext.isEmpty())
                {
                    setName(temptext);
                }
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void SetJoystick::writeConfig(QXmlStreamWriter *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (!isSetEmpty())
    {
        xml->writeStartElement("set");

        xml->writeAttribute("index", QString::number(index+1));

        if (!name.isEmpty())
        {
            xml->writeTextElement("name", name);
        }

        for (int i=0; i < getNumberSticks(); i++)
        {
            JoyControlStick *stick = getJoyStick(i);
            stick->writeConfig(xml);
        }

        for (int i=0; i < getNumberVDPads(); i++)
        {
            VDPad *vdpad = getVDPad(i);
            if (vdpad)
            {
                vdpad->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberAxes(); i++)
        {
            JoyAxis *axis = getJoyAxis(i);
            if (!axis->isPartControlStick() && axis->hasControlOfButtons())
            {
                axis->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberHats(); i++)
        {
            JoyDPad *dpad = getJoyDPad(i);
            dpad->writeConfig(xml);
        }

        for (int i=0; i < getNumberButtons(); i++)
        {
            JoyButton *button = getJoyButton(i);
            if (button && !button->isPartVDPad())
            {
                button->writeConfig(xml);
            }
        }

        xml->writeEndElement();
    }
}

bool SetJoystick::isSetEmpty()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = true;
    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext() && result)
    {
        JoyButton *button = iter.next().value();
        if (!button->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext() && result)
    {
        JoyAxis *axis = iter2.next().value();
        if (!axis->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyDPad*> iter3(hats);
    while (iter3.hasNext() && result)
    {
        JoyDPad *dpad = iter3.next().value();
        if (!dpad->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyControlStick*> iter4(sticks);
    while (iter4.hasNext() && result)
    {
        JoyControlStick *stick = iter4.next().value();
        if (!stick->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, VDPad*> iter5(vdpads);
    while (iter5.hasNext() && result)
    {
        VDPad *vdpad = iter5.next().value();
        if (!vdpad->isDefault())
        {
            result = false;
        }
    }

    return result;
}

void SetJoystick::propogateSetAxisThrottleSetting(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxis *axis = axes.value(index);
    if (axis)
    {
        emit setAssignmentAxisThrottleChanged(index, axis->getCurrentlyAssignedSet());
    }
}

void SetJoystick::addControlStick(int index, JoyControlStick *stick)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    sticks.insert(index, stick);
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(propogateSetStickNameChange()));

    QHashIterator<JoyStickDirectionsType::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        if (button)
        {
            connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
            connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));
            connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetStickButtonClick(int)), Qt::QueuedConnection);
            connect(button, SIGNAL(released(int)), this, SLOT(propogateSetStickButtonRelease(int)), Qt::QueuedConnection);
            connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetStickButtonNameChange()));
        }
    }
}

void SetJoystick::removeControlStick(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (sticks.contains(index))
    {
        JoyControlStick *stick = sticks.value(index);
        sticks.remove(index);
        delete stick;
        stick = nullptr;
    }
}

void SetJoystick::addVDPad(int index, VDPad *vdpad)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    vdpads.insert(index, vdpad);
    connect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetVDPadNameChange()));

    QHashIterator<int, JoyDPadButton*> iter(*vdpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        if (button)
        {
            connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
            connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetVDPadButtonAssociation(int,int,int,int)));
            connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetDPadButtonClick(int)), Qt::QueuedConnection);
            connect(button, SIGNAL(released(int)), this, SLOT(propogateSetDPadButtonRelease(int)), Qt::QueuedConnection);
            connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetVDPadButtonNameChange()));
        }
    }
}

void SetJoystick::removeVDPad(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (vdpads.contains(index))
    {
        VDPad *vdpad = vdpads.value(index);
        vdpads.remove(index);
        delete vdpad;
        vdpad = nullptr;
    }
}

int SetJoystick::getIndex() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return index;
}

int SetJoystick::getRealIndex() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return index + 1;
}

void SetJoystick::propogateSetButtonClick(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    //JoyButton *jButton = qobject_cast<JoyButton*>(sender()); // static_cast
    JoyButton* jButton = qobject_cast<JoyButton*>(sender());
    if (jButton != nullptr)
    {
        if (!jButton->getIgnoreEventState())
        {
            emit setButtonClick(index, button);
            lastClickedButtons.append(jButton);
            qDebug() << "Added button " << jButton->getPartialName(false,true) << " to list";
            qDebug() << "List has " << lastClickedButtons.count() << " buttons";
        }
    }
}

QList<JoyButton*> SetJoystick::getLastClickedButtons() const {

    return lastClickedButtons;
}

void SetJoystick::removeAllBtnFromQueue() {

    if (!lastClickedButtons.isEmpty())
        lastClickedButtons.clear();

}

int SetJoystick::getCountBtnInList(QString partialName) {

    int count = 0;

    foreach(JoyButton* joyBtn, lastClickedButtons) {
        if (joyBtn->getPartialName(false, true) == partialName) count++;
    }

    return count;
}

void SetJoystick::propogateSetButtonRelease(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton *jButton = qobject_cast<JoyButton*>(sender()); // static_cast
    if (jButton)
    {
        if (!jButton->getIgnoreEventState())
        {
            emit setButtonRelease(index, button);
        }
    }
}

void SetJoystick::propogateSetAxisButtonClick(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    if (axisButton)
    {
        JoyAxis *axis = axisButton->getAxis();
        if (!axisButton->getIgnoreEventState())
        {
            emit setAxisButtonClick(index, axis->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetAxisButtonRelease(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    if (axisButton)
    {
        JoyAxis *axis = axisButton->getAxis();
        if (!axisButton->getIgnoreEventState())
        {
            emit setAxisButtonRelease(index, axis->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetStickButtonClick(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    if (stickButton)
    {
        JoyControlStick *stick = stickButton->getStick();
        if (stick && !stickButton->getIgnoreEventState())
        {
            emit setStickButtonClick(index, stick->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetStickButtonRelease(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    if (stickButton)
    {
        JoyControlStick *stick = stickButton->getStick();
        if (!stickButton->getIgnoreEventState())
        {
            emit setStickButtonRelease(index, stick->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetDPadButtonClick(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadButton)
    {
        JoyDPad *dpad = dpadButton->getDPad();
        if (dpad && dpadButton->getButtonState() &&
            !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonClick(index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetDPadButtonRelease(int button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadButton)
    {
        JoyDPad *dpad = dpadButton->getDPad();
        if (dpad && !dpadButton->getButtonState() &&
            !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonRelease(index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetButtonNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton *button = qobject_cast<JoyButton*>(sender()); // static_cast
    disconnect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetButtonNameChange()));
    emit setButtonNameChange(button->getJoyNumber());
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetButtonNameChange()));
}

void SetJoystick::propogateSetAxisButtonNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxisButton *button = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    disconnect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetAxisButtonNameChange()));
    emit setAxisButtonNameChange(button->getAxis()->getIndex(), button->getJoyNumber());
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetAxisButtonNameChange()));
}

void SetJoystick::propogateSetStickButtonNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyControlStickButton *button = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    disconnect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetStickButtonNameChange()));
    emit setStickButtonNameChange(button->getStick()->getIndex(), button->getJoyNumber());
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetStickButtonNameChange()));
}

void SetJoystick::propogateSetDPadButtonNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyDPadButton *button = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    disconnect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetDPadButtonNameChange()));
    emit setDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetDPadButtonNameChange()));
}

void SetJoystick::propogateSetVDPadButtonNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyDPadButton *button = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    disconnect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetVDPadButtonNameChange()));
    emit setVDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetVDPadButtonNameChange()));
}

void SetJoystick::propogateSetAxisNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    disconnect(axis, SIGNAL(axisNameChanged()), this, SLOT(propogateSetAxisNameChange()));
    emit setAxisNameChange(axis->getIndex());
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(propogateSetAxisNameChange()));
}

void SetJoystick::propogateSetStickNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyControlStick *stick = qobject_cast<JoyControlStick*>(sender()); // static_cast
    disconnect(stick, SIGNAL(stickNameChanged()), this, SLOT(propogateSetStickNameChange()));
    emit setStickNameChange(stick->getIndex());
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(propogateSetStickNameChange()));
}

void SetJoystick::propogateSetDPadNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyDPad *dpad = qobject_cast<JoyDPad*>(sender()); // static_cast
    disconnect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetDPadButtonNameChange()));
    emit setDPadNameChange(dpad->getIndex());
    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetDPadButtonNameChange()));
}

void SetJoystick::propogateSetVDPadNameChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    VDPad *vdpad = qobject_cast<VDPad*>(sender()); // static_cast
    disconnect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetVDPadNameChange()));
    emit setVDPadNameChange(vdpad->getIndex());
    connect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetVDPadNameChange()));
}

void SetJoystick::setIgnoreEventState(bool ignore)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            button->setIgnoreEventState(ignore);
        }
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext())
    {
        JoyAxis *axis = iter2.next().value();
        if (axis)
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            naxisbutton->setIgnoreEventState(ignore);

            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            paxisbutton->setIgnoreEventState(ignore);
        }
    }

    QHashIterator<int, JoyDPad*> iter3(hats);
    while (iter3.hasNext())
    {
        JoyDPad *dpad = iter3.next().value();

        if (dpad)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iterdpadbuttons(*dpadbuttons);
            while (iterdpadbuttons.hasNext())
            {
                JoyDPadButton *dpadbutton = iterdpadbuttons.next().value();
                if (dpadbutton)
                {
                    dpadbutton->setIgnoreEventState(ignore);
                }
            }
        }
    }

    QHashIterator<int, JoyControlStick*> iter4(sticks);
    while (iter4.hasNext())
    {
        JoyControlStick *stick = iter4.next().value();
        if (stick)
        {
            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iterstickbuttons(*stickButtons);
            while (iterstickbuttons.hasNext())
            {
                JoyControlStickButton *stickbutton = iterstickbuttons.next().value();
                stickbutton->setIgnoreEventState(ignore);
            }
        }
    }

    QHashIterator<int, VDPad*> iter5(vdpads);
    while (iter5.hasNext())
    {
        VDPad *vdpad = iter5.next().value();
        if (vdpad)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton*> itervdpadbuttons(*dpadbuttons);
            while (itervdpadbuttons.hasNext())
            {
                JoyDPadButton *dpadbutton = itervdpadbuttons.next().value();
                dpadbutton->setIgnoreEventState(ignore);
            }
        }
    }

}

void SetJoystick::propogateSetAxisActivated(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    emit setAxisActivated(this->index, axis->getIndex(), value);
}

void SetJoystick::propogateSetAxisReleased(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    emit setAxisReleased(this->index, axis->getIndex(), value);
}

void SetJoystick::enableButtonConnections(JoyButton *button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    connect(button, SIGNAL(setAssignmentChanged(int,int,int)), this, SLOT(propogateSetButtonAssociation(int,int,int)));
    connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetButtonClick(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(clicked(int)), device, SLOT(buttonClickEvent(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(propogateSetButtonRelease(int)));
    connect(button, SIGNAL(released(int)), device, SLOT(buttonReleaseEvent(int)));
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetButtonNameChange()));
}

void SetJoystick::enableAxisConnections(JoyAxis *axis)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(axis, SIGNAL(throttleChangePropogated(int)), this, SLOT(propogateSetAxisThrottleSetting(int)));
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(propogateSetAxisNameChange()));
    connect(axis, SIGNAL(active(int)), this, SLOT(propogateSetAxisActivated(int)));
    connect(axis, SIGNAL(released(int)), this, SLOT(propogateSetAxisReleased(int)));

    JoyAxisButton *button = axis->getNAxisButton();
    connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetAxisButtonAssociation(int,int,int,int)));
    connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetAxisButtonClick(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(propogateSetAxisButtonRelease(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetAxisButtonNameChange()));

    button = axis->getPAxisButton();
    connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetAxisButtonAssociation(int,int,int,int)));
    connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetAxisButtonClick(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(propogateSetAxisButtonRelease(int)), Qt::QueuedConnection);
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetAxisButtonNameChange()));
}

void SetJoystick::enableHatConnections(JoyDPad *dpad)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(propogateSetDPadNameChange()));

    QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();
    QHashIterator<int, JoyDPadButton*> iter(*buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetDPadButtonAssociation(int,int,int,int)));

        connect(button, SIGNAL(clicked(int)), this, SLOT(propogateSetDPadButtonClick(int)), Qt::QueuedConnection);
        connect(button, SIGNAL(clicked(int)), device, SLOT(dpadButtonClickEvent(int)), Qt::QueuedConnection);
        connect(button, SIGNAL(released(int)), this, SLOT(propogateSetDPadButtonRelease(int)), Qt::QueuedConnection);
        connect(button, SIGNAL(released(int)), device, SLOT(dpadButtonReleaseEvent(int)), Qt::QueuedConnection);
        connect(button, SIGNAL(buttonNameChanged()), this, SLOT(propogateSetDPadButtonNameChange()));
    }
}

InputDevice* SetJoystick::getInputDevice() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return device;
}

void SetJoystick::setName(QString name)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (name.length() <= MAXNAMELENGTH)
    {
        this->name = name;
        emit propertyUpdated();
    }
    else if (name.length() > MAXNAMELENGTH)
    {
        // Truncate name to 27 characters. Add ellipsis at the end.
        name.truncate(MAXNAMELENGTH-3);
        this->name = QString(name).append("...");
        emit propertyUpdated();
    }
}

QString SetJoystick::getName() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return name;
}

void SetJoystick::copyAssignments(SetJoystick *destSet)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    for (int i=0; i < device->getNumberAxes(); i++)
    {
        JoyAxis *sourceAxis = axes.value(i);
        JoyAxis *destAxis = destSet->axes.value(i);
        if (sourceAxis && destAxis)
        {
            sourceAxis->copyAssignments(destAxis);
        }
    }

    QHashIterator<int, JoyControlStick*> stickIter(sticks);
    while (stickIter.hasNext())
    {
        stickIter.next();
        int index = stickIter.key();
        JoyControlStick *sourceStick = stickIter.value();
        JoyControlStick *destStick = destSet->sticks.value(index);
        if (sourceStick && destStick)
        {
            sourceStick->copyAssignments(destStick);
        }
    }

    for (int i=0; i < device->getNumberHats(); i++)
    {
        JoyDPad *sourceDPad = hats.value(i);
        JoyDPad *destDPad = destSet->hats.value(i);
        if (sourceDPad && destDPad)
        {
            sourceDPad->copyAssignments(destDPad);
        }
    }

    QHashIterator<int, VDPad*> vdpadIter(vdpads);
    while (vdpadIter.hasNext())
    {
        vdpadIter.next();
        int index = vdpadIter.key();
        VDPad *sourceVDpad = vdpadIter.value();
        VDPad *destVDPad = destSet->vdpads.value(index);
        if (sourceVDpad && destVDPad)
        {
            sourceVDpad->copyAssignments(destVDPad);
        }
    }

    for (int i=0; i < device->getNumberButtons(); i++)
    {
        JoyButton *sourceButton = buttons.value(i);
        JoyButton *destButton = destSet->buttons.value(i);
        if (sourceButton && destButton)
        {
            sourceButton->copyAssignments(destButton);
        }
    }
}

QString SetJoystick::getSetLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (!name.isEmpty())
    {
        temp = trUtf8("Set %1: %2").arg(index+1).arg(name);
    }
    else
    {
        temp = trUtf8("Set %1").arg(index+1);
    }

    return temp;
}

void SetJoystick::establishPropertyUpdatedConnection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(this, SIGNAL(propertyUpdated()), getInputDevice(), SLOT(profileEdited()));
}

void SetJoystick::disconnectPropertyUpdatedConnection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(this, SIGNAL(propertyUpdated()), getInputDevice(), SLOT(profileEdited()));
}

/**
 * @brief Raise the dead zones for axes. Used when launching
 *     the controller mapping window.
 */
void SetJoystick::raiseAxesDeadZones(int deadZone)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int tempDeadZone = deadZone;
    if ((deadZone <= 0) || (deadZone > 32767))
    {
        tempDeadZone = RAISEDDEADZONE;
    }

    QHashIterator<int, JoyAxis*> axisIter(axes);
    while (axisIter.hasNext())
    {
        JoyAxis *temp = axisIter.next().value();
        temp->disconnectPropertyUpdatedConnection();
        temp->setDeadZone(tempDeadZone);
        temp->establishPropertyUpdatedConnection();
    }
}

void SetJoystick::currentAxesDeadZones(QList<int> *axesDeadZones)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHashIterator<int, JoyAxis*> axisIter(axes);
    while (axisIter.hasNext())
    {
        JoyAxis *temp = axisIter.next().value();
        axesDeadZones->append(temp->getDeadZone());
    }
}

void SetJoystick::setAxesDeadZones(QList<int> *axesDeadZones)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QListIterator<int> iter(*axesDeadZones);
    int axisNum = 0;
    while (iter.hasNext())
    {
        int deadZoneValue = iter.next();
        if (axes.contains(axisNum))
        {
            JoyAxis *temp = getJoyAxis(axisNum);
            temp->disconnectPropertyUpdatedConnection();
            temp->setDeadZone(deadZoneValue);
            temp->establishPropertyUpdatedConnection();
        }

        axisNum++;
    }
}

void SetJoystick::setAxisThrottle(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (axes.contains(axisNum))
    {
        JoyAxis *temp = axes.value(axisNum);
        temp->setInitialThrottle(throttle);
    }
}

QHash<int, JoyAxis*>* SetJoystick::getAxes() {

    return &axes;
}
