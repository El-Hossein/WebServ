#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/html")
print()
print("Method: ", os.environ.get('REQUEST_METHOD'))