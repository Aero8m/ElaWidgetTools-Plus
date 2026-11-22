#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELAMESSAGEBARPRIVATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELAMESSAGEBARPRIVATE_H_

#include <QFont>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QTimer>
#include <QVariantAnimation>

#include "ElaPropertyMacro.h"
#include "ElaWidgetToolsDef.h"

class ElaIconButton;
class ElaToolTip;
class QPainter;
class QWidget;
class ElaMessageBar;
class ElaMessageBarPrivate : public QObject
{
    Q_OBJECT
    ELA_PIMPL_D(ElaMessageBar)
public:
    explicit ElaMessageBarPrivate(QObject* parent = nullptr);
    ~ElaMessageBarPrivate() override;
    Q_SLOT void messageBarEnd();

private:
    static QMap<QWidget*, QMap<ElaMessageBarType::PositionPolicy, QList<ElaMessageBar*>>> _stacks;
    ElaThemeType::ThemeMode _themeMode;
    QString _title;
    QString _text;
    ElaMessageBarType::PositionPolicy _policy;
    ElaMessageBarType::MessageMode _messageMode;
    QWidget* _ownerKey{nullptr};
    QPointer<QWidget> _owner;
    QFont _titleFont;
    QFont _bodyFont;
    QRect _titleRect;
    QRect _bodyRect;
    QString _visibleTitle;
    QString _visibleBody;
    ElaIconButton* _closeButton{nullptr};
    ElaToolTip* _overflowTip{nullptr};
    int _displayMsec{0};
    bool _started{false};
    bool _closing{false};
    qreal _opacity{1};
    qreal _timeRemaining{1};
    int _entryOffset{0};
    QPoint _slot;
    QTimer _displayTimer;
    QVariantAnimation _entry;
    QVariantAnimation _fade;
    QVariantAnimation _progress;

    void _messageBarCreate(int displayMsec);
    void _removeFromStack();
    void _ownerResized();
    void _reflowStack();
    void _updateSize(int maxWidth, int maxHeight);
    void _startDisplay(int entryOffset);
    void _setSlot(const QPoint& slot);
    void _setOpacity(qreal opacity);
    void _drawBackground(QPainter* painter);
    void _drawText(QPainter* painter) const;
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELAMESSAGEBARPRIVATE_H_
