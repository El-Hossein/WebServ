#!/usr/bin/env python3

import os
import sys
import base64
import json
import secrets
from datetime import datetime, timezone
import fcntl
import hashlib
from urllib.parse import parse_qs

# ---------------- Send pages ----------------
def send(content, set_cookie_value=None):
    """Send HTTP response with optional Set-Cookie header and content."""
    body_bytes = content.encode("utf-8")
    print("Content-Type: text/html; charset=utf-8")
    if set_cookie_value is not None:
        # Only servers send Set-Cookie; never send a "Cookie" header in responses
        # Add secure-ish attributes as appropriate for your environment (add 'Secure' if HTTPS)
        print(f"Set-Cookie: token={set_cookie_value}; Path=/; HttpOnly; SameSite=Strict")
    print(f"Content-Length: {len(body_bytes)}")
    print()
    # Write bytes to avoid accidental encoding issues with large bodies
    sys.stdout.flush()
    sys.stdout.buffer.write(body_bytes)

# ---------------- HTML pages ----------------
def get_login_page(cookie_payload, cookie_header):
    return f"""<!DOCTYPE html>
    <html lang="en">
    <head><meta charset="utf-8"><title>Login</title></head>
    <body>
    <h1>Login Page</h1>
    <form action="/Desktop/WebServ/cgi/cgiScripts/login.py" method="POST">
    <label>Username: <input type="text" name="username" required></label><br><br>
    <label>Password: <input type="password" name="password" required></label><br><br>
    <input type="submit" value="Login">
    </form>
    <p>Debug: cookie_payload={cookie_payload}, cookie_header={cookie_header}</p>
    </body>
    </html>"""

# ---------------- Post data ----------------
def get_post_body():
    """Read exact bytes from stdin as given by CONTENT_LENGTH."""
    clen = os.environ.get('CONTENT_LENGTH', '')
    try:
        length = int(clen) if clen else 0
    except ValueError:
        length = 0
    if length > 0:
        return sys.stdin.read(length)
    return ''

# ------------------ Hash password ---------------
def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode('utf-8')).hexdigest()

def base64url_encode(data):
    if isinstance(data, str):
        data = data.encode('utf-8')
    return base64.urlsafe_b64encode(data).decode('utf-8').rstrip('=')

def base64url_decode(data):
    padding = '=' * (-len(data) % 4)
    return base64.urlsafe_b64decode(data + padding)

def token_create(username):
    """Create a simple unsigned token containing the username (base64url of JSON)."""
    payload = {'username': str(username)}
    return base64url_encode(json.dumps(payload))

def store_token(username, password_hash, token_value):
    """Append 'username,password_hash,token' to CookiesData.txt with a lock."""
    with open('CookiesData.txt', 'a') as f:
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        f.write(f"{username},{password_hash},{token_value}\n")
        fcntl.flock(f.fileno(), fcntl.LOCK_UN)

def file_read_lines_locked():
    try:
        with open("CookiesData.txt", 'r') as f:
            fcntl.flock(f.fileno(), fcntl.LOCK_SH)
            lines = f.readlines()
            fcntl.flock(f.fileno(), fcntl.LOCK_UN)
            return [ln.rstrip('\n') for ln in lines]
    except FileNotFoundError:
        return []

def token_parse(token):
    """Return payload dict or None on error."""
    try:
        decoded = base64url_decode(token)
        return json.loads(decoded)
    except Exception:
        return None

def validate_token(token):
    """Return line 'username,password_hash,token' if token exists and parses, else None."""
    payload = token_parse(token)
    if not payload or 'username' not in payload:
        return None
    for ln in file_read_lines_locked():
        # Strict token match in the 3rd CSV field
        parts = ln.split(",", 2)
        if len(parts) == 3 and parts[2] == token:
            return ln
    return None

def extract_token_from_cookie(cookie_header):
    """Extract 'token' value from HTTP_COOKIE header."""
    if not cookie_header:
        return None
    # Split on ';' and strip spaces
    for part in cookie_header.split(';'):
        part = part.strip()
        if part.startswith('token='):
            return part[len('token='):]
    return None

def find_user_by_username(username):
    """Return (username, password_hash, token) or None."""
    for ln in file_read_lines_locked():
        parts = ln.split(",", 2)
        if len(parts) == 3 and parts[0] == username:
            return tuple(parts)
    return None

def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    method = os.environ.get("REQUEST_METHOD", "GET").upper()

    if method == "POST":
        raw_body = get_post_body()
        parsed = parse_qs(raw_body, keep_blank_values=True)
        username = parsed.get('username', [''])[0]
        password = parsed.get('password', [''])[0]

        if not username or not password:
            send("<html><body><h1>Missing username or password</h1></body></html>")
            return

        found = find_user_by_username(username)
        if found:
            # Existing user
            userna, stored_pwd_hash, existing_token = found
            if hash_password(password) == stored_pwd_hash:
                # Valid login; refresh cookie (optional) by re-setting it
                send(f"<html><body><h1>Welcome back, user {userna}</h1></body></html>", set_cookie_value=existing_token)
            else:
                send(f"<html><body><h1>Incorrect password for user {userna}</h1></body></html>")
            return
        else:
            # New user: create account and set cookie
            token = token_create(username)
            store_token(username, hash_password(password), token)
            send(f"<html><body><h1>Login successful! Welcome, new user {username}</h1></body></html>", set_cookie_value=token)
            return

    # GET
    token = extract_token_from_cookie(cookie_header)
    valid_line = validate_token(token) if token else None
    if valid_line:
        user, pswd_hash, tok = valid_line.split(",", 2)
        # Already authenticated; no need to send Set-Cookie again unless you want to refresh expiry
        send(f"<html><body><h1>Welcome back, user {user}</h1></body></html>")
        return

    # Not authenticated: show login page
    login_page = get_login_page(cookie_payload=token_parse(token) if token else None,
                                cookie_header=cookie_header)
    send(login_page)

if __name__ == "__main__":
    main()
