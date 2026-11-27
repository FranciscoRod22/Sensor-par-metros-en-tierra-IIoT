# Sistema IIoT para monitoreo edáfico en el CIBNOR

Sistema de Internet Industrial de las Cosas que mide variables fisicoquímicas
del suelo en tiempo real, las almacena en una base de datos relacional y las
visualiza en un panel web. Desarrollado para el Centro de Investigaciones
Biológicas del Noroeste (CIBNOR) como proyecto de la materia Internet
Industrial de las Cosas, Ingeniería Mecatrónica, Instituto Tecnológico de Tepic.

## Problema que resuelve

El CIBNOR riega cultivos con agua enriquecida mediante mineralización. Evaluar
su efecto en el suelo requería muestreos manuales enviados a laboratorio
(aproximadamente 500 pesos por muestra y hasta 48 horas de espera). Este
sistema entrega las lecturas continuamente y sin costo por muestra.

## Variables medidas

Temperatura, humedad, pH, conductividad eléctrica (EC) y macronutrientes
Nitrógeno, Fósforo y Potasio (NPK), con el sensor NPKPHCTH-S.

## Arquitectura

    Sensor NPKPHCTH-S
          | RS-485 / Modbus RTU
          v
    ESP32 (WiFi)
          | MQTT, tópico cibnor/suelo/datos
          v
    Mosquitto (broker, VM Ubuntu)
          |
          v
    Script Python (paho-mqtt + pymysql)
          |
          v
    MariaDB (base liac, tabla sensores)
          |
          +--> Grafana (visualización en tiempo real)
          +--> Excel + rclone --> Google Drive (respaldos)
          +--> API Flask + ngrok --> botón en Google Sheets

## Estructura del repositorio

| Carpeta | Contenido |
|---------|-----------|
| `firmware/esp32/sensor_suelo/` | Código del ESP32 (Arduino) y plantilla `secrets.example.h` |
| `backend/` | Scripts de Python y de respaldo, y plantillas `config_privada.example.*` |
| `config/systemd/` | Servicios que mantienen el sistema corriendo |
| `config/mosquitto/` | Configuración actual del broker y plantilla con mTLS |
| `config/grafana/` | Datos de conexión y dashboard exportado en JSON |
| `docs/` | Documentación y evidencias |

## Requisitos

- ESP32 con convertidor TTL a RS-485 y sensor NPKPHCTH-S.
- Ubuntu con Mosquitto, MariaDB, Grafana, Python 3 y rclone.
- Librerías de Python: paho-mqtt, pymysql, pandas, xlsxwriter, flask.
- Arduino IDE con la librería PubSubClient.

## Configuración de credenciales

Las contraseñas y datos privados NO están en el repositorio. Para cada
archivo de plantilla hay que crear una copia con los datos reales:

    cp firmware/esp32/sensor_suelo/secrets.example.h firmware/esp32/sensor_suelo/secrets.h
    cp backend/config_privada.example.py backend/config_privada.py
    cp backend/config_privada.example.sh backend/config_privada.sh

Después se editan las copias con los valores reales. Git las ignora.

## Seguridad

- Contraseñas y llaves fuera del código, excluidas con `.gitignore`.
- La configuración actual del broker (`config/mosquitto/mosquitto.conf`) está
  pensada para una red local de pruebas y NO usa TLS.
- La plantilla `config/mosquitto/mosquitto.mtls.example.conf` activa autenticación
  mutua con certificados (mTLS) en el puerto 8883. Requiere generar una
  Autoridad Certificadora y un certificado por cliente.
- Se recomienda activar el firewall UFW y limitar el acceso por IP.
- El endpoint `/run-backup` de la API no tiene autenticación: no publiques
  el dominio de ngrok.

## Estado del proyecto

- Lectura de temperatura, humedad, EC y NPK funcionando.
- Pendiente: validar qué registro Modbus corresponde al pH.
- Pendiente: reconexión WiFi automática durante la operación.

## Autores

- Francisco de Jesús Rodríguez Robles
- Adrián Rico Bautista

Docente: Ing. Rogelio Sanchez López.

## Licencia

MIT. Consulta el archivo LICENSE.
