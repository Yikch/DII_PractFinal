const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const { requireEdgeApiKey } = require('../middleware/auth');

// Ruta 1: SYNC
// El Edge llama aquí al iniciarse para saber qué MACs y Tokens son válidos.
router.get('/sync-devices', requireEdgeApiKey, async (req, res) => {
    try {
        // Devolvemos solo lo que el broker MQTT necesita saber
        const devices = await Device.findAll({
            attributes: ['macAddress', 'mqttToken', 'telegramAddress']
        });
        res.json(devices);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Ruta 2: REPORT STATUS
// El Edge nos avisa: "El dispositivo X se ha conectado/desconectado"
router.post('/device-status', requireEdgeApiKey, async (req, res) => {
    const { macAddress, status } = req.body;
    try {
        // Sequelize update devuelve un array: [numero_de_filas_afectadas]
        const [updatedRows] = await Device.update(
            { status: status, lastSync: new Date() },
            { where: { macAddress: macAddress } }
        );

        if (updatedRows > 0) {
            // Se encontró y se actualizó
            res.json({ success: true, message: 'Estado actualizado' });
        } else {
            // No hubo errores técnicos, pero ninguna fila cumplía la condición (MAC no existe)
            res.status(404).json({ error: 'Dispositivo no encontrado con esa MAC' });
        }
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

module.exports = router;