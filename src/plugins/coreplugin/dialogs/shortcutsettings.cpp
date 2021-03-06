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

#include "shortcutsettings.h"
#include "ui_shortcutsettings.h"
#include "actionmanager.h"
#include "command.h"
#include "coreconstants.h"
#include "coreimpl.h"
#include "commandsfile.h"
#include "filemanager.h"

#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/icommand.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QShortcut>
#include <QtGui/QHeaderView>
#include <QtGui/QFileDialog>
#include <QtDebug>

Q_DECLARE_METATYPE(Core::Internal::ShortcutItem*);

using namespace Core;
using namespace Core::Internal;

ShortcutSettings::ShortcutSettings(QObject *parent)
    : IOptionsPage(parent)
{
}

ShortcutSettings::~ShortcutSettings()
{
}

// IOptionsPage
QString ShortcutSettings::name() const
{
    return tr("Keyboard");
}

QString ShortcutSettings::category() const
{
    return QLatin1String("Environment");
}

QString ShortcutSettings::trCategory() const
{
    return tr("Environment");
}

QWidget *ShortcutSettings::createPage(QWidget *parent)
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;

    m_page = new Ui_ShortcutSettings();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->resetButton->setIcon(QIcon(Constants::ICON_RESET));
    m_page->shortcutEdit->installEventFilter(this);

    connect(m_page->resetButton, SIGNAL(clicked()),
        this, SLOT(resetKeySequence()));
    connect(m_page->removeButton, SIGNAL(clicked()),
        this, SLOT(removeKeySequence()));
    connect(m_page->exportButton, SIGNAL(clicked()),
        this, SLOT(exportAction()));
    connect(m_page->importButton, SIGNAL(clicked()),
        this, SLOT(importAction()));
    connect(m_page->defaultButton, SIGNAL(clicked()),
        this, SLOT(defaultAction()));

    initialize();

    m_page->commandList->sortByColumn(0, Qt::AscendingOrder);

    connect(m_page->filterEdit, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));
    connect(m_page->commandList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        this, SLOT(commandChanged(QTreeWidgetItem *)));
    connect(m_page->shortcutEdit, SIGNAL(textChanged(QString)), this, SLOT(keyChanged()));

    QHeaderView *hv = m_page->commandList->header();
    hv->resizeSection(0, 210);
    hv->resizeSection(1, 110);
    hv->setStretchLastSection(true);

    commandChanged(0);

    return w;
}

void ShortcutSettings::finished(bool accepted)
{
    if (accepted) {
        foreach (ShortcutItem *item, m_scitems) {
            item->m_cmd->setKeySequence(item->m_key);
        }
    }

    qDeleteAll(m_scitems);
    m_scitems.clear();
}

bool ShortcutSettings::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *k = static_cast<QKeyEvent*>(e);
        handleKeyEvent(k);
        return true;
    }

    if ( e->type() == QEvent::Shortcut ||
         e->type() == QEvent::ShortcutOverride  ||
         e->type() == QEvent::KeyRelease )
        return true;

    return false;
}

void ShortcutSettings::commandChanged(QTreeWidgetItem *current)
{
    if (!current || !current->data(0, Qt::UserRole).isValid()) {
        m_page->shortcutEdit->setText("");
        m_page->seqGrp->setEnabled(false);
        return;
    }
    m_page->seqGrp->setEnabled(true);
    ShortcutItem *scitem = qVariantValue<ShortcutItem *>(current->data(0, Qt::UserRole));
    setKeySequence(scitem->m_key);
}

void ShortcutSettings::filterChanged(const QString &f)
{
    for (int i=0; i<m_page->commandList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_page->commandList->topLevelItem(i);
        item->setHidden(filter(f, item));
    }
}

void ShortcutSettings::keyChanged()
{
    QTreeWidgetItem *current = m_page->commandList->currentItem();
    if (current && current->data(0, Qt::UserRole).isValid()) {
        ShortcutItem *scitem = qVariantValue<ShortcutItem *>(current->data(0, Qt::UserRole));
        scitem->m_key = QKeySequence(m_key[0], m_key[1], m_key[2], m_key[3]);
        current->setText(2, scitem->m_key);
    }
}

void ShortcutSettings::setKeySequence(const QKeySequence &key)
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    m_keyNum = key.count();
    for (int i = 0; i < m_keyNum; ++i) {
        m_key[i] = key[i];
    }
    m_page->shortcutEdit->setText(key);
}

bool ShortcutSettings::filter(const QString &f, const QTreeWidgetItem *item)
{
    if (item->childCount() == 0) {
        if (f.isEmpty())
            return false;
        for (int i = 0; i < item->columnCount(); ++i) {
            if (item->text(i).contains(f, Qt::CaseInsensitive))
                return false;
        }
        return true;
    }

    bool found = false;
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *citem = item->child(i);
        if (filter(f, citem)) {
            citem->setHidden(true);
        } else {
            citem->setHidden(false);
            found = true;
        }
    }
    return !found;
}

void ShortcutSettings::resetKeySequence()
{
    QTreeWidgetItem *current = m_page->commandList->currentItem();
    if (current && current->data(0, Qt::UserRole).isValid()) {
        ShortcutItem *scitem = qVariantValue<ShortcutItem *>(current->data(0, Qt::UserRole));
        setKeySequence(scitem->m_cmd->defaultKeySequence());
    }
}

void ShortcutSettings::removeKeySequence()
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    m_page->shortcutEdit->clear();
}

void ShortcutSettings::importAction()
{
    UniqueIDManager *uidm =
        CoreImpl::instance()->uniqueIDManager();

    QString fileName = QFileDialog::getOpenFileName(0, tr("Import Keyboard Mapping Scheme"),
        CoreImpl::instance()->resourcePath() + "/schemes/",
        tr("Keyboard Mapping Scheme (*.kms)"));
    if (!fileName.isEmpty()) {
        CommandsFile cf(fileName);
        QMap<QString, QKeySequence> mapping = cf.importCommands();

        foreach (ShortcutItem *item, m_scitems) {
            QString sid = uidm->stringForUniqueIdentifier(item->m_cmd->id());
            if (mapping.contains(sid)) {
                item->m_key = mapping.value(sid);
                item->m_item->setText(2, item->m_key);
                if (item->m_item == m_page->commandList->currentItem())
                    commandChanged(item->m_item);
            }
        }
    }
}

void ShortcutSettings::defaultAction()
{
    foreach (ShortcutItem *item, m_scitems) {
        item->m_key = item->m_cmd->defaultKeySequence();
        item->m_item->setText(2, item->m_key);
        if (item->m_item == m_page->commandList->currentItem())
            commandChanged(item->m_item);
    }
}

void ShortcutSettings::exportAction()
{
    QString fileName = CoreImpl::instance()->fileManager()->getSaveFileNameWithExtension(
        tr("Export Keyboard Mapping Scheme"),
        CoreImpl::instance()->resourcePath() + "/schemes/",
        tr("Keyboard Mapping Scheme (*.kms)"),
        ".kms");
    if (!fileName.isEmpty()) {
        CommandsFile cf(fileName);
        cf.exportCommands(m_scitems);
    }
}

void ShortcutSettings::initialize()
{
    QMap<QString, QTreeWidgetItem *> categories;

    m_am = ActionManager::instance();
    UniqueIDManager *uidm =
        CoreImpl::instance()->uniqueIDManager();

    QList<Command *> cmds = m_am->commands();
    for (int i = 0; i < cmds.size(); ++i) {
        Command *c = cmds.at(i);
        if (c->hasAttribute(Command::CA_NonConfigureable))
            continue;
        if (c->action() && c->action()->isSeparator())
            continue;

        QTreeWidgetItem *item = 0;
        ShortcutItem *s = new ShortcutItem;
        m_scitems << s;
        if (c->category().isEmpty()) {
            item = new QTreeWidgetItem(m_page->commandList);
        } else {
            if (!categories.contains(c->category())) {
                QTreeWidgetItem *cat = new QTreeWidgetItem(m_page->commandList);
                cat->setText(0, c->category());
                categories.insert(c->category(), cat);
                cat->setExpanded(true);
            }
            item = new QTreeWidgetItem(categories.value(c->category()));
        }
        s->m_cmd = c;
        s->m_item = item;

        item->setText(0, uidm->stringForUniqueIdentifier(c->id()));

        if (c->action()) {
            QString text = c->hasAttribute(Command::CA_UpdateText) && !c->defaultText().isNull() ? c->defaultText() : c->action()->text();
            s->m_key = c->action()->shortcut();
            item->setText(1, text);
        } else {
            s->m_key = c->shortcut()->key();
            item->setText(1, c->shortcut()->whatsThis());
        }

        item->setText(2, s->m_key);
        item->setData(0, Qt::UserRole, qVariantFromValue(s));
    }
}

void ShortcutSettings::handleKeyEvent(QKeyEvent *e)
{
    int nextKey = e->key();
    if ( m_keyNum > 3 ||
         nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift ||
         nextKey == Qt::Key_Meta ||
         nextKey == Qt::Key_Alt )
         return;

    nextKey |= translateModifiers(e->modifiers(), e->text());
    switch (m_keyNum) {
        case 0:
            m_key[0] = nextKey;
            break;
        case 1:
            m_key[1] = nextKey;
            break;
        case 2:
            m_key[2] = nextKey;
            break;
        case 3:
            m_key[3] = nextKey;
            break;
        default:
            break;
    }
    m_keyNum++;
    QKeySequence ks(m_key[0], m_key[1], m_key[2], m_key[3]);
    m_page->shortcutEdit->setText(ks);
    e->accept();
}

int ShortcutSettings::translateModifiers(Qt::KeyboardModifiers state,
                                         const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetter()
                                        || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}
