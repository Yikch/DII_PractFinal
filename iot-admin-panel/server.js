require('dotenv').config();
const express = require('express');
const session = require('express-session');
const methodOverride = require('method-override');
const sequelize = require('./config/database');

// Importar rutas
const webRoutes = require('./routes/webRoutes');
const apiRoutes = require('./routes/apiRoutes');

const app = express();

// Configuración
app.set('view engine', 'ejs');
app.use(express.urlencoded({ extended: true })); // Para formularios HTML
app.use(express.json()); // Para peticiones API JSON del Edge
app.use(express.static('public'));
app.use(methodOverride('_method'));

app.use(session({
    secret: process.env.SESSION_SECRET || 'secret',
    resave: false,
    saveUninitialized: false
}));

// Rutas
app.use('/', webRoutes);      // Rutas visuales (Navegador)
app.use('/api/edge', apiRoutes); // Rutas API (Para el Edge)

// Inicio
const PORT = process.env.PORT || 3000;

sequelize.sync().then(() => {
    console.log('MySQL Conectado y Sincronizado');
    app.listen(PORT, () => console.log(`Azure Web App corriendo en puerto ${PORT}`));
}).catch(err => console.error('Error DB:', err));