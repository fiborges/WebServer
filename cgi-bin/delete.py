#!/usr/bin/env python3
import os
import cgi
import cgitb
cgitb.enable()

DATA_DIR = os.path.expanduser("~/fred/FRED/42_PORTO/13-WEBSERV/get-server-fred-tudo-para-grupo-2/cgi-bin")

def list_files():
    try:
        with open(os.path.join(DATA_DIR, "fileList.txt"), "r") as f:
            files = f.read().splitlines()
        print(f"Files: {files}")  # Debug print
        options = ''.join(f'<option value="{file}">{file}</option>' for file in files)
        return options
    except Exception as e:
        print(f"Exception: {e}")  # Debug print
        return ""

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
    print("Content-Type: text/plain\n")
    print("success" if success else "failure")
else:
    files = list_files()
    print("Content-Type: text/html\n")
    print(files)
