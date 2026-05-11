# Guia do Usuário - ESP32 Audio Pitch Engine

Bem-vindo ao guia completo do usuário para o sistema de detecção de pitch musical em tempo real para ESP32.

## 📋 Índice

- [Visão Geral](#visão-geral)
- [Instalação Rápida](#instalação-rápida)
- [Configuração de Hardware](#configuração-de-hardware)
- [Operação do Sistema](#operação-do-sistema)
- [Interface do Display](#interface-do-display)
- [Configuração Avançada](#configuração-avançada)
- [Solução de Problemas](#solução-de-problemas)

## 🎯 Visão Geral

O ESP32 Audio Pitch Engine é um dispositivo portátil que detecta notas musicais em tempo real com alta precisão. Ideal para:

- Músicos que desejam afinar instrumentos
- Professores de música para aulas práticas
- Estudantes de teoria musical
- Desenvolvedores de aplicações musicais

## ⚡ Instalação Rápida

### Requisitos Mínimos
- ESP32 Development Board
- Display OLED SSD1306 (128x64)
- Microfone I2S ou entrada de áudio
- Cabos de conexão
- Computador com PlatformIO instalado

### Passos para Instalação

1. **Montagem do Hardware**
   ```
   ESP32 → Display OLED
   GPIO21 → SDA
   GPIO22 → SCL
   3.3V  → VCC
   GND   → GND
   
   ESP32 → Módulo I2S
   GPIO25 → WS
   GPIO26 → SCK
   GPIO34 → SD
   3.3V  → VCC
   GND   → GND
   ```

2. **Configuração do Software**
   ```bash
   # Clonar repositório
   git clone <repository-url>
   cd esp32-audio-pitch-engine
   
   # Compilar e enviar
   pio run --target upload
   pio device monitor
   ```

3. **Primeira Execução**
   - Conecte o ESP32 via USB
   - Aguarde a inicialização do display
   - O LED verde deve piscar indicando funcionamento

## 🔧 Configuração de Hardware

### Diagrama de Conexões

```
    ESP32-DOIT-DEVKIT-V1
    ┌─────────────────────┐
    │                     │
    │  [USB]  [EN] [3V3]  │
    │                     │
    │  GND  GPIO23 GPIO22 │→ OLED_SCL
    │  GPIO34 GPIO35 GPIO25 │→ I2S_WS
    │  GPIO26 GPIO27 GPIO14 │→ LED_PIN
    │  GPIO12 GPIO13 GPIO15 │
    │  GPIO2  GPIO0  GPIO4  │
    │  GPIO16 GPIO17 GPIO5  │
    │  GPIO18 GPIO19 GPIO21 │→ OLED_SDA
    │  GPIO21 GPIO22 GPIO23 │
    │  GND   TX0   RX0     │
    └─────────────────────┘
```

### Componentes Recomendados

#### Display OLED
- **Modelo**: SSD1306 128x64 pixels
- **Comunicação**: I2C
- **Tensão**: 3.3V - 5V
- **Endereço I2C**: 0x3C (padrão)

#### Módulo de Áudio I2S
- **Microfones**: INMP441, SPH0645
- **Taxa de Amostragem**: 16kHz
- **Resolução**: 16-bit
- **Formato**: I2S padrão

## 🎮 Operação do Sistema

### Modos de Operação

#### 1. **Modo Standalone**
- Funcionamento independente
- Display mostra informações em tempo real
- Ideal para afinação de instrumentos

#### 2. **Modo Streaming**
- Envia dados via WiFi
- Integração com aplicações externas
- Análise e armazenamento em nuvem

#### 3. **Modo Debug**
- Informações detalhadas no Serial Monitor
- Diagnóstico de problemas
- Calibração do sistema

### Controles e Indicadores

#### LED Indicador
- **Apagado**: Sem sinal de áudio
- **Verde Piscando**: Áudio detectado, processando
- **Vermelho Contínuo**: Erro no sistema

#### Botões (se disponíveis)
- **Boot**: Reset do sistema
- **Enable**: Ativa/desativa detecção

## 📱 Interface do Display

### Layout Principal

```
┌─────────────────────┐
│  A4  440.2Hz  +3.5c │  ← Nota, Frequência, Cents
│                     │
│  ████████████░░░░░  │  ← Barra de Afinação
│       ●              │  ← Indicador de Afinação
│                     │
│ Volume: ████████    │  ← Nível do Sinal
│ Noise: ██░░░░░░░░░  │  ← Nível de Ruído
│                     │
│ WiFi: ●  Bat: 85%   │  ← Status do Sistema
└─────────────────────┘
```

### Interpretação das Informações

#### Nota Musical
- **Formato**: [Nota][Oitava] (ex: A4, C3)
- **Precisão**: ±5 cents
- **Range**: C2 a B6

#### Cents
- **Positivo (+)**: Nota acima da afinação
- **Negativo (-)**: Nota abaixo da afinação
- **Zero (0)**: Perfeitamente afinado

#### Barra de Afinação
- **Central (●)**: Afinação perfeita
- **Direita**: Agudo demais
- **Esquerda**: Grave demais

## ⚙️ Configuração Avançada

### Parâmetros de Áudio

#### Sensibilidade
```cpp
// Ajuste no main.cpp
float activityThreshold = 0.35;  // 0.1 - 0.8
```
- **Baixo (0.1)**: Detecta sons suaves
- **Alto (0.8)**: Apenas sons fortes

#### Frequência de Referência
```cpp
#define A4_FREQ 432.0  // 440.0 para padrão moderno
```
- **432 Hz**: Afinação clássica/verdi
- **440 Hz**: Padrão internacional

### Configuração WiFi

#### Rede Local
```cpp
const char* ssid = "SUA_REDE_WIFI";
const char* password = "SUA_SENHA_WIFI";
```

#### Servidor Remoto
```cpp
String serverURL = "http://servidor.com/api/pitch";
```

### Filtros e Processamento

#### Filtro de Ruído
- **Threshold**: Limiar de ativação
- **Histerese**: Prevenção de flicker
- **Noise Floor**: Detecção automática de ruído

#### Validação de Sinal
- **SNR**: Relação sinal/ruído
- **Coerência**: Consistência da frequência
- **Amplitude**: Nível mínimo do sinal

## 🔧 Solução de Problemas

### Problemas Comuns

#### 1. **Display Não Liga**
**Causas Possíveis:**
- Conexões I2C incorretas
- Endereço I2C errado
- Falta de alimentação

**Soluções:**
```
# Verificar endereço I2C
i2cdetect -y 0

# Testar conexões
pio device monitor
```

#### 2. **Sem Detecção de Áudio**
**Causas Possíveis:**
- Microfone desconectado
- Configuração I2S incorreta
- Sensibilidade muito baixa

**Soluções:**
```cpp
// Aumentar sensibilidade
float activityThreshold = 0.15;

// Verificar conexões I2S
// GPIO25 → WS
// GPIO26 → SCK  
// GPIO34 → SD
```

#### 3. **Detecção Instável**
**Causas Possíveis:**
- Ruído ambiente excessivo
- Threshold muito baixo
- Interferência elétrica

**Soluções:**
```cpp
// Aumentar threshold
float activityThreshold = 0.45;

// Reduzir ganho
// Verificar aterramento
```

#### 4. **WiFi Não Conecta**
**Causas Possíveis:**
- Credenciais incorretas
- Fora do alcance
- Firewall bloqueando

**Soluções:**
```cpp
// Verificar credenciais
Serial.println("Conectando WiFi...");
Serial.println(ssid);

// Testar conectividade
ping servidor.com
```

### Diagnóstico Avançado

#### Serial Monitor
```bash
pio device monitor -b 115200
```

#### Informações de Debug
- **Buffer Status**: Estado do buffer de áudio
- **FFT Results**: Resultados da transformada
- **Pitch History**: Histórico de detecções
- **System Metrics**: Memória, CPU, temperatura

#### Calibração
1. Use um gerador de tons conhecido
2. Compare com frequência detectada
3. Ajuste parâmetros se necessário
4. Salve configuração

## 📞 Suporte Técnico

### Recursos Adicionais
- [📖 Documentação Completa](../README.md)
- [🔧 Referência da API](../api/API_REFERENCE.md)
- [🏗️ Arquitetura](ARCHITECTURE.md)
- [🐛 Histórico de Correções](../development/)

### Comunidade
- **GitHub Issues**: Reportar problemas
- **Fórum**: Discussões técnicas
- **Discord**: Chat em tempo real

### Contato
- **Email**: support@pitchengine.com
- **Wiki**: Documentação colaborativa
- **Tutorials**: Vídeos e guias

---

**Precisa de ajuda? Consulte nosso [guia de desenvolvimento](../development/) ou abra uma issue no GitHub!**
