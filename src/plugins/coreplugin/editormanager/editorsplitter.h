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

#ifndef EDITORSPLITTER_H
#define EDITORSPLITTER_H

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QAction>
#include <QtGui/QSplitter>

namespace Core {

class EditorGroup;
class ICore;
class IEditor;

namespace Internal {

class EditorSplitter : public QWidget
{
    Q_OBJECT

public:
    EditorSplitter(ICore *core, QWidget *parent = 0);
    ~EditorSplitter();

    void setCurrentGroup(Core::EditorGroup *group);
    EditorGroup *currentGroup() const;
    QList<EditorGroup*> groups() const;

    void split(Qt::Orientation orientation);

    void saveSettings(QSettings *settings) const;
    void readSettings(QSettings *settings);
    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    QMap<QString,EditorGroup *> pathGroupMap();

public slots:
    void unsplit();

signals:
    void closeRequested(Core::IEditor *editor);
    void editorGroupsChanged();

private slots:
    void splitHorizontal();
    void splitVertical();
    void gotoNextEditor();
    void gotoPreviousEditor();
    void updateActions();
    void selectNextGroup();
    void selectPreviousGroup();
    void moveDocToNextGroup();
    void moveDocToPreviousGroup();
    void saveCurrentLayout();
    void restoreDefaultLayout();

private:
    enum Side {LEFT = 0, RIGHT = 1};

    void registerActions();
    void createRootGroup();
    EditorGroup *createGroup();
    void deleteGroup(EditorGroup *group);
    void collectGroups(QWidget *widget, QList<EditorGroup*> &groups) const;
    EditorGroup *groupFarthestOnSide(QWidget *node, Side side) const;
    EditorGroup *nextGroup(EditorGroup *curGroup, Side side) const;
    void moveDocToAdjacentGroup(Side side);
    void saveState(QWidget *current, QDataStream &stream) const;
    EditorGroup *restoreState(EditorGroup *current, QDataStream &stream);
    QSplitter *split(Qt::Orientation orientation, EditorGroup *group);
    void fillPathGroupMap(QWidget *current, QString currentPath,
                          QMap<QString,EditorGroup *> &map);
    void unsplitAll();
    void unsplitAll(QWidget *node, QList<IEditor *> &editors);
    QWidget *recreateGroupTree(QWidget *node);
    int editorCount() const;

    QWidget *m_root;
    EditorGroup *m_curGroup;
    ICore *m_core;

    QAction *m_horizontalSplitAction;
    QAction *m_verticalSplitAction;
    QAction *m_unsplitAction;
#if 0
    QAction *m_gotoNextEditorAction;
    QAction *m_gotoPreviousEditorAction;
#endif
    QAction *m_gotoNextGroupAction;
    QAction *m_gotoPreviousGroupAction;
    QAction *m_moveDocToNextGroupAction;
    QAction *m_moveDocToPreviousGroupAction;
    QAction *m_currentAsDefault;
    QAction *m_restoreDefault;
};

} // namespace Internal
} // namespace Core

#endif // EDITORSPLITTER_H
