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

#include "coreimpl.h"

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

using namespace Core;
using namespace Core::Internal;

CoreImpl *CoreImpl::m_instance = 0;

CoreImpl *CoreImpl::instance()
{
    return m_instance;
}

CoreImpl::CoreImpl(MainWindow *mainwindow)
{
    m_instance = this;
    m_mainwindow = mainwindow;
}

QStringList CoreImpl::showNewItemDialog(const QString &title,
                                        const QList<IWizard *> &wizards,
                                        const QString &defaultLocation)
{
    return m_mainwindow->showNewItemDialog(title, wizards, defaultLocation);
}

void CoreImpl::showOptionsDialog(const QString &group, const QString &page)
{
    m_mainwindow->showOptionsDialog(group, page);
}

ActionManagerInterface *CoreImpl::actionManager() const
{
    return m_mainwindow->actionManager();
}

FileManager *CoreImpl::fileManager() const
{
    return m_mainwindow->fileManager();
}

UniqueIDManager *CoreImpl::uniqueIDManager() const
{
    return m_mainwindow->uniqueIDManager();
}

MessageManager *CoreImpl::messageManager() const
{
    return m_mainwindow->messageManager();
}

ViewManagerInterface *CoreImpl::viewManager() const
{
    return m_mainwindow->viewManager();
}

ExtensionSystem::PluginManager *CoreImpl::pluginManager() const
{
    return m_mainwindow->pluginManager();
}

EditorManager *CoreImpl::editorManager() const
{
    return m_mainwindow->editorManager();
}

ProgressManagerInterface *CoreImpl::progressManager() const
{
    return m_mainwindow->progressManager();
}

ScriptManagerInterface *CoreImpl::scriptManager() const
{
    return m_mainwindow->scriptManager();
}

VariableManager *CoreImpl::variableManager() const
{
    return m_mainwindow->variableManager();
}

VCSManager *CoreImpl::vcsManager() const
{
    return m_mainwindow->vcsManager();
}

ModeManager *CoreImpl::modeManager() const
{
    return m_mainwindow->modeManager();
}

MimeDatabase *CoreImpl::mimeDatabase() const
{
    return m_mainwindow->mimeDatabase();
}

QSettings *CoreImpl::settings() const
{
    return m_mainwindow->settings();
}

QPrinter *CoreImpl::printer() const
{
    return m_mainwindow->printer();
}

QString CoreImpl::resourcePath() const
{
#if defined(Q_OS_MAC)
    return QDir::cleanPath(QCoreApplication::applicationDirPath()+QLatin1String("/../Resources"));
#else
    return QDir::cleanPath(QCoreApplication::applicationDirPath());
#endif
}

QString CoreImpl::libraryPath() const
{
#if defined(Q_OS_MAC)
    return QDir::cleanPath(QCoreApplication::applicationDirPath()+QLatin1String("/../PlugIns"));
#else
    return QDir::cleanPath(QCoreApplication::applicationDirPath()+QLatin1String("/../lib"));
#endif
}

IContext *CoreImpl::currentContextObject() const
{
    return m_mainwindow->currentContextObject();
}


QMainWindow *CoreImpl::mainWindow() const
{
    return m_mainwindow;
}

QStatusBar *CoreImpl::statusBar() const
{
    return m_mainwindow->statusBar();
}


// adds and removes additional active contexts, this context is appended to the
// currently active contexts. call updateContext after changing
void CoreImpl::addAdditionalContext(int context)
{
    m_mainwindow->addAdditionalContext(context);
}

void CoreImpl::removeAdditionalContext(int context)
{
    m_mainwindow->removeAdditionalContext(context);
}

bool CoreImpl::hasContext(int context) const
{
    return m_mainwindow->hasContext(context);
}

void CoreImpl::addContextObject(IContext *context)
{
    m_mainwindow->addContextObject(context);
}

void CoreImpl::removeContextObject(IContext *context)
{
    m_mainwindow->removeContextObject(context);
}

void CoreImpl::updateContext()
{
    return m_mainwindow->updateContext();
}

void CoreImpl::openFiles(const QStringList &arguments)
{
    m_mainwindow->openFiles(arguments);
}
