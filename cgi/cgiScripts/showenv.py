#!/usr/bin/env python3
import os

# CGI scripts must print headers first
print("Content-Type: text/html\r\n")

# The environment variables you defined
vars_to_check = [
    "REQUEST_METHOD",
    "SCRIPT_NAME",
    "SCRIPT_FILENAME",
    "QUERY_STRING",
    "SERVER_PROTOCOL",
    "GATEWAY_INTERFACE",
    "SERVER_SOFTWARE",
    "PATH_INFO",
    "CONTENT_TYPE",
    "CONTENT_LENGTH",
    "SERVER_NAME",
    "SERVER_PORT"
]

# Print each variable and its value
for var in vars_to_check:
    print(f"{var} = {os.environ.get(var, '')}")
