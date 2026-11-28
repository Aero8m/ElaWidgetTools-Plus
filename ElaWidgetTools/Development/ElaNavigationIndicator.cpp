#include "ElaNavigationIndicator.h"

#include "ElaTheme.h"

#include <QEasingCurve>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QtMath>

ElaNavigationIndicator::ElaNavigationIndicator(QWidget* parent)
    : QWidget{parent}
{
    setObjectName("ElaNavigationIndicator");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    hide();

    _themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, this, [this](ElaThemeType::ThemeMode themeMode) {
        _themeMode = themeMode;
        if (_isActive)
        {
            update();
        }
    });

    _animationGroup = new QParallelAnimationGroup(this);
    connect(_animationGroup, &QParallelAnimationGroup::finished, this, [this]() {
        _isActive = false;
        hide();
        Q_EMIT animationFinished();
    });
}

ElaNavigationIndicator::~ElaNavigationIndicator()
{
}

bool ElaNavigationIndicator::isActive() const
{
    return _isActive;
}

void ElaNavigationIndicator::startAnimation(const QRectF& startRect, const QRectF& endRect)
{
    if (!endRect.isValid() || endRect.isEmpty())
    {
        stopAnimation();
        return;
    }

    const QRectF effectiveStart = _isActive ? _currentRect : startRect;
    if (!effectiveStart.isValid() || effectiveStart.isEmpty())
    {
        stopAnimation();
        return;
    }

    const bool alreadyAtTarget = qAbs(effectiveStart.x() - endRect.x()) < 0.5
        && qAbs(effectiveStart.y() - endRect.y()) < 0.5
        && qAbs(effectiveStart.width() - endRect.width()) < 0.5
        && qAbs(effectiveStart.height() - endRect.height()) < 0.5;
    if (alreadyAtTarget)
    {
        stopAnimation();
        return;
    }

    _animationGroup->stop();
    _animationGroup->clear();

    _currentRect = effectiveStart;
    _currentRect.setWidth(endRect.width());
    _isActive = true;
    if (parentWidget())
    {
        setGeometry(parentWidget()->rect());
    }
    raise();
    show();

    const qreal startY = effectiveStart.y();
    const qreal endY = endRect.y();
    const qreal startLen = effectiveStart.height();
    const qreal endLen = endRect.height();
    const QPointF startPos = effectiveStart.topLeft();
    const QPointF endPos = endRect.topLeft();

    if (qAbs(endY - startY) < 0.5)
    {
        auto* posAni = new QPropertyAnimation(this, "indicatorPos");
        posAni->setDuration(400);
        posAni->setEasingCurve(_createBezierCurve(0.1, 0.9, 0.2, 1.0));
        posAni->setStartValue(startPos);
        posAni->setEndValue(endPos);

        auto* lengthAni = new QPropertyAnimation(this, "indicatorLength");
        lengthAni->setDuration(400);
        lengthAni->setEasingCurve(_createBezierCurve(0.1, 0.9, 0.2, 1.0));
        lengthAni->setStartValue(startLen);
        lengthAni->setEndValue(endLen);

        _animationGroup->addAnimation(posAni);
        _animationGroup->addAnimation(lengthAni);
    }
    else
    {
        // WinUI 3 / PyQt-Fluent-Widgets ScaleSlideAnimation: stretch toward target, then settle.
        const bool isForward = endY > startY;
        const qreal midLength = isForward ? (endY + endLen - startY) : (startY + startLen - endY);
        const QEasingCurve stretchCurve = _createBezierCurve(0.9, 0.1, 1.0, 0.2);
        const QEasingCurve settleCurve = _createBezierCurve(0.1, 0.9, 0.2, 1.0);

        auto* posAni1 = new QPropertyAnimation(this, "indicatorPos");
        auto* posAni2 = new QPropertyAnimation(this, "indicatorPos");
        posAni1->setDuration(200);
        posAni2->setDuration(400);
        posAni1->setEasingCurve(stretchCurve);
        posAni2->setEasingCurve(settleCurve);

        auto* lengthAni1 = new QPropertyAnimation(this, "indicatorLength");
        auto* lengthAni2 = new QPropertyAnimation(this, "indicatorLength");
        lengthAni1->setDuration(200);
        lengthAni2->setDuration(400);
        lengthAni1->setEasingCurve(stretchCurve);
        lengthAni2->setEasingCurve(settleCurve);

        if (isForward)
        {
            posAni1->setStartValue(startPos);
            posAni1->setEndValue(startPos);
            lengthAni1->setStartValue(startLen);
            lengthAni1->setEndValue(midLength);

            posAni2->setStartValue(startPos);
            posAni2->setEndValue(endPos);
            lengthAni2->setStartValue(midLength);
            lengthAni2->setEndValue(endLen);
        }
        else
        {
            posAni1->setStartValue(startPos);
            posAni1->setEndValue(endPos);
            lengthAni1->setStartValue(startLen);
            lengthAni1->setEndValue(midLength);

            posAni2->setStartValue(endPos);
            posAni2->setEndValue(endPos);
            lengthAni2->setStartValue(midLength);
            lengthAni2->setEndValue(endLen);
        }

        auto* posGroup = new QSequentialAnimationGroup();
        posGroup->addAnimation(posAni1);
        posGroup->addAnimation(posAni2);

        auto* lengthGroup = new QSequentialAnimationGroup();
        lengthGroup->addAnimation(lengthAni1);
        lengthGroup->addAnimation(lengthAni2);

        _animationGroup->addAnimation(posGroup);
        _animationGroup->addAnimation(lengthGroup);
    }

    _animationGroup->start();
    update();
}

void ElaNavigationIndicator::stopAnimation()
{
    if (!_isActive && _animationGroup->state() == QAbstractAnimation::Stopped)
    {
        return;
    }
    _animationGroup->stop();
    _animationGroup->clear();
    _isActive = false;
    hide();
    Q_EMIT animationFinished();
}

QPointF ElaNavigationIndicator::indicatorPos() const
{
    return _currentRect.topLeft();
}

void ElaNavigationIndicator::setIndicatorPos(const QPointF& pos)
{
    const QRectF previousRect = _currentRect;
    _currentRect.moveTopLeft(pos);
    _updateDirtyRect(previousRect);
}

qreal ElaNavigationIndicator::indicatorLength() const
{
    return _currentRect.height();
}

void ElaNavigationIndicator::setIndicatorLength(qreal length)
{
    const QRectF previousRect = _currentRect;
    _currentRect.setHeight(qMax(0.0, length));
    _updateDirtyRect(previousRect);
}

void ElaNavigationIndicator::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    if (!_isActive || _currentRect.isEmpty())
    {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ElaThemeColor(_themeMode, PrimaryNormal));
    painter.drawRoundedRect(_currentRect, 1.5, 1.5);
}

QEasingCurve ElaNavigationIndicator::_createBezierCurve(qreal x1, qreal y1, qreal x2, qreal y2)
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(x1, y1), QPointF(x2, y2), QPointF(1.0, 1.0));
    return curve;
}

void ElaNavigationIndicator::_updateDirtyRect(const QRectF& previousRect)
{
    update(previousRect.united(_currentRect).adjusted(-2.0, -2.0, 2.0, 2.0).toAlignedRect());
}
