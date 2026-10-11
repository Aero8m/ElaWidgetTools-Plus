#include "ElaCentralStackedWidget.h"

#include "ElaTheme.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsBlurEffect>
#include <QHideEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QTimer>
#include <cmath>
ElaCentralStackedWidget::ElaCentralStackedWidget(QWidget* parent)
    : QWidget(parent)
{
    _pPopupAnimationYOffset = 0;
    _pScaleAnimationRatio = 1;
    _pScaleAnimationPixOpacity = 1;
    _pFlipAnimationRatio = 1;
    _pBlurAnimationRadius = 0;
    _pLastTargetIndex = 0;

    setObjectName("ElaCentralStackedWidget");
    setStyleSheet("#ElaCentralStackedWidget{background-color:transparent;}");

    _containerStackedWidget = new QStackedWidget(this);
    _containerStackedWidget->setObjectName("ElaCentralStackedWidget");
    _containerStackedWidget->setStyleSheet("#ElaCentralStackedWidget{background-color:transparent;}");

    _blurEffect = new QGraphicsBlurEffect(_containerStackedWidget);
    _blurEffect->setBlurHints(QGraphicsBlurEffect::BlurHint::QualityHint);
    _blurEffect->setBlurRadius(0);
    _blurEffect->setEnabled(false);
    _containerStackedWidget->setGraphicsEffect(_blurEffect);

    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setSpacing(0);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->addWidget(_containerStackedWidget);

    _themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, this, &ElaCentralStackedWidget::onThemeModeChanged);

    _popupSwitchTimer = new QTimer(this);
    _popupSwitchTimer->setSingleShot(true);
    connect(_popupSwitchTimer, &QTimer::timeout, this, &ElaCentralStackedWidget::_startPopupSwitch);
    _popupAnimation = new QPropertyAnimation(this, "pPopupAnimationYOffset", this);
    _popupAnimation->setDuration(300);
    _popupAnimation->setEasingCurve(QEasingCurve::OutQuart);
    connect(_popupAnimation, &QPropertyAnimation::valueChanged, this, [this]() {
        if (_popupActive) update();
    });
    connect(_containerStackedWidget, &QStackedWidget::widgetRemoved, this, [this]() {
        if ((_pendingPopupTarget && _containerStackedWidget->indexOf(_pendingPopupTarget) < 0) ||
            (_popupActive && (!_popupTarget || _containerStackedWidget->indexOf(_popupTarget) < 0)))
        {
            _finishPopupSwitch();
        }
        // Removal can renumber both the current page and a pending target.
        _pLastTargetIndex = _pendingPopupTarget ? _containerStackedWidget->indexOf(_pendingPopupTarget)
                                               : _containerStackedWidget->currentIndex();
    });
}

ElaCentralStackedWidget::~ElaCentralStackedWidget()
{
    _finishPopupSwitch(false);
}

QStackedWidget* ElaCentralStackedWidget::getContainerStackedWidget() const
{
    return _containerStackedWidget;
}

void ElaCentralStackedWidget::setCustomWidget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
    if (_customWidget)
    {
        _mainLayout->removeWidget(_customWidget);
    }
    _mainLayout->insertWidget(0, widget);
    _customWidget = widget;
}

QWidget* ElaCentralStackedWidget::getCustomWidget() const
{
    return this->_customWidget;
}

void ElaCentralStackedWidget::onThemeModeChanged(ElaThemeType::ThemeMode themeMode)
{
    _themeMode = themeMode;
}

void ElaCentralStackedWidget::setIsTransparent(bool isTransparent)
{
    this->_isTransparent = isTransparent;
    update();
}

bool ElaCentralStackedWidget::getIsTransparent() const
{
    return _isTransparent;
}

void ElaCentralStackedWidget::setIsHasRadius(bool isHasRadius)
{
    this->_isHasRadius = isHasRadius;
    update();
}

void ElaCentralStackedWidget::doWindowStackSwitch(ElaWindowType::StackSwitchMode stackSwitchMode, int nodeIndex, bool isRouteBack)
{
    QPointer<QWidget> target = _containerStackedWidget->widget(nodeIndex);
    if (!target) return;
    if (stackSwitchMode == ElaWindowType::Popup)
    {
        if (_pendingPopupTarget == target || _popupTarget == target ||
            (!_popupSwitchTimer->isActive() && _containerStackedWidget->currentWidget() == target))
        {
            return;
        }
        const bool rapid = _popupActive || _pendingPopupTarget ||
                           (_lastPopupRequest.isValid() && _lastPopupRequest.elapsed() < 200);
        _lastPopupRequest.start();
        _finishPopupSwitch();
        if (!target || _containerStackedWidget->indexOf(target) < 0) return;
        _stackSwitchMode = stackSwitchMode;
        _pLastTargetIndex = _containerStackedWidget->indexOf(target);
        _pendingPopupTarget = target;
        _pendingPopupSerial = _popupRequestSerial;
        _pendingPopupAnimated = !rapid;
        const quint64 serial = _popupRequestSerial;
        _popupDestroyedConnection = connect(target, &QObject::destroyed, this, [this, serial]() {
            if (serial == _popupRequestSerial) _finishPopupSwitch();
        });
        // Keep the ordinary transition's pause. Rapid requests replace this
        // pending task and settle immediately instead of queueing old pages.
        _popupSwitchTimer->start(_pendingPopupAnimated ? 180 : 0);
        return;
    }
    _finishPopupSwitch();
    if (!target || _containerStackedWidget->indexOf(target) < 0) return;
    nodeIndex = _containerStackedWidget->indexOf(target);
    if (stackSwitchMode == ElaWindowType::None)
    {
        _stackSwitchMode = stackSwitchMode;
        _pLastTargetIndex = nodeIndex;
        _containerStackedWidget->setCurrentWidget(target);
        return;
    }
    if (_pLastTargetIndex == nodeIndex)
    {
        return;
    }
    _pLastTargetIndex = nodeIndex;
    _stackSwitchMode = stackSwitchMode;
    switch (stackSwitchMode)
    {
    case ElaWindowType::None:
    {
        _containerStackedWidget->setCurrentIndex(nodeIndex);
        break;
    }
    case ElaWindowType::Popup:
    {
        // Handled above so rapid requests never enter another animation path.
        break;
    }
    case ElaWindowType::Scale:
    {
        QWidget* targetWidget = _containerStackedWidget->widget(nodeIndex);
        _getCurrentStackPix();
        _containerStackedWidget->setCurrentIndex(nodeIndex);
        targetWidget->setVisible(true);
        _getTargetStackPix();
        targetWidget->setVisible(false);
        _isDrawNewPix = false;
        QPropertyAnimation* currentPixZoomAnimation = new QPropertyAnimation(this, "pScaleAnimationRatio");
        connect(currentPixZoomAnimation, &QPropertyAnimation::valueChanged, this, [=]() {
            update();
        });
        connect(currentPixZoomAnimation, &QPropertyAnimation::finished, this, [=]() {
            _isDrawNewPix = true;
            QPropertyAnimation* targetPixZoomAnimation = new QPropertyAnimation(this, "pScaleAnimationRatio");
            connect(targetPixZoomAnimation, &QPropertyAnimation::valueChanged, this, [=]() {
                update();
            });
            connect(targetPixZoomAnimation, &QPropertyAnimation::finished, this, [=]() {
                _targetStackPix = QPixmap();
                _currentStackPix = QPixmap();
                targetWidget->setVisible(true);
            });
            if (isRouteBack)
            {
                targetPixZoomAnimation->setStartValue(1.5);
                targetPixZoomAnimation->setEndValue(1);
            }
            else
            {
                // 放大
                targetPixZoomAnimation->setStartValue(0.85);
                targetPixZoomAnimation->setEndValue(1);
            }
            targetPixZoomAnimation->setDuration(300);
            targetPixZoomAnimation->setEasingCurve(QEasingCurve::OutCubic);
            targetPixZoomAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        });
        if (isRouteBack)
        {
            // 缩小
            currentPixZoomAnimation->setStartValue(1);
            currentPixZoomAnimation->setEndValue(0.85);
        }
        else
        {
            // 放大
            currentPixZoomAnimation->setStartValue(1);
            currentPixZoomAnimation->setEndValue(1.15);
        }
        currentPixZoomAnimation->setDuration(150);
        currentPixZoomAnimation->start(QAbstractAnimation::DeleteWhenStopped);

        QPropertyAnimation* currentPixOpacityAnimation = new QPropertyAnimation(this, "pScaleAnimationPixOpacity");
        connect(currentPixZoomAnimation, &QPropertyAnimation::finished, this, [=]() {
            QPropertyAnimation* targetPixOpacityAnimation = new QPropertyAnimation(this, "pScaleAnimationPixOpacity");
            targetPixOpacityAnimation->setStartValue(0);
            targetPixOpacityAnimation->setEndValue(1);
            targetPixOpacityAnimation->setDuration(300);
            targetPixOpacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        });
        currentPixOpacityAnimation->setStartValue(1);
        currentPixOpacityAnimation->setEndValue(0);
        currentPixOpacityAnimation->setDuration(150);
        currentPixOpacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        break;
    }
    case ElaWindowType::Flip:
    {
        QWidget* targetWidget = _containerStackedWidget->widget(nodeIndex);
        _getCurrentStackPix();
        _containerStackedWidget->setCurrentIndex(nodeIndex);
        _getTargetStackPix();
        targetWidget->setVisible(false);
        QPropertyAnimation* flipAnimation = new QPropertyAnimation(this, "pFlipAnimationRatio");
        connect(flipAnimation, &QPropertyAnimation::valueChanged, this, [=]() {
            update();
        });
        connect(flipAnimation, &QPropertyAnimation::finished, this, [=]() {
            _targetStackPix = QPixmap();
            _currentStackPix = QPixmap();
            targetWidget->setVisible(true);
        });
        flipAnimation->setEasingCurve(QEasingCurve::InOutSine);
        flipAnimation->setDuration(650);
        flipAnimation->setStartValue(0);
        flipAnimation->setEndValue(isRouteBack ? -180 : 180);
        flipAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        break;
    }
    case ElaWindowType::Blur:
    {
        _targetStackPix = QPixmap();
        _blurEffect->setEnabled(true);
        QPropertyAnimation* blurAnimation = new QPropertyAnimation(this, "pBlurAnimationRadius");
        connect(blurAnimation, &QPropertyAnimation::valueChanged, this, [=]() {
            _blurEffect->setBlurRadius(_pBlurAnimationRadius);
        });
        connect(blurAnimation, &QPropertyAnimation::finished, this, [=]() {
            _blurEffect->setEnabled(false);
        });
        blurAnimation->setEasingCurve(QEasingCurve::InOutSine);
        blurAnimation->setDuration(350);
        blurAnimation->setStartValue(40);
        blurAnimation->setEndValue(2);
        blurAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        QApplication::processEvents();
        _containerStackedWidget->setCurrentIndex(nodeIndex);
        break;
    }
    }
}

void ElaCentralStackedWidget::_startPopupSwitch()
{
    const quint64 serial = _pendingPopupSerial;
    QPointer<QWidget> target = _pendingPopupTarget;
    const bool animated = _pendingPopupAnimated && isVisible();
    if (serial != _popupRequestSerial) return;
    if (!target || _containerStackedWidget->indexOf(target) < 0)
    {
        _finishPopupSwitch();
        return;
    }
    _containerStackedWidget->setCurrentWidget(target);
    // A page's show handler may remove itself or request another page.
    if (serial != _popupRequestSerial) return;
    if (!target || _containerStackedWidget->currentWidget() != target)
    {
        _finishPopupSwitch();
        return;
    }
    _pendingPopupTarget.clear();
    _pLastTargetIndex = _containerStackedWidget->currentIndex();
    if (!animated)
    {
        _finishPopupSwitch();
        return;
    }
    _popupTarget = target;
    _popupActive = true;
    // Rendering can synchronously deliver layout/resize events. Publish the
    // snapshot only if this request is still current after grab() returns.
    const bool transparent = _isTransparent;
    _isTransparent = true;
    const QPixmap snapshot = _containerStackedWidget->grab();
    _isTransparent = transparent;
    if (serial != _popupRequestSerial || !target) return;
    _targetStackPix = snapshot;
    if (_targetStackPix.isNull())
    {
        _finishPopupSwitch();
        return;
    }
    target->hide();
    if (serial != _popupRequestSerial || !target) return;
    const int y = _containerStackedWidget->mapToParent(QPoint(0, 0)).y();
    _popupAnimation->setStartValue(y + 120);
    _popupAnimation->setEndValue(y);
    _popupFinishedConnection = connect(_popupAnimation, &QPropertyAnimation::finished, this, [this, serial]() {
        if (serial == _popupRequestSerial) _finishPopupSwitch();
    });
    _popupAnimation->start();
}

void ElaCentralStackedWidget::_finishPopupSwitch(bool restorePage)
{
    const bool ownedPopup = _popupActive || _pendingPopupTarget || _popupSwitchTimer->isActive();
    ++_popupRequestSerial;
    _popupSwitchTimer->stop();
    disconnect(_popupFinishedConnection);
    disconnect(_popupDestroyedConnection);
    _popupAnimation->stop();
    QPointer<QWidget> target = _popupTarget;
    _pendingPopupTarget.clear();
    _popupTarget.clear();
    _popupActive = false;
    if (ownedPopup)
    {
        _targetStackPix = QPixmap();
        _currentStackPix = QPixmap();
        _pPopupAnimationYOffset = 0;
        _pLastTargetIndex = _containerStackedWidget->currentIndex();
        if (restorePage && target && _containerStackedWidget->indexOf(target) >= 0)
        {
            // Internal contract: synthetic animation restoration must not reload
            // page data. Normal page activation and explicit refreshes still do.
            const QVariant previous = target->property("ElaPageTransitionRestore");
            target->setProperty("ElaPageTransitionRestore", true);
            target->show();
            if (target) target->setProperty("ElaPageTransitionRestore", previous);
        }
        update();
    }
}

void ElaCentralStackedWidget::_settlePopupSwitch()
{
    QPointer<QWidget> target = _pendingPopupTarget;
    _finishPopupSwitch();
    if (target && _containerStackedWidget->indexOf(target) >= 0)
    {
        _containerStackedWidget->setCurrentWidget(target);
        _pLastTargetIndex = _containerStackedWidget->currentIndex();
    }
}

void ElaCentralStackedWidget::hideEvent(QHideEvent* event)
{
    _settlePopupSwitch();
    QWidget::hideEvent(event);
}

void ElaCentralStackedWidget::resizeEvent(QResizeEvent* event)
{
    // A snapshot made for the old size must never be stretched or left visible.
    _settlePopupSwitch();
    QWidget::resizeEvent(event);
}

void ElaCentralStackedWidget::paintEvent(QPaintEvent* event)
{
    QRect targetRect = this->rect();
    targetRect.adjust(1, 1, 10, 10);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    if (!_isTransparent)
    {
        painter.setPen(QPen(ElaThemeColor(_themeMode, BasicBaseLine), 1.5));
        painter.setBrush(ElaThemeColor(_themeMode, WindowCentralStackBase));
        if (_isHasRadius)
        {
            painter.drawRoundedRect(targetRect, 10, 10);
        }
        else
        {
            painter.drawRect(targetRect);
        }
    }
    // 切换动画
    if (!_targetStackPix.isNull())
    {
        QPoint centralStackPos = _containerStackedWidget->mapToParent(QPoint(0, 0));
        QRect centralStackRect = QRect(centralStackPos.x(), centralStackPos.y(), _containerStackedWidget->width(), _containerStackedWidget->height());
        QPainterPath clipPath;
        clipPath.addRoundedRect(centralStackRect, 10, 10);
        painter.setClipPath(clipPath);
        switch (_stackSwitchMode)
        {
        case ElaWindowType::None:
        {
            break;
        }
        case ElaWindowType::Popup:
        {
            painter.drawPixmap(QRect(0, _pPopupAnimationYOffset, width(), _containerStackedWidget->height()), _targetStackPix);
            break;
        }
        case ElaWindowType::Scale:
        {
            painter.setOpacity(_pScaleAnimationPixOpacity);
            painter.translate(_containerStackedWidget->rect().center());
            painter.scale(_pScaleAnimationRatio, _pScaleAnimationRatio);
            painter.translate(-_containerStackedWidget->rect().center());
            painter.drawPixmap(centralStackRect, _isDrawNewPix ? _targetStackPix : _currentStackPix);
            break;
        }
        case ElaWindowType::Flip:
        {
            QTransform transform;
            transform.translate(centralStackRect.center().x(), 0);
            if (abs(_pFlipAnimationRatio) >= 90)
            {
                transform.rotate(-180 + _pFlipAnimationRatio, Qt::YAxis);
            }
            else
            {
                transform.rotate(_pFlipAnimationRatio, Qt::YAxis);
            }
            transform.translate(-centralStackRect.center().x(), 0);
            painter.setTransform(transform);
            if (abs(_pFlipAnimationRatio) >= 90)
            {
                painter.drawPixmap(centralStackRect, _targetStackPix);
            }
            else
            {
                painter.drawPixmap(centralStackRect, _currentStackPix);
            }
            break;
        }
        case ElaWindowType::Blur:
        {
            break;
        }
        }
    }
    painter.restore();
}

void ElaCentralStackedWidget::_getCurrentStackPix()
{
    _targetStackPix = QPixmap();
    bool isTransparent = _isTransparent;
    _isTransparent = true;
    _containerStackedWidget->currentWidget()->setVisible(true);
    _currentStackPix = _containerStackedWidget->grab();
    _containerStackedWidget->currentWidget()->setVisible(false);
    _isTransparent = isTransparent;
}

void ElaCentralStackedWidget::_getTargetStackPix()
{
    _targetStackPix = QPixmap();
    bool isTransparent = _isTransparent;
    _isTransparent = true;
    _targetStackPix = _containerStackedWidget->grab();
    _isTransparent = isTransparent;
}
