#!/usr/bin/env python3

import cgi
import os
import cgitb
import mimetypes

# erros CGI
cgitb.enable()

# diretório onde os arquivos são armazenados
#upload_dir = "/home/brolivei/Documents/WebServer"
# diretório onde os arquivos são armazenados
upload_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

# ficheiros permitidos
allowed_file_types = ["image/jpeg", "image/png", "image/gif", "application/pdf", "text/plain", "application/msword"]

print("Content-Type: text/html\n")

form_html = '''
<!DOCTYPE html>
<html>
<head>
    <title>Upload de Ficheiro</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            background-color: #fff;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            text-align: center;
            width: 60%;
            max-width: 600px;
        }
        h2 {
            color: #333;
            font-size: 28px;
            margin-bottom: 20px;
        }
        input[type="file"] {
            font-size: 18px;
            margin-bottom: 20px;
            display: block;
            margin: 0 auto;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 15px 30px;
            font-size: 18px;
            border-radius: 5px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
        p {
            font-size: 18px;
            color: #333;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Upload de Ficheiro</h2>
        <form enctype="multipart/form-data" action="/cgi-bin/upload.py" method="post">
            <input type="file" name="file" />
            <br />
            <input type="submit" value="Upload" />
        </form>
    </div>
</body>
</html>
'''

# Processar o formulário
form = cgi.FieldStorage()
if "file" in form:
    fileitem = form["file"]

    if fileitem.filename:
        file_type, _ = mimetypes.guess_type(fileitem.filename)

        if file_type in allowed_file_types:
            if not os.path.exists(upload_dir):
                os.makedirs(upload_dir)

            filepath = os.path.join(upload_dir, os.path.basename(fileitem.filename))

            try:

                with open(filepath, 'wb') as fout:
                    fout.write(fileitem.file.read())
                print(f"<p>O arquivo '{fileitem.filename}' foi enviado com sucesso!</p>")
            except Exception as e:
                print(f"<p>Erro ao gravar o ficheiro: {str(e)}</p>")
        else:
            print(f"<p>Erro: Tipo de ficheiro não permitido. Apenas os seguintes tipos são permitidos: {', '.join(allowed_file_types)}</p>")
    else:
        print("<p>Erro: Nenhum ficheiro foi enviado.</p>")

print(form_html)

