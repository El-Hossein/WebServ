print("Status: 302 Found")
print("Location: https://www.youtube.com/")
print("Content-Type: text/html\n")

print("""
<html>
  <body>
    <h1>Redirecting...</h1>
    <p>If not redirected, <a href="https://www.youtube.com/">click here</a>.</p>
  </body>
</html>
""")