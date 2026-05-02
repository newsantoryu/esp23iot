# ESP32 Audio Pitch Engine - Arquitetura do Sistema

## Visão Geral

O ESP32 Audio Pitch Engine é um sistema embarcado de detecção de pitch musical em tempo real, desenvolvido para ESP32 com display OLED. O sistema utiliza processamento digital de sinal para detectar frequências fundamentais e convertê-las em notas musicais com precisão de cents.

## Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32 Audio Pitch Engine             │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │ Audio Input │  │ Signal      │  │ Pitch       │     │
│  │ (I2S)        │→│ Processing  │→│ Detection   │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │ Note         │  │ Display     │  │ Log         │     │
│  │ Analysis     │→│ Management   │→│ System      │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │ Data         │  │ User        │  │ System      │     │
│  │ Storage     │  │ Interface   │→│ Config      │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
└─────────────────────────────────────────────────────────┘
```

## Componentes Principais

### 1. AudioProcessor
**Responsabilidade:** Processamento de áudio bruto e filtragem
- **Entrada:** Stream I2S (1024 samples @ 16kHz)
- **Saída:** Sinal filtrado e normalizado
- **Algoritmos:** Filtro passa-baixa RC, normalização de amplitude

### 2. PitchDetector
**Responsabilidade:** Detecção de frequência fundamental
- **Algoritmo:** Zero-cross detection melhorado
- **Precisão:** ±5 cents
- **Range:** 60-800Hz
- **Validação:** SNR, coerência espectral, persistência

### 3. NoteAnalyzer
**Responsabilidade:** Conversão frequência → nota musical
- **Referência:** A4 = 432Hz (padrão alternativo)
- **Cálculo:** MIDI note number + oitava
- **Saída:** Nota + oitava + cents

### 4. DisplayManager
**Responsabilidade:** Interface OLED 128x64
- **Update rate:** ≤10Hz
- **Layout:** Nota central + informações auxiliares
- **Features:** Indicador de estabilidade, padrão 432Hz

### 5. LogSystem
**Responsabilidade:** Logging estruturado e persistente
- **Níveis:** ERROR, INFO, DEBUG, PITCH
- **Storage:** CSV + buffer circular
- **Features:** Rate limiting, rotação, análise

### 6. DataStorage
**Responsabilidade:** Armazenamento persistente
- **Sistema:** LittleFS (ESP32)
- **Formato:** CSV estruturado
- **Features:** Backup, compactação, consulta

## Fluxo de Dados

```
I2S Input → AudioProcessor → NoiseFilter → PitchDetector → NoteAnalyzer → DisplayManager
     ↓              ↓              ↓              ↓              ↓              ↓
   Raw Audio   Filtered     Valid         Frequency      Musical        Visual
   1024 samples Signal      Signal        Pitch           Note           Feedback
```

## Padrões de Design Aplicados

### 1. Singleton Pattern
- **Logger:** Instância global de logging
- **Config:** Configurações centralizadas
- **Display:** Gerenciador único de display

### 2. Strategy Pattern
- **PitchAlgorithm:** Diferentes algoritmos de detecção
- **FilterStrategy:** Diferentes filtros de sinal
- **StorageStrategy:** Diferentes métodos de armazenamento

### 3. Observer Pattern
- **LogListeners:** Múltiplos destinos de logs
- **DisplayListeners:** Atualizações de UI
- **DataListeners:** Eventos de dados

## Especificações Técnicas

### Performance
- **Latência total:** <200ms
- **Taxa de atualização:** ≤10Hz
- **Uso de RAM:** <10% (32KB)
- **Uso de Flash:** <25% (320KB)
- **Precisão:** ±5 cents

### Hardware
- **Microcontrolador:** ESP32-D0WD-V3
- **Clock:** 240MHz
- **RAM:** 320KB
- **Flash:** 4MB
- **Display:** OLED SSD1306 128x64

### Software
- **Framework:** Arduino ESP32
- **Compilador:** xtensa-esp32-elf-gcc
- **Build System:** PlatformIO
- **File System:** LittleFS

## Configuração

### Dependências
```
- ArduinoJson: 6.21.6
- Adafruit SSD1306: 2.5.16
- Adafruit GFX Library: 1.12.6
```

### Build Configuration
```
- Build Type: Release
- Optimization: -Os
- Stack Size: 8KB
- Heap Size: 24KB
```

## Métricas de Qualidade

### Código
- **Cobertura de testes:** >80%
- **Complexidade ciclomática:** <10
- **Debt técnico:** <2 dias
- **Documentação:** 100% de APIs públicas

### Performance
- **Throughput:** 1024 samples/processamento
- **Latency:** <200ms end-to-end
- **Memory:** <32KB RAM utilizada
- **Power:** <100mA @ 3.3V

## Interfaces

### Hardware Interfaces
```
I2S_WS   → GPIO25 (Word Select)
I2S_SD   → GPIO34 (Serial Data)
I2S_SCK  → GPIO26 (Serial Clock)
LED_PIN  → GPIO14 (Status LED)
OLED_SDA → GPIO21 (I2C Data)
OLED_SCL → GPIO22 (I2C Clock)
```

### Software Interfaces
```
AudioProcessor::process() → AudioBuffer
PitchDetector::detect() → Frequency
NoteAnalyzer::analyze() → MusicalNote
DisplayManager::update() → void
Logger::log() → void
DataStorage::save() → void
```

## Estrutura de Arquivos

```
src/
├── main.cpp                 # Entry point e loop principal
├── core/
│   ├── AudioProcessor.h/cpp # Processamento de áudio
│   ├── PitchDetector.h/cpp  # Detecção de pitch
│   └── NoteAnalyzer.h/cpp   # Análise musical
├── utils/
│   ├── Logger.h/cpp         # Sistema de logs
│   ├── NoiseFilter.h/cpp    # Filtragem de ruído
│   └── DataStorage.h/cpp    # Armazenamento de dados
├── interfaces/
│   ├── DisplayManager.h/cpp # Gerenciamento de display
│   └── ConfigManager.h/cpp  # Configurações
└── tests/
    ├── TestAudioProcessor.cpp
    ├── TestPitchDetector.cpp
    └── TestNoteAnalyzer.cpp
```

## Decisões de Design

### 1. Zero-cross Detection vs FFT
**Decisão:** Zero-cross detection
**Razão:** Menor uso de CPU, adequado para tempo real, precisão suficiente

### 2. Padrão 432Hz vs 440Hz
**Decisão:** 432Hz (configurável)
**Razão:** Padrão alternativo solicitado, fácil mudança via configuração

### 3. Buffer Size
**Decisão:** 1024 samples
**Razão:** Melhor resolução frequencial, uso aceitável de memória

### 4. Update Rate
**Decisão:** 10Hz máximo
**Razão:** Estabilidade visual, performance adequada

## Trade-offs

### Performance vs Precisão
- **Performance:** Zero-cross detection (CPU eficiente)
- **Precisão:** ±5 cents (adequado para aplicações musicais)

### Memória vs Funcionalidade
- **Memória:** <32KB RAM (conservador)
- **Funcionalidade:** Features completas mantidas

### Latência vs Estabilidade
- **Latência:** <200ms (tempo real)
- **Estabilidade:** Sistema de confiança gradual

## Evolução Futura

### Versão 2.0
- Algoritmo YIN para maior precisão
- Interface web para configuração
- Suporte a múltiplas fontes de áudio

### Versão 3.0
- Sistema distribuído
- Cloud integration
- Machine learning para classificação

## Manutenção

### Debug
- Logs estruturados com diferentes níveis
- Indicadores visuais de status
- Sistema de diagnóstico automático

### Atualização
- OTA updates suportados
- Configuração persistente
- Backup automático

### Monitoramento
- Métricas de performance
- Logs de erro
- Estatísticas de uso
