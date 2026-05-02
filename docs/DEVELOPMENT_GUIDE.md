# Guia de Desenvolvimento - ESP32 Audio Pitch Engine

## Visão Geral

Este guia estabelece padrões, procedimentos e melhores práticas para o desenvolvimento do sistema de detecção de pitch. Seguir estas diretrizes garante código consistente, maintainável e de alta qualidade.

## Ambiente de Desenvolvimento

### Pré-requisitos

#### Hardware
- ESP32 DevKit V1 ou compatível
- Cabo USB para programação
- Display OLED SSD1306 128x64
- Microfone I2S ou fonte de áudio

#### Software
- **PlatformIO**: 6.13.0 ou superior
- **VSCode**: Editor de código recomendado
- **Python**: 3.8+ (dependências do PlatformIO)
- **Git**: Controle de versão

### Setup do Ambiente

#### 1. Instalar PlatformIO
```bash
# Instalar VSCode Extension
code --install-extension platformio.platformio-ide

# Verificar instalação
pio --version
```

#### 2. Clonar Repositório
```bash
git clone <repository-url>
cd esp32audio
```

#### 3. Configurar Projeto
```bash
# Instalar dependências
pio lib install

# Verificar configuração
pio project config

# Compilar projeto
pio run
```

#### 4. Hardware Setup
```
ESP32 Connections:
├── GPIO25 → I2S_WS (Word Select)
├── GPIO34 → I2S_SD (Serial Data)
├── GPIO26 → I2S_SCK (Serial Clock)
├── GPIO14 → LED_PIN (Status LED)
├── GPIO21 → OLED_SDA (I2C Data)
├── GPIO22 → OLED_SCL (I2C Clock)
└── 3.3V   → Power Supply
```

## Padrões de Código

### Nomenclatura

#### Classes e Structs
```cpp
// PascalCase
class AudioProcessor;
class PitchDetector;
struct MusicalNote;
```

#### Variáveis e Funções
```cpp
// camelCase
float currentFrequency;
int stabilityCounter;
void updateDisplay();
bool validatePitch();
```

#### Constantes e Macros
```cpp
// UPPER_SNAKE_CASE
const float A4_FREQ = 432.0;
const int MAX_SAMPLES = 1024;
#define BUFFER_SIZE 1024
```

#### Membros Privados
```cpp
// Prefixo underscore
private:
    float _internalBuffer;
    int _sampleRate;
    bool _isInitialized;
```

### Organização de Arquivos

#### Estrutura de Diretórios
```
esp32audio/
├── src/
│   ├── main.cpp                 # Entry point
│   ├── core/                    # Componentes principais
│   │   ├── AudioProcessor.h/cpp
│   │   ├── PitchDetector.h/cpp
│   │   └── NoteAnalyzer.h/cpp
│   ├── utils/                   # Utilitários
│   │   ├── Logger.h/cpp
│   │   ├── NoiseFilter.h/cpp
│   │   └── DataStorage.h/cpp
│   ├── interfaces/              # Interfaces externas
│   │   ├── DisplayManager.h/cpp
│   │   └── ConfigManager.h/cpp
│   └── tests/                   # Testes unitários
│       ├── TestAudioProcessor.cpp
│       └── TestPitchDetector.cpp
├── docs/                        # Documentação
│   ├── ARCHITECTURE.md
│   ├── API_REFERENCE.md
│   └── DEVELOPMENT_GUIDE.md
├── tests/                       # Testes de integração
├── examples/                    # Exemplos de uso
├── platformio.ini              # Configuração PlatformIO
├── README.md                   # Documentação do projeto
└── CHANGELOG.md                # Histórico de mudanças
```

#### Convenções de Arquivos
- **Headers**: `.h` com declarações de classe
- **Implementations**: `.cpp` com código fonte
- **Testes**: `Test*.cpp` para testes unitários
- **Examples**: `example_*.cpp` para exemplos

### Padrões de Design

#### 1. Singleton Pattern
```cpp
class Logger {
private:
    static Logger* _instance;
    Logger() = default;

public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    // Delete copy constructor and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
```

#### 2. RAII (Resource Acquisition Is Initialization)
```cpp
class AudioProcessor {
public:
    AudioProcessor(int sampleRate) : _sampleRate(sampleRate) {
        _buffer = new float[_bufferSize];
        initialize();
    }
    
    ~AudioProcessor() {
        delete[] _buffer;
        cleanup();
    }
    
private:
    float* _buffer;
    int _sampleRate;
};
```

#### 3. Strategy Pattern
```cpp
class PitchAlgorithm {
public:
    virtual ~PitchAlgorithm() = default;
    virtual float detect(const AudioBuffer& buffer) = 0;
};

class ZeroCrossAlgorithm : public PitchAlgorithm {
public:
    float detect(const AudioBuffer& buffer) override;
};
```

### Documentação de Código

#### Comentários de Classe
```cpp
/**
 * @brief Processamento de áudio bruto e filtragem de sinal
 * 
 * Esta classe é responsável por processar amostras de áudio brutas do I2S,
 * aplicar filtros passa-baixa e normalizar o sinal para detecção de pitch.
 * 
 * @note Usa filtro RC com cutoff configurável
 * @warning Requer inicialização antes do uso
 * @see PitchDetector para detecção de frequência
 * 
 * @example
 * AudioProcessor processor(16000, 1024);
 * AudioBuffer buffer = processor.process(rawSamples, 1024);
 */
class AudioProcessor {
    // ...
};
```

#### Comentários de Método
```cpp
/**
 * @brief Detecta frequência fundamental usando zero-cross detection
 * 
 * @param buffer Buffer de áudio processado
 * @return float Frequência detectada em Hz, 0.0 se inválido
 * 
 * @throws std::invalid_argument se buffer for nulo
 * @note Precisão de ±5 cents
 * @see validate() para validação de qualidade
 */
float detect(const AudioBuffer& buffer);
```

#### Comentários Inline
```cpp
// Aplicar filtro passa-baixa RC
float filtered = _previous + alpha * (input - _previous);

// Validar SNR mínimo para rejeitar ruído
if (snr < MIN_SNR) return 0.0;

// Incrementar contador de estabilidade
_stabilityCounter++;
```

## Processo de Desenvolvimento

### 1. Feature Development

#### Branch Strategy
```bash
# Criar branch para nova feature
git checkout -b feature/pitch-algorithm-improvement

# Desenvolver e testar
# ...

# Commit com mensagem descritiva
git commit -m "feat: implement YIN algorithm for improved pitch detection"

# Push para review
git push origin feature/pitch-algorithm-improvement
```

#### Commit Message Format
```
type(scope): description

[optional body]

[optional footer]
```

**Types:**
- `feat`: Nova feature
- `fix`: Bug fix
- `docs`: Documentação
- `style`: Formatação
- `refactor`: Refatoração
- `test`: Testes
- `chore`: Manutenção

**Exemplos:**
```
feat(pitch): implement YIN algorithm for improved accuracy

- Add YIN implementation as alternative to zero-cross
- Improve precision from ±5 cents to ±1 cent
- Add configuration option for algorithm selection

Closes #123
```

### 2. Code Review

#### Checklist de Review
- [ ] Código segue padrões de nomenclatura
- [ ] Comentários adequados e claros
- [ ] Sem warnings de compilação
- [ ] Testes unitários passando
- [ ] Performance adequada
- [ ] Segurança considerada
- [ ] Documentação atualizada

#### Processo de Review
1. **Self-review**: Autor revisa próprio código
2. **Peer review**: Outro desenvolvedor revisa
3. **Automated review**: Ferramentas automáticas
4. **Approval**: Aprovação necessária para merge

### 3. Testing

#### Testes Unitários
```cpp
#include <gtest/gtest.h>
#include "PitchDetector.h"

class PitchDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_unique<PitchDetector>(60.0, 800.0, 0.7);
    }
    
    std::unique_ptr<PitchDetector> detector;
};

TEST_F(PitchDetectorTest, DetectA4Frequency) {
    AudioBuffer buffer = createTestBuffer(432.0);  // A4 = 432Hz
    float frequency = detector->detect(buffer);
    
    EXPECT_NEAR(frequency, 432.0, 5.0);  // ±5Hz tolerance
    EXPECT_GT(detector->getConfidence(), 0.7);
}
```

#### Testes de Integração
```cpp
TEST(IntegrationTest, EndToEndPitchDetection) {
    // Setup completo do sistema
    AudioProcessor processor(16000, 1024);
    PitchDetector detector(60.0, 800.0, 0.7);
    NoteAnalyzer analyzer(432.0);
    
    // Processar áudio de teste
    int32_t* testAudio = generateTestAudio(432.0);
    AudioBuffer buffer = processor.process(testAudio, 1024);
    
    // Detectar e analisar
    float frequency = detector.detect(buffer);
    MusicalNote note = analyzer.analyze(frequency);
    
    // Validar resultado
    EXPECT_NEAR(frequency, 432.0, 5.0);
    EXPECT_EQ(note.name, "A");
    EXPECT_EQ(note.octave, 4);
    EXPECT_NEAR(note.cents, 0.0, 5.0);
}
```

#### Testes de Performance
```cpp
TEST(PerformanceTest, RealTimeProcessing) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Processar 1000 buffers
    for (int i = 0; i < 1000; i++) {
        AudioBuffer buffer = generateRandomBuffer();
        float frequency = detector->detect(buffer);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Deve processar em <1000ms (1ms por buffer)
    EXPECT_LT(duration.count(), 1000);
}
```

### 4. Debug e Troubleshooting

#### Logging Levels
```cpp
// Em produção
Logger::setLevel(LOG_INFO);

// Em desenvolvimento
Logger::setLevel(LOG_DEBUG);

// Em debug profundo
Logger::setLevel(LOG_ALL);
```

#### Debug Techniques
```cpp
// 1. Asserts para verificação em tempo de desenvolvimento
assert(buffer != nullptr);
assert(buffer->length > 0);

// 2. Logs detalhados para diagnóstico
Logger::debug("Processing buffer: " + String(buffer->length) + " samples");
Logger::debug("SNR: " + String(calculateSNR(buffer)));

// 3. Indicadores visuais
digitalWrite(LED_PIN, HIGH);  // Indicar processamento
// ... processamento ...
digitalWrite(LED_PIN, LOW);   // Indicar conclusão

// 4. Serial output para debug rápido
#ifdef DEBUG
Serial.println("Frequency: " + String(frequency));
Serial.println("Confidence: " + String(confidence));
#endif
```

#### Common Issues e Solutions

**Issue: Memory corruption**
```cpp
// Solução: Bounds checking
if (index >= bufferSize) {
    Logger::error("Buffer overflow detected");
    return false;
}
```

**Issue: Real-time constraints**
```cpp
// Solução: Timer measurement
auto start = micros();
processAudioBuffer();
auto duration = micros() - start;

if (duration > MAX_PROCESSING_TIME_US) {
    Logger::warning("Processing time exceeded: " + String(duration) + "us");
}
```

**Issue: I2S initialization failure**
```cpp
// Solução: Hardware reset e retry
bool initializeI2S() {
    for (int attempt = 0; attempt < 3; attempt++) {
        if (i2s_install_driver() == ESP_OK) {
            return true;
        }
        delay(100);
        i2s_driver_uninstall();
    }
    return false;
}
```

## Build e Deploy

### Build Configuration

#### platformio.ini
```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino

# Build options
build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DCONFIG_ARDUHAL_LOG_COLORS
    -DCONFIG_ARDUHAL_ESP_LOG

# Optimization
build_type = release
build_unflags = -Os
build_flags = -O2

# Libraries
lib_deps = 
    adafruit/Adafruit SSD1306@^2.5.16
    adafruit/Adafruit GFX Library@^1.12.6

# Monitor
monitor_speed = 115200
monitor_filters = esp32_exception_decoder

# Upload
upload_speed = 921600
```

### Build Commands

#### Development Build
```bash
# Compilação rápida
pio run

# Com upload
pio run --target upload

# Com monitoramento
pio run --target upload --target monitor
```

#### Production Build
```bash
# Build otimizado
pio run --environment production

# Verificar tamanho
pio run --target size

# Testes automatizados
pio test --environment production
```

### OTA Updates

#### Configuration
```cpp
#include <ArduinoOTA.h>

void setupOTA() {
    ArduinoOTA.onStart([]() {
        Logger::info("OTA update started");
    });
    
    ArduinoOTA.onEnd([]() {
        Logger::info("OTA update completed");
    });
    
    ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    // ... resto do loop
}
```

## Performance e Otimização

### Memory Management

#### Stack vs Heap
```cpp
// Stack (preferido para objetos pequenos)
AudioProcessor processor(16000, 1024);

// Heap (necessário para objetos grandes ou dinâmicos)
float* largeBuffer = new float[4096];
// ... usar buffer ...
delete[] largeBuffer;
```

#### Memory Pooling
```cpp
class MemoryPool {
private:
    static const int POOL_SIZE = 1024;
    static uint8_t pool[POOL_SIZE];
    static size_t poolIndex;
    
public:
    static void* allocate(size_t size) {
        if (poolIndex + size > POOL_SIZE) return nullptr;
        void* ptr = &pool[poolIndex];
        poolIndex += size;
        return ptr;
    }
    
    static void reset() {
        poolIndex = 0;
    }
};
```

### CPU Optimization

#### Inline Functions
```cpp
// Funções pequenas e frequentemente usadas
inline float calculateSNR(float signal, float noise) {
    return (signal > 0) ? (signal * signal) / noise : 0.0;
}
```

#### Lookup Tables
```cpp
// Pré-calcular valores frequentemente usados
static const float LOG2_TABLE[256] = {
    // valores pré-calculados de log2
};

inline float fastLog2(float x) {
    if (x < 1.0 || x > 256.0) return log2(x);
    return LOG2_TABLE[(int)x];
}
```

#### Fixed Point Arithmetic
```cpp
// Usar fixed point para cálculos críticos
typedef int32_t fixed_t;
#define FIXED_SCALE 1000

fixed_t floatToFixed(float f) {
    return (fixed_t)(f * FIXED_SCALE);
}

float fixedToFloat(fixed_t f) {
    return (float)f / FIXED_SCALE;
}
```

## Segurança

### Input Validation
```cpp
bool validateAudioInput(int32_t* samples, int count) {
    if (!samples || count <= 0 || count > MAX_SAMPLES) {
        return false;
    }
    
    // Verificar valores válidos
    for (int i = 0; i < count; i++) {
        if (abs(samples[i]) > MAX_SAMPLE_VALUE) {
            return false;
        }
    }
    
    return true;
}
```

### Buffer Overflow Protection
```cpp
class SafeBuffer {
private:
    float* _data;
    size_t _size;
    size_t _index;
    
public:
    bool add(float value) {
        if (_index >= _size) {
            Logger::error("Buffer overflow");
            return false;
        }
        _data[_index++] = value;
        return true;
    }
};
```

## Ferramentas e Utilitários

### Code Analysis
```bash
# Verificar complexidade
pio check --skip-packages

# Formatar código
pio run --target checkfmt

# Análise estática
pio run --target static-analysis
```

### Profiling
```cpp
#include "esp_timer.h"

class Profiler {
private:
    uint64_t _start;
    const char* _name;
    
public:
    Profiler(const char* name) : _name(name) {
        _start = esp_timer_get_time();
    }
    
    ~Profiler() {
        uint64_t duration = esp_timer_get_time() - _start;
        Logger::debug(String(_name) + ": " + String(duration) + "us");
    }
};

#define PROFILE_SCOPE(name) Profiler _prof(name)
```

### Memory Debugging
```cpp
void checkHeap() {
    size_t freeHeap = ESP.getFreeHeap();
    size_t minFreeHeap = ESP.getMinFreeHeap();
    
    Logger::debug("Free heap: " + String(freeHeap) + " bytes");
    Logger::debug("Min free heap: " + String(minFreeHeap) + " bytes");
    
    if (freeHeap < MIN_HEAP_THRESHOLD) {
        Logger::warning("Low memory detected");
    }
}
```

## Best Practices

### 1. Error Handling
```cpp
// Preferir retornar códigos de erro
bool processAudio(AudioBuffer& buffer) {
    if (!validateBuffer(buffer)) {
        Logger::error("Invalid buffer");
        return false;
    }
    
    if (!processAlgorithm(buffer)) {
        Logger::error("Algorithm failed");
        return false;
    }
    
    return true;
}

// Usar RAII para recursos
class ScopedTimer {
public:
    ScopedTimer() { startTimer(); }
    ~ScopedTimer() { stopTimer(); }
};
```

### 2. Configuração
```cpp
// Usar configuração centralizada
class Config {
public:
    static float getA4Frequency() { return _a4Freq; }
    static void setA4Frequency(float freq) { _a4Freq = freq; }
    
private:
    static float _a4Freq;
};

// Carregar de arquivo
void loadConfig() {
    // Implementar carregamento de JSON/INI
}
```

### 3. Testing
```cpp
// Test-driven development
TEST(PitchDetectorTest, DetectKnownFrequency) {
    // Arrange
    AudioBuffer buffer = createTestBuffer(440.0);
    PitchDetector detector;
    
    // Act
    float result = detector.detect(buffer);
    
    // Assert
    EXPECT_NEAR(result, 440.0, 5.0);
}
```

Este guia estabelece as bases para desenvolvimento profissional do sistema, garantindo código de alta qualidade, maintainável e performático.
