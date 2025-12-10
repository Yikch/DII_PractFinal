// Middleware para proteger el Panel Web
const requireAdmin = (req, res, next) => {
    if (req.session.isAdmin) {
        next();
    } else {
        res.redirect('/login');
    }
};

// Middleware para proteger la API (El Edge debe enviar un Header)
const requireEdgeApiKey = (req, res, next) => {
    const apiKey = req.headers['x-edge-api-key'];
    if (apiKey && apiKey === process.env.EDGE_API_KEY) {
        next();
    } else {
        res.status(401).json({ error: 'Acceso denegado. API Key inválida.' });
    }
};

module.exports = { requireAdmin, requireEdgeApiKey };