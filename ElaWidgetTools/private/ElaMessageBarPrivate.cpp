#include "ElaMessageBarPrivate.h"

#include "ElaApplication.h"
#include "ElaColorAnimation.h"
#include "ElaControlStyle.h"
#include "ElaIconButton.h"
#include "ElaMessageBar.h"
#include "ElaText.h"
#include "ElaTheme.h"
#include "ElaToolTip.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QTextLayout>

namespace
{
constexpr int borderRadius = 6;
constexpr int shadowWidth = 6;
constexpr int closeWidth = 30;
constexpr int closeInset = 10;
constexpr int textLeft = 48;
constexpr int textRight = closeInset + closeWidth + 8;
constexpr int textGap = 12;
constexpr int stackedTextGap = 4;
constexpr int textTop = 10;
constexpr int textBottom = 12;
constexpr int progressHeight = 2;
constexpr int horizontalMargin = 20;
constexpr int topMargin = 25;
constexpr int bottomMargin = 20;
constexpr int spacing = 15;
constexpr int maxBarWidth = 500;
constexpr int titleMaxWidth = 100;
constexpr int textFlags = Qt::AlignLeading | Qt::AlignVCenter | Qt::TextWordWrap | Qt::TextWrapAnywhere;

int naturalWidth(const QString& text, const QFontMetrics& metrics)
{
    int width = 0;
    for (const auto& line : text.split(QLatin1Char('\n')))
    {
        width = qMax(width, metrics.horizontalAdvance(line));
    }
    return width;
}

int textHeight(const QString& text, const QFontMetrics& metrics, int width)
{
    return text.isEmpty() ? 0 : metrics.boundingRect(QRect(0, 0, width, 0), textFlags, text).height();
}

QString fitText(const QString& text, const QFont& font, const QRect& rect)
{
    const QFontMetrics metrics(font);
    if (textHeight(text, metrics, rect.width()) <= rect.height())
    {
        return text;
    }
    QString paragraphs = text;
    paragraphs.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    paragraphs.replace(QLatin1Char('\n'), QChar(QChar::LineSeparator));
    QTextLayout layout(paragraphs, font);
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(option);
    layout.beginLayout();
    int lastLineStart = -1;
    qreal height = 0;
    for (auto line = layout.createLine(); line.isValid(); line = layout.createLine())
    {
        line.setLineWidth(rect.width());
        if (height + line.height() > rect.height())
        {
            break;
        }
        lastLineStart = line.textStart();
        height += line.height();
    }
    layout.endLayout();
    if (lastLineStart < 0)
    {
        return {};
    }
    QString tail = paragraphs.mid(lastLineStart);
    tail.replace(QChar(QChar::LineSeparator), QLatin1Char(' '));
    const QString ellipsis(QChar(0x2026));
    const QString elided = metrics.elidedText(tail.trimmed() + ellipsis, Qt::ElideRight, rect.width());
    return paragraphs.left(lastLineStart) + elided;
}
}

QMap<QWidget*, QMap<ElaMessageBarType::PositionPolicy, QList<ElaMessageBar*>>> ElaMessageBarPrivate::_stacks;

ElaMessageBarPrivate::ElaMessageBarPrivate(QObject* parent)
    : QObject{parent}
{
}

ElaMessageBarPrivate::~ElaMessageBarPrivate()
{
    _removeFromStack();
}

void ElaMessageBarPrivate::_messageBarCreate(int displayMsec)
{
    Q_Q(ElaMessageBar);
    _ownerKey = q->parentWidget();
    _owner = _ownerKey;
    _displayMsec = qMax(0, displayMsec);
    q->setAccessibleName(_title);
    q->setAccessibleDescription(_text);
    _closeButton = new ElaIconButton(ElaIconType::Xmark, eApp->getFontPixelSize() + 3, q);
    _closeButton->setFixedWidth(closeWidth);
    _closeButton->setBorderRadius(5);
    connect(_closeButton, &ElaIconButton::clicked, this, &ElaMessageBarPrivate::messageBarEnd);
    auto* layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, closeInset, 0);
    layout->addStretch();
    layout->addWidget(_closeButton, 0, Qt::AlignVCenter);

    _displayTimer.setSingleShot(true);
    connect(&_displayTimer, &QTimer::timeout, this, &ElaMessageBarPrivate::messageBarEnd);
    connect(&_entry, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        _entryOffset = value.toInt();
        _setSlot(_slot);
    });
    connect(&_entry, &QVariantAnimation::finished, this, [this]() {
        _displayTimer.start(_displayMsec);
        _progress.start();
        _reflowStack();
    });
    connect(&_progress, &QVariantAnimation::valueChanged, this, [this, q](const QVariant& value) {
        _timeRemaining = value.toReal();
        q->update();
    });
    connect(&_fade, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        _setOpacity(value.toReal());
    });
    connect(&_fade, &QVariantAnimation::finished, this, [this, q]() {
        q->hide();
        _removeFromStack();
        _reflowStack();
        q->deleteLater();
    });
    connect(eApp, &ElaApplication::pFontPixelSizeChanged, this, &ElaMessageBarPrivate::_ownerResized);

    _entry.setEasingCurve(QEasingCurve::InOutSine);
    _fade.setDuration(300);
    _fade.setEasingCurve(QEasingCurve::InOutSine);
    _fade.setStartValue(1.0);
    _fade.setEndValue(0.0);
    _progress.setDuration(_displayMsec);
    _progress.setStartValue(1.0);
    _progress.setEndValue(0.0);
    _stacks[_ownerKey][_policy].append(q);
    _reflowStack();
}

void ElaMessageBarPrivate::_removeFromStack()
{
    auto owner = _stacks.find(_ownerKey);
    if (owner == _stacks.end())
    {
        return;
    }
    auto stack = owner->find(_policy);
    if (stack == owner->end())
    {
        return;
    }
    stack->removeOne(q_ptr);
    if (stack->isEmpty())
    {
        owner->erase(stack);
    }
    if (owner->isEmpty())
    {
        _stacks.erase(owner);
    }
}

void ElaMessageBarPrivate::_ownerResized()
{
    const auto stack = _stacks.value(_ownerKey).value(_policy);
    if (!stack.isEmpty() && stack.front() == q_ptr)
    {
        _reflowStack();
    }
}

void ElaMessageBarPrivate::_reflowStack()
{
    using namespace ElaMessageBarType;
    if (!_owner)
    {
        return;
    }
    const auto bars = _stacks.value(_ownerKey).value(_policy);
    const bool bottom = _policy == Bottom || _policy == BottomLeft || _policy == BottomRight;
    const bool left = _policy == Left || _policy == TopLeft || _policy == BottomLeft;
    const bool right = _policy == Right || _policy == TopRight || _policy == BottomRight;
    const bool middle = _policy == Left || _policy == Right;
    const int width = _owner->width();
    const int height = _owner->height();
    const int sideMargin = qMin(horizontalMargin, width / 10);
    const int firstY = middle ? height / 2 : qMin(topMargin, height / 10);
    const int endY = height - qMin(bottomMargin, height / 10);
    const int capacity = endY - firstY;
    const int maxWidth = qMin(maxBarWidth, width - 2 * sideMargin);
    if (capacity < textTop + textBottom + 18 || maxWidth <= textLeft + textRight + textGap + 2)
    {
        return;
    }

    int used = 0;
    bool waitingForSpace = false;
    bool fading = false;
    ElaMessageBarPrivate* oldest = nullptr;
    for (auto* bar : bars)
    {
        auto* item = bar->d_func();
        item->_updateSize(maxWidth, capacity);
        const int offset = used == 0 ? 0 : used + spacing;
        if (!item->_started && offset + bar->height() > capacity)
        {
            waitingForSpace = true;
            break;
        }
        const int x = left ? sideMargin : right ? width - sideMargin - bar->width() : (width - bar->width()) / 2;
        const int y = bottom ? endY - offset - bar->height() : firstY + offset;
        item->_setSlot(QPoint(x, y));
        if (!item->_started)
        {
            const int entrance = left ? -bar->width() - x : right ? width - x : 0;
            item->_startDisplay(entrance);
        }
        used = offset + bar->height();
        fading |= item->_closing;
        if (!oldest)
        {
            oldest = item;
        }
    }
    if ((waitingForSpace || used > capacity) && !fading && oldest &&
        oldest->_entry.state() != QAbstractAnimation::Running)
    {
        oldest->messageBarEnd();
    }
}

void ElaMessageBarPrivate::_startDisplay(int entryOffset)
{
    Q_Q(ElaMessageBar);
    _started = true;
    _entryOffset = entryOffset;
    _setSlot(_slot);
    _entry.setDuration(entryOffset == 0 ? 250 : 350);
    _entry.setStartValue(entryOffset);
    _entry.setEndValue(0);
    q->show();
    _entry.start();
}

void ElaMessageBarPrivate::_setSlot(const QPoint& slot)
{
    Q_Q(ElaMessageBar);
    _slot = slot;
    q->move(slot + QPoint(_entryOffset, 0));
}

void ElaMessageBarPrivate::messageBarEnd()
{
    if (!_started || _closing)
    {
        return;
    }
    _closing = true;
    _closeButton->setEnabled(false);
    _displayTimer.stop();
    _entry.stop();
    _progress.stop();
    // Keep occupying this slot until the fading widget has disappeared.
    _fade.start();
}

void ElaMessageBarPrivate::_setOpacity(qreal opacity)
{
    Q_Q(ElaMessageBar);
    _opacity = opacity;
    _closeButton->setOpacity(opacity);
    q->update();
}

void ElaMessageBarPrivate::_updateSize(int maxWidth, int maxHeight)
{
    Q_Q(ElaMessageBar);
    const int pixelSize = eApp->getFontPixelSize();
    _titleFont = q->font();
    _titleFont.setPixelSize(pixelSize);
    _titleFont.setWeight(QFont::Bold);
    _bodyFont = _titleFont;
    _bodyFont.setWeight(QFont::Light);
    const QFontMetrics titleMetrics(_titleFont);
    const QFontMetrics bodyMetrics(_bodyFont);
    const bool hasTitle = !_title.isEmpty();
    const bool hasBody = !_text.isEmpty();
    const int naturalTitle = hasTitle ? qMax(1, naturalWidth(_title, titleMetrics) + 1) : 0;
    const int naturalBody = hasBody ? qMax(1, naturalWidth(_text, bodyMetrics) + 1) : 0;
    const bool stacked = hasTitle && hasBody && naturalTitle > titleMaxWidth;
    const int gap = hasTitle && hasBody && !stacked ? textGap : 0;
    const int preferredText = stacked ? qMax(naturalTitle, naturalBody) : naturalTitle + gap + naturalBody;
    const int width = qMin(maxWidth, textLeft + textRight + qMax(1, preferredText));
    const int textWidth = width - textLeft - textRight;
    const int columnWidth = textWidth - gap;
    const int titleLimit = qMax(columnWidth - naturalBody, columnWidth / 2);
    const int titleWidth = gap ? qBound(1, naturalTitle, qBound(1, titleLimit, columnWidth - 1))
                               : hasTitle ? textWidth : 0;
    const int bodyWidth = stacked ? textWidth : textWidth - titleWidth - gap;
    const int titleHeight = textHeight(_title, titleMetrics, titleWidth);
    const int bodyHeight = textHeight(_text, bodyMetrics, bodyWidth);
    const int naturalHeight = stacked ? titleHeight + stackedTextGap + bodyHeight : qMax(titleHeight, bodyHeight);
    const int height = qMin(maxHeight, qMax(_closeButton->sizeHint().height(), naturalHeight) + textTop + textBottom);
    const int contentHeight = height - textTop - textBottom;
    _titleRect = QRect(textLeft, textTop, titleWidth, contentHeight);
    _bodyRect = QRect(textLeft + titleWidth + gap, textTop, bodyWidth, contentHeight);
    if (stacked)
    {
        const int bodyLine = qMin(bodyHeight, bodyMetrics.height());
        const int titleSpace = qMax(0, contentHeight - stackedTextGap - bodyLine);
        _titleRect.setHeight(qMin(titleHeight, titleSpace));
        _bodyRect = QRect(textLeft, textTop + _titleRect.height() + stackedTextGap, bodyWidth,
            qMax(0, contentHeight - _titleRect.height() - stackedTextGap));
    }
    _visibleTitle = fitText(_title, _titleFont, _titleRect);
    _visibleBody = fitText(_text, _bodyFont, _bodyRect);
    const bool overflow = titleHeight > _titleRect.height() || bodyHeight > _bodyRect.height();
    if (overflow && !_overflowTip)
    {
        _overflowTip = new ElaToolTip(q);
        auto* details = new ElaText(hasTitle && hasBody ? _title + QLatin1Char('\n') + _text : _title + _text);
        details->setTextFormat(Qt::PlainText);
        details->setIsWrapAnywhere(true);
        details->setMaximumWidth(maxBarWidth);
        _overflowTip->setCustomWidget(details);
    }
    else if (!overflow && _overflowTip)
    {
        delete _overflowTip;
        _overflowTip = nullptr;
    }
    q->setFixedSize(width, height);
    q->update();
}

void ElaMessageBarPrivate::_drawBackground(QPainter* painter)
{
    Q_Q(ElaMessageBar);
    const auto foreground = q->rect().adjusted(shadowWidth, shadowWidth, -shadowWidth, -shadowWidth);
    eTheme->drawEffectShadow(painter, q->rect(), shadowWidth, borderRadius);
    painter->save();
    painter->setPen(ElaThemeColor(_themeMode, BasicBorder));
    painter->setBrush(ElaColorAnimation::getColor(q, ElaControlStyle::getStatusColor(_themeMode, _messageMode, true)));
    painter->drawRoundedRect(foreground, borderRadius, borderRadius);
    ElaControlStyle::drawStatusIcon(painter, QRectF(17, foreground.center().y() - 9, 18, 18), _themeMode, _messageMode);
    QPainterPath clip;
    clip.addRoundedRect(foreground, borderRadius, borderRadius);
    painter->setClipPath(clip);
    painter->setPen(Qt::NoPen);
    painter->setBrush(ElaColorAnimation::getColor(q, ElaControlStyle::getStatusColor(_themeMode, _messageMode), "Progress"));
    painter->drawRect(QRectF(foreground.x(), foreground.bottom() - progressHeight + 1,
        foreground.width() * _timeRemaining, progressHeight));
    painter->restore();
}

void ElaMessageBarPrivate::_drawText(QPainter* painter) const
{
    painter->setPen(ElaThemeColor(_themeMode, BasicText));
    if (!_visibleTitle.isEmpty())
    {
        painter->setFont(_titleFont);
        painter->drawText(_titleRect, textFlags, _visibleTitle);
    }
    if (!_visibleBody.isEmpty())
    {
        painter->setFont(_bodyFont);
        painter->drawText(_bodyRect, textFlags, _visibleBody);
    }
}
