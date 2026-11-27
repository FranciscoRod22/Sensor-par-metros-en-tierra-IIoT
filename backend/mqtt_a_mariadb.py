import paho.mqtt.client as mqtt
import pymysql
import json
from datetime import datetime

# ==========================================
# Configuración de la base de datos
# ==========================================
DB_HOST = "localhost"
DB_USER = "cibnor_user"
from config_privada import DB_PASS
DB_NAME = "liac"
DB_TABLE = "sensores"

# ==========================================
# Configuración MQTT (TCP plano, sin seguridad)
# ==========================================
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "cibnor/suelo/datos"

# ==========================================
# Conexión a MariaDB
# ==========================================
def guardar_en_db(datos):
    try:
        conexion = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASS,
            database=DB_NAME
        )
        with conexion.cursor() as cursor:
            sql = (
                f"INSERT INTO {DB_TABLE} "
                "(temperatura, humedad, ph, ec, nitrogeno, fosforo, potasio) "
                "VALUES (%s, %s, %s, %s, %s, %s, %s)"
            )
            valores = (
                datos.get("temperatura"),
                datos.get("humedad"),
                datos.get("ph"),
                datos.get("ec"),
                datos.get("nitrogeno"),
                datos.get("fosforo"),
                datos.get("potasio"),
            )
            cursor.execute(sql, valores)
            conexion.commit()

        print(f"[{datetime.now()}] Dato guardado en MariaDB: {valores}")
    except Exception as e:
        print(f"[{datetime.now()}] ERROR al guardar en MariaDB: {e}")

# ==========================================
# Callbacks MQTT
# ==========================================
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Conectado al broker MQTT. Esperando datos...")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"Error de conexión MQTT: {reason_code}")

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")
        datos = json.loads(payload)
        print(f"Mensaje recibido: {datos}")
        guardar_en_db(datos)
    except json.JSONDecodeError as e:
        print(f"JSON inválido recibido: {e}")
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

# ==========================================
# Configuración y arranque del cliente
# ==========================================
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
