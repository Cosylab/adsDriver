#!/usr/bin/env python3
import sys
import os
import signal
import logging
from pyads.testserver import AdsTestServer
from handler import ADSTestHandler

# Allow LOG_LEVEL environment variable, default to INFO
log_level = os.environ.get('LOG_LEVEL', 'INFO')
logging.basicConfig(
    level=getattr(logging, log_level.upper()),
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

# Create handler and server
handler = ADSTestHandler()
server = AdsTestServer(handler, ip_address="127.0.0.1", port=48898, logging=True)

print("Starting ADS test server on 127.0.0.1:48898")
server.start()
print("Server running. Press Ctrl+C to stop.")

# Handle Ctrl+C gracefully
def signal_handler(sig, frame):
    print("\nStopping server...")
    server.stop()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

# Keep running
try:
    while True:
        import time
        time.sleep(1)
except KeyboardInterrupt:
    server.stop()
