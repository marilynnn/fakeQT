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

#include "perforceversioncontrol.h"
#include "perforceplugin.h"

namespace Perforce {
namespace Internal {

PerforceVersionControl::PerforceVersionControl(PerforcePlugin *plugin) :
    m_enabled(true),
    m_plugin(plugin)
{
}

QString PerforceVersionControl::name() const
{
    return QLatin1String("perforce");
}

bool PerforceVersionControl::isEnabled() const
{
     return m_enabled;
}

void PerforceVersionControl::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
}

bool PerforceVersionControl::supportsOperation(Operation operation) const
{
    bool rc = true;
    switch (operation) {
    case AddOperation:
    case DeleteOperation:
    case OpenOperation:
        break;
    }
    return rc;
}

bool PerforceVersionControl::vcsOpen(const QString &fileName)
{
    return m_plugin->vcsOpen(fileName);
}

bool PerforceVersionControl::vcsAdd(const QString &fileName)
{
    return m_plugin->vcsAdd(fileName);
}

bool PerforceVersionControl::vcsDelete(const QString &fileName)
{
    return m_plugin->vcsDelete(fileName);
}

bool PerforceVersionControl::managesDirectory(const QString &directory) const
{
    return m_plugin->managesDirectory(directory);
}

QString PerforceVersionControl::findTopLevelForDirectory(const QString &directory) const
{
    return m_plugin->findTopLevelForDirectory(directory);
}

} // Internal
} // Perforce
