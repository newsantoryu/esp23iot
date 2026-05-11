# Contributing to ESP32 Audio Pitch Engine

Thank you for your interest in contributing to the ESP32 Audio Pitch Engine! This document provides guidelines and information for contributors.

## 🤝 How to Contribute

### Reporting Bugs

Before creating a bug report, please check the existing issues to avoid duplicates.

When filing a bug report, please include:

- **Clear description** of the issue
- **Steps to reproduce** the problem
- **Expected vs actual behavior**
- **Environment details** (hardware, software versions)
- **Logs and screenshots** if applicable
- **Minimum reproducible example** if possible

### Suggesting Features

Feature suggestions are welcome! Please provide:

- **Problem statement** you're trying to solve
- **Proposed solution** or implementation idea
- **Alternative approaches** considered
- **Use cases** and benefits

### Code Contributions

We welcome code contributions in the following areas:

- 🐛 **Bug fixes**
- ✨ **New features**
- 📚 **Documentation improvements**
- 🧪 **Test coverage**
- ⚡ **Performance optimizations**
- 🎨 **UI/UX improvements**

## 🛠️ Development Setup

### Prerequisites

- **PlatformIO** for ESP32 development
- **Git** for version control
- **VS Code** with PlatformIO extension (recommended)
- **ESP32 hardware** for testing

### Quick Start

1. **Fork the repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/your-username/esp32-audio-pitch-engine.git
   cd esp32-audio-pitch-engine
   ```

2. **Set up development environment**
   ```bash
   # Install dependencies
   pio lib install
   
   # Build the project
   pio run
   ```

3. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

4. **Make your changes**
   - Write code following our [coding standards](#coding-standards)
   - Add tests for new functionality
   - Update documentation

5. **Test your changes**
   ```bash
   # Build and upload to device
   pio run --target upload
   
   # Run tests
   pio test
   
   # Monitor serial output
   pio device monitor
   ```

6. **Submit your contribution**
   ```bash
   git add .
   git commit -m "feat: add your feature description"
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Provide clear description of changes
   - Link related issues
   - Include screenshots if applicable

## 📝 Coding Standards

### Code Style

We follow these conventions for consistency:

#### Naming Conventions
- **Classes/Structs**: `PascalCase` (e.g., `NoiseFilter`)
- **Functions/Methods**: `camelCase` (e.g., `calculatePitch()`)
- **Variables**: `camelCase` (e.g., `audioBuffer`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)
- **Files**: `PascalCase.h/.cpp` (e.g., `PitchDetector.h`)

#### Code Formatting
- **Indentation**: 4 spaces (no tabs)
- **Line length**: Maximum 120 characters
- **Braces**: K&R style
- **Spacing**: Around operators and after commas

#### Comments
```cpp
/**
 * @brief Calculate musical pitch from audio buffer
 * @param buffer Audio samples buffer
 * @param size Buffer size
 * @return Detected frequency in Hz, or 0 if not detected
 */
float calculatePitch(const int32_t* buffer, size_t size);
```

### Best Practices

#### Memory Management
```cpp
// Good: Use static allocation for performance-critical code
static float audioBuffer[BUFFER_SIZE];

// Avoid: Dynamic allocation in real-time code
float* buffer = new float[size];  // Don't do this in audio processing
```

#### Error Handling
```cpp
// Always check return values
if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    logger.error("Display initialization failed");
    return false;
}
```

#### Constants vs Magic Numbers
```cpp
// Good: Use named constants
const float A4_FREQUENCY = 432.0;
if (frequency > A4_FREQUENCY) { }

// Avoid: Magic numbers
if (frequency > 432.0) { }  // What does 432.0 mean?
```

## 🧪 Testing

### Running Tests

```bash
# Run all tests
pio test

# Run specific test file
pio test -f test_pitch_detector

# Run with verbose output
pio test -v
```

### Writing Tests

Tests should be placed in the `test/` directory:

```cpp
// test/test_pitch_detector.cpp
#include <unity.h>
#include "../src/utils/PitchDetector.h"

void test_frequency_to_note_conversion() {
    PitchDetector detector;
    
    // Test A4 = 440Hz
    TEST_ASSERT_EQUAL_STRING("A4", detector.frequencyToNote(440.0).c_str());
    
    // Test C4 = 261.63Hz
    TEST_ASSERT_EQUAL_STRING("C4", detector.frequencyToNote(261.63).c_str());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_frequency_to_note_conversion);
    UNITY_END();
}
```

### Test Coverage

Aim for:
- **Unit tests** for individual functions
- **Integration tests** for component interaction
- **Hardware tests** for device-specific functionality

## 📚 Documentation

### Types of Documentation

1. **User Documentation** (`docs/user/`)
   - Installation guides
   - Usage instructions
   - Troubleshooting

2. **API Documentation** (`docs/api/`)
   - Function references
   - Class documentation
   - Code examples

3. **Developer Documentation** (`docs/development/`)
   - Architecture overview
   - Development setup
   - Contribution guidelines

### Writing Documentation

- Use clear, concise language
- Include code examples
- Add screenshots/diagrams when helpful
- Keep documentation up-to-date with code changes

## 🔄 Pull Request Process

### Before Submitting

- [ ] Code follows project standards
- [ ] Tests pass locally
- [ ] Documentation is updated
- [ ] Commits are well-formatted
- [ ] No merge conflicts

### Commit Message Format

Use conventional commits:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Test additions/changes
- `chore`: Build process or auxiliary tool changes

Examples:
```
feat(audio): add pitch validation filter
fix(display): resolve OLED initialization issue
docs(readme): update installation instructions
```

### Review Process

1. **Self-review** - Check your own work
2. **Automated checks** - CI/CD pipeline validation
3. **Peer review** - Another developer reviews
4. **Approval** - Maintainer approval required
5. **Merge** - Changes merged to main branch

## 🏗️ Project Structure

Understanding the project structure helps with contributions:

```
esp32audio/
├── src/                    # Source code
│   ├── main.cpp           # Main application
│   ├── interfaces/        # Communication interfaces
│   └── utils/             # Utility classes
├── include/               # Public headers
├── docs/                  # Documentation
├── test/                  # Test files
├── lib/                   # External libraries
├── scripts/               # Build/deployment scripts
└── assets/                # Images, diagrams
```

## 🎯 Areas Needing Help

We're currently looking for help with:

- 🧪 **Test coverage** - More comprehensive tests
- 📱 **Mobile app** - Companion mobile application
- 🌐 **Web interface** - Real-time web dashboard
- 🔊 **Audio algorithms** - Improved pitch detection
- 📊 **Data analysis** - Advanced analytics features
- 🌍 **Internationalization** - Multiple language support

Check our [issues page](https://github.com/your-repo/issues) for specific tasks.

## 💬 Getting Help

### Questions and Support

- **GitHub Discussions**: General questions and ideas
- **GitHub Issues**: Bug reports and feature requests
- **Discord**: Real-time chat with community
- **Email**: support@pitchengine.com (for maintainers)

### Resources

- [📖 User Guide](docs/user/README.md)
- [🔧 API Reference](docs/api/API_REFERENCE.md)
- [🏗️ Architecture](docs/user/ARCHITECTURE.md)
- [👨‍💻 Development Guide](docs/development/README.md)

## 📜 Code of Conduct

### Our Pledge

We are committed to making participation in this project a harassment-free experience for everyone, regardless of:

- Age, body size, disability, ethnicity, gender identity and expression
- Level of experience, education, socioeconomic status, nationality, personal appearance
- Race, religion, or sexual identity

### Our Standards

**Positive behavior includes:**
- Using welcoming and inclusive language
- Being respectful of differing viewpoints and experiences
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

**Unacceptable behavior includes:**
- Harassment, trolling, or derogatory comments
- Personal or political attacks
- Public or private harassment
- Publishing private information without permission
- Any other conduct which could reasonably be considered inappropriate

### Enforcement

Project maintainers have the right and responsibility to remove, edit, or reject comments, commits, code, wiki edits, issues, and other contributions that are not aligned with this Code of Conduct.

## 🎉 Recognition

Contributors are recognized in:

- **README.md** - Major contributors section
- **CHANGELOG.md** - Release notes with credits
- **Contributors page** - Detailed contributor profiles
- **Release announcements** - Public acknowledgment

---

Thank you for contributing to the ESP32 Audio Pitch Engine! Your contributions help make this project better for everyone. 🚀
