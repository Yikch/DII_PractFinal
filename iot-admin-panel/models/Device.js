const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');

const Device = sequelize.define('Device', {
    id: {
        type: DataTypes.INTEGER,
        primaryKey: true,
        autoIncrement: true
    },
    alias: {
        type: DataTypes.STRING, 
        allowNull: false 
    },
    telegramAddress: {
        type: DataTypes.STRING,
        allowNull: false
    },
    macAddress: {
        type: DataTypes.STRING,
        allowNull: false,
        unique: true
    },
    mqttToken: {
        type: DataTypes.STRING, // Contraseña para el MQTT del Edge
        allowNull: false
    },
    status: {
        type: DataTypes.ENUM('online', 'offline'), // Reportado por el Edge
        defaultValue: 'offline'
    },
    lastSync: {
        type: DataTypes.DATE // Cuándo el Edge reportó algo de este equipo
    }
});

module.exports = Device;