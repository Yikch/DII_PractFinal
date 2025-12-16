import paho.mqtt.client as mqtt
import time
import os
from datetime import datetime

# --- CONFIGURACIÓN DE TÓPICOS ---
# Base del topic para la suscripción (usamos el comodín de varios niveles #)
# Esto capturará: Telefonillos/FaceDetectedAlarm/Telefonillo1,
# Telefonillos/FaceDetectedAlarm/Telefono2, etc.
BASE_TOPIC_SUBSCRIBE = "Telefonillos/FaceDetectedAlarm/#"

# Prefijo para la base del topic de la que extraeremos el nombre del dispositivo.
# El nombre del dispositivo será lo que venga DESPUÉS de este prefijo.
TOPIC_PREFIX_FOR_DEVICE_NAME = "Telefonillos/FaceDetectedAlarm/"

# Configuración del Broker
BROKER = "localhost" # O la IP de tu PC: "192.168.1.41"
PORT = 1883
KEEP_ALIVE_INTERVAL = 60

# Directorio donde se guardarán las imágenes
OUTPUT_DIR = "imagenes_recibidas"
# -------------------------------


def on_connect(client, userdata, flags, rc):
    """Callback que se ejecuta cuando el cliente se conecta al broker."""
    if rc == 0:
        print(f"✅ Conectado con éxito al broker MQTT ({BROKER}:{PORT}).")
        # Suscribirse al topic base con el comodín
        client.subscribe(BASE_TOPIC_SUBSCRIBE)
        print(f"👂 Escuchando en el topic: {BASE_TOPIC_SUBSCRIBE}")
    else:
        print(f"❌ Fallo en la conexión. Código de error: {rc}")

def get_device_name_from_topic(topic: str) -> str:
    """Extrae el nombre del dispositivo de la última parte del topic."""
    
    # 1. Asegurarse de que el topic comienza con el prefijo esperado.
    if topic.startswith(TOPIC_PREFIX_FOR_DEVICE_NAME):
        # 2. Quitar el prefijo para obtener solo el nombre del dispositivo.
        device_name = topic[len(TOPIC_PREFIX_FOR_DEVICE_NAME):]
        
        # 3. Limpiar el nombre por si tiene barras adicionales.
        # Si el topic es Telefonillos/FaceDetectedAlarm/Telefonillo1
        # El resultado será "Telefonillo1"
        return device_name.strip('/')
    
    # Si el topic no coincide con el formato esperado
    return "DispositivoDesconocido"

def on_message(client, userdata, msg):
    """Callback que se ejecuta al recibir un mensaje."""
    
    # 1. Extraer el nombre del dispositivo del topic
    device_name = get_device_name_from_topic(msg.topic)
    
    # 2. Crear el nombre de archivo: NombreDispositivo_Timestamp.jpg
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    nombre_archivo = os.path.join(OUTPUT_DIR, f"{device_name}_{timestamp}.jpg")
    
    print(f"\n📥 Mensaje recibido en {msg.topic}")
    print(f"  > Dispositivo: {device_name}")
    print(f"  > Tamaño: {len(msg.payload)} bytes")

    # 3. Guardar el payload (bytes de la imagen) en un archivo
    try:
        with open(nombre_archivo, "wb") as f:
            f.write(msg.payload)
        print(f"  > Imagen guardada como: {nombre_archivo}")
    except IOError as e:
        print(f"❌ Error al guardar el archivo: {e}")


# --- INICIALIZACIÓN Y BUCLE PRINCIPAL ---

# 1. Crear el directorio de salida si no existe
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)
    print(f"📂 Creado directorio de salida: {OUTPUT_DIR}")

# 2. Configurar el cliente
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.reconnect_delay_set(min_delay=1, max_delay=120) # Retraso de reconexión

# 3. Conectar y empezar a escuchar
try:
    client.connect(BROKER, PORT, KEEP_ALIVE_INTERVAL)
    client.loop_forever()
except KeyboardInterrupt:
    print("\n👋 Suscriptor detenido por el usuario.")
    client.disconnect()
except Exception as e:
    print(f"❌ Error de conexión: {e}")