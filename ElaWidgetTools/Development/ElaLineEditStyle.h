#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELALINEEDITSTYLE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELALINEEDITSTYLE_H_

#include <QProxyStyle>

#include "ElaWidgetToolsDef.h"
class ElaLineEditStyle : public QProxyStyle
{
    Q_OBJECT
public:
    static constexpr int TextMargin = 12;

    explicit ElaLineEditStyle(QStyle* style = nullptr);
    ~ElaLineEditStyle();
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;

private:
    ElaThemeType::ThemeMode _themeMode;
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELALINEEDITSTYLE_H_

