# ESP32 Audio Pitch Engine

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-blue.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Arduino](https://img.shields.io/badge/Framework-Arduino-orange.svg)](https://www.arduino.cc/)

Um sistema embarcado de detecção de pitch musical em tempo real desenvolvido para ESP32 com display OLED. Utiliza processamento digital de sinal para detectar frequências fundamentais e convertê-las em notas musicais com precisão de cents.

## 🎯 Características Principais

- **Detecção de Pitch em Tempo Real**: Algoritmo YIN otimizado para ESP32
- **Processamento de Áudio I2S**: Entrada de áudio digital de alta qualidade
- **Display OLED**: Interface visual com SSD1306 (128x64)
- **Filtragem Avançada de Ruído**: Redução inteligente de interferências
- **Streaming WiFi**: Envio de dados para servidores remotos
- **Sistema de Debug**: Dashboard completo para diagnóstico
- **Análise de Notas**: Detecção precisa de notas musicais com oitavas e cents
- **Configuração A4**: Suporte para diferentes padrões de afinação (432Hz, 440Hz)

## 📋 Requisitos de Hardware

### Componentes Essenciais
- **ESP32 Development Board** (ESP32-DOIT-DEVKIT-V1 ou compatível)
- **Display OLED SSD1306** (128x64 pixels, comunicação I2C)
- **Módulo de Áudio I2S** (microfone digital ou entrada de linha)
- **Fios Jumper** e **protoboard** para conexões

### Pinagem
```
LED_PIN     = 14    // LED indicador de atividade
I2S_WS      = 25    // Word Select (LRCK)
I2S_SD      = 34    // Serial Data
I2S_SCK     = 26    // Serial Clock
OLED_SDA    = 21    // I2C Data (padrão)
OLED_SCL    = 22    // I2C Clock (padrão)
```

## 🚀 Instalação e Configuração

### 1. Clonar o Repositório
```bash
git clone https://github.com/newsantoryu/esp23iot.git
cd esp23iot
```

### 2. Instalar PlatformIO
```bash
pip install platformio
```

### 3. Compilar e Enviar
```bash
pio run --target upload
pio device monitor
```

### 4. Configuração do WiFi
Edite `src/main.cpp` e configure suas credenciais WiFi:
```cpp
const char* ssid = "SUA_REDE_WIFI";
const char* password = "SUA_SENHA";
```

## 📖 Uso

### Operação Básica
1. Conecte o hardware conforme a pinagem
2. Ligue o ESP32
3. Aguarde a inicialização do display OLED
4. Toque um instrumento ou crie perto do microfone
5. Observe a nota detectada no display

### Interface do Display
- **Nota Musical**: Nota atual com oitava (ex: A4, C3)
- **Frequência**: Valor em Hz da nota detectada
- **Cents**: Desvio em cents da afinação perfeita
- **Indicador LED**: Acende quando há atividade de áudio
- **Barra de Volume**: Visualização da amplitude do sinal

### Streaming de Dados
O sistema pode enviar dados via WiFi para análise remota:
```json
{
  "device_id": "esp32_pitch_001",
  "timestamp": 1634567890,
  "frequency": 440.2,
  "note": "A",
  "octave": 4,
  "cents": 3.5,
  "confidence": 0.95,
  "stable": true
}
```

## 🔧 Configuração Avançada

### Parâmetros de Áudio
```cpp
#define BUFFER_LEN 1024        // Tamanho do buffer de áudio
#define A4_FREQ 432.0         // Frequência de referência A4
float activityThreshold = 0.35; // Sensibilidade de detecção
```

### Filtros e Processamento
- **Filtro de Ruído**: Redução de interferências ambientais
- **Validação SNR**: Rejeição de sinais de baixa qualidade
- **Histerese**: Prevenção de flicker na detecção
- **Média Móvel**: Suavização de leituras

## 📚 Documentação

- [📖 Guia do Usuário](docs/user/README.md)
- [🔧 Referência da API](docs/api/API_REFERENCE.md)
- 🏗️ [Arquitetura do Sistema](docs/user/ARCHITECTURE.md)
- 👨‍💻 [Guia de Desenvolvimento](docs/development/README.md)
- 🐛 [Histórico de Correções](docs/development/)

## 🧪 Testes e Validação

### Testes de Precisão
- Frequências testadas: 80Hz - 800Hz
- Precisão: ±5 cents
- Tempo de resposta: <100ms
- Taxa de atualização: 10Hz

### Testes de Ruído
- Rejeição de ruído ambiente: >90%
- Limiar de ativação configurável
- Filtro adaptativo automático

## 🤝 Contribuição

Contribuições são bem-vindas! Por favor:

1. Fork o projeto
2. Crie uma branch (`git checkout -b feature/nova-funcionalidade`)
3. Commit suas mudanças (`git commit -am 'Adiciona nova funcionalidade'`)
4. Push para a branch (`git push origin feature/nova-funcionalidade`)
5. Abra um Pull Request

### Diretrizes de Contribuição
- Siga o padrão de código existente
- Adicione comentários para funções complexas
- Teste suas modificações
- Atualize a documentação quando necessário

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 🙏 Agradecimentos

- **Algoritmo YIN**: Fundação para detecção de pitch
- **ESP32 Arduino Core**: Framework de desenvolvimento
- **Adafruit GFX Library**: Renderização gráfica
- **PlatformIO**: Build system e gerenciamento de dependências

## 📞 Suporte

Para dúvidas e suporte:
- Abra uma **Issue** no GitHub
- Consulte a [documentação](docs/)
- Verifique o [guia de desenvolvimento](docs/development/)

---

**Desenvolvido com ❤️ para a comunidade de música e tecnologia**
