#include "ElaContentDialogPrivate.h"

#include "ElaContentDialog.h"
#include <QGuiApplication>
#include <QScreen>
#include <QScopedValueRollback>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

ElaContentDialogPrivate::ElaContentDialogPrivate(QObject* parent)
    : QObject{parent}
{
}

ElaContentDialogPrivate::~ElaContentDialogPrivate()
{
}

void ElaContentDialogPrivate::_scheduleLayoutUpdate()
{
    Q_Q(ElaContentDialog);
    if (_layoutUpdatePending || !q->isVisible())
    {
        return;
    }
    _layoutUpdatePending = true;
    QTimer::singleShot(0, q, [this, q]() {
        _layoutUpdatePending = false;
        if (q->isVisible())
        {
            _fitAndCenter();
        }
    });
}

void ElaContentDialogPrivate::_fitAndCenter()
{
    Q_Q(ElaContentDialog);
    const QRect ownerRect = _owner ? QRect(_owner->mapToGlobal(QPoint()), _owner->size()) : QRect();
    QScreen* screen = ownerRect.isValid() ? QGuiApplication::screenAt(ownerRect.center()) : nullptr;
    QWidget* referenceWindow = _owner ? _owner.data() : q;
    if (!screen && referenceWindow->windowHandle())
    {
        screen = referenceWindow->windowHandle()->screen();
    }
    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }
    if (_screen != screen)
    {
        QObject::disconnect(_workAreaChanged);
        _screen = screen;
        if (screen)
        {
            _workAreaChanged = QObject::connect(screen, &QScreen::availableGeometryChanged, q, [this]() {
                _scheduleLayoutUpdate();
            });
        }
    }

    QRect workArea = screen ? screen->availableGeometry() : QRect();
    if (auto* handle = q->windowHandle(); screen && handle)
    {
        // The native window must use the target screen's scale before applying its geometry.
        auto* currentScreen = handle->screen();
        if (currentScreen && currentScreen != screen && currentScreen->virtualSiblings().contains(screen))
        {
            handle->setScreen(screen);
        }
        const QRect clientArea = workArea.marginsRemoved(handle->frameMargins());
        if (clientArea.isValid())
        {
            workArea = clientArea;
        }
    }
    _mainLayout->activate();
    const QSize minimum = q->minimumSize();
    const QSize available = workArea.isValid() ? workArea.size().boundedTo(q->maximumSize()) : q->maximumSize();
    const QSize limit = available.expandedTo(minimum);
    const QSize hint = _mainLayout->totalSizeHint();
    const int width = qBound(minimum.width(), qMax(400, hint.width()), limit.width());
    const int wrappedHeight = _mainLayout->hasHeightForWidth() ? _mainLayout->totalHeightForWidth(width) : -1;
    const int height = qBound(minimum.height(), wrappedHeight >= 0 ? wrappedHeight : hint.height(), limit.height());
    QRect target(q->pos(), QSize(width, height));
    if (workArea.isValid())
    {
        const QRect visibleOwner = ownerRect.intersected(workArea);
        target.moveCenter(visibleOwner.isValid() ? visibleOwner.center() : workArea.center());
        target.moveLeft(qBound(workArea.left(), target.left(), qMax(workArea.left(), workArea.right() - width + 1)));
        target.moveTop(qBound(workArea.top(), target.top(), qMax(workArea.top(), workArea.bottom() - height + 1)));
    }
    if (target != q->geometry())
    {
        const QScopedValueRollback<bool> applying(_applyingGeometry, true);
        q->setGeometry(target);
    }
}
