<div align="center">

# ⚡ Quik

**Reactive UIs for Qt — no QML required.**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE) [![Qt](https://img.shields.io/badge/Qt-5.15+-41CD52?logo=qt&logoColor=white)](https://www.qt.io/) [![C++](https://img.shields.io/badge/C++-11+-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/) [![GitHub stars](https://img.shields.io/github/stars/liewstar/quik?style=social)](https://github.com/liewstar/quik/stargazers) [![GitHub forks](https://img.shields.io/github/forks/liewstar/quik?style=social)](https://github.com/liewstar/quik/network/members)

[📖 Docs](https://liewstar.github.io/quik-docs/) · [🚀 Quick Start](#quick-start) · [💡 Examples](#examples) · [🤝 Contributing](#contributing)

English | [中文](README.md)

</div>

---

## ✨ Features

- 📝 **XML Declarative UI** - Define interfaces with concise XML syntax, no need to learn QML
- 🔄 **Two-way Data Binding** - Auto-sync UI and data, UI updates automatically when variables change
- ⚡ **Conditional Expressions** - Support `visible`/`enabled` conditional binding for dynamic display logic
- 🎯 **Type Safety** - `QuikViewModel` provides type-safe variable access with IDE auto-completion
- 🧩 **Rich Components** - Built-in common Qt components with custom component extension support
- 🚀 **Lightweight & Efficient** - Pure C++ implementation, no additional runtime dependencies

## 📦 Installation

### Clone Repository

```bash
git clone https://github.com/liewstar/quik.git
cd quik
```

### Build with qmake

```bash
qmake
make
```

### Integrate into Project

Add to your `.pro` file:

```makefile
include(/path/to/quik/quik.pri)
```

## 🚀 Quick Start

### 1. Create XML Interface File

```xml
<!-- MyPanel.xml -->
<Panel>
    <GroupBox title="Settings">
        <CheckBox title="Enable Feature" var="chkEnable" default="1"/>
        <LineEdit title="Input Value" var="txtValue" visible="$chkEnable==1"/>
    </GroupBox>
    <Button title="OK" var="btnOk"/>
</Panel>
```

### 2. Use in C++

```cpp
#include "Quik/Quik.h"

// Build UI from XML
Quik::XMLUIBuilder builder;
QWidget* ui = builder.buildFromFile("MyPanel.xml");

// Type-safe variable access
Quik::QuikViewModel vm(&builder);
auto enabled = vm.var<bool>("chkEnable");
auto value = vm.var<QString>("txtValue");

// Use like normal variables
if (enabled) {
    value = "Hello Quik!";  // UI updates automatically
}

// Watch for changes
builder.watch("chkEnable", [](const QVariant& v) {
    qDebug() << "Enable changed:" << v.toBool();
});

// Connect button events
builder.connectButton("btnOk", []() {
    qDebug() << "Button clicked!";
});
```

## 💡 Examples

### Conditional Display

```xml
<Panel>
    <ComboBox title="Type" var="cmbType" items="Option A,Option B,Option C"/>
    <LineEdit title="Value A" var="txtA" visible="$cmbType==0"/>
    <SpinBox title="Value B" var="spnB" visible="$cmbType==1"/>
    <CheckBox title="Value C" var="chkC" visible="$cmbType==2"/>
</Panel>
```

### Compound Conditions

```xml
<Panel>
    <CheckBox title="Advanced Mode" var="chkAdvanced"/>
    <CheckBox title="Enable Logging" var="chkLog"/>
    <LineEdit title="Log Path" var="txtLogPath" 
              visible="$chkAdvanced==1 && $chkLog==1"/>
</Panel>
```

## 📚 Documentation

Full documentation available at: **[https://liewstar.github.io/quik-docs/](https://liewstar.github.io/quik-docs/)**

- [Introduction](https://liewstar.github.io/quik-docs/guide/introduction)
- [Getting Started](https://liewstar.github.io/quik-docs/guide/getting-started)
- [XML Syntax](https://liewstar.github.io/quik-docs/guide/xml-syntax)
- [API Reference](https://liewstar.github.io/quik-docs/api/overview)

## 📊 Star History

<a href="https://star-history.com/#liewstar/quik&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=liewstar/quik&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=liewstar/quik&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=liewstar/quik&type=Date" />
 </picture>
</a>

## 🤝 Contributing

Contributions are welcome! Please check the [Contributing Guide](CONTRIBUTING_EN.md) for details.

1. Fork this repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Submit a Pull Request

## 📄 License

This project is open source under the MIT License - see the [LICENSE](LICENSE) file for details.

---

<div align="center">

**If you find this useful, please give it a ⭐ Star!**

Made with ❤️ by [liewstar](https://github.com/liewstar)

</div>

## 🔗 Related Projects

| Project | Description | Links |
|---------|-------------|-------|
| **Quik** | Core Framework | [GitHub](https://github.com/liewstar/quik) |
| **Quik Docs** | Official Documentation | [GitHub](https://github.com/liewstar/quik-docs) · [Online Docs](https://liewstar.github.io/quik-docs/) |
| **Quik VSCode** | VSCode Preview Extension | [GitHub](https://github.com/liewstar/quik-vscode) |
