#!/usr/bin/env python3

import cgi
import cgitb

cgitb.enable()  # Enable debugging

print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")
print()

form = cgi.FieldStorage()

name = form.getvalue("name", "Guest")
age = form.getvalue("age", "Unknown")

print(f"""
<html>
<head>
    <title>CGI Example</title>
</head>
<body>
    <h1>Hello, {name}!</h1>
    <p>You are {age} years old.</p>
</body>
</html>
""")
