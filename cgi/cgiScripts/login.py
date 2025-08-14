#!/usr/bin/env python

import os
import sys
import base64
import json
import secrets
from datetime import datetime, timezone
import fcntl

def send(content, content_type="text/html", token=None, cookie=None):
    """Send HTTP response with optional Set-Cookie header and content."""
    print(f"Content-Type: {content_type}")
    if token:
        print(f"Set-Cookie: token={token}; Path=/; HttpOnly; SameSite=Strict")
    elif cookie:
        print(f"Cookie: {cookie}")
    print(f"Content-Length: {len(content)}")
    print()
    print(content)

def base64url_encode(data):
    """Encode data to base64url format."""
    if isinstance(data, str):
        data = data.encode('ascii')
    return base64.urlsafe_b64encode(data).decode('utf-8').rstrip('=')

def base64url_decode(data):
    """Decode base64url data."""
    padding = '=' * (-len(data) % 4)
    return base64.urlsafe_b64decode(data + padding)

def get_current_time():
    """Get current UTC timestamp."""
    return int(datetime.now(timezone.utc).timestamp())

def jwt_creator(userid):
    """Create a JWT-like token with userid and expiration."""
    expiration = get_current_time() + 3600  # 1 hour expiration
    payload = {
        'userid': str(userid),
        'expired': expiration
    }
    payload_encoded = base64url_encode(json.dumps(payload))
    return f"{payload_encoded}"

def get_next_userid():
    """Get the next available user ID from CookiesData.txt."""
    try:
        with open('CookiesData.txt', 'r') as f:
            fcntl.flock(f.fileno(), fcntl.LOCK_SH)
            lines = f.readlines()
            fcntl.flock(f.fileno(), fcntl.LOCK_UN)
            if not lines:
                return 1
            last_line = lines[-1].strip()
            if not last_line:
                return 1
            user_id, _ = last_line.split(",", 1)
            return int(user_id) + 1
    except (FileNotFoundError, ValueError, OSError):
        return 1

def store_token(userid, token):
    """Store user ID and token in CookiesData.txt."""
    with open('CookiesData.txt', 'a') as f:
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        f.write(f"{userid},{token}\n")
        fcntl.flock(f.fileno(), fcntl.LOCK_UN)

def validate_token(token):
    """Validate JWT token and return payload if valid."""
    try:
        payload_data = json.loads(base64url_decode(token))
        if int(payload_data['expired']) < get_current_time():
            return None
        return payload_data
    except Exception:
        return None

def check_cookie(cookie):
    """Check if the cookie contains a valid token."""
    if not cookie:
        return None
    try:
        token = cookie.split('token=')[1].split(';')[0]
        # print
        return validate_token(token)
    except IndexError:
        return None

def get_login_page(payload, cookie):
    """Return the HTML login page."""
    return f"""<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Login</title>
    </head>
    <body>
        <h1>test {payload} , {cookie}</h1>
        <h1>Login Page</h1>
        <form action="/Desktop/WebServ/cgi/cgiScripts/login.py" method="POST">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required><br><br>
            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required><br><br>
            <input type="submit" value="Login">
        </form>
    </body>
    </html>"""

def main():
    cookie = os.environ.get("HTTP_COOKIE", None)
    method = os.environ.get("REQUEST_METHOD", "GET")

    # Handle login form submission
    post_data = "username:test,password:test"
    if method == "POST":
        if "username" in post_data and "password" in post_data:
            userid = get_next_userid()
            token = jwt_creator(userid)
            store_token(userid, token)
            send(f"<html><body><h1>Login successful! Welcome, user {userid}</h1></body></html>", token=token)
        else:
            send(f"<html><body><h1>Invalid login credentials</h1></body></html>")
        return

# Check for existing valid cookie
    payload = check_cookie(cookie)
    if payload:
        send(f"<html><body><h1>Welcome back, user {payload['userid']}</h1></body></html>")
    else:
        # Serve login page if no valid cookie
        login_page = get_login_page(payload, cookie)
        send(login_page)

if __name__ == "__main__":
    main()




# #!/usr/bin/env python

# import os
# import sys
# import base64
# import json
# import secrets
# from datetime import datetime, timezone
# import fcntl

# def send(content, content_type="text/html", token=None, cookie=None):
#     """Send HTTP response with optional Set-Cookie header and content."""
#     print(f"Content-Type: {content_type}")
#     if token:
#         print(f"Set-Cookie: token={token}; Path=/; HttpOnly; SameSite=Strict")
#     elif cookie:
#         print(f"Cookie: {cookie}")
#     print(f"Content-Length: {len(content)}")
#     print()
#     print(content)

# def base64url_encode(data):
#     """Encode data to base64url format."""
#     if isinstance(data, str):
#         data = data.encode('ascii')
#     return base64.urlsafe_b64encode(data).decode('utf-8').rstrip('=')

# def base64url_decode(data):
#     """Decode base64url data."""
#     padding = '=' * (-len(data) % 4)
#     return base64.urlsafe_b64decode(data + padding)

# def get_current_time():
#     """Get current UTC timestamp."""
#     return int(datetime.now(timezone.utc).timestamp())

# def jwt_creator(userid):
#     """Create a JWT-like token with userid and expiration."""
#     expiration = get_current_time() + 3600  # 1 hour expiration
#     payload = {
#         'userid': str(userid),
#         'expired': expiration
#     }
#     payload_encoded = base64url_encode(json.dumps(payload))
#     return f"{payload_encoded}"

# def get_next_userid():
#     """Get the next available user ID from CookiesData.txt."""
#     try:
#         with open('CookiesData.txt', 'r') as f:
#             fcntl.flock(f.fileno(), fcntl.LOCK_SH)
#             lines = f.readlines()
#             fcntl.flock(f.fileno(), fcntl.LOCK_UN)
#             if not lines:
#                 return 1
#             last_line = lines[-1].strip()
#             if not last_line:
#                 return 1
#             user_id, _ = last_line.split(",", 1)
#             return int(user_id) + 1
#     except (FileNotFoundError, ValueError, OSError):
#         return 1

# def store_token(userid, token):
#     """Store user ID and token in CookiesData.txt."""
#     with open('CookiesData.txt', 'a') as f:
#         fcntl.flock(f.fileno(), fcntl.LOCK_EX)
#         f.write(f"{userid},{token}\n")
#         fcntl.flock(f.fileno(), fcntl.LOCK_UN)

# def validate_token(token):
#     """Validate JWT token and return payload if valid."""
#     try:
#         payload_data = json.loads(base64url_decode(token))
#         if int(payload_data['expired']) < get_current_time():
#             return None
#         return payload_data
#     except Exception:
#         return None

# def check_cookie(cookie):
#     """Check if the cookie contains a valid token."""
#     if not cookie:
#         return None
#     try:
#         token = cookie.split('token=')[1].split(';')[0]
#         # print
#         return validate_token(token)
#     except IndexError:
#         return None

# def get_login_page():
#     """Return the HTML login page."""
#     return """<!DOCTYPE html>
#     <html lang="en">
#     <head>
#         <meta charset="UTF-8">
#         <title>Login</title>
#     </head>
#     <body>
#         <h1>Login Page</h1>
#         <form action="/Desktop/webserv/WebServ/cgi/cgiScripts/login.py" method="POST">
#             <label for="username">Username:</label>
#             <input type="text" id="username" name="username" required><br><br>
#             <label for="password">Password:</label>
#             <input type="password" id="password" name="password" required><br><br>
#             <input type="submit" value="Login">
#         </form>
#     </body>
#     </html>"""

# def main():
#     cookie = os.environ.get("HTTP_COOKIE", None)
#     method = os.environ.get("REQUEST_METHOD", "GET")

#     if method == "POST":
#         # Handle login form submission
#         post_data = "username:test,password:test"
#         if "username" in post_data and "password" in post_data:
#             userid = get_next_userid()
#             token = jwt_creator(userid)
#             store_token(userid, token)
#             send(f"<html><body><h1>Login successful! Welcome, user {userid}</h1></body></html>", token=token)
#         else:
#             send(f"<html><body><h1>Invalid login credentials</h1></body></html>")
#         return

#     # Check for existing valid cookie
#     payload = check_cookie(cookie)
#     if payload:
#         send(f"<html><body><h1>Welcome back, user {payload['userid']}</h1></body></html>")
#     else:
#         # Serve login page if no valid cookie
#         login_page = get_login_page()
#         send(login_page)

# if __name__ == "__main__":
#     main()



# #!/usr/bin/env python

# import os
# import sys
# import base64
# import json
# import secrets
# from datetime import datetime, timezone
# import fcntl
# import hmac
# import hashlib

# def send(content, content_type="text/html", token=None, cookie=None):
#     """Send HTTP response with optional Set-Cookie header and content."""
#     print(f"Content-Type: {content_type}")
#     if token:
#         print(f"Set-Cookie: token={token}; Path=/; HttpOnly; SameSite=Strict")
#     elif cookie:
#         print(f"Cookie: {cookie}")
#     print(f"Content-Length: {len(content)}")
#     print()
#     print(content)

# def base64url_encode(data):
#     """Encode data to base64url format."""
#     if isinstance(data, str):
#         data = data.encode('ascii')
#     return base64.urlsafe_b64encode(data).decode('utf-8').rstrip('=')

# def base64url_decode(data):
#     """Decode base64url data."""
#     padding = '=' * (4 - (len(data) % 4))
#     return base64.urlsafe_b64decode(data + padding)

# def get_current_time():
#     """Get current UTC timestamp."""
#     return int(datetime.now(timezone.utc).timestamp())

# def jwt_creator(userid):
#     """Create a JWT-like token with userid and expiration."""
#     expiration = get_current_time() + 3600  # 1 hour expiration
#     payload = {
#         'userid': str(userid),
#         'expired': expiration
#     }
#     payload_encoded = base64url_encode(json.dumps(payload))
#     return f{payload_encoded}"


# def get_next_userid():
#     """Get the next available user ID from CookiesData.txt."""
#     try:
#         with open('CookiesData.txt', 'r') as f:
#             fcntl.flock(f.fileno(), fcntl.LOCK_SH)
#             lines = f.readlines()
#             if not lines:
#                 return 1
#             last_line = lines[-1].strip()
#             if not last_line:
#                 return 1
#             user_id, _ = last_line.split(",", 1)
#             return int(user_id) + 1
#     except (FileNotFoundError, ValueError, OSError):
#         return 1

# def store_token(userid, token):
#     """Store user ID and token in CookiesData.txt."""
#     with open('CookiesData.txt', 'a') as f:
#         fcntl.flock(f.fileno(), fcntl.LOCK_EX)
#         f.write(f"{userid},{token}\n")

# # def validate_token(token):
# #     """Validate JWT token and return payload if valid."""
# #     try:
# #         payload = token.split('.')
# #         # print("header: ", header, " payload: ", payload, " signature: ", signature)
# #         # secret_key = "DonDoIt"  # Same as in jwt_creator
# #         # expected_signature = base64url_encode(
# #         #     hmac.new(secret_key.encode('utf-8'), f"{header}.{payload}".encode('utf-8'), hashlib.sha256).digest()
# #         # )
# #         # if signature != expected_signature:
# #         #     return None
# #         # print("bypass signature")
# #         payload_data = json.loads(base64url_decode(payload))
# #         if int(payload_data['expired']) < get_current_time():
# #             return None
# #         # print("bypass expired")
# #         return payload_data
# #     except Exception:
# #         return None

# # def check_cookie(cookie):
# #     """Check if the cookie contains a valid token."""
# #     if not cookie:
# #         return None
# #     try:
# #         token = cookie.split('token=')[1].split(';')[0]
# #         print(token)
# #         return validate_token(token)
# #     except IndexError:
# #         return None

# def get_login_page():
#     """Return the HTML login page."""
#     return """<!DOCTYPE html>
#     <html lang="en">
#     <head>
#         <meta charset="UTF-8">
#         <title>Login</title>
#     </head>
#     <body>
#         <h1>Login Page</h1>
#         <form action="/Desktop/webserv/WebServ/cgi/cgiScripts/login.py" method="POST">
#             <label for="username">Username:</label>
#             <input type="text" id="username" name="username" required><br><br>
#             <label for="password">Password:</label>
#             <input type="password" id="password" name="password" required><br><br>
#             <input type="submit" value="Login">
#         </form>
#     </body>
#     </html>"""

# def main():
#     cookie = os.environ.get("HTTP_COOKIE", None)
#     method = os.environ.get("REQUEST_METHOD", "GET")

#     if method == "POST":
#         # Handle login form submission
#         # content_length = int(os.environ.get('CONTENT_LENGTH', 0))
#         # post_data = sys.stdin.read(content_length)
#         # Simple username/password check (replace with actual auth logic)
#         post_data = "username:test,password:test"
#         if "username" in post_data and "password" in post_data:
#             userid = get_next_userid()
#             token = jwt_creator(userid)
#             store_token(userid, token)
#             send(f"<html><body><h1>Login successful! Welcome, user {userid}</h1></body></html>", token=token)
#         else:
#             send(f"<html><body><h1>Invalid login credentials</h1></body></html>")
#         return

#     # Check for existing valid cookie
#     payload = None #check_cookie(cookie)
#     if payload:
#         send(f"<html><body><h1>Welcome back, user {payload['userid']}</h1></body></html>")
#     else:
#         # Serve login page if no valid cookie
#         login_page = get_login_page()
#         send(login_page)

# if __name__ == "__main__":
#     main()

