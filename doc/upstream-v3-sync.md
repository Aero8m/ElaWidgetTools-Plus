# V3.0 同步记录

同步日期：2026-11-14。

- 分支：`codex/sync-upstream-v3`。
- Fork 基线：`a00b840bba4ce8237a275b38fbdf04f23d4373b1`，包含当前目录原先缺少的四个远程提交。
- 上游：`cfb761287ff8848f4f39496f8648b8adf97d54ff`。
- 备份：`backup/before-upstream-sync-20261114`，指向同步前的 `316e87e`。
- 使用普通三方合并，保留双方历史；19 个冲突均已解决，修改集中在同步分支中，`main` 保持不变。
- 本次仅修改组件库；没有修改 ClassTopLand-Next 源码或子模块指针。

## 19 个冲突文件及处理结果

| 文件 | 处理结果 |
| --- | --- |
| `CMakeLists.txt` | 采用新版结构与示例开关，保留 C++17，允许 Qt 6.9+，Qt SDK 路径改为可配置的空默认值。 |
| `ElaWidgetTools/ElaAppBar.cpp` | 保留 Plus 标题栏外观，整合新版 Windows 最大化、悬停、命中检测与 DPI 处理。 |
| `ElaWidgetTools/ElaContentDialog.cpp` | 采用 V3 外观、按钮顺序和生命周期保护；保留安全替换中心控件的逻辑。 |
| `ElaWidgetTools/ElaLineEdit.cpp` | 采用 V3 输入框与焦点动画，同时清理旧样式文件中的 12px 边距适配。 |
| `ElaWidgetTools/ElaMultiSelectComboBox.cpp` | 采用 V3 的标签与文本绘制。 |
| `ElaWidgetTools/Development/ElaActionCommanderViewStyle.cpp` | 采用 V3 绘制，保留必要的图标字符显式转换。 |
| `ElaWidgetTools/Development/ElaComboBoxStyle.cpp` | 采用 V3 样式。 |
| `ElaWidgetTools/Development/ElaDrawerHeader.cpp` | 采用 V3 样式和动画，补齐图标字符转换。 |
| `ElaWidgetTools/Development/ElaMenuStyle.cpp` | 采用 V3 菜单样式，补齐图标字符转换。 |
| `ElaWidgetTools/Development/ElaNavigationStyle.cpp` | 采用 V3 导航与动画所有权管理，补齐图标字符转换。 |
| `ElaWidgetTools/Development/ElaNavigationView.cpp` | 使用 V3 导航样式构造方式。 |
| `ElaWidgetTools/Development/ElaRibbonBarModel.cpp` | 采用 V3 模型，补齐图标字符转换。 |
| `ElaWidgetTools/Development/ElaTabBarStyle.cpp` | 采用 V3 标签样式，补齐图标字符转换。 |
| `ElaWidgetTools/Development/ElaToolButtonStyle.cpp` | 使用 V3 指示器与尺寸计算。 |
| `ElaWidgetTools/private/ElaApplicationPrivate.cpp` | 使用 V3 背景缓存和线程管理，保留原生窗口重新显示时的主题恢复。 |
| `ElaWidgetTools/private/ElaMessageBarPrivate.cpp` | 使用 V3 状态图标、配色和进度绘制。 |
| `README.md` | 纳入 V3 文档，保留 Plus 名称、来源和修改者说明，补充差异与验证范围。 |
| `doc/preview/ElaWidgetTools_Dark.png` | 使用上游图片，并在 README 中注明仅为上游效果参考。 |
| `doc/preview/ElaWidgetTools_Light.png` | 同上。 |

## 保留的定制和兼容处理

- 共享标题栏默认高度 48px，窗口按钮宽 46px、图标 16px；按钮高度跟随 `setAppBarHeight()`，保留辅助按钮尺寸、垂直居中和红色关闭按钮悬停。
- 保留快速页面切换的旧请求取消、销毁保护、隐藏或缩放时结束动画，以及 `ElaPageTransitionRestore` 行为。
- 保留对话框重命名后的透明背景、默认隐藏用户信息卡、示例运行目录的 DLL 同步。
- 补齐 Qt 6.9 所需的图标字符显式转换，以及 `QPoint`、`QIODevice` 的直接包含。
- Mica 非活动时仅记住新图片，并标记缓存失效；重新开启时通过 V3 请求队列刷新，避免复用旧图。
- 未恢复 V3 已删除的组件、单例头文件和旧属性宏；不保证旧二进制兼容。

为 ClassTopLand-Next 保留以下源码兼容别名，显式映射到 V3 配色，放在 `ThemeColorCount` 之后，不改变颜色表大小和现有索引：

| 旧名称 | V3 名称 |
| --- | --- |
| `BasicAlternating` | `BasicAltBase` |
| `BasicBaseDeep` | `WindowBase` |
| `BasicDetailsText` | `BasicTextNoFocus` |
| `DialogLayoutArea` | `WindowBase` |
| `PopupBorder` | `BasicBorder` |

## 已执行验证

环境：Windows 11、Qt 6.9.3 MSVC 2022 x64 套件、clang-cl 21.1、CMake 4.0.2、Ninja。

| 检查 | 结果 |
| --- | --- |
| Debug 组件库与示例构建 | 通过；最后的 Mica 缓存失效修改后也完成重新编译和链接。 |
| Release 组件库与示例构建 | 通过；最后的 Mica 缓存失效修改后也完成重新编译和链接。 |
| `ELAWIDGETTOOLS_BUILD_EXAMPLE=OFF` 独立构建组件库 | 通过。 |
| ClassTopLand-Next 临时源码副本构建 | 通过完整编译、链接和部署；使用应用当时的工作区源码，不改真实应用目录。该构建早于最后的 Mica 缓存失效修改。 |
| 临时 QtTest 回归，100% 和 150% 缩放 | 各 15 项通过、0 失败；包含别名、36/48/64px 标题栏、最大化/还原/最小化/关闭、拖动区与缩放边界命中、中心控件替换、长按钮文本、回调销毁、快速切页、隐藏/缩放、原生深浅色恢复。 |
| 示例界面 | 查看了浅色和深色效果；后续界面自动化由用户通过 Esc 停止。 |

测试代码与专用配置仅位于临时目录，未加入正式源码或 CMake 目标。

按用户要求已停止继续测试。最后的 Mica 缓存失效修改只完成编译验证，未运行新增的图片切换回归；也未完整验证跨屏拖动和 Windows 贴靠菜单的实际悬停显示。极小内容对话框测试出现 Windows 最小尺寸调整警告，但相关断言全部通过。Linux、macOS 和其他 Qt 版本未验证。

上游原有的弃用 API、缺少 `override` 等编译警告未在本次同步中集中整理。新增修改相对上游通过 `git diff --check`；完整合并差异仍包含上游自带的部分头文件末尾空行提示。
