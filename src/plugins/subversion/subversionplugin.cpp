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

#include "subversionplugin.h"

#include "settingspage.h"
#include "subversioneditor.h"

#include "subversionoutputwindow.h"
#include "subversionsubmiteditor.h"
#include "changenumberdialog.h"
#include "subversionconstants.h"
#include "subversioncontrol.h"

#include <vcsbase/basevcseditorfactory.h>
#include <vcsbase/vcsbaseeditor.h>
#include <vcsbase/basevcssubmiteditorfactory.h>
#include <utils/synchronousprocess.h>

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actionmanagerinterface.h>
#include <coreplugin/editormanager/editormanager.h>
#include <projectexplorer/ProjectExplorerInterfaces>
#include <utils/qtcassert.h>

#include <QtCore/qplugin.h>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryFile>
#include <QtCore/QDir>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QFileDialog>

using namespace Subversion::Internal;

// Timeout for normal output commands
enum { subversionShortTimeOut = 10000 };
// Timeout for submit, update
enum { subversionLongTimeOut = 120000 };

// #pragma mark -- SubversionPlugin

const char * const SubversionPlugin::SUBVERSION_MENU    = "Subversion.Menu";
const char * const SubversionPlugin::ADD                = "Subversion.Add";
const char * const SubversionPlugin::DELETE_FILE        = "Subversion.Delete";
const char * const SubversionPlugin::REVERT             = "Subversion.Revert";
const char * const SubversionPlugin::SEPARATOR0         = "Subversion.Separator0";
const char * const SubversionPlugin::DIFF_PROJECT       = "Subversion.DiffAll";
const char * const SubversionPlugin::DIFF_CURRENT       = "Subversion.DiffCurrent";
const char * const SubversionPlugin::SEPARATOR1         = "Subversion.Separator1";
const char * const SubversionPlugin::COMMIT_ALL         = "Subversion.CommitAll";
const char * const SubversionPlugin::COMMIT_CURRENT     = "Subversion.CommitCurrent";
const char * const SubversionPlugin::SEPARATOR2         = "Subversion.Separator2";
const char * const SubversionPlugin::FILELOG_CURRENT    = "Subversion.FilelogCurrent";
const char * const SubversionPlugin::ANNOTATE_CURRENT   = "Subversion.AnnotateCurrent";
const char * const SubversionPlugin::SEPARATOR3         = "Subversion.Separator3";
const char * const SubversionPlugin::STATUS             = "Subversion.Status";
const char * const SubversionPlugin::UPDATE             = "Subversion.Update";

static const VCSBase::VCSBaseEditorParameters editorParameters[] = {
{
    VCSBase::RegularCommandOutput,
    "Subversion Command Log Editor",
    Core::Constants::C_GLOBAL,
    "application/vnd.nokia.text.scs_svn_commandlog",
    "scslog"},
{   VCSBase::LogOutput,
    "Subversion File Log Editor",
    Core::Constants::C_GLOBAL,
    "application/vnd.nokia.text.scs_svn_filelog",
    "scsfilelog"},
{    VCSBase::AnnotateOutput,
    "Subversion Annotation Editor",
    Core::Constants::C_GLOBAL,
    "application/vnd.nokia.text.scs_svn_annotation",
    "scsannotate"},
{   VCSBase::DiffOutput,
    "Subversion Diff Editor",
    Core::Constants::C_GLOBAL,
    "text/x-patch","diff"}
};

// Utility to find a parameter set by type
static inline const VCSBase::VCSBaseEditorParameters *findType(int ie)
{
    const VCSBase::EditorContentType et = static_cast<VCSBase::EditorContentType>(ie);
    return  VCSBase::VCSBaseEditor::findType(editorParameters, sizeof(editorParameters)/sizeof(VCSBase::VCSBaseEditorParameters), et);
}

static inline QString debugCodec(const QTextCodec *c)
{
    return c ? QString::fromAscii(c->name()) : QString::fromAscii("Null codec");
}

inline Core::IEditor* locateEditor(const Core::ICore *core, const char *property, const QString &entry)
{
    foreach (Core::IEditor *ed, core->editorManager()->openedEditors())
        if (ed->property(property).toString() == entry)
            return ed;
    return 0;
}

// ------------- SubversionPlugin
Core::ICore *SubversionPlugin::m_coreInstance = 0;
SubversionPlugin *SubversionPlugin::m_subversionPluginInstance = 0;

SubversionPlugin::SubversionPlugin() :
    m_svnDotDirectory(QLatin1String(".svn")),
    m_versionControl(0),
    m_coreListener(0),
    m_settingsPage(0),
    m_changeTmpFile(0),
    m_submitEditorFactory(0),
    m_subversionOutputWindow(0),
    m_projectExplorer(0),
    m_addAction(0),
    m_deleteAction(0),
    m_revertAction(0),
    m_diffProjectAction(0),
    m_diffCurrentAction(0),
    m_commitAllAction(0),
    m_commitCurrentAction(0),
    m_filelogCurrentAction(0),
    m_annotateCurrentAction(0),
    m_statusAction(0),
    m_updateProjectAction(0),
    m_submitCurrentLogAction(0),
    m_submitDiffAction(0),
    m_submitUndoAction(0),
    m_submitRedoAction(0)
{
}

SubversionPlugin::~SubversionPlugin()
{
    if (m_versionControl) {
        removeObject(m_versionControl);
        delete m_versionControl;
        m_versionControl = 0;
    }

    if (m_settingsPage) {
        removeObject(m_settingsPage);
        delete m_settingsPage;
        m_settingsPage = 0;
    }
    if (m_subversionOutputWindow) {
        removeObject(m_subversionOutputWindow);
        delete m_subversionOutputWindow;
        m_subversionOutputWindow = 0;
    }
    if (m_submitEditorFactory) {
        removeObject(m_submitEditorFactory);
        delete m_submitEditorFactory;
        m_submitEditorFactory = 0;
    }

    if (!m_editorFactories.empty()) {
        foreach (Core::IEditorFactory* pf, m_editorFactories)
            removeObject(pf);
        qDeleteAll(m_editorFactories);
        m_editorFactories.clear();
    }

    if (m_coreListener) {
        removeObject(m_coreListener);
        delete m_coreListener;
        m_coreListener = 0;
    }
    cleanChangeTmpFile();
}

void SubversionPlugin::cleanChangeTmpFile()
{
    if (m_changeTmpFile) {
        if (m_changeTmpFile->isOpen())
            m_changeTmpFile->close();
        delete m_changeTmpFile;
        m_changeTmpFile = 0;
    }
}

static const VCSBase::VCSBaseSubmitEditorParameters submitParameters = {
    Subversion::Constants::SUBVERSION_SUBMIT_MIMETYPE,
    Subversion::Constants::SUBVERSIONCOMMITEDITOR_KIND,
    Subversion::Constants::SUBVERSIONCOMMITEDITOR,
    Core::Constants::UNDO,
    Core::Constants::REDO,
    Subversion::Constants::SUBMIT_CURRENT,
    Subversion::Constants::DIFF_SELECTED
};

bool SubversionPlugin::initialize(const QStringList & /*arguments*/, QString *errorMessage)
{
    typedef VCSBase::VCSSubmitEditorFactory<SubversionSubmitEditor> SubversionSubmitEditorFactory;
    typedef VCSBase::VCSEditorFactory<SubversionEditor> SubversionEditorFactory;
    using namespace Constants;

    using namespace Core::Constants;
    using namespace ExtensionSystem;

    m_subversionPluginInstance = this;
    m_coreInstance = PluginManager::instance()->getObject<Core::ICore>();

    if (!m_coreInstance->mimeDatabase()->addMimeTypes(QLatin1String(":/trolltech.subversion/Subversion.mimetypes.xml"), errorMessage))
        return false;

    m_versionControl = new SubversionControl(this);
    addObject(m_versionControl);

    if (QSettings *settings = m_coreInstance->settings())
        m_settings.fromSettings(settings);

    m_coreListener = new CoreListener(this);
    addObject(m_coreListener);

    m_settingsPage = new SettingsPage;
    addObject(m_settingsPage);

    m_submitEditorFactory = new SubversionSubmitEditorFactory(&submitParameters);
    addObject(m_submitEditorFactory);

    static const char *describeSlot = SLOT(describe(QString,QString));
    const int editorCount = sizeof(editorParameters)/sizeof(VCSBase::VCSBaseEditorParameters);
    for (int i = 0; i < editorCount; i++) {
        m_editorFactories.push_back(new SubversionEditorFactory(editorParameters + i, m_coreInstance, this, describeSlot));
        addObject(m_editorFactories.back());
    }

    m_subversionOutputWindow = new SubversionOutputWindow(this);
    addObject(m_subversionOutputWindow);

    //register actions
    Core::ActionManagerInterface *ami = m_coreInstance->actionManager();
    Core::IActionContainer *toolsContainer = ami->actionContainer(M_TOOLS);

    Core::IActionContainer *subversionMenu =
        ami->createMenu(QLatin1String(SUBVERSION_MENU));
    subversionMenu->menu()->setTitle(tr("&Subversion"));
    toolsContainer->addMenu(subversionMenu);
    if (QAction *ma = subversionMenu->menu()->menuAction()) {
        ma->setEnabled(m_versionControl->isEnabled());
        connect(m_versionControl, SIGNAL(enabledChanged(bool)), ma, SLOT(setVisible(bool)));
    }

    QList<int> globalcontext;
    globalcontext << m_coreInstance->uniqueIDManager()->uniqueIdentifier(C_GLOBAL);

    Core::ICommand *command;
    m_addAction = new QAction(tr("Add"), this);
    command = ami->registerAction(m_addAction, SubversionPlugin::ADD,
        globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Alt+S,Alt+A")));
    connect(m_addAction, SIGNAL(triggered()), this, SLOT(addCurrentFile()));
    subversionMenu->addAction(command);

    m_deleteAction = new QAction(tr("Delete"), this);
    command = ami->registerAction(m_deleteAction, SubversionPlugin::DELETE_FILE,
        globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteCurrentFile()));
    subversionMenu->addAction(command);

    m_revertAction = new QAction(tr("Revert"), this);
    command = ami->registerAction(m_revertAction, SubversionPlugin::REVERT,
        globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    connect(m_revertAction, SIGNAL(triggered()), this, SLOT(revertCurrentFile()));
    subversionMenu->addAction(command);

    QAction *tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    subversionMenu->addAction(ami->registerAction(tmpaction,
        SubversionPlugin::SEPARATOR0, globalcontext));

    m_diffProjectAction = new QAction(tr("Diff Project"), this);
    command = ami->registerAction(m_diffProjectAction, SubversionPlugin::DIFF_PROJECT,
        globalcontext);
    connect(m_diffProjectAction, SIGNAL(triggered()), this, SLOT(diffProject()));
    subversionMenu->addAction(command);

    m_diffCurrentAction = new QAction(tr("Diff Current File"), this);
    command = ami->registerAction(m_diffCurrentAction,
        SubversionPlugin::DIFF_CURRENT, globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Alt+S,Alt+D")));
    connect(m_diffCurrentAction, SIGNAL(triggered()), this, SLOT(diffCurrentFile()));
    subversionMenu->addAction(command);

    tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    subversionMenu->addAction(ami->registerAction(tmpaction,
        SubversionPlugin::SEPARATOR1, globalcontext));

    m_commitAllAction = new QAction(tr("Commit All Files"), this);
    command = ami->registerAction(m_commitAllAction, SubversionPlugin::COMMIT_ALL,
        globalcontext);
    connect(m_commitAllAction, SIGNAL(triggered()), this, SLOT(startCommitAll()));
    subversionMenu->addAction(command);

    m_commitCurrentAction = new QAction(tr("Commit Current File"), this);
    command = ami->registerAction(m_commitCurrentAction,
        SubversionPlugin::COMMIT_CURRENT, globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Alt+S,Alt+C")));
    connect(m_commitCurrentAction, SIGNAL(triggered()), this, SLOT(startCommitCurrentFile()));
    subversionMenu->addAction(command);

    tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    subversionMenu->addAction(ami->registerAction(tmpaction,
        SubversionPlugin::SEPARATOR2, globalcontext));

    m_filelogCurrentAction = new QAction(tr("Filelog Current File"), this);
    command = ami->registerAction(m_filelogCurrentAction,
        SubversionPlugin::FILELOG_CURRENT, globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    connect(m_filelogCurrentAction, SIGNAL(triggered()), this,
        SLOT(filelogCurrentFile()));
    subversionMenu->addAction(command);

    m_annotateCurrentAction = new QAction(tr("Annotate Current File"), this);
    command = ami->registerAction(m_annotateCurrentAction,
        SubversionPlugin::ANNOTATE_CURRENT, globalcontext);
    command->setAttribute(Core::ICommand::CA_UpdateText);
    connect(m_annotateCurrentAction, SIGNAL(triggered()), this,
        SLOT(annotateCurrentFile()));
    subversionMenu->addAction(command);

    tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    subversionMenu->addAction(ami->registerAction(tmpaction,
        SubversionPlugin::SEPARATOR3, globalcontext));

    m_statusAction = new QAction(tr("Project Status"), this);
    command = ami->registerAction(m_statusAction, SubversionPlugin::STATUS,
        globalcontext);
    connect(m_statusAction, SIGNAL(triggered()), this, SLOT(projectStatus()));
    subversionMenu->addAction(command);

    m_updateProjectAction = new QAction(tr("Update Project"), this);
    command = ami->registerAction(m_updateProjectAction, SubversionPlugin::UPDATE, globalcontext);
    connect(m_updateProjectAction, SIGNAL(triggered()), this, SLOT(updateProject()));
    subversionMenu->addAction(command);

    // Actions of the submit editor
    QList<int> svncommitcontext;
    svncommitcontext << m_coreInstance->uniqueIDManager()->uniqueIdentifier(Constants::SUBVERSIONCOMMITEDITOR);

    m_submitCurrentLogAction = new QAction(VCSBase::VCSBaseSubmitEditor::submitIcon(), tr("Commit"), this);
    command = ami->registerAction(m_submitCurrentLogAction, Constants::SUBMIT_CURRENT, svncommitcontext);
    connect(m_submitCurrentLogAction, SIGNAL(triggered()), this, SLOT(submitCurrentLog()));

    m_submitDiffAction = new QAction(VCSBase::VCSBaseSubmitEditor::diffIcon(), tr("Diff Selected Files"), this);
    command = ami->registerAction(m_submitDiffAction , Constants::DIFF_SELECTED, svncommitcontext);

    m_submitUndoAction = new QAction(tr("&Undo"), this);
    command = ami->registerAction(m_submitUndoAction, Core::Constants::UNDO, svncommitcontext);

    m_submitRedoAction = new QAction(tr("&Redo"), this);
    command = ami->registerAction(m_submitRedoAction, Core::Constants::REDO, svncommitcontext);

    connect(m_coreInstance, SIGNAL(contextChanged(Core::IContext *)), this, SLOT(updateActions()));

    return true;
}

void SubversionPlugin::extensionsInitialized()
{
    using namespace ExtensionSystem;
    using namespace ProjectExplorer;

    m_projectExplorer = PluginManager::instance()->getObject<ProjectExplorerPlugin>();
    if (m_projectExplorer) {
        connect(m_projectExplorer,
            SIGNAL(currentProjectChanged(ProjectExplorer::Project*)),
            m_subversionPluginInstance, SLOT(updateActions()));
    }
    updateActions();
}

bool SubversionPlugin::editorAboutToClose(Core::IEditor *iEditor)
{
    if (!m_changeTmpFile || !iEditor || qstrcmp(Constants::SUBVERSIONCOMMITEDITOR, iEditor->kind()))
        return true;

    Core::IFile *fileIFace = iEditor->file();
    const SubversionSubmitEditor *editor = qobject_cast<SubversionSubmitEditor *>(iEditor);
    if (!fileIFace || !editor)
        return true;

    // Submit editor closing. Make it write out the commit message
    // and retrieve files
    const QFileInfo editorFile(fileIFace->fileName());
    const QFileInfo changeFile(m_changeTmpFile->fileName());
    if (editorFile.absoluteFilePath() != changeFile.absoluteFilePath())
        return true; // Oops?!

    // Prompt user.
    const QMessageBox::StandardButton answer = QMessageBox::question(
            m_coreInstance->mainWindow(), tr("Closing Subversion Editor"),
            tr("Do you want to commit the change?"),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
    switch (answer) {
    case QMessageBox::Cancel:
        return false; // Keep editing and change file
    case QMessageBox::No:
        cleanChangeTmpFile();
        return true; // Cancel all
    default:
        break;
    }

    const QStringList fileList = editor->checkedFiles();
    if (!fileList.empty()) {
        // get message & commit
        m_coreInstance->fileManager()->blockFileChange(fileIFace);
        fileIFace->save();
        m_coreInstance->fileManager()->unblockFileChange(fileIFace);
        commit(m_changeTmpFile->fileName(), fileList);
    }
    cleanChangeTmpFile();
    return true;
}

void SubversionPlugin::diffFiles(const QStringList &files)
{
    svnDiff(files);
}

void SubversionPlugin::svnDiff(const QStringList &files, QString diffname)
{
    if (Subversion::Constants::debug)
        qDebug() << Q_FUNC_INFO << files << diffname;
    const QString source = files.empty() ? QString() : files.front();
    QTextCodec *codec =  source.isEmpty() ? static_cast<QTextCodec *>(0) : VCSBase::VCSBaseEditor::getCodec(m_coreInstance, source);

    if (files.count() == 1 && diffname.isEmpty())
        diffname = QFileInfo(files.front()).fileName();

    QStringList args(QLatin1String("diff"));
    args << files;

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, false, codec);
    if (response.error)
        return;

    // diff of a single file? re-use an existing view if possible to support
    // the common usage pattern of continuously changing and diffing a file
    if (files.count() == 1) {
        // Show in the same editor if diff has been executed before
        if (Core::IEditor *editor = locateEditor(m_coreInstance, "originalFileName", files.front())) {
            editor->createNew(response.stdOut);
            m_coreInstance->editorManager()->setCurrentEditor(editor);
            return;
        }
    }
    const QString title = tr("svn diff %1").arg(diffname);
    Core::IEditor *editor = showOutputInEditor(title, response.stdOut, VCSBase::DiffOutput, source, codec);
    if (files.count() == 1)
        editor->setProperty("originalFileName", files.front());
}

SubversionSubmitEditor *SubversionPlugin::openSubversionSubmitEditor(const QString &fileName)
{
    Core::IEditor *editor = m_coreInstance->editorManager()->openEditor(fileName, QLatin1String(Constants::SUBVERSIONCOMMITEDITOR_KIND));
    SubversionSubmitEditor *submitEditor = qobject_cast<SubversionSubmitEditor*>(editor);
    QTC_ASSERT(submitEditor, /**/);
    // The actions are for some reason enabled by the context switching
    // mechanism. Disable them correctly.
    m_submitDiffAction->setEnabled(false);
    m_submitUndoAction->setEnabled(false);
    m_submitRedoAction->setEnabled(false);
    connect(submitEditor, SIGNAL(diffSelectedFiles(QStringList)), this, SLOT(diffFiles(QStringList)));

    return submitEditor;
}

void SubversionPlugin::updateActions()
{
    QString fileName = currentFileName();
    const bool hasFile = !fileName.isEmpty();

    m_addAction->setEnabled(hasFile);
    m_deleteAction->setEnabled(hasFile);
    m_revertAction->setEnabled(hasFile);
    m_diffProjectAction->setEnabled(true);
    m_diffCurrentAction->setEnabled(hasFile);
    m_commitAllAction->setEnabled(true);
    m_commitCurrentAction->setEnabled(hasFile);
    m_filelogCurrentAction->setEnabled(hasFile);
    m_annotateCurrentAction->setEnabled(hasFile);
    m_statusAction->setEnabled(true);

    QString baseName;
    if (hasFile)
        baseName = QFileInfo(fileName).fileName();

    m_addAction->setText(tr("Add %1").arg(baseName));
    m_deleteAction->setText(tr("Delete %1").arg(baseName));
    m_revertAction->setText(tr("Revert %1").arg(baseName));
    m_diffCurrentAction->setText(tr("Diff %1").arg(baseName));
    m_commitCurrentAction->setText(tr("Commit %1").arg(baseName));
    m_filelogCurrentAction->setText(tr("Filelog %1").arg(baseName));
    m_annotateCurrentAction->setText(tr("Annotate %1").arg(baseName));
}

void SubversionPlugin::addCurrentFile()
{
    const QString file = currentFileName();
    if (!file.isEmpty())
        vcsAdd(file);
}

void SubversionPlugin::deleteCurrentFile()
{
    const QString file = currentFileName();
    if (!file.isEmpty())
        vcsDelete(file);
}

void SubversionPlugin::revertCurrentFile()
{
    const QString file = QDir::toNativeSeparators(currentFileName());
    if (file.isEmpty())
        return;

    QStringList args(QLatin1String("diff"));
    args.push_back(file);

    const SubversionResponse diffResponse = runSvn(args, subversionShortTimeOut, false);
    if (diffResponse.error)
        return;

    if (diffResponse.stdOut.isEmpty())
        return;
    if (QMessageBox::warning(0, tr("svn revert"), tr("The file has been changed. Do you want to revert it?"),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        return;

    Core::FileManager *fm = m_coreInstance->fileManager();
    QList<Core::IFile *> files = fm->managedFiles(file);
    foreach (Core::IFile *file, files)
        fm->blockFileChange(file);

    // revert
    args.clear();
    args.push_back(QLatin1String("revert"));
    args.append(file);

    const SubversionResponse revertResponse = runSvn(args, subversionShortTimeOut, true);
    if (revertResponse.error) {
        foreach (Core::IFile *file, files)
            fm->unblockFileChange(file);
        return;
    }

    Core::IFile::ReloadBehavior tempBehavior = Core::IFile::ReloadAll;
    foreach (Core::IFile *file, files) {
        file->modified(&tempBehavior);
        fm->unblockFileChange(file);
    }
}

// Get a unique set of toplevel directories for the current projects.
// To be used for "diff all" or "commit all".
QStringList SubversionPlugin::currentProjectsTopLevels(QString *name) const
{
    typedef QList<ProjectExplorer::Project *> ProjectList;
    ProjectList projects;
    // Compile list of projects
    if (ProjectExplorer::Project *currentProject = m_projectExplorer->currentProject()) {
        projects.push_back(currentProject);
    } else {
        if (const ProjectExplorer::SessionManager *session = m_projectExplorer->session())
            projects.append(session->projects());
    }
    // Get unique set of toplevels and concat project names
    QStringList toplevels;
    const QChar blank(QLatin1Char(' '));
    foreach (const ProjectExplorer::Project *p,  projects) {
        if (name) {
            if (!name->isEmpty())
                name->append(blank);
            name->append(p->name());
        }

        const QString projectPath = QFileInfo(p->file()->fileName()).absolutePath();
        const QString topLevel = findTopLevelForDirectory(projectPath);
        if (!topLevel.isEmpty() && !toplevels.contains(topLevel))
            toplevels.push_back(topLevel);
    }
    return toplevels;
}

void SubversionPlugin::diffProject()
{
    QString diffName;
    const QStringList topLevels = currentProjectsTopLevels(&diffName);
    if (!topLevels.isEmpty())
        svnDiff(topLevels, diffName);
}

void SubversionPlugin::diffCurrentFile()
{
    svnDiff(QStringList(currentFileName()));
}

void SubversionPlugin::startCommitCurrentFile()
{
    const QString file = QDir::toNativeSeparators(currentFileName());
    if (!file.isEmpty())
        startCommit(QStringList(file));
}

void SubversionPlugin::startCommitAll()
{
    // Make sure we have only repository for commit
    const QStringList files = currentProjectsTopLevels();
    switch (files.size()) {
    case 0:
        break;
    case 1:
        startCommit(files);
        break;
    default: {
        const QString msg = tr("The commit list spans several respositories (%1). Please commit them one by one.").
            arg(files.join(QString(QLatin1Char(' '))));
        QMessageBox::warning(0, tr("svn commit"), msg, QMessageBox::Ok);
    }
        break;
    }
}

/* Start commit of files of a single repository by displaying
 * template and files in a submit editor. On closing, the real
 * commit will start. */
void SubversionPlugin::startCommit(const QStringList &files)
{
    if (files.empty())
        return;

    if (m_changeTmpFile) {
        showOutput(tr("Another commit is currently being executed."));
        return;
    }

    QStringList args(QLatin1String("status"));
    args += files;
    if (args.size() == 1)
        return;

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, false);
    if (response.error)
        return;
    // Get list of added/modified/deleted files
    const QStringList statusOutput = parseStatusOutput(response.stdOut);
    if (statusOutput.empty()) {
        showOutput(tr("There are no modified files."), true);
        return;
    }

    // Create a new submit change file containing the submit template
    QTemporaryFile *changeTmpFile = new QTemporaryFile(this);
    changeTmpFile->setAutoRemove(true);
    if (!changeTmpFile->open()) {
        showOutput(tr("Cannot create temporary file: %1").arg(changeTmpFile->errorString()));
        delete changeTmpFile;
        return;
    }
    m_changeTmpFile = changeTmpFile;
    // TODO: Retrieve submit template from
    const QString submitTemplate;
    // Create a submit
    m_changeTmpFile->write(submitTemplate.toUtf8());
    m_changeTmpFile->flush();
    m_changeTmpFile->seek(0);
    // Create a submit editor and set file list
    SubversionSubmitEditor *editor = openSubversionSubmitEditor(m_changeTmpFile->fileName());
    editor->setFileList(statusOutput);
}

// Parse "status" output for added/modified/deleted files
QStringList SubversionPlugin::parseStatusOutput(const QString &output) const
{
    QStringList changeSet;
    const QString newLine = QString(QLatin1Char('\n'));
    const QStringList list = output.split(newLine, QString::SkipEmptyParts);
    foreach (const QString &l, list) {
        QString line(l.trimmed());
        if (line.startsWith(QLatin1Char('A')) || line.startsWith(QLatin1Char('D'))
            || line.startsWith(QLatin1Char('M')))
            changeSet.append(line);
    }
    return changeSet;
}

bool SubversionPlugin::commit(const QString &messageFile,
                              const QStringList &subVersionFileList)
{
    if (Subversion::Constants::debug)
        qDebug() << Q_FUNC_INFO << messageFile << subVersionFileList;
    // Transform the status list which is sth
    // "[ADM]<blanks>file" into an args list. The files of the status log
    // can be relative or absolute depending on where the command was run.
    QStringList args = QStringList(QLatin1String("commit"));
    args << QLatin1String("--non-interactive") << QLatin1String("--file") << messageFile;
    args.append(subVersionFileList);
    const SubversionResponse response = runSvn(args, subversionLongTimeOut, true);
    return !response.error ;
}

void SubversionPlugin::filelogCurrentFile()
{
    const QString file = currentFileName();
    if (!file.isEmpty())
        filelog(file);
}

void SubversionPlugin::filelog(const QString &file)
{
    QTextCodec *codec = VCSBase::VCSBaseEditor::getCodec(m_coreInstance, file);
    // no need for temp file
    QStringList args(QLatin1String("log"));
    args.append(QDir::toNativeSeparators(file));

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, false, codec);
    if (response.error)
        return;

    // Re-use an existing view if possible to support
    // the common usage pattern of continuously changing and diffing a file

    if (Core::IEditor *editor = locateEditor(m_coreInstance, "logFileName", file)) {
        editor->createNew(response.stdOut);
        m_coreInstance->editorManager()->setCurrentEditor(editor);
    } else {
        const QString title = tr("svn log %1").arg(QFileInfo(file).fileName());
        Core::IEditor *newEditor = showOutputInEditor(title, response.stdOut, VCSBase::LogOutput, file, codec);
        newEditor->setProperty("logFileName", file);
    }
}

void SubversionPlugin::updateProject()
{
    const QStringList topLevels = currentProjectsTopLevels();
    if (topLevels.empty())
        return;

    QStringList args(QLatin1String("update"));
    args.append(topLevels);
    runSvn(args, subversionLongTimeOut, false);
}

void SubversionPlugin::annotateCurrentFile()
{
    const QString file = currentFileName();
    if (!file.isEmpty())
        annotate(file);
}

void SubversionPlugin::annotate(const QString &file)
{
    QTextCodec *codec = VCSBase::VCSBaseEditor::getCodec(m_coreInstance, file);

    QStringList args(QLatin1String("annotate"));
    args.push_back(QLatin1String("-v"));
    args.append(QDir::toNativeSeparators(file));

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, false, codec);
    if (response.error)
        return;

    // Re-use an existing view if possible to support
    // the common usage pattern of continuously changing and diffing a file

    if (Core::IEditor *editor = locateEditor(m_coreInstance, "annotateFileName", file)) {
        editor->createNew(response.stdOut);
        m_coreInstance->editorManager()->setCurrentEditor(editor);
    } else {
        const QString title = tr("svn annotate %1").arg(QFileInfo(file).fileName());
        Core::IEditor *newEditor = showOutputInEditor(title, response.stdOut, VCSBase::AnnotateOutput, file, codec);
        newEditor->setProperty("annotateFileName", file);
    }
}

void SubversionPlugin::projectStatus()
{
    if (!m_projectExplorer)
        return;

    QStringList args(QLatin1String("status"));
    args += currentProjectsTopLevels();

    if (args.size() == 1)
        return;

    runSvn(args, subversionShortTimeOut, true);
}

void SubversionPlugin::describe(const QString &source, const QString &changeNr)
{
    // To describe a complete change, find the top level and then do
    //svn diff -r 472958:472959 <top level>
    const QFileInfo fi(source);
    const QString topLevel = findTopLevelForDirectory(fi.isDir() ? source : fi.absolutePath());
    if (topLevel.isEmpty())
        return;
    if (Subversion::Constants::debug)
        qDebug() << Q_FUNC_INFO << source << topLevel << changeNr;
    // Number must be > 1
    bool ok;
    const int number = changeNr.toInt(&ok);
    if (!ok || number < 2)
        return;
    QStringList args(QLatin1String("diff"));
    args.push_back(QLatin1String("-r"));
    QString diffArg;
    QTextStream(&diffArg) << (number - 1) << ':' << number;
    args.push_back(diffArg);
    args.push_back(topLevel);

    QTextCodec *codec = VCSBase::VCSBaseEditor::getCodec(m_coreInstance, source);
    const SubversionResponse response = runSvn(args, subversionShortTimeOut, false, codec);
    if (response.error)
        return;

    // Re-use an existing view if possible to support
    // the common usage pattern of continuously changing and diffing a file
    const QString id = diffArg + source;
    if (Core::IEditor *editor = locateEditor(m_coreInstance, "describeChange", id)) {
        editor->createNew(response.stdOut);
        m_coreInstance->editorManager()->setCurrentEditor(editor);
    } else {
        const QString title = tr("svn describe %1#%2").arg(QFileInfo(source).fileName(), changeNr);
        Core::IEditor *newEditor = showOutputInEditor(title, response.stdOut, VCSBase::DiffOutput, source, codec);
        newEditor->setProperty("describeChange", id);
    }
}

void SubversionPlugin::submitCurrentLog()
{
    m_coreInstance->editorManager()->closeEditors(QList<Core::IEditor*>()
        << m_coreInstance->editorManager()->currentEditor());
}

QString SubversionPlugin::currentFileName() const
{
    const QString fileName = m_coreInstance->fileManager()->currentFile();
    if (!fileName.isEmpty()) {
        const QFileInfo fi(fileName);
        if (fi.exists())
            return fi.canonicalFilePath();
    }
    return QString();
}

static inline QString processStdErr(QProcess &proc)
{
    return QString::fromLocal8Bit(proc.readAllStandardError()).remove(QLatin1Char('\r'));
}

static inline QString processStdOut(QProcess &proc, QTextCodec *outputCodec = 0)
{
    const QByteArray stdOutData = proc.readAllStandardOutput();
    QString stdOut = outputCodec ? outputCodec->toUnicode(stdOutData) : QString::fromLocal8Bit(stdOutData);
    return stdOut.remove(QLatin1Char('\r'));
}

SubversionResponse SubversionPlugin::runSvn(const QStringList &arguments,
                                            int timeOut,
                                            bool showStdOutInOutputWindow,
                                            QTextCodec *outputCodec)
{
    const QString executable = m_settings.svnCommand;
    SubversionResponse response;
    if (executable.isEmpty()) {
        response.error = true;
        response.message =tr("No subversion executable specified!");
        return response;
    }
    const QStringList allArgs = m_settings.addOptions(arguments);

    // Hide passwords, etc in the log window
    const QString timeStamp = QTime::currentTime().toString(QLatin1String("HH:mm"));
    const QString outputText = tr("%1 Executing: %2 %3\n").arg(timeStamp, executable, SubversionSettings::formatArguments(allArgs));
    showOutput(outputText, false);

    if (Subversion::Constants::debug)
        qDebug() << "runSvn" << timeOut << outputText;

    // Run, connect stderr to the output window
    Core::Utils::SynchronousProcess process;
    process.setTimeout(timeOut);
    process.setStdOutCodec(outputCodec);

    process.setStdErrBufferedSignalsEnabled(true);
    connect(&process, SIGNAL(stdErrBuffered(QString,bool)), m_subversionOutputWindow, SLOT(append(QString,bool)));

    // connect stdout to the output window if desired
    if (showStdOutInOutputWindow) {
        process.setStdOutBufferedSignalsEnabled(true);
        connect(&process, SIGNAL(stdOutBuffered(QString,bool)), m_subversionOutputWindow, SLOT(append(QString,bool)));
    }

    const Core::Utils::SynchronousProcessResponse sp_resp = process.run(executable, allArgs);
    response.error = true;
    response.stdErr = sp_resp.stdErr;
    response.stdOut = sp_resp.stdOut;
    switch (sp_resp.result) {
    case Core::Utils::SynchronousProcessResponse::Finished:
        response.error = false;
        break;
    case Core::Utils::SynchronousProcessResponse::FinishedError:
        response.message = tr("The process terminated with exit code %1.").arg(sp_resp.exitCode);
        break;
    case Core::Utils::SynchronousProcessResponse::TerminatedAbnormally:
        response.message = tr("The process terminated abnormally.");
        break;
    case Core::Utils::SynchronousProcessResponse::StartFailed:
        response.message = tr("Could not start subversion '%1'. Please check your settings in the preferences.").arg(executable);
        break;
    case Core::Utils::SynchronousProcessResponse::Hang:
        response.message = tr("Subversion did not respond within timeout limit (%1 ms).").arg(timeOut);
        break;
    }
    if (response.error)
        m_subversionOutputWindow->append(response.message, true);

    return response;
}

void SubversionPlugin::showOutput(const QString &output, bool bringToForeground)
{
    m_subversionOutputWindow->append(output);
    if (bringToForeground)
        m_subversionOutputWindow->popup();
}

Core::IEditor * SubversionPlugin::showOutputInEditor(const QString& title, const QString &output,
                                                     int editorType, const QString &source,
                                                     QTextCodec *codec)
{
    const VCSBase::VCSBaseEditorParameters *params = findType(editorType);
    QTC_ASSERT(params, return 0);
    const QString kind = QLatin1String(params->kind);
    if (Subversion::Constants::debug)
        qDebug() << "SubversionPlugin::showOutputInEditor" << title << kind <<  "Size= " << output.size() <<  " Type=" << editorType << debugCodec(codec);
    QString s = title;
    Core::IEditor *ediface = m_coreInstance->editorManager()->newFile(kind, &s, output.toLocal8Bit());
    SubversionEditor *e = qobject_cast<SubversionEditor*>(ediface->widget());
    if (!e)
        return 0;
    s.replace(QLatin1Char(' '), QLatin1Char('_'));
    e->setSuggestedFileName(s);
    if (!source.isEmpty())
        e->setSource(source);
    if (codec)
        e->setCodec(codec);
    return e->editableInterface();
}

SubversionSettings SubversionPlugin::settings() const
{
    return m_settings;
}

void SubversionPlugin::setSettings(const SubversionSettings &s)
{
    if (s != m_settings) {
        m_settings = s;
        if (QSettings *settings = m_coreInstance->settings())
            m_settings.toSettings(settings);
    }
}

Core::ICore *SubversionPlugin::coreInstance()
{
    QTC_ASSERT(m_coreInstance, return 0);
    return m_coreInstance;
}

SubversionPlugin *SubversionPlugin::subversionPluginInstance()
{
    QTC_ASSERT(m_subversionPluginInstance, return m_subversionPluginInstance);
    return m_subversionPluginInstance;
}

bool SubversionPlugin::vcsAdd(const QString &rawFileName)
{
    const QString file = QDir::toNativeSeparators(rawFileName);
    QStringList args(QLatin1String("add"));
    args.push_back(file);

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, true);
    return !response.error;
}

bool SubversionPlugin::vcsDelete(const QString &rawFileName)
{
    const QString file = QDir::toNativeSeparators(rawFileName);

    QStringList args(QLatin1String("delete"));
    args.push_back(file);

    const SubversionResponse response = runSvn(args, subversionShortTimeOut, true);
    return !response.error;
}

/* Subversion has ".svn" directory in each directory
 * it manages. The top level is the first directory
 * under the directory that does not have a  ".svn". */
bool SubversionPlugin::managesDirectory(const QString &directory) const
{
    const QDir dir(directory);
    const bool rc = dir.exists() && managesDirectory(dir);
    if (Subversion::Constants::debug)
        qDebug() << "SubversionPlugin::managesDirectory" << directory << rc;
    return rc;
}

bool SubversionPlugin::managesDirectory(const QDir &directory) const
{
    const QString svnDir = directory.absoluteFilePath(m_svnDotDirectory);
    return QFileInfo(svnDir).isDir();
}

QString SubversionPlugin::findTopLevelForDirectory(const QString &directory) const
{
    // Debug wrapper
    const QString rc = findTopLevelForDirectoryI(directory);
    if (Subversion::Constants::debug)
        qDebug() << "SubversionPlugin::findTopLevelForDirectory" << directory << rc;
    return rc;
}

QString SubversionPlugin::findTopLevelForDirectoryI(const QString &directory) const
{
    /* Recursing up, the top level is a child of the first directory that does
     * not have a  ".svn" directory. The starting directory must be a managed
     * one. Go up and try to find the first unmanaged parent dir. */
    QDir lastDirectory = QDir(directory);
    if (!lastDirectory.exists() || !managesDirectory(lastDirectory))
        return QString();
    for (QDir parentDir = lastDirectory; parentDir.cdUp() ; lastDirectory = parentDir) {
        if (!managesDirectory(parentDir))
            return QDir::toNativeSeparators(lastDirectory.absolutePath());
    }
    return QString();
}

Q_EXPORT_PLUGIN(SubversionPlugin)
