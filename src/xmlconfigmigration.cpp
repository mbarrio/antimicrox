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

#include "xmlconfigmigration.h"
#include "event.h"
#include "antkeymapper.h"
#include "common.h"

#ifdef Q_OS_UNIX
#include "eventhandlerfactory.h"
#endif

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


XMLConfigMigration::XMLConfigMigration(QXmlStreamReader *reader, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->reader = reader;
    if (reader->device() && reader->device()->isOpen())
    {
        this->fileVersion = reader->attributes().value("configversion").toString().toInt();
    }
    else
    {
        this->fileVersion = 0;
    }
}

bool XMLConfigMigration::requiresMigration()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool toMigrate = false;
    if (fileVersion == 0)
    {
        toMigrate = false;
    }
    else if ((fileVersion >= 2) && (fileVersion <= PadderCommon::LATESTCONFIGMIGRATIONVERSION))
    {
        toMigrate = true;
    }

    return toMigrate;
}

QString XMLConfigMigration::migrate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString tempXmlString = QString();
    if (requiresMigration())
    {
        int tempFileVersion = fileVersion;
        QString initialData = readConfigToString();
        reader->clear();
        reader->addData(initialData);

        if ((tempFileVersion >= 2) && (tempFileVersion <= 5))
        {
            tempXmlString = version0006Migration();
            tempFileVersion = PadderCommon::LATESTCONFIGFILEVERSION;
        }
    }

    return tempXmlString;
}

QString XMLConfigMigration::readConfigToString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString tempXmlString = QString();
    QXmlStreamWriter writer(&tempXmlString);
    writer.setAutoFormatting(true);
    while (!reader->atEnd())
    {
        writer.writeCurrentToken(*reader);
        reader->readNext();
    }

    return tempXmlString;
}

QString XMLConfigMigration::version0006Migration()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString tempXmlString = QString();
    QXmlStreamWriter writer(&tempXmlString);
    writer.setAutoFormatting(true);
    reader->readNextStartElement();
    reader->readNextStartElement();

    writer.writeStartDocument();
    writer.writeStartElement("joystick");
    writer.writeAttribute("configversion", QString::number(6));
    writer.writeAttribute("appversion", PadderCommon::programVersion);

    while (!reader->atEnd())
    {
        if ((reader->name() == "slot") && reader->isStartElement())
        {
            int slotcode = 0;
            QString slotmode = QString();
            writer.writeCurrentToken(*reader);
            reader->readNext();

            // Grab current slot code and slot mode
            while (!reader->atEnd() && (!reader->isEndElement() && (reader->name() != "slot")))
            {
                if ((reader->name() == "code") && reader->isStartElement())
                {
                    QString tempcode = reader->readElementText();
                    slotcode = tempcode.toInt();
                }
                else if ((reader->name() == "mode") && reader->isStartElement())
                {
                    slotmode = reader->readElementText();
                }
                else
                {
                    writer.writeCurrentToken(*reader);
                }

                reader->readNext();
            }

            // Reformat slot code if associated with the keyboard
            if (slotcode && !slotmode.isEmpty())
            {
                if (slotmode == "keyboard")
                {
                    int tempcode = slotcode;
#ifdef Q_OS_WIN
                    slotcode = AntKeyMapper::getInstance()->returnQtKey(slotcode);
#elif defined(Q_OS_UNIX)
                    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
                    if (handler->getIdentifier() == "xtest")
                    {
                        slotcode = AntKeyMapper::getInstance()->returnQtKey(X11KeyCodeToX11KeySym(slotcode));
                    }
                    else
                    {
                        slotcode = 0;
                        tempcode = 0;
                    }

#endif
                    if (slotcode > 0)
                    {
                        writer.writeTextElement("code", QString("0x%1").arg(slotcode, 0, 16));
                    }
                    else if (tempcode > 0)
                    {
                        writer.writeTextElement("code", QString("0x%1").arg(tempcode | QtKeyMapperBase::nativeKeyPrefix, 0, 16));
                    }
                }
                else
                {
                    writer.writeTextElement("code", QString::number(slotcode));
                }

                writer.writeTextElement("mode", slotmode);
            }
            writer.writeCurrentToken(*reader);
        }
        else
        {
            writer.writeCurrentToken(*reader);
        }
        reader->readNext();
    }

    return tempXmlString;
}

QXmlStreamReader* XMLConfigMigration::getReader() const {

    return reader;
}

int XMLConfigMigration::getFileVersion() const {

    return fileVersion;
}
