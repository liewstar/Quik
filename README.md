<div align="center">

# ⚡ Quik

**Reactive UIs for Qt — no QML required.**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE) [![Qt](https://img.shields.io/badge/Qt-5.15+-41CD52?logo=qt&logoColor=white)](https://www.qt.io/) [![C++](https://img.shields.io/badge/C++-11+-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/) [![GitHub stars](https://img.shields.io/github/stars/liewstar/quik?style=social)](https://github.com/liewstar/quik/stargazers) [![GitHub forks](https://img.shields.io/github/forks/liewstar/quik?style=social)](https://github.com/liewstar/quik/network/members)

[📖 文档](https://liewstar.github.io/quik-docs/) · [🚀 快速开始](#快速开始) · [💡 示例](#示例) · [🤝 贡献](#贡献)

[English](README_EN.md) | 中文

</div>

---

## ✨ 特性

- 📝 **XML 声明式 UI** - 使用简洁的 XML 语法定义界面，无需学习 QML
- 🔄 **双向数据绑定** - 自动同步 UI 和数据，修改变量 UI 自动更新
- ⚡ **条件表达式** - 支持 `visible`/`enabled` 条件绑定，轻松实现动态显示逻辑
- 🎯 **类型安全** - `QuikViewModel` 提供类型安全的变量访问，IDE 智能补全支持
- 🧩 **丰富组件** - 内置常用 Qt 组件，支持自定义组件扩展
- 🚀 **轻量高效** - 纯 C++ 实现，无额外运行时依赖

## 📦 安装

### 克隆仓库

```bash
git clone https://github.com/liewstar/quik.git
cd quik
```

### 使用 qmake 编译

```bash
qmake
make
```

### 集成到项目

在你的 `.pro` 文件中添加：

```makefile
include(/path/to/quik/quik.pri)
```

## 🚀 快速开始

### 1. 创建 XML 界面文件

```xml
<!-- MyPanel.xml -->
<Panel>
    <GroupBox title="设置">
        <CheckBox title="启用功能" var="chkEnable" default="1"/>
        <LineEdit title="输入值" var="txtValue" visible="$chkEnable==1"/>
    </GroupBox>
    <Button title="确定" var="btnOk"/>
</Panel>
```

### 2. 在 C++ 中使用

```cpp
#include "Quik/Quik.h"

// 从 XML 构建 UI
Quik::XMLUIBuilder builder;
QWidget* ui = builder.buildFromFile("MyPanel.xml");

// 类型安全的变量访问
Quik::QuikViewModel vm(&builder);
auto enabled = vm.var<bool>("chkEnable");
auto value = vm.var<QString>("txtValue");

// 像普通变量一样使用
if (enabled) {
    value = "Hello Quik!";  // UI 自动更新
}

// 监听变化
builder.watch("chkEnable", [](const QVariant& v) {
    qDebug() << "Enable changed:" << v.toBool();
});

// 连接按钮事件
builder.connectButton("btnOk", []() {
    qDebug() << "Button clicked!";
});
```

## 💡 示例

### 条件显示

```xml
<Panel>
    <ComboBox title="类型" var="cmbType" items="选项A,选项B,选项C"/>
    <LineEdit title="A的值" var="txtA" visible="$cmbType==0"/>
    <SpinBox title="B的值" var="spnB" visible="$cmbType==1"/>
    <CheckBox title="C的值" var="chkC" visible="$cmbType==2"/>
</Panel>
```

### 复合条件

```xml
<Panel>
    <CheckBox title="高级模式" var="chkAdvanced"/>
    <CheckBox title="启用日志" var="chkLog"/>
    <LineEdit title="日志路径" var="txtLogPath" 
              visible="$chkAdvanced==1 && $chkLog==1"/>
</Panel>
```

## 📚 文档

完整文档请访问：**[https://liewstar.github.io/quik-docs/](https://liewstar.github.io/quik-docs/)**

- [简介](https://liewstar.github.io/quik-docs/guide/introduction)
- [快速开始](https://liewstar.github.io/quik-docs/guide/getting-started)
- [XML 语法](https://liewstar.github.io/quik-docs/guide/xml-syntax)
- [API 参考](https://liewstar.github.io/quik-docs/api/overview)

## 📊 Star 历史

<a href="https://star-history.com/#liewstar/quik&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=liewstar/quik&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=liewstar/quik&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=liewstar/quik&type=Date" />
 </picture>
</a>

## 🤝 贡献

欢迎贡献代码！请查看 [贡献指南](CONTRIBUTING.md) 了解详情。

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 提交 Pull Request

## 📄 许可证

本项目基于 MIT 许可证开源 - 查看 [LICENSE](LICENSE) 文件了解详情。

---

<div align="center">

**如果觉得有用，请给个 ⭐ Star 支持一下！**

Made with ❤️ by [liewstar](https://github.com/liewstar)

</div>

## 🔗 相关项目

| 项目 | 描述 | 链接 |
|------|------|------|
| **Quik** | 核心框架 | [GitHub](https://github.com/liewstar/quik) |
| **Quik Docs** | 官方文档 | [GitHub](https://github.com/liewstar/quik-docs) · [在线文档](https://liewstar.github.io/quik-docs/) |
| **Quik VSCode** | VSCode 预览插件 | [GitHub](https://github.com/liewstar/quik-vscode) |
