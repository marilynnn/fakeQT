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

#ifndef ITEXTEDITOR_H
#define ITEXTEDITOR_H

#include "texteditor_global.h"

#include <coreplugin/editormanager/ieditor.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QColor>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QMenu;
class QTextBlock;
QT_END_NAMESPACE

namespace TextEditor {

class ITextEditor;

class TEXTEDITOR_EXPORT ITextMark : public QObject
{
    Q_OBJECT
public:
    ITextMark(QObject *parent = 0) : QObject(parent) {}
    virtual ~ITextMark() {}

    virtual QIcon icon() const = 0;

    virtual void updateLineNumber(int lineNumber) = 0;
    virtual void updateBlock(const QTextBlock &block) = 0;
    virtual void removedFromEditor() = 0;
    virtual void documentClosing() = 0;
};

typedef QList<ITextMark *> TextMarks;


class TEXTEDITOR_EXPORT ITextMarkable : public QObject
{
    Q_OBJECT
public:
    ITextMarkable(QObject *parent = 0) : QObject(parent) {}
    virtual ~ITextMarkable() {}
    virtual bool addMark(ITextMark *mark, int line) = 0;

    virtual TextMarks marksAt(int line) const = 0;
    virtual void removeMark(ITextMark *mark) = 0;
    virtual bool hasMark(ITextMark *mark) const = 0;
    virtual void updateMark(ITextMark *mark) = 0;
};

class TEXTEDITOR_EXPORT ITextEditor : public Core::IEditor
{
    Q_OBJECT
public:
    enum PositionOperation {
        Current = 1,
        EndOfLine = 2,
        StartOfLine = 3,
        Anchor = 4,
        EndOfDoc = 5
    };

    ITextEditor() {}
    virtual ~ITextEditor() {}

    virtual int find(const QString &string) const = 0;

    virtual void gotoLine(int line, int column = 0) = 0;

    virtual int position(PositionOperation posOp = Current, int at = -1) const = 0;
    virtual void convertPosition(int pos, int *line, int *column) const = 0;
    virtual QRect cursorRect(int pos = -1) const = 0;

    virtual QString contents() const = 0;
    virtual QString selectedText() const = 0;
    virtual QString textAt(int pos, int length) const = 0;
    virtual QChar characterAt(int pos) const = 0;

    virtual void triggerCompletions() = 0;

    virtual ITextMarkable *markableInterface() = 0;

    virtual void setContextHelpId(const QString &) = 0;

    virtual void setTextCodec(QTextCodec *) = 0;
    virtual QTextCodec *textCodec() const = 0;


signals:
    void contentsChanged();
    void markRequested(TextEditor::ITextEditor *editor, int line);
    void markContextMenuRequested(TextEditor::ITextEditor *editor, int line, QMenu *menu);
    void tooltipRequested(TextEditor::ITextEditor *editor, const QPoint &globalPos, int position);
    void contextHelpIdRequested(TextEditor::ITextEditor *editor, int position);
};

} // namespace TextEditor

#endif // ITEXTEDITOR_H
