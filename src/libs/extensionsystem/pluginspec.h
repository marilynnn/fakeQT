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

#ifndef PLUGINSPEC_H
#define PLUGINSPEC_H

#include "extensionsystem_global.h"

#include <QtCore/QString>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

namespace ExtensionSystem {

namespace Internal {
    class PluginSpecPrivate;
    class PluginManagerPrivate;
}

class IPlugin;

struct EXTENSIONSYSTEM_EXPORT PluginDependency
{
    QString name;
    QString version;
    bool operator==(const PluginDependency &other);
};

struct EXTENSIONSYSTEM_EXPORT PluginArgumentDescription
{
    QString name;
    QString parameter;
    QString description;
};

class EXTENSIONSYSTEM_EXPORT PluginSpec
{
public:
    enum State { Invalid, Read, Resolved, Loaded, Initialized, Running, Stopped, Deleted};

    ~PluginSpec();

    // information from the xml file, valid after 'Read' state is reached
    QString name() const;
    QString version() const;
    QString compatVersion() const;
    QString vendor() const;
    QString copyright() const;
    QString license() const;
    QString description() const;
    QString url() const;
    QList<PluginDependency> dependencies() const;

    typedef QList<PluginArgumentDescription> PluginArgumentDescriptions;
    PluginArgumentDescriptions argumentDescriptions() const;

    // other information, valid after 'Read' state is reached
    QString location() const;
    QString filePath() const;

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);
    void addArgument(const QString &argument);

    bool provides(const QString &pluginName, const QString &version) const;

    // dependency specs, valid after 'Resolved' state is reached
    QList<PluginSpec *> dependencySpecs() const;

    // linked plugin instance, valid after 'Loaded' state is reached
    IPlugin *plugin() const;

    // state
    State state() const;
    bool hasError() const;
    QString errorString() const;

private:
    PluginSpec();

    Internal::PluginSpecPrivate *d;
    friend class Internal::PluginManagerPrivate;
};

} // namespace ExtensionSystem

#endif // PLUGINSPEC_H

