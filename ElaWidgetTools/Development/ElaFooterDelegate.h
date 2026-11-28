#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELAFOOTERDELEGATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELAFOOTERDELEGATE_H_

#include "ElaPropertyMacro.h"
#include <QModelIndex>
#include <QPointer>
#include <QStyledItemDelegate>

#include "ElaWidgetToolsDef.h"
class ElaNavigationNode;
class ElaNavigationIndicator;
class ElaBaseListView;
class ElaFooterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    ELA_VARIABLE(QModelIndex, PressIndex);

public:
    explicit ElaFooterDelegate(ElaBaseListView* listView, QObject* parent = nullptr);
    ~ElaFooterDelegate();
    void setNavigationIndicator(ElaNavigationIndicator* indicator);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    int _leftPadding{11};
    int _iconAreaWidth{40};
    int _textRightSpacing{3};
    int _indicatorIconAreaWidth{24};
    ElaThemeType::ThemeMode _themeMode;
    QPointer<ElaNavigationIndicator> _indicator;
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELAFOOTERDELEGATE_H_
