# 贡献指南

[English](CONTRIBUTING_EN.md)

感谢你对 Quik 项目的关注！我们欢迎任何形式的贡献。

## 如何贡献

### 报告 Bug

如果你发现了 Bug，请在 [GitHub Issues](https://github.com/liewstar/quik/issues) 中提交，并包含以下信息：

- 问题的简要描述
- 复现步骤
- 期望的行为
- 实际的行为
- 你的环境信息（Qt 版本、编译器、操作系统等）

### 功能建议

如果你有新功能的想法，欢迎在 [GitHub Issues](https://github.com/liewstar/quik/issues) 中提交 Feature Request。

### 提交代码

1. **Fork 仓库**
   
   点击 GitHub 页面右上角的 Fork 按钮

2. **克隆你的 Fork**
   ```bash
   git clone https://github.com/你的用户名/quik.git
   cd quik
   ```

3. **创建特性分支**
   ```bash
   git checkout -b feature/你的特性名称
   ```

4. **进行修改**
   
   - 确保代码风格与现有代码一致
   - 添加必要的测试
   - 更新相关文档

5. **提交更改**
   ```bash
   git add .
   git commit -m "feat: 添加某某功能"
   ```

6. **推送到你的 Fork**
   ```bash
   git push origin feature/你的特性名称
   ```

7. **创建 Pull Request**
   
   在 GitHub 上创建 Pull Request，描述你的更改

## 代码规范

### C++ 代码风格

- 使用 4 空格缩进
- 类名使用 PascalCase（如 `XMLUIBuilder`）
- 函数名使用 camelCase（如 `buildFromFile`）
- 成员变量使用 `m_` 前缀（如 `m_widgets`）
- 常量使用 UPPER_SNAKE_CASE（如 `MAX_SIZE`）

### 提交信息规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

- `feat:` 新功能
- `fix:` Bug 修复
- `docs:` 文档更新
- `style:` 代码格式调整
- `refactor:` 代码重构
- `test:` 测试相关
- `chore:` 构建/工具相关

示例：
```
feat: 添加 RadioButton 组件支持
fix: 修复条件表达式解析错误
docs: 更新 API 文档
```

## 开发环境

### 依赖

- Qt 5.15+ 或 Qt 6.x
- C++11 或更高版本
- qmake

### 编译

```bash
qmake
make
```

### 运行示例

```bash
cd example
qmake
make
./example
```

## 问题反馈

如有任何问题，可以通过以下方式联系：

- [GitHub Issues](https://github.com/liewstar/quik/issues)
- [GitHub Discussions](https://github.com/liewstar/quik/discussions)

再次感谢你的贡献！
