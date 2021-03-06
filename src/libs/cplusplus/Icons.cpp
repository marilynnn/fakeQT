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

#include "Icons.h"

#include <FullySpecifiedType.h>
#include <Scope.h>
#include <Symbols.h>
#include <Type.h>

using namespace CPlusPlus;
using CPlusPlus::Icons;

Icons::Icons()
    : _classIcon(QLatin1String(":/codemodel/images/class.png")),
      _enumIcon(QLatin1String(":/codemodel/images/enum.png")),
      _enumeratorIcon(QLatin1String(":/codemodel/images/enumerator.png")),
      _funcPublicIcon(QLatin1String(":/codemodel/images/func.png")),
      _funcProtectedIcon(QLatin1String(":/codemodel/images/func_prot.png")),
      _funcPrivateIcon(QLatin1String(":/codemodel/images/func_priv.png")),
      _namespaceIcon(QLatin1String(":/codemodel/images/namespace.png")),
      _varPublicIcon(QLatin1String(":/codemodel/images/var.png")),
      _varProtectedIcon(QLatin1String(":/codemodel/images/var_prot.png")),
      _varPrivateIcon(QLatin1String(":/codemodel/images/var_priv.png")),
      _signalIcon(QLatin1String(":/codemodel/images/signal.png")),
      _slotPublicIcon(QLatin1String(":/codemodel/images/slot.png")),
      _slotProtectedIcon(QLatin1String(":/codemodel/images/slot_prot.png")),
      _slotPrivateIcon(QLatin1String(":/codemodel/images/slot_priv.png")),
      _keywordIcon(QLatin1String(":/codemodel/images/keyword.png")),
      _macroIcon(QLatin1String(":/codemodel/images/macro.png"))
{
}

QIcon Icons::iconForSymbol(const Symbol *symbol) const
{
    if (symbol->isFunction() || (symbol->isDeclaration() && symbol->type()->isFunction()))
    {
        const Function *function = symbol->asFunction();
        if (!function)
            function = symbol->type()->asFunction();

        if (function->isSlot()) {
            if (function->isPublic()) {
                return _slotPublicIcon;
            } else if (function->isProtected()) {
                return _slotProtectedIcon;
            } else if (function->isPrivate()) {
                return _slotPrivateIcon;
            }
        } else if (function->isSignal()) {
            return _signalIcon;
        } else if (symbol->isPublic()) {
            return _funcPublicIcon;
        } else if (symbol->isProtected()) {
            return _funcProtectedIcon;
        } else if (symbol->isPrivate()) {
            return _funcPrivateIcon;
        }
    } else if (symbol->scope()->isEnumScope()) {
        return _enumeratorIcon;
    } else if (symbol->isDeclaration() || symbol->isArgument()) {
        if (symbol->isPublic()) {
            return _varPublicIcon;
        } else if (symbol->isProtected()) {
            return _varProtectedIcon;
        } else if (symbol->isPrivate()) {
            return _varPrivateIcon;
        }
    } else if (symbol->isEnum()) {
        return _enumIcon;
    } else if (symbol->isClass()) {
        return _classIcon;
    } else if (symbol->isNamespace()) {
        return _namespaceIcon;
    } else if (symbol->isUsingNamespaceDirective() ||
               symbol->isUsingDeclaration()) {
        // TODO: Might be nice to have a different icons for these things
        return _namespaceIcon;
    }

    return QIcon();
}

QIcon Icons::keywordIcon() const
{
    return _keywordIcon;
}

QIcon Icons::macroIcon() const
{
    return _macroIcon;
}
