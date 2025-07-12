#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Hello CGI</h1>")
print("Method: ", os.environ.get('REQUEST_METHOD'))
print("Content-Length: ", os.environ.get('CONTENT_LENGTH'))
data = sys.stdin.read(int(os.environ.get('CONTENT_LENGTH', 0)))
print("POST data: ", data)
print("</body></html>")