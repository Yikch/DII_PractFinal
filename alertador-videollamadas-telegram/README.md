Módulo Quarkus alertador de videollamadas a Telegram.

Escucha desde MQTT de forma reactiva para obtener imágenes de personas que llaman desde los videoporteros y cachea los datos en memoria SQLite para resiliencia.

Cuando se registra un rostro se obtiene conoce a qué usuario de Telegram enviarlo por el topic de dispositivo, dentro de los permitidos por la aplicación de administracón.

Con el topic de dispositivo se identifica al usuario al que se envía en Telegram, que recibe las notificaciones debidas cada minuto.

A la noche, un proceso limpia todos los elementos de la base de datos de caché que hayan sido enviados.
