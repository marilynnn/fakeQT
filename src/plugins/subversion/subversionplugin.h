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

#ifndef SUBVERSIONPLUGIN_H
#define SUBVERSIONPLUGIN_H

#include "subversionsettings.h"

#include <coreplugin/editormanager/ieditorfactory.h>
#include <coreplugin/iversioncontrol.h>
#include <coreplugin/icorelistener.h>
#include <extensionsystem/iplugin.h>
#include <coreplugin/icorelistener.h>

#include <QtCore/QObject>
#include <QtCore/QProcess>

QT_BEGIN_NAMESPACE
class QFile;
class QDir;
class QAction;
class QTemporaryFile;
class QTextCodec;
QT_END_NAMESPACE

namespace Core {
    class ICore;
    class IEditorFactory;
    class IVersionControl;
}

namespace ProjectExplorer {
    class ProjectExplorerPlugin;
}

namespace Subversion {
namespace Internal {

class CoreListener;
class SettingsPage;
class SubversionOutputWindow;
class SubversionSubmitEditor;

struct SubversionResponse
{
    SubversionResponse() : error(false) {}
    bool error;
    QString stdOut;
    QString stdErr;
    QString message;
};

class SubversionPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    SubversionPlugin();
    ~SubversionPlugin();

    bool initialize(const QStringList &arguments, QString *error_message);
    void extensionsInitialized();
    bool editorAboutToClose(Core::IEditor *editor);

    void svnDiff(const QStringList &files, QString diffname = QString());

    SubversionSubmitEditor *openSubversionSubmitEditor(const QString &fileName);

    SubversionSettings settings() const;
    void setSettings(const SubversionSettings &s);

    // IVersionControl
    bool vcsAdd(const QString &fileName);
    bool vcsDelete(const QString &fileName);
    bool managesDirectory(const QString &directory) const;
    QString findTopLevelForDirectory(const QString &directory) const;

    static Core::ICore *coreInstance();
    static SubversionPlugin *subversionPluginInstance();

private slots:
    void updateActions();
    void addCurrentFile();
    void deleteCurrentFile();
    void revertCurrentFile();
    void diffProject();
    void diffCurrentFile();
    void startCommitAll();
    void startCommitCurrentFile();
    void filelogCurrentFile();
    void annotateCurrentFile();
    void projectStatus();
    void describe(const QString &source, const QString &changeNr);
    void updateProject();
    void submitCurrentLog();
    void diffFiles(const QStringList &);

private:
    QString currentFileName() const;
    Core::IEditor * showOutputInEditor(const QString& title, const QString &output,
                                       int editorType, const QString &source,
                                       QTextCodec *codec);
    SubversionResponse runSvn(const QStringList &arguments, int timeOut,
                              bool showStdOutInOutputWindow, QTextCodec *outputCodec = 0);
    void showOutput(const QString &output, bool bringToForeground = true);
    QStringList parseStatusOutput(const QString &output) const;
    void annotate(const QString &file);
    void filelog(const QString &file);
    bool managesDirectory(const QDir &directory) const;
    QString findTopLevelForDirectoryI(const QString &directory) const;
    QStringList currentProjectsTopLevels(QString *name = 0) const;
    void startCommit(const QStringList &files);
    bool commit(const QString &messageFile, const QStringList &subVersionFileList);
    void cleanChangeTmpFile();

    const QString m_svnDotDirectory;

    SubversionSettings m_settings;
    Core::IVersionControl *m_versionControl;
    CoreListener *m_coreListener;
    SettingsPage *m_settingsPage;
    QTemporaryFile *m_changeTmpFile;

    Core::IEditorFactory *m_submitEditorFactory;
    QList<Core::IEditorFactory*> m_editorFactories;

    SubversionOutputWindow *m_subversionOutputWindow;
    ProjectExplorer::ProjectExplorerPlugin *m_projectExplorer;

    QAction *m_addAction;
    QAction *m_deleteAction;
    QAction *m_revertAction;
    QAction *m_diffProjectAction;
    QAction *m_diffCurrentAction;
    QAction *m_commitAllAction;
    QAction *m_commitCurrentAction;
    QAction *m_filelogCurrentAction;
    QAction *m_annotateCurrentAction;
    QAction *m_statusAction;
    QAction *m_updateProjectAction;

    QAction *m_submitCurrentLogAction;
    QAction *m_submitDiffAction;
    QAction *m_submitUndoAction;
    QAction *m_submitRedoAction;

    static const char * const SUBVERSION_MENU;
    static const char * const ADD;
    static const char * const DELETE_FILE;
    static const char * const REVERT;
    static const char * const SEPARATOR0;
    static const char * const DIFF_PROJECT;
    static const char * const DIFF_CURRENT;
    static const char * const SEPARATOR1;
    static const char * const COMMIT_ALL;
    static const char * const COMMIT_CURRENT;
    static const char * const SEPARATOR2;
    static const char * const FILELOG_CURRENT;
    static const char * const ANNOTATE_CURRENT;
    static const char * const SEPARATOR3;
    static const char * const STATUS;
    static const char * const UPDATE;

    static Core::ICore *m_coreInstance;
    static SubversionPlugin *m_subversionPluginInstance;

    friend class SubversionOutputWindow;
};

// Just a proxy for SubversionPlugin
class CoreListener : public Core::ICoreListener
{
    Q_OBJECT
public:
    CoreListener(SubversionPlugin *plugin) : m_plugin(plugin) { }

    // Start commit when submit editor closes
    bool editorAboutToClose(Core::IEditor *editor) {
        return m_plugin->editorAboutToClose(editor);
    }

    // TODO: how to handle that ???
    bool coreAboutToClose() {
        return true;
    }

private:
    SubversionPlugin *m_plugin;
};

} // namespace Subversion
} // namespace Internal

#endif // SUBVERSIONPLUGIN_H
