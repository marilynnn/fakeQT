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

#include "fileiconprovider.h"

using namespace Core;

/*!
  \class FileIconProvider

  Provides icons based on file suffixes.

  The class is a singleton: It's instance can be accessed via the static instance() method.
  Plugins can register custom icons via registerIconSuffix(), and retrieve icons via the icon()
  method.
  */

FileIconProvider *FileIconProvider::m_instance = 0;

FileIconProvider::FileIconProvider()
    : m_unknownFileIcon(QLatin1String(":/qworkbench/images/unknownfile.png"))
{
}

FileIconProvider::~FileIconProvider()
{
    m_instance = 0;
}

/*!
  Returns the icon associated with the file suffix in fileInfo. If there is none,
  the default icon of the operating system is returned.
  */
QIcon FileIconProvider::icon(const QFileInfo &fileInfo)
{
    const QString suffix = fileInfo.suffix();
    QIcon icon = iconForSuffix(suffix);

    if (icon.isNull()) {
        // Get icon from OS and store it in the cache

        // Disabled since for now we'll make sure that all icons fit with our
        // own custom icons by returning an empty one if we don't know it.
#if 0
        // This is incorrect if the OS does not always return the same icon for the
        // same suffix (Mac OS X), but should speed up the retrieval a lot ...
        icon = m_systemIconProvider.icon(fileInfo);
        if (!suffix.isEmpty())
            registerIconForSuffix(icon, suffix);
#else
        if (fileInfo.isDir()) {
            icon = m_systemIconProvider.icon(fileInfo);
        } else {
            icon = m_unknownFileIcon;
        }
#endif
    }

    return icon;
}

/*!
  Registers an icon for a given suffix, overriding any existing icon.
  */
void FileIconProvider::registerIconForSuffix(const QIcon &icon, const QString &suffix)
{
    // delete old icon, if it exists
    QList<QPair<QString,QIcon> >::iterator iter = m_cache.begin();
    for (; iter != m_cache.end(); ++iter) {
        if ((*iter).first == suffix) {
            iter = m_cache.erase(iter);
            break;
        }
    }

    QPair<QString,QIcon> newEntry(suffix, icon);
    m_cache.append(newEntry);
}

/*!
  Returns an icon for the given suffix, or an empty one if none registered.
  */
QIcon FileIconProvider::iconForSuffix(const QString &suffix) const
{
    QIcon icon;

    if (suffix.isEmpty())
        return icon;

    QList<QPair<QString,QIcon> >::const_iterator iter = m_cache.constBegin();
    for (; iter != m_cache.constEnd(); ++iter) {
        if ((*iter).first == suffix) {
            icon = (*iter).second;
            break;
        }
    }

    return icon;
}

/*!
  Returns the sole instance of FileIconProvider.
  */
FileIconProvider *FileIconProvider::instance()
{
    if (!m_instance)
        m_instance = new FileIconProvider;
    return m_instance;
}
