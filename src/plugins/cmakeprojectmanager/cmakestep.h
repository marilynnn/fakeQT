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

#ifndef CMAKESTEP_H
#define CMAKESTEP_H

#include <projectexplorer/buildstep.h>
#include <projectexplorer/abstractprocessstep.h>

namespace CMakeProjectManager {
namespace Internal {

class CMakeProject;

class CMakeBuildStepConfigWidget;

class CMakeStep : public ProjectExplorer::AbstractProcessStep
{
public:
    CMakeStep(CMakeProject *pro);
    ~CMakeStep();
    virtual bool init(const QString &buildConfiguration);

    virtual void run(QFutureInterface<bool> &fi);

    virtual QString name();
    virtual QString displayName();
    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
    virtual bool immutable() const;
private:
    CMakeProject *m_pro;
};

class CMakeBuildStepConfigWidget :public ProjectExplorer::BuildStepConfigWidget
{
public:
    virtual QString displayName() const;
    virtual void init(const QString &buildConfiguration);
};

class CMakeBuildStepFactory : public ProjectExplorer::IBuildStepFactory
{
    virtual bool canCreate(const QString &name) const;
    virtual ProjectExplorer::BuildStep *create(ProjectExplorer::Project *pro, const QString &name) const;
    virtual QStringList canCreateForProject(ProjectExplorer::Project *pro) const;
    virtual QString displayNameForName(const QString &name) const;
};


}
}
#endif // CMAKESTEP_H
