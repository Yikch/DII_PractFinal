import paho.mqtt.client as mqtt
import time

# Configuración
BROKER = "localhost" # O la IP de tu PC si el ESP32 no conecta por nombre
TOPIC = "camara/imagen"

def on_connect(client, userdata, flags, rc):
    print(f"Conectado con código {rc}")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    print(f"Recibido mensaje en {msg.topic} de tamaño {len(msg.payload)} bytes")
    
    # Guardar el payload (bytes de la imagen) en un archivo
    nombre_archivo = f"foto_{int(time.time())}.jpg"
    with open(nombre_archivo, "wb") as f:
        f.write(msg.payload)
    print(f"Imagen guardada como: {nombre_archivo}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, 1883, 60)

print("Esperando imágenes...")
client.loop_forever()