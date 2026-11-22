#include "ElaMaskWidget.h"

#include <QPainter>
#include <QPropertyAnimation>
ElaMaskWidget::ElaMaskWidget(QWidget* parent)
    : QWidget{parent}, _animation(new QPropertyAnimation(this, "pMaskAlpha", this))
{
    setObjectName("ElaMaskWidget");
    setStyleSheet("#ElaMaskWidget{background-color:transparent;}");
    _pMaskAlpha = 0;
    setAttribute(Qt::WA_TransparentForMouseEvents);
    _animation->setEasingCurve(QEasingCurve::InOutSine);
    _animation->setDuration(250);
    connect(_animation, &QPropertyAnimation::valueChanged, this, [this]() { update(); });
    connect(_animation, &QPropertyAnimation::finished, this, [this]() {
        if (_pMaskAlpha == 0)
        {
            hide();
        }
    });
}

ElaMaskWidget::~ElaMaskWidget()
{
}

void ElaMaskWidget::doMaskAnimation(int endValue)
{
    _animation->stop();
    if (endValue == _pMaskAlpha)
    {
        if (endValue == 0)
        {
            hide();
        }
        return;
    }
    _animation->setStartValue(_pMaskAlpha);
    _animation->setEndValue(endValue);
    _animation->start();
}

void ElaMaskWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0x00, 0x00, 0x00, _pMaskAlpha));
    painter.drawRect(rect());
    painter.restore();
}
