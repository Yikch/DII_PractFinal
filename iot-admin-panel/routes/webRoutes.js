const express = require('express');
const router = express.Router();
const { v4: uuidv4 } = require('uuid'); // Para generar tokens random
const Device = require('../models/Device');
const { requireAdmin } = require('../middleware/auth');

// Dashboard
router.get('/', requireAdmin, async (req, res) => {
    const devices = await Device.findAll();
    res.render('dashboard', { devices });
});

// Crear Dispositivo
router.post('/devices', requireAdmin, async (req, res) => {
    try {
        await Device.create({
            alias: req.body.alias,
            physicalAddress: req.body.physicalAddress,
            macAddress: req.body.macAddress,
            mqttToken: uuidv4() // Generamos un token seguro automáticamente
        });
        res.redirect('/');
    } catch (error) {
        res.status(400).send("Error: " + error.message);
    }
});

// Borrar Dispositivo
router.delete('/devices/:id', requireAdmin, async (req, res) => {
    await Device.destroy({ where: { id: req.params.id } });
    res.redirect('/');
});

// Login
router.get('/login', (req, res) => res.render('login', { error: null }));
router.post('/login', (req, res) => {
    if (req.body.username === process.env.ADMIN_USER && req.body.password === process.env.ADMIN_PASS) {
        req.session.isAdmin = true;
        res.redirect('/');
    } else {
        res.render('login', { error: 'Credenciales incorrectas' });
    }
});
router.get('/logout', (req, res) => {
    req.session.destroy();
    res.redirect('/login');
});

// Ruta auxiliar para obtener datos en tiempo real (Polling)
router.get('/api/live-data', requireAdmin, async (req, res) => {
    try {
        const devices = await Device.findAll({
            attributes: ['macAddress', 'status', 'lastSync']
        });
        res.json(devices);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

module.exports = router;