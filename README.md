# ElaWidgetTools-Plus

## 简介

本项目是 [Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools) 的衍生版本，已同步至上游 `d489319`，保留自定义共享标题栏、Qt 6.9+ 适配及应用所需的少量修复。

ElaWidgetTools 是基于 Qt Widgets 开发的 Fluent 风格组件库，提供统一的浅色与深色主题、无边框窗口、导航、输入控件、数据视图及交互动画，适用于桌面工具和业务应用。

项目沿用 QWidget 的布局、信号槽与 Model/View 开发方式，可通过 CMake 集成到现有 Qt 项目。仓库同时提供组件展示程序 ElaWidgetToolsExample，可直接查看控件效果与对应示例源码。

## 项目官网

> 🌐 **[访问 ElaWidgetTools 官网](https://liniyous.com)**
>
> 查阅 **API 文档** 与 **二次开发文档**，获取项目最新动态。

## 主要功能

- **窗口与导航**：无边框标题栏、侧边导航、面包屑、Ribbon、Pivot、可拖拽标签页，以及工具栏、状态栏和停靠窗口。
- **输入与选择**：按钮、开关、单选与复选框、文本输入、数字输入、下拉与多选、日期与颜色选择。
- **内容与数据**：列表、表格、树形视图、图片与交互卡片、流式布局、抽屉、进度条和进度环。
- **主题与反馈**：浅色与深色主题、全局字号、悬停与焦点动画、菜单、消息栏、教学提示和工具提示。
- **背景与工具**：跨平台 ElaMica 背景、命令式撤销与重做、事件总线、日志，以及 Windows DXGI 屏幕采集。

ElaMica 使用 Qt 图像与绘制接口实现，支持自定义背景图；它与 Windows 原生 Mica、Mica Alt、Acrylic 等显示模式分别提供。原生系统材质和 DXGI 的可用性取决于 Windows 版本与系统环境。

## 组件概览

| 分类 | 常用组件 |
| --- | --- |
| 窗口与布局 | ElaWindow、ElaWidget、ElaDialog、ElaAppBar、ElaDockWidget、ElaSplitter、ElaFlowLayout、ElaDrawerArea |
| 导航与页面 | ElaNavigationBar、ElaBreadcrumbBar、ElaRibbonBar、ElaPivot、ElaTabBar、ElaTabWidget、ElaScrollPage、ElaScrollPageArea |
| 按钮与输入 | ElaPushButton、ElaIconButton、ElaToolButton、ElaToggleButton、ElaToggleSwitch、ElaRadioButton、ElaCheckBox、ElaLineEdit、ElaPlainTextEdit、ElaSpinBox、ElaDoubleSpinBox、ElaKeyBinder |
| 选择控件 | ElaComboBox、ElaMultiSelectComboBox、ElaSuggestBox、ElaCalendar、ElaCalendarPicker、ElaColorDialog、ElaRoller、ElaRollerPicker、ElaSlider |
| 数据视图 | ElaListView、ElaTableView、ElaTreeView |
| 内容与卡片 | ElaImageCard、ElaInteractiveCard、ElaAcrylicUrlCard、ElaPopularCard、ElaPromotionCard、ElaPromotionView、ElaText |
| 提示与进度 | ElaContentDialog、ElaMessageBar、ElaMessageButton、ElaTeachingTip、ElaToolTip、ElaProgressBar、ElaProgressRing、ElaLCDNumber |
| 菜单与滚动 | ElaMenu、ElaMenuBar、ElaToolBar、ElaStatusBar、ElaScrollArea、ElaScrollBar |
| 应用工具 | ElaApplication、ElaTheme、ElaIcon、ElaEventBus、ElaActionCommander、ElaLog、ElaExponentialBlur、ElaDxgiManager |

## 导航栏自动宽度

`ElaNavigationBar` 和 `ElaWindow` 的导航栏默认按最长节点文字、图标、层级缩进及用户卡片内容计算展开宽度。添加、删除、重命名节点以及字体、窗口大小变化时自动更新；子节点即使收起也参与测量，避免展开时宽度跳动。宽度最小为 180 个逻辑像素，最大为 360，并且在窗口中不超过窗口宽度的一半（最小宽度优先）。过长文字沿用省略显示。

通过 `setNavigationBarMaximumWidthRatio(ratio)` 可调整自动宽度占窗口宽度的上限比例（默认 0.5，范围 0～1，非有限值忽略），例如 `0.28` 为聊天窗口保留更多正文空间；放大窗口后上限随之增加，仍遵循 180～360 的宽度范围，单独作为顶层窗口使用的导航栏只应用此像素范围。

调用 `setNavigationBarWidth(正数)` 可使用原有的固定宽度行为；调用 `setNavigationBarWidth(0)` 恢复自动宽度。`getNavigationBarWidth()` 返回当前展开宽度，自动计算在下一轮事件循环生效。此功能与 `setNavigationBarDisplayMode(Auto)` 按窗口大小切换展开、紧凑和隐藏模式相互独立，不改变紧凑模式的 42 像素宽度与现有过渡动画。

## 最新界面

以下图片沿用上游 V3.0 的实际运行界面，作为新版控件效果参考。本衍生版本保留的标题栏尺寸、布局及关闭按钮悬停颜色与图中有所不同。

### 浅色主题

![ElaWidgetTools 浅色主题](doc/preview/ElaWidgetTools_Light.png)

### 深色主题

![ElaWidgetTools 深色主题](doc/preview/ElaWidgetTools_Dark.png)

### 基础控件

![ElaWidgetTools 基础控件](doc/preview/ElaWidgetTools_Controls.png)

### 表格视图

![ElaWidgetTools 表格视图](doc/preview/ElaWidgetTools_Table.png)

截图环境：Windows、Qt 5.15.2、MSVC x64，更新于 2026-11-14。

## 近期更新

- 同步上游默认控件尺寸，常规单行控件为 32 个逻辑像素，普通列表和菜单项为 35 个逻辑像素。
- 下拉框选中标记跟随已确认值，搜索建议弹层按窗口可用空间向上或向下展开。

1. 优化控件配色、主题切换与焦点动画，统一列表和表格的交互反馈。
2. 完善菜单与下拉框的展开效果、贴边定位和圆角阴影，改进抽屉与日历动画。
3. 优化 ElaMica 背景缓存与跨屏绘制，处理屏幕外黑色区域及原生拖动时的动画停顿。
4. 更新组件展示程序与数据视图示例，保留本地屏幕采集，移除录制回放及网络传输依赖。

## 平台与构建环境

| 项目 | 说明 |
| --- | --- |
| 平台 | Windows、Linux；DXGI 与原生 Windows 材质仅用于 Windows |
| Qt | 沿用上游 Qt 5.12+ 支持，并适配 Qt 6.9+；具体限制见根目录 CMakeLists.txt |
| CMake | 3.14 或更高；Qt 6 构建还需满足相应 Qt 版本要求 |
| 编译器 | 与所选 Qt SDK 匹配的 C++ 工具链 |
| Qt 组件 | Widgets，以及相应 Qt SDK 的私有开发头文件 |
| 验证环境 | 上游使用 Windows、Qt 5.15.2、MSVC x64；本衍生版本以 Windows、Qt 6.9.3 MSVC x64 套件、clang-cl 与 Ninja 验证 |

本次同步未验证 Linux、macOS 或其他 Qt 版本。使用其他环境时，请遵循对应 Qt SDK 的编译器与开发组件要求。

### 与上游的差异和兼容范围

完整冲突清单、处理结果与验证范围见 [V3.0 同步记录](doc/upstream-v3-sync.md)。

- 共享 `ElaAppBar` 保留 48px 默认高度、46px 宽窗口按钮、16px 窗口按钮图标和垂直居中布局；通过 `setAppBarHeight()` 修改高度时，窗口按钮同步调整。Windows 命中检测与最大化交互采用 V3 实现。
- 其他控件外观优先跟随 V3；保留快速页面切换取消旧请求、销毁保护、`ElaPageTransitionRestore`、原生主题恢复等尚未被上游替代的修复。
- 为 ClassTopLand-Next 保留五个旧主题色名称：`BasicAlternating → BasicAltBase`、`BasicBaseDeep → WindowBase`、`BasicDetailsText → BasicTextNoFocus`、`DialogLayoutArea → WindowBase`、`PopupBorder → BasicBorder`。它们使用 V3 配色，不保留旧枚举数值。
- V3 属于破坏性升级，未恢复上游移除的 `ElaReminderCard`、`ElaSingleton` 或旧属性宏；使用方需要重新编译，不保证与旧版二进制兼容。

### 构建示例程序

```bash
git clone https://github.com/Aero8m/ElaWidgetTools-Plus.git
cd ElaWidgetTools-Plus
cmake -S . -B build -DQT_SDK_DIR="<Qt SDK 路径>" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

`QT_SDK_DIR` 应指向具体工具链对应的 Qt SDK 根目录，例如 `D:/Qt/5.15.2/msvc2019_64`。如果 Qt 已可由 CMake 找到，可省略该参数。

在 Windows 使用 MSVC 与 Ninja 时，请先进入 x64 开发者命令行；Qt Creator 或 CLion 用户也可直接选择与 Qt SDK 匹配的 Kit / 工具链。多配置生成器构建时，可追加 `--config Release`。

### 仅构建组件库

```bash
cmake -S . -B build -DQT_SDK_DIR="<Qt SDK 路径>" -DELAWIDGETTOOLS_BUILD_EXAMPLE=OFF
cmake --build build --parallel 4
```

默认构建动态库。

### 集成到现有项目

将仓库加入项目目录后，在已有 Qt 项目的 CMakeLists.txt 中链接 `Ela::WidgetTools`：

```cmake
set(ELAWIDGETTOOLS_BUILD_EXAMPLE OFF CACHE BOOL "Build the widget gallery")
add_subdirectory(third_party/ElaWidgetTools)
target_link_libraries(MyApp PRIVATE Ela::WidgetTools)
```

程序创建 `QApplication` 后，调用 `eApp->init()` 初始化字体、图标与平台支持，再创建窗口和控件：

```cpp
#include "ElaApplication.h"
#include "ElaWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    eApp->init();
    ElaWindow window;
    window.show();
    return application.exec();
}
```

### 安装

```bash
cmake --install build --prefix "<安装目录>"
```

安装后组件库提供 `ElaWidgetToolsConfig.cmake` 与 `Ela::WidgetTools` 目标。在调用方项目中使用 `find_package(ElaWidgetTools CONFIG REQUIRED)`，并在配置时指定包文件目录：

```bash
cmake -S . -B build -DElaWidgetTools_DIR="<安装目录>/ElaWidgetTools/lib/cmake" -DCMAKE_PREFIX_PATH="<Qt SDK 路径>"
```

## 示例与源码导航

| 目录 | 内容 |
| --- | --- |
| `ElaWidgetTools/` | 对外组件、接口与资源 |
| `ElaWidgetTools/private/` | PIMPL 私有实现 |
| `ElaWidgetTools/Development/` | 内部样式、模型、代理与绘制辅助实现 |
| `ElaWidgetToolsExample/ExamplePage/` | 基础控件、导航、弹出层、卡片、数据视图和屏幕采集示例 |
| `ElaWidgetToolsExample/ModelView/` | 列表、表格、树形数据模型 |
| `doc/preview/` | 当前版本的实际界面截图 |

## 反馈与交流

由于项目维护方式的原因，当前不接受 PR。如确认组件库存在问题，请提供复现步骤、操作系统、Qt 版本与工具链信息，反馈至 QQ 群：**850243692**。

## 参考范例

以下是一些使用 ElaWidgetTools 的开源项目：

- [KikoPlay：全功能弹幕播放器](https://github.com/KikoPlayProject/KikoPlay)
- [EricTool：工业级多协议数据分析平台](https://www.erictool.online/)

## 许可证

ElaWidgetTools 使用 [MIT 许可证](LICENSE)。分发源码或软件时，请保留相应的版权声明与许可证。

项目包含的第三方图标资源保留各自的来源和许可证，见 [Fluent System Icons 说明](ElaWidgetTools/Image/Fluent/SOURCE.txt) 与 [许可证](ElaWidgetTools/Image/Fluent/LICENSE.txt)。

版权所有 © 2024–2026 Liniyous。

Modified By Aero8m
