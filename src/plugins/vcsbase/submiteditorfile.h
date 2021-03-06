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

#ifndef SUBMITEDITORFILE_H
#define SUBMITEDITORFILE_H

#include <coreplugin/ifile.h>

namespace VCSBase {
namespace Internal {

// A non-saveable IFile for submit editor files.
class SubmitEditorFile : public Core::IFile
{
    Q_OBJECT
public:
    explicit SubmitEditorFile(const QString &mimeType,
                              QObject *parent = 0);

    QString fileName() const { return m_fileName; }
    QString defaultPath() const { return QString(); }
    QString suggestedFileName() const { return QString(); }

    bool isModified() const { return m_modified; }
    virtual QString mimeType() const;
    bool isReadOnly() const { return false; }
    bool isSaveAsAllowed() const { return false; }
    bool save(const QString &fileName);
    void modified(ReloadBehavior * /*behavior*/) { return; }

    void setFileName(const QString name);
    void setModified(bool modified = true);

signals:
    void changed();
    void saveMe(const QString &fileName);

private:
    const QString m_mimeType;
    bool m_modified;
    QString m_fileName;
};


} // namespace Internal
} // namespace VCSBase

#endif // SUBMITEDITORFILE_H
