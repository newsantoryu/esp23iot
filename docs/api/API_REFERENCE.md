# API Reference - ESP32 Audio Pitch Engine

## Visão Geral

Documentação completa das APIs públicas do sistema de detecção de pitch. Todas as interfaces estão documentadas com exemplos de uso, parâmetros, valores de retorno e considerações importantes.

## AudioProcessor

### Classe: AudioProcessor

Gerenciamento de processamento de áudio bruto e filtragem de sinal.

#### Construtor

```cpp
AudioProcessor(int sampleRate = 16000, int bufferSize = 1024);
```

**Parâmetros:**
- `sampleRate`: Taxa de amostragem em Hz (padrão: 16000)
- `bufferSize`: Tamanho do buffer de processamento (padrão: 1024)

**Exemplo:**
```cpp
AudioProcessor processor(16000, 1024);
```

#### Métodos Públicos

##### process()

Processa amostras de áudio brutas do I2S.

```cpp
AudioBuffer process(int32_t* samples, int count);
```

**Parâmetros:**
- `samples`: Array de amostras I2S brutas
- `count`: Número de amostras no array

**Retorno:** `AudioBuffer` com sinal processado

**Exemplo:**
```cpp
int32_t rawSamples[1024];
AudioBuffer processed = processor.process(rawSamples, 1024);
```

##### filter()

Aplica filtro passa-baixa RC ao sinal.

```cpp
float filter(float input, float cutoff = 800.0);
```

**Parâmetros:**
- `input`: Amostra de entrada
- `cutoff`: Frequência de corte em Hz (padrão: 800.0)

**Retorno:** Amostra filtrada

**Exemplo:**
```cpp
float filtered = processor.filter(rawSample, 800.0);
```

##### normalize()

Normaliza amplitude do sinal para range [-1.0, 1.0].

```cpp
float normalize(float input, float maxAmplitude);
```

**Parâmetros:**
- `input`: Amostra de entrada
- `maxAmplitude`: Amplitude máxima para normalização

**Retorno:** Amostra normalizada

**Exemplo:**
```cpp
float normalized = processor.normalize(sample, 32767.0);
```

## PitchDetector

### Classe: PitchDetector

Detecção de frequência fundamental usando algoritmo zero-cross melhorado.

#### Construtor

```cpp
PitchDetector(float minFreq = 60.0, float maxFreq = 800.0, float confidence = 0.7);
```

**Parâmetros:**
- `minFreq`: Frequência mínima detectável em Hz
- `maxFreq`: Frequência máxima detectável em Hz
- `confidence`: Nível mínimo de confiança

**Exemplo:**
```cpp
PitchDetector detector(60.0, 800.0, 0.7);
```

#### Métodos Públicos

##### detect()

Detecta frequência fundamental do buffer de áudio.

```cpp
float detect(const AudioBuffer& buffer);
```

**Parâmetros:**
- `buffer`: Buffer de áudio processado

**Retorno:** Frequência detectada em Hz, 0.0 se inválido

**Exemplo:**
```cpp
float frequency = detector.detect(audioBuffer);
if (frequency > 0.0) {
    // Frequência válida detectada
}
```

##### validate()

Valida qualidade do pitch detectado.

```cpp
bool validate(float freq, const AudioBuffer& buffer);
```

**Parâmetros:**
- `freq`: Frequência detectada
- `buffer`: Buffer de áudio para cálculo SNR

**Retorno:** `true` se pitch for válido

**Exemplo:**
```cpp
bool isValid = detector.validate(frequency, audioBuffer);
```

##### getConfidence()

Retorna nível de confiança da última detecção.

```cpp
float getConfidence() const;
```

**Retorno:** Nível de confiança [0.0, 1.0]

**Exemplo:**
```cpp
float confidence = detector.getConfidence();
```

## NoteAnalyzer

### Classe: NoteAnalyzer

Conversão de frequência para nota musical com oitava e cents.

#### Construtor

```cpp
NoteAnalyzer(float a4Freq = 432.0);
```

**Parâmetros:**
- `a4Freq`: Frequência de referência A4 em Hz

**Exemplo:**
```cpp
NoteAnalyzer analyzer(432.0);  // Padrão 432Hz
```

#### Métodos Públicos

##### analyze()

Analisa frequência e retorna nota musical completa.

```cpp
MusicalNote analyze(float frequency);
```

**Parâmetros:**
- `frequency`: Frequência em Hz

**Retorno:** Estrutura `MusicalNote` com nota, oitava e cents

**Exemplo:**
```cpp
MusicalNote note = analyzer.analyze(432.0);
// note.name = "A"
// note.octave = 4
// note.cents = 0.0
```

##### frequencyToNote()

Converte frequência para MIDI note number.

```cpp
int frequencyToNote(float frequency);
```

**Parâmetros:**
- `frequency`: Frequência em Hz

**Retorno:** MIDI note number (0-127)

**Exemplo:**
```cpp
int midiNote = analyzer.frequencyToNote(432.0);  // Retorna 69
```

##### calculateCents()

Calcula desvio em cents da frequência ideal.

```cpp
float calculateCents(float frequency, float idealFrequency);
```

**Parâmetros:**
- `frequency`: Frequência detectada
- `idealFrequency`: Frequência ideal da nota

**Retorno:** Desvio em cents

**Exemplo:**
```cpp
float cents = analyzer.calculateCents(440.0, 432.0);  // +31.8 cents
```

## DisplayManager

### Classe: DisplayManager

Gerenciamento do display OLED 128x64 com interface de usuário.

#### Construtor

```cpp
DisplayManager(int width = 128, int height = 64, int sda = 21, int scl = 22);
```

**Parâmetros:**
- `width`: Largura do display
- `height`: Altura do display
- `sda`: Pino I2C SDA
- `scl`: Pino I2C SCL

**Exemplo:**
```cpp
DisplayManager display(128, 64, 21, 22);
```

#### Métodos Públicos

##### initialize()

Inicializa display OLED.

```cpp
bool initialize();
```

**Retorno:** `true` se inicialização bem-sucedida

**Exemplo:**
```cpp
if (!display.initialize()) {
    Serial.println("Display initialization failed");
}
```

##### update()

Atualiza display com informações atuais.

```cpp
void update(const MusicalNote& note, float frequency, bool stable);
```

**Parâmetros:**
- `note`: Nota musical atual
- `frequency`: Frequência atual em Hz
- `stable`: Indicador de estabilidade

**Exemplo:**
```cpp
display.update(currentNote, currentFreq, isStable);
```

##### clear()

Limpa o display.

```cpp
void clear();
```

**Exemplo:**
```cpp
display.clear();
```

##### showMessage()

Exibe mensagem temporária no display.

```cpp
void showMessage(const String& message, unsigned long duration = 2000);
```

**Parâmetros:**
- `message`: Mensagem a exibir
- `duration`: Duração em milissegundos

**Exemplo:**
```cpp
display.showMessage("Initializing...", 2000);
```

## Logger

### Classe: Logger (Singleton)

Sistema de logging estruturado com múltiplos níveis e destinos.

#### Métodos Estáticos

##### setLevel()

Define nível mínimo de logging.

```cpp
static void setLevel(LogLevel level);
```

**Parâmetros:**
- `level`: Nível mínimo (LOG_NONE, LOG_ERROR, LOG_INFO, LOG_DEBUG)

**Exemplo:**
```cpp
Logger::setLevel(LOG_INFO);  // Apenas INFO e ERROR
```

##### error()

Log de nível ERROR.

```cpp
static void error(const String& message);
```

**Parâmetros:**
- `message`: Mensagem de erro

**Exemplo:**
```cpp
Logger::error("Failed to initialize I2S");
```

##### info()

Log de nível INFO.

```cpp
static void info(const String& message);
```

**Parâmetros:**
- `message`: Mensagem informativa

**Exemplo:**
```cpp
Logger::info("System initialized successfully");
```

##### debug()

Log de nível DEBUG.

```cpp
static void debug(const String& message);
```

**Parâmetros:**
- `message`: Mensagem de debug

**Exemplo:**
```cpp
Logger::debug("Processing sample: " + String(sample));
```

##### pitch()

Log específico para informações de pitch.

```cpp
static void pitch(const String& message);
```

**Parâmetros:**
- `message`: Mensagem sobre detecção de pitch

**Exemplo:**
```cpp
Logger::pitch("Frequency: " + String(freq) + " Note: " + note);
```

## DataStorage

### Classe: DataStorage

Armazenamento persistente de dados e logs.

#### Construtor

```cpp
DataStorage(const String& filename = "/data/logs.csv");
```

**Parâmetros:**
- `filename`: Nome do arquivo de armazenamento

**Exemplo:**
```cpp
DataStorage storage("/data/logs.csv");
```

#### Métodos Públicos

##### initialize()

Inicializa sistema de arquivos.

```cpp
bool initialize();
```

**Retorno:** `true` se inicialização bem-sucedida

**Exemplo:**
```cpp
if (!storage.initialize()) {
    Logger::error("Failed to initialize storage");
}
```

##### save()

Salva entrada de log.

```cpp
bool save(const LogEntry& entry);
```

**Parâmetros:**
- `entry`: Estrutura com dados do log

**Retorno:** `true` se salvamento bem-sucedido

**Exemplo:**
```cpp
LogEntry entry = {timestamp, freq, note, octave, cents, confidence};
storage.save(entry);
```

##### query()

Consulta logs com filtros.

```cpp
std::vector<LogEntry> query(const String& filter = "");
```

**Parâmetros:**
- `filter`: Filtro de consulta (opcional)

**Retorno:** Vetor de entradas de log

**Exemplo:**
```cpp
auto logs = storage.query("frequency>400");
```

##### clear()

Limpa todos os logs.

```cpp
bool clear();
```

**Retorno:** `true` se limpeza bem-sucedida

**Exemplo:**
```cpp
storage.clear();
```

## Estruturas de Dados

### AudioBuffer

```cpp
struct AudioBuffer {
    float* samples;      // Array de amostras
    int length;          // Número de amostras
    int sampleRate;      // Taxa de amostragem
    float amplitude;     // Amplitude máxima
};
```

### MusicalNote

```cpp
struct MusicalNote {
    String name;         // Nome da nota (A, B, C, etc.)
    int octave;          // Oitava (0-8)
    float cents;         // Desvio em cents
    float frequency;     // Frequência em Hz
    bool isValid;        // Indicador de validade
};
```

### LogEntry

```cpp
struct LogEntry {
    unsigned long timestamp;  // Timestamp em ms
    float frequency;          // Frequência detectada
    String note;              // Nome da nota
    int octave;               // Oitava
    float cents;               // Desvio em cents
    float confidence;          // Nível de confiança
    bool stable;              // Indicador de estabilidade
};
```

## Constantes e Configurações

### Frequências

```cpp
const float A4_FREQ = 432.0;           // Frequência de referência A4
const float MIN_PITCH_FREQ = 60.0;     // Frequência mínima detectável
const float MAX_PITCH_FREQ = 800.0;    // Frequência máxima detectável
```

### Thresholds

```cpp
const int NOTE_STABILITY_THRESHOLD = 18;  // Amostras para estabilidade
const int NOTE_CHANGE_THRESHOLD = 10;     // Amostras para mudança
const float MIN_CONFIDENCE = 0.7;         // Confiança mínima
```

### Timing

```cpp
const unsigned long NOTE_COOLDOWN = 500;      // Cooldown entre mudanças (ms)
const unsigned long DISPLAY_UPDATE_INTERVAL = 100;  // Update rate do display (ms)
const unsigned long LOG_RATE_LIMIT = 200;       // Rate limit de logs (ms)
```

## Exemplos de Uso

### Exemplo Completo

```cpp
#include "AudioProcessor.h"
#include "PitchDetector.h"
#include "NoteAnalyzer.h"
#include "DisplayManager.h"
#include "Logger.h"

void setup() {
    // Inicializar componentes
    AudioProcessor processor(16000, 1024);
    PitchDetector detector(60.0, 800.0, 0.7);
    NoteAnalyzer analyzer(432.0);
    DisplayManager display(128, 64, 21, 22);
    DataStorage storage("/data/logs.csv");
    
    // Configurar logging
    Logger::setLevel(LOG_INFO);
    
    // Inicializar hardware
    if (!display.initialize()) {
        Logger::error("Display initialization failed");
        return;
    }
    
    if (!storage.initialize()) {
        Logger::error("Storage initialization failed");
        return;
    }
    
    Logger::info("System initialized successfully");
}

void loop() {
    // Processar áudio
    int32_t rawSamples[1024];
    AudioBuffer audioBuffer = processor.process(rawSamples, 1024);
    
    // Detectar pitch
    float frequency = detector.detect(audioBuffer);
    if (frequency > 0.0 && detector.validate(frequency, audioBuffer)) {
        // Analisar nota
        MusicalNote note = analyzer.analyze(frequency);
        
        // Atualizar display
        display.update(note, frequency, detector.getConfidence() >= 0.7);
        
        // Salvar log
        LogEntry entry = {
            millis(),
            frequency,
            note.name,
            note.octave,
            note.cents,
            detector.getConfidence(),
            detector.getConfidence() >= 0.7
        };
        storage.save(entry);
        
        // Log de pitch
        Logger::pitch("Freq: " + String(frequency) + 
                     " Note: " + note.name + String(note.octave) + 
                     " Cents: " + String(note.cents));
    }
    
    delay(100);  // 10Hz update rate
}
```

## Considerações Importantes

### Performance
- Use `LOG_INFO` em produção para melhor performance
- Buffer de 1024 samples oferece bom balanço precisão/memória
- Rate limiting de logs previne spam

### Precisão
- Algoritmo zero-cross tem precisão ±5 cents
- Para maior precisão, considere algoritmo YIN
- Calibração regular recomendada

### Memória
- Uso total <32KB RAM
- Logs em buffer circular para economizar memória
- LittleFS para armazenamento persistente

### Confiabilidade
- Validação múltipla de pitch (SNR, coerência, persistência)
- Sistema de confiança gradual
- Cooldown entre mudanças de nota
