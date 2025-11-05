# ESP32 WROOM-32 - PN532 RFID Reader with PostgreSQL

Projeto completo de leitor RFID/NFC usando ESP32 WROOM-32 com módulo PN532, integrado com PostgreSQL via API REST ou conexão direta.

## Hardware

- **ESP32 WROOM-32** (placa de desenvolvimento)
- **PN532 NFC/RFID Module** (via I2C)
- **Cartões RFID/NFC** (MIFARE Classic, NTAG, etc.)

## Pinout

- **SDA (I2C)**: GPIO 21
- **SCL (I2C)**: GPIO 19
- **LED Onboard**: GPIO 2

## Funcionalidades

- Leitura de cartões RFID/NFC via PN532
- Leitura de blocos de dados do cartão (plate, vehicle, department)
- Envio automático para PostgreSQL via API REST (HTTPS)
- Fallback automático para conexão direta PostgreSQL
- Sistema de auto-recovery I2C com heartbeat
- Indicador visual com LED onboard
- Retry automático em caso de falhas
- Configuração otimizada para velocidade máxima

## Estrutura do Projeto

```
src/
├── main.cpp              - Loop principal e orquestração
├── rfid_handler.h/cpp    - Gerenciamento do PN532 e leitura de cartões
├── postgres_handler.h/cpp - Comunicação com PostgreSQL (API + direto)
├── GPIOS.h               - Definições de pinos
├── config.h              - Configurações (WiFi, API, Database)
└── config_h_template    - Template de configuração
```

## Configuração

1. Copie `src/config_h_template` para `src/config.h`
2. Configure as credenciais:

```cpp
// WiFi
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// PostgreSQL API (prioridade)
#define POSTGRES_API_URL "https://your-api.com/api/rfid-readings"

// PostgreSQL Direct Connection (fallback)
#define DATABASE_PUBLIC_URL "postgresql://user:password@host:port/database"

// Device
#define DEVICE_LOCATION "Your Location"
```

## Instalação

1. Clone o repositório
2. Instale as dependências do PlatformIO:
   ```bash
   pio lib install
   ```
3. Configure `src/config.h` com suas credenciais
4. Faça upload para o ESP32:
   ```bash
   pio run -t upload
   ```

## Bibliotecas Utilizadas

- `adafruit/Adafruit PN532@^1.3.1` - Driver para módulo PN532
- `bblanchon/ArduinoJson@^6.21.3` - Parsing JSON para comunicação API

## Comportamento

### LED Onboard
- **Apagado**: Aguardando cartão ou sistema não inicializado
- **Aceso**: Cartão detectado e processando (leitura + envio)
- **Apaga**: Processamento concluído, pronto para próximo cartão

### Fluxo de Dados
1. Cartão detectado → LED acende
2. Leitura UID e blocos (4, 5, 6)
3. Envio imediato para PostgreSQL (via API ou conexão direta)
4. LED apaga após envio
5. Aguarda próximo cartão

### Sistema de Recovery
- **Heartbeat I2C**: Mantém barramento ativo a cada 30s
- **Auto-recovery**: Reinicializa I2C após 60s sem sucesso
- **Retry init**: Tenta reinicializar PN532 a cada 30s se falhar no setup

## API Endpoint

O ESP32 envia dados via POST para `/api/rfid-readings`:

```json
{
  "card_uid": "36:45:42:02",
  "card_type": "MIFARE Classic 1K",
  "reader_name": "Vila Mariana",
  "metadata": {
    "location": "Vila Mariana",
    "plate": "ABC1234",
    "vehicle": "Carro",
    "department": "TI"
  }
}
```

## Banco de Dados

O sistema insere na tabela `rfid_readings`:

- `id`: SERIAL PRIMARY KEY
- `card_uid`: VARCHAR(50) NOT NULL
- `card_type`: VARCHAR(50)
- `reader_name`: VARCHAR(100)
- `metadata`: JSONB
- `created_at`: TIMESTAMP

## Troubleshooting

### PN532 não inicializa
- Verifique conexões I2C (SDA=GPIO21, SCL=GPIO19)
- Confirme alimentação 3.3V (não 5V)
- Verifique pull-up resistors (4.7kΩ)

### Erros I2C após inatividade
- Sistema tem auto-recovery automático
- Se persistir, verifique qualidade da fiação

### PostgreSQL falha
- Verifique conectividade WiFi
- Confirme URL da API ou conexão direta
- Logs de debug mostram detalhes do erro

## Licença

Este projeto está sob licença livre para uso pessoal e educacional.
