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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QtCore/QPointer>
#include <QtCore/QTime>
#include <QtCore/QBasicTimer>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QWidget>

/* 
 * This is a set of helper classes to allow for widget animations in
 * the style. Its mostly taken from Vista style so it should be fully documented
 * there.
 *
 */
 
class Animation
{
public :
    Animation() : _running(true) { }
    virtual ~Animation() { }
    QWidget * widget() const { return _widget; }
    bool running() const { return _running; }
    const QTime &startTime() const { return _startTime; }
    void setRunning(bool val) { _running = val; }
    void setWidget(QWidget *widget) { _widget = widget; }
    void setStartTime(const QTime &startTime) { _startTime = startTime; }
    virtual void paint(QPainter *painter, const QStyleOption *option);

protected:
    void drawBlendedImage(QPainter *painter, QRect rect, float value);
    QTime _startTime;
    QPointer<QWidget> _widget;
    QImage _primaryImage;
    QImage _secondaryImage;
    QImage _tempImage;
    bool _running;
};

// Handles state transition animations
class Transition : public Animation
{
public :
    Transition() : Animation() {}
    virtual ~Transition() { }
    void setDuration(int duration) { _duration = duration; }
    void setStartImage(const QImage &image) { _primaryImage = image; }
    void setEndImage(const QImage &image) { _secondaryImage = image; }
    virtual void paint(QPainter *painter, const QStyleOption *option);
    int duration() const { return _duration; }
    int _duration; //set time in ms to complete a state transition
};

class StyleAnimator : public QObject
{
    Q_OBJECT;

public:
    StyleAnimator(QObject *parent = 0) : QObject(parent) {}

    void timerEvent(QTimerEvent *);
    void startAnimation(Animation *);
    void stopAnimation(const QWidget *);
    Animation* widgetAnimation(const QWidget *) const;
    
private:
    QBasicTimer animationTimer;
    QList <Animation*> animations;
};

#endif // ANIMATION_H
