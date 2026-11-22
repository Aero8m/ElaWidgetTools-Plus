#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELACONTENTDIALOGPRIVATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELACONTENTDIALOGPRIVATE_H_

#include "ElaPropertyMacro.h"
#include <QObject>
#include <QPointer>

#include "ElaWidgetToolsDef.h"
class QVBoxLayout;
class ElaPushButton;
class ElaContentDialog;
class ElaMaskWidget;
class ElaAppBar;
class QScreen;
class ElaContentDialogPrivate : public QObject
{
    Q_OBJECT
    ELA_PIMPL_D(ElaContentDialog)
public:
    explicit ElaContentDialogPrivate(QObject* parent = nullptr);
    ~ElaContentDialogPrivate() override;

private:
    qint64 _currentWinID{0};
    ElaAppBar* _appBar{nullptr};
    ElaThemeType::ThemeMode _themeMode;
    QPointer<QWidget> _owner;
    QPointer<ElaMaskWidget> _maskWidget;
    QPointer<QScreen> _screen;
    QMetaObject::Connection _workAreaChanged;
    bool _layoutUpdatePending{false};
    bool _applyingGeometry{false};
    QWidget* _centralWidget{nullptr};
    QWidget* _buttonWidget{nullptr};
    QVBoxLayout* _mainLayout{nullptr};
    QString _leftButtonText{"取消"};
    QString _middleButtonText{"最小化"};
    QString _rightButtonText{"退出"};
    ElaPushButton* _leftButton{nullptr};
    ElaPushButton* _middleButton{nullptr};
    ElaPushButton* _rightButton{nullptr};
    void _scheduleLayoutUpdate();
    void _fitAndCenter();
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELACONTENTDIALOGPRIVATE_H_

