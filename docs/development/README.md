# Guia de Desenvolvimento - ESP32 Audio Pitch Engine

Bem-vindo ao guia de desenvolvimento para contribuir com o projeto ESP32 Audio Pitch Engine.

## 📋 Índice

- [Estrutura do Projeto](#estrutura-do-projeto)
- [Ambiente de Desenvolvimento](#ambiente-de-desenvolvimento)
- [Fluxo de Trabalho](#fluxo-de-trabalho)
- [Padrões de Código](#padrões-de-código)
- [Testes e Validação](#testes-e-validação)
- [Debug e Diagnóstico](#debug-e-diagnóstico)
- [Contribuição](#contribuição)

## 🏗️ Estrutura do Projeto

### Organização de Diretórios

```
esp32audio/
├── src/                    # Código fonte principal
│   ├── main.cpp           # Programa principal
│   ├── interfaces/        # Interfaces de comunicação
│   │   ├── WiFiStreamer.* # Streaming via WiFi
│   │   └── SerialStreamer.* # Serial output
│   └── utils/             # Utilitários e classes
│       ├── Logger.*       # Sistema de logging
│       ├── NoiseFilter.*  # Filtragem de ruído
│       ├── DataStorage.*  # Armazenamento de dados
│       └── DebugDashboard.* # Debug interface
├── include/               # Headers públicos
├── docs/                  # Documentação
│   ├── user/             # Documentação do usuário
│   ├── api/              # Referência da API
│   └── development/      # Docs de desenvolvimento
├── test/                 # Testes unitários
├── lib/                  # Bibliotecas externas
└── scripts/              # Scripts de build e deploy
```

### Componentes Principais

#### 1. **AudioProcessor**
- Processamento de áudio em tempo real
- Implementação do algoritmo YIN
- Buffer management e FFT

#### 2. **NoiseFilter**
- Filtragem adaptativo de ruído
- Detecção de atividade de sinal
- Threshold management

#### 3. **PitchDetector**
- Conversão frequência → nota
- Cálculo de cents e oitavas
- Validação de pitch

#### 4. **DisplayManager**
- Renderização OLED
- Interface do usuário
- Atualização em tempo real

#### 5. **DataStreamer**
- Envio de dados via WiFi
- Formato JSON
- Buffer management

## 🛠️ Ambiente de Desenvolvimento

### Pré-requisitos

#### Software
- **PlatformIO**: Build system e gerenciamento de dependências
- **VS Code**: IDE recomendado com extensão PlatformIO
- **Git**: Controle de versão
- **Python**: Requerido pelo PlatformIO

#### Hardware
- **ESP32 Development Board**: Para testes reais
- **Display OLED**: Para testes de UI
- **Módulo I2S**: Para testes de áudio
- **Protoboard**: Para montagem rápida

### Configuração do Ambiente

#### 1. Instalação do PlatformIO
```bash
pip install platformio
pio upgrade
```

#### 2. Configuração do VS Code
```bash
# Instalar extensões
code --install-extension platformio.platformio-ide
```

#### 3. Clonar e Configurar
```bash
git clone <repository-url>
cd esp32-audio-pitch-engine
pio lib install
```

### Build System

#### Comandos Principais
```bash
# Compilação
pio run

# Upload para o dispositivo
pio run --target upload

# Monitor serial
pio device monitor

# Limpar build
pio run --target clean

# Testes
pio test
```

#### Configurações do platformio.ini
```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
build_flags =
    -DCORE_DEBUG_LEVEL=0
    -DCONFIG_ARDUHAL_LOG_COLORS
lib_deps =
    bblanchon/ArduinoJson @ ^6.21.0
    adafruit/Adafruit SSD1306 @ ^2.5.7
    adafruit/Adafruit GFX Library @ ^1.11.5
```

## 🔄 Fluxo de Trabalho

### 1. **Setup Inicial**
```bash
# Criar branch de feature
git checkout -b feature/nova-funcionalidade

# Configurar ambiente
pio run
```

### 2. **Desenvolvimento**
- Editar código fonte
- Testes locais
- Validação com hardware

### 3. **Testes**
```bash
# Compilar
pio run

# Testar no hardware
pio run --target upload
pio device monitor

# Executar testes unitários
pio test
```

### 4. **Commit e Push**
```bash
git add .
git commit -m "feat: adiciona nova funcionalidade"
git push origin feature/nova-funcionalidade
```

### 5. **Pull Request**
- Abrir PR no GitHub
- Aguardar review
- Merge na main

## 📝 Padrões de Código

### Convenções de Nomenclatura

#### Classes e Structs
```cpp
class NoiseFilter {
    // PascalCase
};

struct WiFiDataEntry {
    // PascalCase
};
```

#### Funções e Métodos
```cpp
void setupAudioProcessor();
float calculatePitch();
bool isSignalActive();
// camelCase
```

#### Variáveis e Constantes
```cpp
int bufferSize;
float frequency;
const int MAX_SAMPLES = 1024;
// camelCase para variáveis
// UPPER_CASE para constantes
```

#### Arquivos
```cpp
// Headers: PascalCase.h
// Sources: PascalCase.cpp
NoiseFilter.h
NoiseFilter.cpp
```

### Formatação de Código

#### Indentação
- **Tamanho**: 4 espaços
- **Sem tabs**: Apenas espaços
- **Consistência**: Mantenha padrão existente

#### Comentários
```cpp
// Comentário de linha simples
/* 
 * Comentário de bloco
 * para explicações detalhadas
 */

/**
 * @brief Descrição breve da função
 * @param param1 Descrição do parâmetro 1
 * @return Descrição do retorno
 * @note Informações adicionais
 */
float calculateFrequency(int param1);
```

#### Organização de Arquivos
```cpp
// 1. Includes
#include <Arduino.h>
#include "utils/Logger.h"

// 2. Defines e constantes
#define BUFFER_SIZE 1024
const float THRESHOLD = 0.5;

// 3. Variáveis globais
float globalVariable;

// 4. Funções
void setup() { }
void loop() { }
```

### Melhores Práticas

#### 1. **Memory Management**
```cpp
// Evitar alocação dinâmica
static float buffer[BUFFER_SIZE];

// Usar stack quando possível
void processAudio() {
    float localBuffer[256];
    // Processamento
}
```

#### 2. **Error Handling**
```cpp
if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display initialization failed");
    return false;
}
```

#### 3. **Constants vs Magic Numbers**
```cpp
// Ruim
if (frequency > 440.0) { }

// Bom
const float A4_FREQUENCY = 440.0;
if (frequency > A4_FREQUENCY) { }
```

#### 4. **Debug Logging**
```cpp
#ifdef DEBUG
Serial.println("Debug: Processing audio sample");
#endif
```

## 🧪 Testes e Validação

### Tipos de Testes

#### 1. **Unit Tests**
```cpp
// test/test_pitch_detector.cpp
#include <unity.h>
#include "../src/utils/PitchDetector.h"

void test_frequency_to_note() {
    PitchDetector detector;
    String note = detector.frequencyToNote(440.0);
    TEST_ASSERT_EQUAL_STRING("A4", note.c_str());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_frequency_to_note);
    UNITY_END();
}
```

#### 2. **Integration Tests**
- Teste completo do sistema
- Integração hardware-software
- Performance em tempo real

#### 3. **Hardware Tests**
- Validação com dispositivos reais
- Testes de conectividade
- Consumo de energia

### Framework de Testes

#### Unity Testing Framework
```ini
# platformio.ini
test_framework = unity
test_build_project_src = true
build_flags =
    -DUNIT_TEST
```

#### Test Commands
```bash
# Executar todos os testes
pio test

# Teste específico
pio test -f test_pitch_detector

# Verbose output
pio test -v
```

### Testes de Performance

#### Métricas Importantes
- **Latency**: < 100ms para detecção
- **Memory**: < 80% do heap disponível
- **CPU**: < 70% de utilização
- **Power**: < 200mA em operação

#### Benchmark Scripts
```cpp
void benchmarkPitchDetection() {
    unsigned long start = micros();
    
    for (int i = 0; i < 1000; i++) {
        float pitch = detector.calculatePitch();
    }
    
    unsigned long end = micros();
    Serial.printf("Average time: %lu us\n", (end - start) / 1000);
}
```

## 🐛 Debug e Diagnóstico

### Sistema de Logging

#### Níveis de Log
```cpp
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};
```

#### Uso do Logger
```cpp
#include "utils/Logger.h"

Logger logger;

void setup() {
    logger.setLevel(LOG_INFO);
    logger.info("System starting...");
    
    if (error) {
        logger.error("Initialization failed");
    }
}
```

### Debug Dashboard

#### Serial Debug Interface
```cpp
#include "utils/DebugDashboard.h"

DebugDashboard dashboard;

void loop() {
    dashboard.update();
    dashboard.showMetrics();
    dashboard.showAudioBuffer();
}
```

#### Métricas Disponíveis
- **Memory Usage**: Heap livre/usado
- **CPU Load**: Porcentagem de utilização
- **Audio Buffer**: Status do buffer
- **Pitch History**: Histórico de detecções
- **Noise Floor**: Nível de ruído

### Ferramentas de Debug

#### 1. **Serial Monitor**
```bash
pio device monitor -b 115200 --filter esp32_exception_decoder
```

#### 2. **Logic Analyzer**
- Análise de sinais I2S
- Timing de comunicação I2C
- Debug de protocolos

#### 3. **Oscilloscope**
- Qualidade do sinal de áudio
- Nível de ruído
- Resposta em frequência

## 🤝 Contribuição

### Tipos de Contribuições

#### 1. **Bug Fixes**
- Reportar bugs com detalhes
- Fornecer steps para reprodução
- Incluir logs e screenshots

#### 2. **Features**
- Propor novas funcionalidades
- Discutir implementação
- Documentar mudanças

#### 3. **Documentation**
- Melhorar docs existentes
- Adicionar exemplos
- Traduções

#### 4. **Testing**
- Escrever testes
- Reportar falhas
- Melhorar coverage

### Processo de Contribuição

#### 1. **Preparação**
```bash
# Fork no GitHub
git clone https://github.com/your-username/esp32-audio-pitch-engine.git
cd esp32-audio-pitch-engine
git remote add upstream https://github.com/original-owner/esp32-audio-pitch-engine.git
```

#### 2. **Desenvolvimento**
```bash
# Criar branch
git checkout -b fix/audio-bug

# Fazer mudanças
# Testar thoroughly
```

#### 3. **Submit**
```bash
# Commit
git commit -m "fix: resolve audio processing issue"

# Push
git push origin fix/audio-bug

# Abrir PR
```

### Code Review

#### Checklist para Review
- [ ] Código segue padrões
- [ ] Testes incluídos
- [ ] Documentação atualizada
- [ ] Sem regressões
- [ ] Performance aceitável

#### Processo de Review
1. Auto-review do autor
2. Review por maintainer
3. Feedback e ajustes
4. Aprovação e merge

## 📚 Recursos Adicionais

### Documentação
- [📖 Guia do Usuário](../user/README.md)
- [🔧 Referência da API](../api/API_REFERENCE.md)
- [🏗️ Arquitetura](../user/ARCHITECTURE.md)

### Comunidade
- **GitHub Discussions**: Discussões técnicas
- **Discord**: Chat em tempo real
- **Stack Overflow**: Suporte geral

### Ferramentas
- **PlatformIO Registry**: Bibliotecas
- **ESP32 Documentation**: Docs oficiais
- **Arduino Reference**: API reference

---

**Pronto para contribuir? Comece com uma [issue](https://github.com/your-repo/issues) ou abra um PR!**
