#!/usr/bin/env bash
# Quick launcher for the Smart City Transit Web UI
echo "=========================================================="
echo " Starting Smart City Transit Interactive Web UI..."
echo " Open your browser at: http://127.0.0.1:8080"
echo " (Press Ctrl+C to stop the server)"
echo "=========================================================="
python3 -m http.server 8080 --bind 127.0.0.1 --directory web
