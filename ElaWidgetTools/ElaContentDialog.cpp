#include "ElaContentDialog.h"

#include <ElaPushButton.h>

#include "ElaMaskWidget.h"
#include "ElaText.h"
#include "ElaTheme.h"
#include "ElaWinShadowHelper.h"
#include "private/ElaContentDialogPrivate.h"
#include <QApplication>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
constexpr int DialogShadowMargin = 12;
constexpr int DialogCornerRadius = 8;
constexpr int DialogFooterHeight = 80;

void setDialogButtonText(ElaPushButton* button, const QString& text, int minimumWidth)
{
    button->setText(text);
    // ElaPushButton reserves three pixels on each side for its shadow.
    button->setMinimumWidth(qMax(minimumWidth, button->fontMetrics().horizontalAdvance(text) + 30));
}
}

Q_TAKEOVER_NATIVEEVENT_CPP(ElaContentDialog, d_func()->_appBar);
ElaContentDialog::ElaContentDialog(QWidget* parent)
    : QDialog{parent}, d_ptr(new ElaContentDialogPrivate())
{
    Q_D(ElaContentDialog);
    d->q_ptr = this;

    d->_maskWidget = new ElaMaskWidget(parent);
    d->_maskWidget->move(0, 0);
    d->_maskWidget->setFixedSize(parent->size());
    d->_maskWidget->setVisible(false);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);

    d->_appBar = new ElaAppBar(this);
    d->_appBar->setWindowButtonFlags(ElaAppBarType::NoneButtonHint);
    d->_appBar->setIsFixedSize(true);
    d->_appBar->setAppBarHeight(0);
#ifdef Q_OS_WIN
    // 防止意外拉伸
    createWinId();
#endif
    d->_leftButton = new ElaPushButton("cancel", this);
    connect(d->_leftButton, &ElaPushButton::clicked, this, [=]() {
        onLeftButtonClicked();
        d->_doCloseAnimation(false);
        QTimer::singleShot(0, nullptr, [=]() {
            Q_EMIT leftButtonClicked();
        });
    });
    d->_leftButton->setMinimumSize(84, 38);
    d->_leftButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    d->_leftButton->setFixedHeight(38);
    d->_leftButton->setBorderRadius(6);
    setDialogButtonText(d->_leftButton, d->_leftButton->text(), 84);
    d->_middleButton = new ElaPushButton("minimum", this);
    connect(d->_middleButton, &ElaPushButton::clicked, this, [=]() {
        onMiddleButtonClicked();
        QTimer::singleShot(0, nullptr, [=]() {
            Q_EMIT middleButtonClicked();
        });
    });
    d->_middleButton->setMinimumSize(94, 38);
    d->_middleButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    d->_middleButton->setFixedHeight(38);
    d->_middleButton->setBorderRadius(6);
    setDialogButtonText(d->_middleButton, d->_middleButton->text(), 94);
    d->_rightButton = new ElaPushButton("exit", this);
    connect(d->_rightButton, &ElaPushButton::clicked, this, [=]() {
        onRightButtonClicked();
        d->_doCloseAnimation(true);
        QTimer::singleShot(0, nullptr, [=]() {
            Q_EMIT rightButtonClicked();
        });
    });
    d->_rightButton->setLightDefaultColor(ElaThemeColor(ElaThemeType::Light, PrimaryNormal));
    d->_rightButton->setLightHoverColor(ElaThemeColor(ElaThemeType::Light, PrimaryHover));
    d->_rightButton->setLightPressColor(ElaThemeColor(ElaThemeType::Light, PrimaryPress));
    d->_rightButton->setLightTextColor(Qt::white);
    d->_rightButton->setDarkDefaultColor(ElaThemeColor(ElaThemeType::Dark, PrimaryNormal));
    d->_rightButton->setDarkHoverColor(ElaThemeColor(ElaThemeType::Dark, PrimaryHover));
    d->_rightButton->setDarkPressColor(ElaThemeColor(ElaThemeType::Dark, PrimaryPress));
    d->_rightButton->setDarkTextColor(Qt::black);
    d->_rightButton->setMinimumSize(90, 38);
    d->_rightButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    d->_rightButton->setFixedHeight(38);
    d->_rightButton->setBorderRadius(6);
    setDialogButtonText(d->_rightButton, d->_rightButton->text(), 90);

    d->_centralWidget = new QWidget(this);
    QVBoxLayout* centralVLayout = new QVBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(30, 28, 30, 30);
    ElaText* title = new ElaText("退出", this);
    title->setTextStyle(ElaTextType::Subtitle);
    title->setMaximumWidth(460);
    ElaText* subTitle = new ElaText("确定要退出程序吗", this);
    subTitle->setTextPixelSize(15);
    subTitle->setMaximumWidth(460);
    centralVLayout->addWidget(title);
    centralVLayout->addSpacing(8);
    centralVLayout->addWidget(subTitle);

    d->_mainLayout = new QVBoxLayout(this);
    d->_mainLayout->setContentsMargins(DialogShadowMargin, DialogShadowMargin, DialogShadowMargin, DialogShadowMargin);
    d->_mainLayout->setSpacing(0);
    d->_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    d->_buttonWidget = new QWidget(this);
    d->_buttonWidget->setFixedHeight(DialogFooterHeight);
    QHBoxLayout* buttonLayout = new QHBoxLayout(d->_buttonWidget);
    buttonLayout->setContentsMargins(24, 0, 24, 0);
    buttonLayout->setSpacing(4);
    buttonLayout->addStretch();
    buttonLayout->addWidget(d->_rightButton);
    buttonLayout->addWidget(d->_middleButton);
    buttonLayout->addWidget(d->_leftButton);
    d->_mainLayout->addWidget(d->_centralWidget);
    d->_mainLayout->addWidget(d->_buttonWidget);

    d->_themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
        update();
    });
}

ElaContentDialog::~ElaContentDialog()
{
    Q_D(ElaContentDialog);
    d->_maskWidget->deleteLater();
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
    d->_mainLayout->activate();
    adjustSize();
    if (isVisible())
    {
        d->_moveToCenter();
    }
}

void ElaContentDialog::setLeftButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    setDialogButtonText(d->_leftButton, text, 84);
    d->_mainLayout->activate();
    adjustSize();
    if (isVisible())
    {
        d->_moveToCenter();
    }
}

void ElaContentDialog::setMiddleButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    setDialogButtonText(d->_middleButton, text, 94);
    d->_mainLayout->activate();
    adjustSize();
    if (isVisible())
    {
        d->_moveToCenter();
    }
}

void ElaContentDialog::setRightButtonText(const QString& text)
{
    Q_D(ElaContentDialog);
    setDialogButtonText(d->_rightButton, text, 90);
    d->_mainLayout->activate();
    adjustSize();
    if (isVisible())
    {
        d->_moveToCenter();
    }
}

void ElaContentDialog::close()
{
    Q_D(ElaContentDialog);
    d->_doCloseAnimation(false);
}

void ElaContentDialog::showEvent(QShowEvent* event)
{
    Q_D(ElaContentDialog);
    d->_maskWidget->setVisible(true);
    d->_maskWidget->raise();
    d->_maskWidget->setFixedSize(parentWidget()->size());
    d->_maskWidget->doMaskAnimation(90);
    d->_mainLayout->activate();
    adjustSize();
    d->_moveToCenter();
    QDialog::showEvent(event);
}

void ElaContentDialog::paintEvent(QPaintEvent* event)
{
    Q_D(ElaContentDialog);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    const QRect cardRect = rect().adjusted(DialogShadowMargin, DialogShadowMargin, -DialogShadowMargin, -DialogShadowMargin);
    eTheme->drawEffectShadow(&painter, rect(), DialogShadowMargin, DialogCornerRadius, 1.5);
    QPainterPath cardPath;
    cardPath.addRoundedRect(QRectF(cardRect), DialogCornerRadius, DialogCornerRadius);
    painter.setPen(QPen(ElaThemeColor(d->_themeMode, PopupBorder), 1));
    painter.setBrush(ElaThemeColor(d->_themeMode, DialogBase));
    painter.drawPath(cardPath);
    painter.setClipPath(cardPath);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ElaThemeColor(d->_themeMode, DialogLayoutArea));
    const int footerTop = cardRect.bottom() - DialogFooterHeight + 1;
    painter.drawRect(QRect(cardRect.left(), footerTop, cardRect.width(), DialogFooterHeight));
    painter.setPen(QPen(ElaThemeColor(d->_themeMode, BasicBorder), 1));
    painter.drawLine(cardRect.left(), footerTop, cardRect.right(), footerTop);
    painter.restore();
}

void ElaContentDialog::keyPressEvent(QKeyEvent* event)
{
    Q_D(ElaContentDialog);
    switch (event->key())
    {
    case Qt::Key_Escape:
    {
        d->_doCloseAnimation(false);
        break;
    }
    default:
    {
        break;
    }
    }
    event->accept();
}
