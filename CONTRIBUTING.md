# Contributing to 2D Truss Analysis

Thank you for your interest in contributing to the 2D Truss Analysis project! This document provides guidelines and best practices for contributing to the codebase.

## Getting Started

### Prerequisites

- Git installed and configured
- C++20 compatible compiler
- Qt6 development libraries
- CMake 3.20+
- Eigen3 library

### Development Environment Setup

1. **Fork and Clone**

   ```bash
   # Fork the repository on GitHub
   git clone https://github.com/YOUR_USERNAME/2D-Truss-Analysis.git
   cd 2D-Truss-Analysis

   # Add upstream remote
   git remote add upstream https://github.com/blackbird410/2D-Truss-Analysis.git
   ```

2. **Install Dependencies**

   ```bash
   # macOS
   brew install qt@6 eigen cmake

   # Linux (Ubuntu/Debian)
   sudo apt install qt6-base-dev libeigen3-dev cmake
   ```

3. **Build and Test**

   ```bash
   # Using production Makefile (recommended)
   make build        # Build release version
   make test         # Run all tests
   make format       # Format code before committing

   # Or use CMake directly
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make -j$(nproc)
   ./TrussAnalysisCLI help       # Show available commands and options
   ```

## Git Workflow

### Branch Naming Convention

We use a structured branch naming convention:

- `feature/description` - New features
- `bugfix/issue-description` - Bug fixes
- `hotfix/critical-issue` - Critical production fixes
- `docs/documentation-update` - Documentation improvements
- `refactor/component-name` - Code refactoring
- `test/test-description` - Adding or improving tests

### Professional Git Workflow

1. **Create Feature Branch**

   ```bash
   git checkout main
   git pull upstream main
   git checkout -b feature/interactive-analysis-panel
   ```

2. **Make Changes**
   - Follow coding standards (see below)
   - Write meaningful commit messages
   - Keep commits atomic and focused

3. **Commit Changes**

   ```bash
   git add .
   git commit -m "feat: add interactive analysis panel with real-time updates"
   ```

4. **Push and Create PR**
   ```bash
   git push origin feature/interactive-analysis-panel
   # Create Pull Request on GitHub
   ```

## Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/) specification:

### Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation only changes
- **style**: Changes that don't affect meaning (formatting, missing semicolons)
- **refactor**: Code change that neither fixes a bug nor adds a feature
- **test**: Adding missing tests or correcting existing tests
- **build**: Changes that affect the build system or external dependencies
- **ci**: Changes to CI configuration files and scripts
- **perf**: Performance improvements
- **chore**: Other changes that don't modify src or test files

### Examples

```bash
feat: add interactive drawing canvas for truss design
fix: resolve memory leak in analysis engine
docs: update installation guide for macOS
style: format code according to project standards
refactor: extract utility functions to separate module
test: add unit tests for matrix operations
build: update CMake configuration for Qt6
```

## Code Standards

### C++ Guidelines

1. **Modern C++20**
   - Use modern C++ features appropriately
   - Prefer `auto` for type deduction when clear
   - Use smart pointers over raw pointers
   - Leverage STL algorithms and containers
   - Prefer `const` references for read-only access
   - Make stateless helper functions `static`
   - Use `[[maybe_unused]]` for intentionally unused public APIs to keep cppcheck clean

2. **Code Style**

   **Automated Formatting:**

   ```bash
   # Format all C++ code (uses .clang-format configuration)
   make format

   # Format Markdown documentation (uses .prettierrc.yaml)
   make format-docs

   # Format YAML files
   make format-yaml

   # Check formatting without modifying files (CI-friendly)
   make format-check-all
   ```

   **Manual Style Guidelines:**

   ```cpp
   // Class names: PascalCase
   class TrussAnalysisEngine {
   public:
       // Method names: camelCase
       void calculateDisplacements();

       // Member variables: m_prefix
       Real m_youngModulus;

   private:
       // Constants: UPPER_CASE
       static constexpr Real DEFAULT_TOLERANCE = 1e-10;
   };

   // Namespace: lowercase
   namespace truss::analysis {
       // Function names: camelCase
       bool validateTrussGeometry(const Truss& truss);
   }
   ```

   **Note:** Code style is enforced by `.clang-format` (C++), `.prettierrc.yaml` (Markdown/YAML),
   and `.editorconfig` (baseline). Run `make format` before committing.

3. **Documentation**
   ```cpp
   /**
    * @brief Calculates nodal displacements using direct stiffness method
    * @param globalStiffness Global stiffness matrix
    * @param loadVector Applied load vector
    * @return Vector of nodal displacements
    */
   VectorXd calculateDisplacements(const MatrixXd& globalStiffness,
                                   const VectorXd& loadVector);
   ```

### Qt Guidelines

1. **Signal-Slot Connections**

   ```cpp
   // Prefer new syntax
   connect(button, &QPushButton::clicked, this, &MainWindow::onAnalyze);

   // Avoid old syntax
   // connect(button, SIGNAL(clicked()), this, SLOT(onAnalyze()));
   ```

2. **Memory Management**

   ```cpp
   // Use parent-child relationship for Qt objects
   auto* widget = new QWidget(this);  // 'this' as parent

   // Use smart pointers for non-Qt objects
   auto engine = std::make_unique<AnalysisEngine>();
   ```

### GUI Architecture (`src/gui/`)

The GUI layer uses a clean Qt MVC architecture. **No domain knowledge or concrete domain classes belong in `src/gui/`** — all interaction travels through `ITrussAnalysisFacade`.

```
src/gui/
├── main.cpp                      ← Composition root; constructs facade + MainWindow
├── main_window.hpp / .cpp        ← Top-level QMainWindow
├── theme_loader.hpp / .cpp       ← QSettings-persisted QSS theme loader
├── controllers/                  ← Business logic, all inherit QObject
│   ├── main_window_controller    ← Owns all sub-controllers + models
│   ├── canvas_controller         ← Node/member CRUD via facade
│   ├── analysis_controller       ← Runs analysis, dispatches results
│   ├── project_controller        ← New/open/save/export file operations
│   ├── inspector_controller      ← Node/member property editing
│   └── export_controller         ← CSV/JSON/XML results export
├── interfaces/
│   └── iconfirmation_provider.hpp ← Abstracts QMessageBox for unit tests
├── models/                       ← Qt Item Models (QAbstractTableModel)
│   ├── node_table_model          ← Backed by ITrussView
│   ├── member_table_model        ← Backed by ITrussView
│   ├── results_table_model       ← Backed by IAnalysisResultsView
│   └── validation_list_model     ← Backed by IValidationView
├── panels/                       ← Composite sub-widgets
│   ├── inspector_panel           ← Node/member property form
│   ├── analysis_control_bar      ← Run + progress bar
│   ├── results_dock_panel        ← Tab: nodes | members | system summary
│   ├── analysis_options_dialog   ← Solver tolerance / iteration dialog
│   └── notification_rail         ← Auto-dismiss inline status messages
├── state/
│   └── workspace_state.hpp       ← Plain struct; mutation signals via controller
└── widgets/
    └── truss_canvas_widget       ← QPainter 7-step rendering pipeline
```

**Adding a new controller:**

1. Inherit `QObject`; inject `ITrussAnalysisFacade&` via constructor
2. Expose `signals:` for state updates; add `public slots:` for user actions
3. Own it (via `std::unique_ptr`) in `MainWindowController`
4. Add a corresponding GTest file in `tests/unit/gui/controllers/`

### CMake Guidelines

1. **Modern CMake** (3.20+)

   ```cmake
   # Use target-based approach
   target_link_libraries(TrussAnalysisGUI PRIVATE
       TrussCore
       Qt6::Core
       Qt6::Widgets
   )

   # Set properties on targets
   set_target_properties(TrussCore PROPERTIES
       CXX_STANDARD 20
       CXX_STANDARD_REQUIRED ON
   )
   ```

## Testing Guidelines

### Unit Tests

```cpp
#include <gtest/gtest.h>
#include "TrussAnalysisEngine.hpp"

class TrussAnalysisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test fixtures
    }
};

TEST_F(TrussAnalysisTest, CalculatesCorrectDisplacements) {
    // Arrange
    auto truss = createSimpleTruss();

    // Act
    auto displacements = engine.calculateDisplacements(truss);

    // Assert
    EXPECT_NEAR(displacements[0], expectedValue, 1e-10);
}
```

### Integration Tests

```bash
# CLI integration test
./TrussAnalysisCLI example > output.txt
diff output.txt expected_output.txt
```

## Pull Request Guidelines

### Before Submitting

- [ ] Code follows project standards
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] Commit messages follow convention
- [ ] No merge conflicts with main branch

### PR Template

```markdown
## Description

Brief description of changes made.

## Type of Change

- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing

- [ ] Unit tests added/updated
- [ ] Integration tests pass
- [ ] Manual testing completed

## Screenshots (if applicable)

Add screenshots to help explain your changes.

## Checklist

- [ ] My code follows the project's style guidelines
- [ ] I have performed a self-review of my code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
```

## Reporting Issues

### Bug Reports

Use the bug report template:

```markdown
**Describe the bug**
A clear and concise description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:

1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

**Expected behavior**
A clear and concise description of what you expected to happen.

**Environment:**

- OS: [e.g. macOS 14.0]
- Qt Version: [e.g. 6.9.1]
- Compiler: [e.g. Clang 15.0]
- Version: [e.g. v2.1.1]

**Additional context**
Add any other context about the problem here.
```

## Release Process

### Version Numbering

We follow [Semantic Versioning](https://semver.org/):

- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

### Release Workflow

1. Create release branch: `release/v2.2.0`
2. Update version numbers
3. Update CHANGELOG.md
4. Create and test release build
5. Create annotated tag: `git tag -a v2.2.0 -m "Release v2.2.0"`
6. Push tag: `git push origin v2.2.0`

## Getting Help

- **GitHub Discussions**: For general questions and ideas
- **GitHub Issues**: For bug reports and feature requests
- **Code Review**: All PRs receive thorough review

## Recognition

Contributors will be recognized in:

- README.md contributors section
- Release notes
- Special recognition for significant contributions

Thank you for contributing to make structural analysis software better for the engineering community!
