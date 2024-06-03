#!/usr/bin/env python3
import os
import cgi
import cgitb
import json
cgitb.enable()

DATA_DIR = os.path.expanduser("~/fred/FRED/42_PORTO/13-WEBSERV/get-server-fred-tudo-para-grupo-2")

def list_files():
    try:
        files = os.listdir(DATA_DIR)
        return files
    except Exception as e:
        return []

def delete_file(file):
    try:
        file_path = os.path.join(DATA_DIR, file)
        if os.path.isfile(file_path):
            os.remove(file_path)
            return True
        else:
            return False
    except Exception as e:
        return False

form = cgi.FieldStorage()
if form.getvalue("file"):
    file_to_delete = form.getvalue("file")
    success = delete_file(file_to_delete)
    print("Content-Type: application/json\n")
    print(json.dumps({"success": success}))
else:
    files = list_files()
    print("Content-Type: application/json\n")
    print(json.dumps(files))