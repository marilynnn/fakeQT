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

#include "formeditorfactory.h"
#include "formeditorw.h"
#include "formwindoweditor.h"
#include "designerconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/editormanager/editormanager.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

using namespace Designer::Internal;
using namespace Designer::Constants;

FormEditorFactory::FormEditorFactory(Core::ICore *core) :
    Core::IEditorFactory(core),
    m_kind(QLatin1String(C_FORMEDITOR)),
    m_mimeTypes(QLatin1String(FORM_MIMETYPE)),
    m_core(core)
{
    Core::FileIconProvider *iconProvider = Core::FileIconProvider::instance();
    iconProvider->registerIconForSuffix(QIcon(":/formeditor/images/qt_ui.png"),
                                        QLatin1String("ui"));
}

QString FormEditorFactory::kind() const
{
    return C_FORMEDITOR;
}

Core::IFile *FormEditorFactory::open(const QString &fileName)
{
    Core::IEditor *iface = m_core->editorManager()->openEditor(fileName, kind());
    return iface ? iface->file() : 0;
}

Core::IEditor *FormEditorFactory::createEditor(QWidget *parent)
{
    return FormEditorW::instance()->createFormWindowEditor(parent);
}

QStringList FormEditorFactory::mimeTypes() const
{
    return m_mimeTypes;
}
