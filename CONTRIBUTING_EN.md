# Contributing Guide

[中文](CONTRIBUTING.md)

Thank you for your interest in the Quik project! We welcome contributions of any kind.

## How to Contribute

### Reporting Bugs

If you find a bug, please submit it in [GitHub Issues](https://github.com/liewstar/quik/issues) with the following information:

- A brief description of the problem
- Steps to reproduce
- Expected behavior
- Actual behavior
- Your environment (Qt version, compiler, OS, etc.)

### Feature Requests

If you have ideas for new features, feel free to submit a Feature Request in [GitHub Issues](https://github.com/liewstar/quik/issues).

### Submitting Code

1. **Fork the Repository**
   
   Click the Fork button in the top right corner of the GitHub page

2. **Clone Your Fork**
   ```bash
   git clone https://github.com/your-username/quik.git
   cd quik
   ```

3. **Create a Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

4. **Make Changes**
   
   - Ensure code style is consistent with existing code
   - Add necessary tests
   - Update relevant documentation

5. **Commit Changes**
   ```bash
   git add .
   git commit -m "feat: add some feature"
   ```

6. **Push to Your Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   
   Create a Pull Request on GitHub describing your changes

## Code Standards

### C++ Code Style

- Use 4-space indentation
- Class names use PascalCase (e.g., `XMLUIBuilder`)
- Function names use camelCase (e.g., `buildFromFile`)
- Member variables use `m_` prefix (e.g., `m_widgets`)
- Constants use UPPER_SNAKE_CASE (e.g., `MAX_SIZE`)

### Commit Message Convention

Use [Conventional Commits](https://www.conventionalcommits.org/) specification:

- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation update
- `style:` Code formatting
- `refactor:` Code refactoring
- `test:` Test related
- `chore:` Build/tooling related

Examples:
```
feat: add RadioButton component support
fix: fix conditional expression parsing error
docs: update API documentation
```

## Development Environment

### Dependencies

- Qt 5.15+ or Qt 6.x
- C++11 or higher
- qmake

### Build

```bash
qmake
make
```

### Run Examples

```bash
cd example
qmake
make
./example
```

## Feedback

If you have any questions, you can reach us through:

- [GitHub Issues](https://github.com/liewstar/quik/issues)
- [GitHub Discussions](https://github.com/liewstar/quik/discussions)

Thank you again for your contribution!
