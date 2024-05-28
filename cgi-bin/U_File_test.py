# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/06 14:28:15 by brolivei          #+#    #+#              #
#    Updated: 2024/05/06 16:49:10 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import cgi
import cgitb

# Enable CGI traceback for debugging
cgitb.enable()

# Set the upload directory
upload_dir = 'U_DATA/'

# Ensure the upload directory exists
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

print("\n\n==========I there! I'm PYTHON SCRIPT==========\n\n")

# Receive the POST request
form = cgi.FieldStorage()

# Check if a file was uploaded
if 'file' in form:
    file_item = form['file']
    if file_item.filename:
        # Save the file to the upload directory
        filename = os.path.join(upload_dir, os.path.basename(file_item.filename))
        with open(filename, 'wb') as f:
            f.write(file_item.file.read())

        print("Content-Type: text/html\n")
        print("<html><body>")
        print("<h2>File uploaded successfully!</h2>")
        print("<p>Filename: %s</p>" % filename)
        print("</body></html>")
    else:
        print("Content-Type: text/html\n")
        print("<html><body>")
        print("<h2>Error: No file selected!</h2>")
        print("</body></html>")
else:
    print("Content-Type: text/html\n")
    print("<html><body>")
    print("<h2>Error: No file received!</h2>")
    print("</body></html>")

