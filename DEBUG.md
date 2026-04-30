# Debug Guide - ESP32 Audio Pitch Engine

## Sistema de Logs Implementado

O sistema agora possui logs detalhados para debug do display e filtragem de ruído.

### Níveis de Log
- **LOG_DEBUG**: Logs detalhados de áudio, pitch e display
- **LOG_INFO**: Logs gerais do sistema
- **LOG_ERROR**: Logs de erros

### Como Usar

#### 1. Monitor Serial
```bash
# Conecte ao monitor serial
pio device monitor -b 115200
```

#### 2. Logs Disponíveis

**Logs de Áudio (AUDIO):**
```
[AUDIO] Amp: 123.45 Norm: 0.38 Filtered: 115.23 Active: YES NoiseFloor: 12.34
```

**Logs de Pitch (PITCH):**
```
[PITCH] Freq: 440.2 Current: A4 Locked: A4
[PITCH] Note change #5: G4 -> A4
[PITCH] Note changed to: A4
[PITCH] Note locked: A4
[PITCH] Frequency out of range
```

**Logs de Display (DISPLAY):**
```
[DISPLAY] UI update: Hz=440 Note=A4 Cents=+3 Stable=YES
```

#### 3. Interpretação dos Logs

**Amplitude e Ativação:**
- `Amp`: Amplitude bruta do buffer
- `Norm`: Amplitude normalizada (0-1)
- `Filtered`: Amplitude média filtrada
- `Active`: YES/NO se o sinal está ativo
- `NoiseFloor`: Nível de ruído detectado

**Estabilidade de Notas:**
- Contador de mudanças por segundo
- Threshold de estabilidade aumentado para 5
- Histerese de 3 detecções para mudanças

## Melhorias de Filtragem

### 1. Threshold Aumentado
- **Antes**: 0.25 (muito sensível)
- **Agora**: 0.35 (mais robusto)

### 2. Histerese Implementada
- **Ativação**: > 0.40
- **Desativação**: < 0.30
- Evita flicker entre ativo/inativo

### 3. Filtro de Amplitude
- Média móvel de 10 amostras
- Reduz picos de ruído
- Noise floor adaptativo

## Testes Sugeridos

### 1. Ambiente Silencioso
- Verifique se `Active: NO` consistentemente
- Confirme ausência de mudanças de nota
- Observe `NoiseFloor` baixo (~10-20)

### 2. Ruído Ambiente
- Monitore `Amp` e `Norm` valores
- Verifique se `Active` permanece `NO`
- Confirme que notas não mudam

### 3. Som Intencional
- Teste com instrumento/vocal
- Observe sequência: detected -> changed -> locked
- Verifique estabilidade da nota final

## Parâmetros Ajustáveis

### NoiseFilter (src/utils/NoiseFilter.h)
```cpp
float activityThreshold = 0.35;  // Sensibilidade
float hysteresis = 0.05;         // Margem de histerese
```

### Logger (src/utils/Logger.cpp)
```cpp
Logger::setLevel(LOG_DEBUG);     // Mudar para LOG_INFO em produção
```

### Estabilidade (src/main.cpp)
```cpp
const int NOTE_STABILITY_THRESHOLD = 5;  // Requer 5 detecções estáveis
const int NOTE_CHANGE_THRESHOLD = 3;     // Requer 3 detecções para mudar
```

## Troubleshooting

### Problema: Notas ainda alternam
**Solução:**
1. Aumente `activityThreshold` para 0.40
2. Aumente `NOTE_STABILITY_THRESHOLD` para 7
3. Verifique logs de `Amp` e `NoiseFloor`

### Problema: Não detecta som fraco
**Solução:**
1. Reduza `activityThreshold` para 0.30
2. Reduza histerese para 0.03
3. Verifique ganho do microfone

### Problema: Muitos logs
**Solução:**
1. Mude `Logger::setLevel(LOG_INFO)`
2. Aumente `LOG_RATE_LIMIT` para 100ms

## Comandos Úteis

```bash
# Compilar e upload
pio run -t upload

# Monitor serial
pio device monitor -b 115200

# Limpar build
pio run -t clean

# Verificar memória
pio run -t checkprogsize
```
