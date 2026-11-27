#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include "secrets.h"

// ==========================================
// 1. CONFIGURACIÓN DE REDES (Fallback)
// ==========================================
struct RedConfig {
  const char* ssid;
  const char* password;
  const char* mqtt_ip;
};

// Red 1 (Principal) y Red 2 (Respaldo): datos definidos en secrets.h
const RedConfig redes[2] = {
  {WIFI1_SSID, WIFI1_PASS, WIFI1_BROKER_IP},
  {WIFI2_SSID, WIFI2_PASS, WIFI2_BROKER_IP}
};

const int mqtt_port = 1883;
const char* mqtt_server = ""; // Se asigna dinámicamente al conectar

WiFiClient espClient;
PubSubClient client(espClient);

// ==========================================
// 2. CONFIGURACIÓN DEL SENSOR
// ==========================================
HardwareSerial sensor(2);
const int PIN_RX = 18;
const int PIN_TX = 19;
const uint8_t DIRECCION = 2;

uint16_t crcModbus(const uint8_t *datos, size_t cantidad) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < cantidad; i++) {
    crc ^= datos[i];
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
    }
  }
  return crc;
}

bool leerSensor(uint16_t *registros) {
  uint8_t consulta[8] = { DIRECCION, 0x03, 0x00, 0x00, 0x00, 0x08, 0, 0 };
  uint16_t crc = crcModbus(consulta, 6);
  consulta[6] = crc & 0xFF;
  consulta[7] = crc >> 8;

  while (sensor.available()) sensor.read();
  sensor.write(consulta, sizeof(consulta));
  sensor.flush(true);

  uint8_t respuesta[64];
  size_t n = 0;
  unsigned long inicio = millis();

  while (millis() - inicio < 700) {
    while (sensor.available()) {
      int dato = sensor.read();
      if (n < sizeof(respuesta)) respuesta[n++] = uint8_t(dato);
    }
    delay(1);
  }

  for (size_t i = 0; i + 5 <= n; i++) {
    if (respuesta[i] != DIRECCION) continue;

    if (respuesta[i + 1] == 0x83 && crcModbus(respuesta + i, 5) == 0) {
      Serial.printf("Excepcion Modbus: %u\n", respuesta[i + 2]);
      return false;
    }

    if (i + 21 <= n && respuesta[i + 1] == 0x03 && respuesta[i + 2] == 16 && crcModbus(respuesta + i, 21) == 0) {
      for (int r = 0; r < 8; r++) {
        size_t posicion = i + 3 + 2 * r;
        registros[r] = (uint16_t(respuesta[posicion]) << 8) | respuesta[posicion + 1];
      }
      return true;
    }
  }
  return false;
}

// ==========================================
// 3. FUNCIONES DE RED
// ==========================================
bool conectarRed(int indice) {
  Serial.printf("\nIntentando conectar a: %s\n", redes[indice].ssid);
  WiFi.begin(redes[indice].ssid, redes[indice].password);

  int intentos = 0;
  // Espera hasta 10 segundos (20 intentos de 500ms)
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado. IP asignada: " + WiFi.localIP().toString());
    mqtt_server = redes[indice].mqtt_ip;
    Serial.println("Broker MQTT asignado a: " + String(mqtt_server));
    return true;
  } else {
    Serial.println("\nFallo la conexión a esta red.");
    WiFi.disconnect();
    return false;
  }
}

void setup_wifi() {
  bool conectado = conectarRed(0); // Intentar Red 1

  if (!conectado) {
    Serial.println("Cambiando a red de respaldo...");
    conectado = conectarRed(1);    // Intentar Red 2
  }

  if (!conectado) {
    Serial.println("No se pudo conectar a ninguna red. Reiniciando en 5s...");
    delay(5000);
    ESP.restart(); // Reinicia el ESP32 para intentar de nuevo desde cero
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP32-CIBNOR-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado al Broker!");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5s");
      delay(5000);
    }
  }
}

// ==========================================
// 4. SETUP PRINCIPAL
// ==========================================
void setup() {
  Serial.begin(115200);

  sensor.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);

  setup_wifi(); // Se conecta a la mejor red disponible y define mqtt_server

  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(1024);
}

// ==========================================
// 5. LOOP PRINCIPAL
// ==========================================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;

  if (millis() - lastMsg > 10000) {
    lastMsg = millis();

    uint16_t registros[8];

    if (leerSensor(registros)) {
      int32_t tempBruta = registros[0];
      if (tempBruta >= 32768) tempBruta -= 65536;

      float temp = tempBruta / 10.0;
      float hum = registros[1] / 10.0;
      int ec = registros[2];
      int nit = registros[4];
      int fos = registros[5];
      int pot = registros[6];
      float ph = registros[7] / 10.0;

      String payload = "{\"temperatura\": " + String(temp, 1) +
                       ", \"humedad\": " + String(hum, 1) +
                       ", \"ph\": " + String(ph, 1) +
                       ", \"ec\": " + String(ec) +
                       ", \"nitrogeno\": " + String(nit) +
                       ", \"fosforo\": " + String(fos) +
                       ", \"potasio\": " + String(pot) + "}";

      Serial.println("Publicando: " + payload);
      client.publish("cibnor/suelo/datos", payload.c_str());

    } else {
      Serial.println("Fallo al leer el sensor. Revisar conexión RS485.");
    }
  }
}
