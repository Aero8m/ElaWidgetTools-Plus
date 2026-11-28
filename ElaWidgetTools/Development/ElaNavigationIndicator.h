#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPMENT_ELANAVIGATIONINDICATOR_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPMENT_ELANAVIGATIONINDICATOR_H_

#include "ElaWidgetToolsDef.h"

#include <QPointF>
#include <QRectF>
#include <QWidget>

class QEasingCurve;
class QParallelAnimationGroup;

class ElaNavigationIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF indicatorPos READ indicatorPos WRITE setIndicatorPos)
    Q_PROPERTY(qreal indicatorLength READ indicatorLength WRITE setIndicatorLength)
public:
    explicit ElaNavigationIndicator(QWidget* parent = nullptr);
    ~ElaNavigationIndicator() override;

    bool isActive() const;
    void startAnimation(const QRectF& startRect, const QRectF& endRect);
    void stopAnimation();

    QPointF indicatorPos() const;
    void setIndicatorPos(const QPointF& pos);

    qreal indicatorLength() const;
    void setIndicatorLength(qreal length);

Q_SIGNALS:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static QEasingCurve _createBezierCurve(qreal x1, qreal y1, qreal x2, qreal y2);
    void _updateDirtyRect(const QRectF& previousRect);

    ElaThemeType::ThemeMode _themeMode;
    QParallelAnimationGroup* _animationGroup{nullptr};
    QRectF _currentRect;
    bool _isActive{false};
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPMENT_ELANAVIGATIONINDICATOR_H_
