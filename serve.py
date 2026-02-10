#!/usr/bin/env python3
"""
Simple local HTTP server for the FireGuard dashboard.
Required for TF.js model loading (file:// doesn't support fetch).
"""
import http.server
import os
from pathlib import Path

PORT = 8080
DATA_DIR = Path(__file__).parent / "data"

os.chdir(DATA_DIR)

print(f"🔥 FireGuard Dashboard Server")
print(f"   Serving: {DATA_DIR}")
print(f"   Open:    http://localhost:{PORT}")
print(f"   Press Ctrl+C to stop\n")

handler = http.server.SimpleHTTPRequestHandler
handler.extensions_map.update({
    '.js': 'application/javascript',
    '.json': 'application/json',
    '.bin': 'application/octet-stream',
    '.wasm': 'application/wasm',
})

httpd = http.server.HTTPServer(('', PORT), handler)
httpd.serve_forever()
