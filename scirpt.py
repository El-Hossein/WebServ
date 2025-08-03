import socket
import time

# Server details
host = "localhost"
port = 6967

# Build a simple HTTP GET request
request = (
    "GET / HTTP/1.1\r\n"
    f"Host: {host}:{port}\r\n"
    "User-Agent: Python-Slow-Sender\r\n"
    "Connection: close\r\n"
    "\r\n"
)

# Convert request to bytes
request_bytes = request.encode()

# Create and connect socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((host, port))
    print("[*] Connected to server")

    # Send request byte by byte
    for byte in request_bytes:
        s.send(bytes([byte]))
        print(f"[>] Sent byte: {repr(chr(byte))}")
        time.sleep(0.5)

    print("[*] Finished sending request")

    # Receive response
    response = b""
    while True:
        data = s.recv(4096)
        if not data:
            break
        response += data

print("[*] Response from server:")
print(response.decode(errors="ignore"))
