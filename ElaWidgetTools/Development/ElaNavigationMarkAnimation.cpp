#include "ElaNavigationMarkAnimation.h"

#include "ElaNavigationNode.h"

#include <QPainter>
#include <QVariantAnimation>
#include <QWidget>

ElaNavigationMarkAnimation::ElaNavigationMarkAnimation(QWidget* viewport, bool isFooter, QObject* parent)
    : QObject(parent)
{
    _viewport = viewport;
    _isFooter = isFooter;
    _animation = new QVariantAnimation(this);
    _animation->setDuration(600);
    _animation->setStartValue(0.0);
    _animation->setEndValue(2.0);
    connect(_animation, &QVariantAnimation::valueChanged, this, [this]() {
        if (_viewport)
        {
            _viewport->update();
        }
    });
    connect(_animation, &QVariantAnimation::finished, this, [this]() {
        _previousNode.clear();
    });
}

void ElaNavigationMarkAnimation::start(ElaNavigationNode* previousNode, ElaNavigationNode* selectedNode, bool followPageTransition)
{
    if (previousNode == selectedNode)
    {
        return;
    }
    _animation->stop();
    _previousNode = previousNode;
    _isUpward = !selectedNode ? _isFooter : !previousNode ? !_isFooter
                                                          : _getIsAbove(selectedNode, previousNode);
    if (followPageTransition)
    {
        setPageTransitionProgress(0.0);
    }
    else
    {
        _pageTransitionProgress = 1.0;
        _animation->start();
    }
}

void ElaNavigationMarkAnimation::setPageTransitionProgress(qreal progress)
{
    if (_pageTransitionProgress == progress)
    {
        return;
    }
    _pageTransitionProgress = progress;
    if (progress >= 1.0)
    {
        _previousNode.clear();
    }
    if (_viewport)
    {
        _viewport->update();
    }
}

ElaNavigationNode* ElaNavigationMarkAnimation::displayedNode(ElaNavigationNode* selectedNode) const
{
    return _pageTransitionProgress < 0.5 ? _previousNode.data() : selectedNode;
}

void ElaNavigationMarkAnimation::paint(QPainter* painter, const QRect& itemRect, ElaNavigationNode* node, bool isSelected, const QColor& color) const
{
    const bool standalone = _animation->state() != QAbstractAnimation::Stopped;
    const qreal progress = standalone ? _animation->currentValue().toReal() : 2.0 * _pageTransitionProgress;
    const bool isPrevious = progress < 1.0 && node == _previousNode;
    if (!isPrevious && !(isSelected && progress >= 1.0))
    {
        return;
    }
    const qreal margin = _isFooter ? 10.0 : 10.5;
    const qreal phase = isPrevious ? progress : progress - 1.0;
    const qreal eased = standalone ? QEasingCurve(QEasingCurve::InOutSine).valueForProgress(phase) : phase;
    const qreal animatedMargin = margin * (isPrevious ? 1.0 - eased : eased);
    const bool isTopAnimated = isPrevious ? _isUpward : !_isUpward;
    const qreal top = isTopAnimated ? animatedMargin : margin;
    const qreal bottom = isTopAnimated ? margin : animatedMargin;
    const qreal radius = isPrevious && _isFooter ? 3.0 : 1.5;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(QRectF(itemRect.x() + 3, itemRect.y() + top, 3, itemRect.height() - top - bottom), radius, radius);
    painter->restore();
}

bool ElaNavigationMarkAnimation::_getIsAbove(ElaNavigationNode* node, ElaNavigationNode* referenceNode)
{
    QList<int> path;
    QList<int> referencePath;
    for (auto current = node; current && !current->getIsRootNode(); current = current->getParentNode())
    {
        path.prepend(current->getModelIndex().row());
    }
    for (auto current = referenceNode; current && !current->getIsRootNode(); current = current->getParentNode())
    {
        referencePath.prepend(current->getModelIndex().row());
    }
    for (int i = 0; i < qMin(path.size(), referencePath.size()); i++)
    {
        if (path[i] != referencePath[i])
        {
            return path[i] < referencePath[i];
        }
    }
    return path.size() < referencePath.size();
}
