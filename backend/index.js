
require('dotenv').config();

const express = require('express');
const axios = require('axios');

const app = express();
app.use(express.json());

const CLIENT_ID = '1289578567151390793';
const CLIENT_SECRET = process.env.CLIENT_SECRET;
const REDIRECT_URI = 'http://localhost:8000';

app.post('/auth', async (req, res) => {
    // console.log("1");
    const code = req.body.code;
    if (!code) {
        // console.log("no code");
        // console.log(req.body);
        return res.status(400).json({ error: 'no code provided?? (auth)' });
    }
    // console.log("2");

    try {
        // console.log("3");
        const params = new URLSearchParams({ client_id: CLIENT_ID, client_secret: CLIENT_SECRET, grant_type: 'authorization_code', code, redirect_uri: REDIRECT_URI});
        const { data } = await axios.post('https://discord.com/api/oauth2/token', params, { headers: { 'Content-Type': 'application/x-www-form-urlencoded' } });
        // console.log(data);
        res.json(data);
    } catch (err) {
        console.error(err.response?.data || err.message);
        res.status(500).json({ error: 'failed to exchange code' });
    }
});

app.post('/refresh', async (req, res) => {
    const code = req.body.code;
    if (!code) return res.status(400).json({ error: 'no code provided?? (refresh)' });

    try {
        const params = new URLSearchParams({ client_id: CLIENT_ID, client_secret: CLIENT_SECRET, grant_type: 'refresh_token', refresh_token: code, redirect_uri: REDIRECT_URI});
        const { data } = await axios.post('https://discord.com/api/oauth2/token', params, { headers: { 'Content-Type': 'application/x-www-form-urlencoded' } });
        // console.log(data);
        res.json(data);
    } catch (err) {
        console.error(err.response?.data || err.message);
        res.status(500).json({ error: 'failed to refresh token' });
    }
});


app.listen(3000, () => console.log("server running"));