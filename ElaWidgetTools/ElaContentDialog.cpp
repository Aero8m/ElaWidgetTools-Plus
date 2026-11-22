#include "ElaContentDialog.h"

#include "ElaColorAnimation.h"
#include "ElaMaskWidget.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaTheme.h"
#include "private/ElaContentDialogPrivate.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QPointer>
#include <QVBoxLayout>

Q_TAKEOVER_NATIVEEVENT_CPP(ElaContentDialog, d_func()->_appBar);
ElaContentDialog::ElaContentDialog(QWidget* parent)
    : QDialog{parent}, d_ptr(new ElaContentDialogPrivate())
{
    Q_D(ElaContentDialog);
    d->q_ptr = this;

    if (parent)
    {
        d->_owner = parent->window();
        d->_maskWidget = new ElaMaskWidget(d->_owner);
        d->_maskWidget->hide();
        d->_owner->installEventFilter(this);
    }

    setWindowModality(Qt::ApplicationModal);

    d->_appBar = new ElaAppBar(this);
    d->_appBar->setWindowButtonFlags(ElaAppBarType::NoneButtonHint);
    d->_appBar->setIsFixedSize(true);
    d->_appBar->setAppBarHeight(0);
#ifdef Q_OS_WIN
    // 防止意外拉伸
    createWinId();
#endif
    d->_leftButton = new ElaPushButton(d->_leftButtonText, this);
    connect(d->_leftButton, &ElaPushButton::clicked, this, [=]() {
        const QPointer<ElaContentDialog> guard(this);
        onLeftButtonClicked();
        if (!guard)
        {
            return;
        }
        reject();
        if (guard)
        {
            Q_EMIT leftButtonClicked();
        }
    });
    d->_leftButton->setBorderRadius(6);
    d->_middleButton = new ElaPushButton(d->_middleButtonText, this);
    connect(d->_middleButton, &ElaPushButton::clicked, this, [=]() {
        const QPointer<ElaContentDialog> guard(this);
        onMiddleButtonClicked();
        if (guard)
        {
            Q_EMIT middleButtonClicked();
        }
    });
    d->_middleButton->setBorderRadius(6);
    d->_rightButton = new ElaPushButton(d->_rightButtonText, this);
    connect(d->_rightButton, &ElaPushButton::clicked, this, [=]() {
        const QPointer<ElaContentDialog> guard(this);
        onRightButtonClicked();
        if (!guard)
        {
            return;
        }
        accept();
        if (guard)
        {
            Q_EMIT rightButtonClicked();
        }
    });
    d->_rightButton->setIsAccent(true);
    d->_rightButton->setBorderRadius(6);

    d->_centralWidget = new QWidget(this);
    QVBoxLayout* centralVLayout = new QVBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(24, 24, 24, 16);
    ElaText* title = new ElaText("退出", this);
    title->setTextStyle(ElaTextType::Title);
    ElaText* subTitle = new ElaText("确定要退出程序吗", this);
    subTitle->setTextStyle(ElaTextType::Body);
    centralVLayout->addWidget(title);
    centralVLayout->addSpacing(2);
    centralVLayout->addWidget(subTitle);
    centralVLayout->addStretch();

    d->_mainLayout = new QVBoxLayout(this);
    d->_mainLayout->setContentsMargins(0, 0, 0, 0);
    d->_mainLayout->setSpacing(0);
    d->_buttonWidget = new QWidget(this);
    d->_buttonWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QHBoxLayout* buttonLayout = new QHBoxLayout(d->_buttonWidget);
    buttonLayout->setContentsMargins(24, 12, 24, 12);
    buttonLayout->setSpacing(8);
    buttonLayout->addWidget(d->_leftButton);
    buttonLayout->addWidget(d->_middleButton);
    buttonLayout->addWidget(d->_rightButton);
    d->_mainLayout->addWidget(d->_centralWidget);
    d->_mainLayout->addWidget(d->_buttonWidget);

    d->_themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaContentDialog::~ElaContentDialog()
{
    Q_D(ElaContentDialog);
    if (d->_owner)
    {
        d->_owner->removeEventFilter(this);
    }
    disconnect(d->_workAreaChanged);
    delete d->_maskWidget.data();
}

void ElaContentDialog::onLeftButtonClicked()
{
}

void ElaContentDialog::onMiddleButtonClicked()
{
}

void ElaContentDialog::onRightButtonClicked()
{
}

void ElaContentDialog::setCentralWidget(QWidget* centralWidget)
{
    Q_D(ElaContentDialog);
    if (!centralWidget || centralWidget == d->_centralWidget)
    {
        return;
    }
    d->_mainLayout->removeWidget(d->_centralWidget);
    delete d->_centralWidget;
    d->_centralWidget = centralWidget;
    d->_mainLayout->insertWidget(0, centralWidget);
    d->_scheduleLayoutUpdate();
}

void ElaContentDialog::setLeftButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    d->_leftButton->setText(text);
}

void ElaContentDialog::setMiddleButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    d->_middleButton->setText(text);
}

void ElaContentDialog::setRightButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    d->_rightButton->setText(text);
}

void ElaContentDialog::close()
{
    reject();
}

bool ElaContentDialog::event(QEvent* event)
{
    const bool handled = QDialog::event(event);
    if (event->type() == QEvent::LayoutRequest || event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange || event->type() == QEvent::ScreenChangeInternal ||
        event->type() == QEvent::Move || event->type() == QEvent::Resize)
    {
        Q_D(ElaContentDialog);
        if (!d->_applyingGeometry)
        {
            d->_scheduleLayoutUpdate();
        }
    }
    return handled;
}

bool ElaContentDialog::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(ElaContentDialog);
    if (watched == d->_owner && d->_maskWidget)
    {
        switch (event->type())
        {
        case QEvent::Resize:
            d->_maskWidget->setGeometry(d->_owner->rect());
            [[fallthrough]];
        case QEvent::Move:
        case QEvent::WindowStateChange:
        case QEvent::ScreenChangeInternal:
            d->_scheduleLayoutUpdate();
            break;
        default:
            break;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void ElaContentDialog::showEvent(QShowEvent* event)
{
    Q_D(ElaContentDialog);
    QDialog::showEvent(event);
    d->_fitAndCenter();
    if (d->_maskWidget)
    {
        d->_maskWidget->setGeometry(d->_owner->rect());
        d->_maskWidget->show();
        d->_maskWidget->raise();
        d->_maskWidget->doMaskAnimation(90);
    }
}

void ElaContentDialog::hideEvent(QHideEvent* event)
{
    Q_D(ElaContentDialog);
    if (d->_maskWidget)
    {
        d->_maskWidget->doMaskAnimation(0);
    }
    QDialog::hideEvent(event);
}

void ElaContentDialog::paintEvent(QPaintEvent* event)
{
    Q_D(ElaContentDialog);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ElaColorAnimation::getColor(this, ElaThemeColor(d->_themeMode, PopupBase), "Background"));
    // 背景绘制
    painter.drawRect(rect());
    // 按钮栏背景绘制
    painter.setBrush(ElaColorAnimation::getColor(this, ElaThemeColor(d->_themeMode, WindowBase), "LayoutArea"));
    painter.drawRoundedRect(d->_buttonWidget->geometry(), 8, 8);
    painter.restore();
}

void ElaContentDialog::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
    {
        reject();
        break;
    }
    default:
    {
        break;
    }
    }
    event->accept();
}
