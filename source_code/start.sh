#!/bin/bash
cd /app/dashboard_gui
source venv/bin/activate
python store.py & python gui.py

cd /app/cloud_websocket/broker-js
node broker.js
