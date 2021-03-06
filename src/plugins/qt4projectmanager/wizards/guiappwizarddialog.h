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

#ifndef GUIAPPWIZARDDIALOG_H
#define GUIAPPWIZARDDIALOG_H

#include <QtGui/QWizard>

namespace Core {
    namespace Utils {
        class ProjectIntroPage;
    }
}

namespace Qt4ProjectManager {
namespace Internal {

struct QtProjectParameters;
class ModulesPage;
class FilesPage;

// Additional parameters required besides QtProjectParameters
struct GuiAppParameters {
    GuiAppParameters();
    QString className;
    QString baseClassName;
    QString sourceFileName;
    QString headerFileName;
    QString formFileName;
    bool designerForm;
};

class GuiAppWizardDialog : public QWizard
{
    Q_OBJECT

public:
    explicit GuiAppWizardDialog(const QString &templateName,
                                const QIcon &icon,
                                const QList<QWizardPage*> &extensionPages,
                                QWidget *parent = 0);

    void setBaseClasses(const QStringList &baseClasses);
    void setSuffixes(const QString &header, const QString &source,  const QString &form);

    QtProjectParameters projectParameters() const;
    GuiAppParameters parameters() const;

public slots:
    void setPath(const QString &path);
    void setName(const QString &name);

private:
    Core::Utils::ProjectIntroPage *m_introPage;
    ModulesPage *m_modulesPage;
    FilesPage *m_filesPage;
};

} // namespace Internal
} // namespace Qt4ProjectManager

#endif // GUIAPPWIZARDDIALOG_H
