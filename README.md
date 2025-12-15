# DII_PractFinal
Trabajo Final. Propuesta 2: Aplicación ESP-IDF con servidor: Telefonillos inteligentes con aviso remoto

## Componentes del equipo de trabajo
El equipo de trabajo lo conformarán las siguientes personas:
- Daniel del Pino Sánchez
- Diego Pellicer Lafuente
- Yikang Chen
- Fernando Viñas Martins
- Alejandro de Celis

## Lógica de negocio
Nuestra idea consiste en un servicio de telefonillos inteligentes, los cuales enviarán tras ser pulsados la información recogida a través de su cámara a su respectivo cliente. Estos telefonillos además dispondrán de un algoritmo de inteligencia artificial que permitirá el reconocimiento de rostros para evitar el envío de avisos que no puedan ser identificados por los clientes.
Los clientes serán avisados a través de telegram de una llamada a su telefonillo recibiendo una imagen tras la llamada en caso de detectarse un rostro. Además se dispondrá de una consola de administrador en la cual se podrá modificar, crear o eliminar nuevos telefonillos.
A través de esta se puede decidir a qué dirección de telegram llegarán las notificaciones o incorporar nuevos clientes, además de poder visualizar gráficos con el número de llamadas por telefonillo, llamadas aceptadas y de más datos que puedan ser útiles para los administradores.

## **WebApp iot-admin-panel**
El directorio `./iot-admin-panel` aloja el código fuente de la **WebApp** de administración desplegada en **Microsoft Azure**. Esta aplicación actúa como el panel de control centralizado para la gestión de los telefonillos (ESP32-EYE). El flujo funcional es el siguiente:

1. **Autenticación y Seguridad**: El acceso está restringido mediante un portal de inicio de sesión (Login) que valida las credenciales del administrador.

2. **Gestión de Inventario**: Tras la autenticación, se presenta un dashboard con el listado de dispositivos registrados en tiempo real.

3. **Provisión de Dispositivos**: El sistema permite el alta de nuevos dispositivos introduciendo su dirección MAC, dirección Telegram y un alias identificativo.

4. **Generación de Credenciales**: Al registrar un dispositivo, el backend genera automáticamente un Token MQTT. Este token es vital, ya que será utilizado por el dispositivo físico para autenticarse de forma segura contra el Gateway IoT (Raspberry Pi) en el borde (Edge).

5. **Persistencia de Datos**: Toda la información relacional del dispositivo se almacena en una instancia de Azure Database for MySQL.

Para obtener los datos de los dispositivos creados se puede usar este comando:

```bash
$ curl -X GET https://iot-admin-panel-a7dub3cmf6f3a4hc.spaincentral-01.azurewebsites.net/api/edge/sync-devices -H "x-edge-api-key: clave_segura_comunicacion_edge_cloud_DII_2025_26"
```

Y para cambiar el campo status de un dispositivo:

```bash
curl -X POST https://iot-admin-panel-a7dub3cmf6f3a4hc.spaincentral-01.azurewebsites.net/api/edge/device-status \
     -H "Content-Type: application/json" \
     -H "x-edge-api-key: clave_segura_comunicacion_edge_cloud_DII_2025_26" \
     -d "{\"macAddress\": \"11:22:33:44:55:66\", \"status\": \"online\"}"
```

Las credenciales son:
```bash
# Base de datos Azure MySQL
DB_HOST=iot-panel-db.mysql.database.azure.com
DB_USER=iotadmin
DB_PASS=admin_DII
DB_NAME=iot-panel-db
DB_SSL=true

# Credenciales Admin Web
ADMIN_USER=admin
ADMIN_PASS=admin_DII
SESSION_SECRET=super_secreto_session

EDGE_API_KEY=clave_segura_comunicacion_edge_cloud_DII_2025_26
```
