/***************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
**
** Non-Open Source Usage
**
** Licensees may use this file in accordance with the Qt Beta Version
** License Agreement, Agreement version 2.2 provided with the Software or,
** alternatively, in accordance with the terms contained in a written
** agreement between you and Nokia.
**
** GNU General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the packaging
** of this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
**
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt GPL Exception
** version 1.3, included in the file GPL_EXCEPTION.txt in this package.
**
***************************************************************************/

#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <coreplugin/core_global.h>

#include <QtGui/QAction>
#include <QtGui/QShortcut>
#include <QtGui/QKeySequence>

namespace Core {

class CORE_EXPORT ICommand : public QObject
{
    Q_OBJECT
public:
    enum CommandType {
        CT_Shortcut = 0x0001,
        CT_OverridableAction = 0x0002,
        CT_Mask = 0x00FF
    };

    enum CommandAttribute {
        CA_Hide             = 0x0100,
        CA_UpdateText       = 0x0200,
        CA_UpdateIcon       = 0x0400,
        CA_NonConfigureable = 0x8000,
        CA_Mask             = 0xFF00
    };

    virtual void setDefaultKeySequence(const QKeySequence &key) = 0;
    virtual void setKeySequence(const QKeySequence &key) = 0;
    virtual QKeySequence defaultKeySequence() const = 0;
    virtual QKeySequence keySequence() const = 0;
    virtual void setDefaultText(const QString &text) = 0;
    virtual QString defaultText() const = 0;

    virtual void setCategory(const QString &name) = 0;

    virtual int id() const = 0;
    virtual CommandType type() const = 0;

    virtual QAction *action() const = 0;
    virtual QShortcut *shortcut() const = 0;

    virtual void setAttribute(CommandAttribute attr) = 0;
    virtual void removeAttribute(CommandAttribute attr) = 0;
    virtual bool hasAttribute(CommandAttribute attr) const = 0;

    virtual bool isActive() const = 0;

    virtual ~ICommand() {}

    virtual QString stringWithAppendedShortcut(const QString &str) const = 0;

signals:
    void keySequenceChanged();
};

} // namespace Core

#endif // ICOMMAND_H
