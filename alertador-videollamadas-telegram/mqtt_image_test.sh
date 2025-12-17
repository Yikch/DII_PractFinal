#!/bin/bash

echo "Preparando elementos..."
# --- CONFIGURACIÓN ---
ARCHIVO_PATH="person.jpg"
TOPIC="device/c0eef043-1989-4444-bb4d-fbe4b902bd70"
MQTT_HOST="172.16.81.220" # Reemplaza con tu host
MQTT_PORT=31860
#CA_CRT="./certs/ca.crt"
# ---------------------

NOMBRE_ARCHIVO=$(basename "$ARCHIVO_PATH")
MIME_TYPE="image/jpeg" # Ajustar si es necesario (o usar 'file -b --mime-type "$ARCHIVO_PATH"')

echo "Chequeamos que el archivo existe"
if [ ! -f "$ARCHIVO_PATH" ]; then
    echo "Error: Archivo no encontrado en $ARCHIVO_PATH"
    exit 1
fi

# 2. Comprobar mosquitto_pub
if ! command -v mosquitto_pub &> /dev/null
then
    echo "Error: 'mosquitto_pub' no está instalado. Instálalo para continuar."
    exit 1
fi

# 3. GENERACIÓN DE LA CADENA BASE64 CRUDA
# Esto crea una sola cadena de texto muy larga.
BASE64_DATA=$(cat "$ARCHIVO_PATH" | base64 -w 0)

echo "Cadena Base64 generada. Tamaño aproximado: $(echo "$BASE64_DATA" | wc -c) bytes."
echo "---"

# 4. ENVÍO POR MQTT
# Usamos printf y -s para evitar saltos de línea y garantizar que la cadena completa
# (aunque sea de cientos de KB) se envíe como un único mensaje.
printf "%s" "$BASE64_DATA" | mosquitto_pub -h "$MQTT_HOST" -p "$MQTT_PORT" -t "$TOPIC" -s

echo "--- Envío finalizado al topic: $TOPIC ---"
