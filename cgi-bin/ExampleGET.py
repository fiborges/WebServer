# #!/usr/bin/env python3

# import cgi
# import cgitb
# import os

# cgitb.enable()  # Enable debugging

# print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")
# print()

# form = cgi.FieldStorage()

# name = form.getvalue("name", "Unknown")
# age = form.getvalue("age", "Unknown")

# print(f"""
# <html>
# <head>
#     <title>CGI Example</title>
# </head>
# <body>
#     <h1>Hello, {name}!</h1>
#     <p>You are {age} years old.</p>
# </body>
# </html>
# """)

#!/usr/bin/env python3

import cgi
import cgitb
import os

cgitb.enable()  # Enable debugging

print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")
print()

form = cgi.FieldStorage()

#num1 = form.getvalue("num1")
#print(f"Valor recebido para num1: {num1}")

# Recebendo dois números do formulário
num1 = form.getvalue("num1", "0")
num2 = form.getvalue("num2", "0")
num3 = form.getvalue("num3", "0")

# Convertendo os números para inteiros e realizando a soma
try:
    soma = int(num1) + int(num2) + int(num3)
except ValueError:
    soma = "Erro: Um ou ambos os valores não são números inteiros."

print(f"""
<html>
<head>
    <title>CGI Example</title>
</head>
<body>
    <h1>Resultado da Soma</h1>
    <p>A soma de {num1} + {num2} + {num3} = {soma}.</p>
</body>
</html>
""")
