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

#include "docsettingspage.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtHelp/QHelpEngine>

using namespace Help::Internal;
    
DocSettingsPage::DocSettingsPage(QHelpEngine *helpEngine)
    : m_helpEngine(helpEngine),
      m_registeredDocs(false)
{
}

QString DocSettingsPage::name() const
{
    return "Documentation";
}

QString DocSettingsPage::category() const
{
    return "Help";
}

QString DocSettingsPage::trCategory() const
{
    return tr("Help");
}

QWidget *DocSettingsPage::createPage(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    m_ui.setupUi(w);
    
    connect(m_ui.addButton, SIGNAL(clicked()),
            this, SLOT(addDocumentation()));
    connect(m_ui.removeButton, SIGNAL(clicked()),
            this, SLOT(removeDocumentation()));
    
    m_ui.docsListWidget->addItems(m_helpEngine->registeredDocumentations());
    m_registeredDocs = false;
    m_removeDocs.clear();
        
    return w;
}

void DocSettingsPage::addDocumentation()
{
    QStringList files = QFileDialog::getOpenFileNames(m_ui.addButton->parentWidget(),
                            tr("Add Documentation"),
                            QString(), tr("Qt Help Files (*.qch)"));
    
    if (files.isEmpty())
        return;

    foreach (const QString &file, files) {
        QString nsName = QHelpEngineCore::namespaceName(file);
        if (nsName.isEmpty()) {
            QMessageBox::warning(m_ui.addButton->parentWidget(),
                                 tr("Add Documentation"),
                                 tr("The file %1 is not a valid Qt Help file!")
                                 .arg(file));
            continue;
        }
        m_helpEngine->registerDocumentation(file);
        m_ui.docsListWidget->addItem(nsName);
    }
    m_registeredDocs = true;
    emit documentationAdded();
}

void DocSettingsPage::removeDocumentation()
{
    QListWidgetItem *item = m_ui.docsListWidget->currentItem();
    if (!item)
        return;
    
    m_removeDocs.append(item->text());
    int row = m_ui.docsListWidget->currentRow();
    m_ui.docsListWidget->takeItem(row);    
    if (row > 0)
        --row;
    if (m_ui.docsListWidget->count())
        m_ui.docsListWidget->setCurrentRow(row);

    delete item;    
}

void DocSettingsPage::finished(bool accepted)
{
    if (!accepted)
        return;

    emit dialogAccepted();
}

bool DocSettingsPage::applyChanges()
{
    QStringList::const_iterator it = m_removeDocs.constBegin();
    while (it != m_removeDocs.constEnd()) {
        if (!m_helpEngine->unregisterDocumentation((*it))) {
            QMessageBox::warning(m_ui.addButton->parentWidget(),
                tr("Documentation"),
                tr("Cannot unregister documentation file %1!")
                .arg((*it)));
        }
        ++it;
    }
    return m_registeredDocs || m_removeDocs.count();
}
