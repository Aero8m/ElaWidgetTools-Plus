#include "ElaMessageBar.h"

#include "ElaTheme.h"
#include "private/ElaMessageBarPrivate.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>

ElaMessageBar::ElaMessageBar(ElaMessageBarType::PositionPolicy policy, ElaMessageBarType::MessageMode messageMode, QString& title, QString& text, int displayMsec, QWidget* parent)
    : QWidget{parent}, d_ptr(new ElaMessageBarPrivate())
{
    Q_D(ElaMessageBar);
    d->q_ptr = this;
    d->_title = title;
    d->_text = text;
    d->_policy = policy;
    d->_messageMode = messageMode;
    d->_themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, this, [this, d](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
        update();
    });
    setMouseTracking(true);
    parent->installEventFilter(this);
    setFont(QFont("微软雅黑"));
    setObjectName("ElaMessageBar");
    setStyleSheet("#ElaMessageBar{background-color:transparent;}");
    d->_messageBarCreate(displayMsec);
}

ElaMessageBar::~ElaMessageBar()
{
}

void ElaMessageBar::success(ElaMessageBarType::PositionPolicy policy, QString title, QString text, int displayMsec, QWidget* parent)
{
    if (!parent)
    {
        QList<QWidget*> widgetList = QApplication::topLevelWidgets();
        for (auto widget: widgetList)
        {
            if (widget->property("ElaBaseClassName").toString() == "ElaWindow")
            {
                parent = widget;
            }
        }
        if (!parent)
        {
            return;
        }
    }

    ElaMessageBar* bar = new ElaMessageBar(policy, ElaMessageBarType::Success, title, text, displayMsec, parent);
    Q_UNUSED(bar);
}

void ElaMessageBar::warning(ElaMessageBarType::PositionPolicy policy, QString title, QString text, int displayMsec, QWidget* parent)
{
    if (!parent)
    {
        QList<QWidget*> widgetList = QApplication::topLevelWidgets();
        for (auto widget: widgetList)
        {
            if (widget->property("ElaBaseClassName").toString() == "ElaWindow")
            {
                parent = widget;
            }
        }
        if (!parent)
        {
            return;
        }
    }
    ElaMessageBar* bar = new ElaMessageBar(policy, ElaMessageBarType::Warning, title, text, displayMsec, parent);
    Q_UNUSED(bar);
}

void ElaMessageBar::information(ElaMessageBarType::PositionPolicy policy, QString title, QString text, int displayMsec, QWidget* parent)
{
    if (!parent)
    {
        QList<QWidget*> widgetList = QApplication::topLevelWidgets();
        for (auto widget: widgetList)
        {
            if (widget->property("ElaBaseClassName").toString() == "ElaWindow")
            {
                parent = widget;
            }
        }
        if (!parent)
        {
            return;
        }
    }
    ElaMessageBar* bar = new ElaMessageBar(policy, ElaMessageBarType::Information, title, text, displayMsec, parent);
    Q_UNUSED(bar);
}

void ElaMessageBar::error(ElaMessageBarType::PositionPolicy policy, QString title, QString text, int displayMsec, QWidget* parent)
{
    if (!parent)
    {
        QList<QWidget*> widgetList = QApplication::topLevelWidgets();
        for (auto widget: widgetList)
        {
            if (widget->property("ElaBaseClassName").toString() == "ElaWindow")
            {
                parent = widget;
            }
        }
        if (!parent)
        {
            return;
        }
    }
    ElaMessageBar* bar = new ElaMessageBar(policy, ElaMessageBarType::Error, title, text, displayMsec, parent);
    Q_UNUSED(bar);
}
void ElaMessageBar::paintEvent(QPaintEvent* event)
{
    Q_D(ElaMessageBar);
    QPainter painter(this);
    painter.setOpacity(d->_opacity);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    d->_drawBackground(&painter);
    d->_drawText(&painter);
}

bool ElaMessageBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::ScreenChangeInternal))
    {
        Q_D(ElaMessageBar);
        d->_ownerResized();
    }
    return QWidget::eventFilter(watched, event);
}
