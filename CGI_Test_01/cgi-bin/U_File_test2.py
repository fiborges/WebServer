# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test2.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/06 16:52:56 by brolivei          #+#    #+#              #
#    Updated: 2024/05/09 16:48:14 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import sys

# Set the upload directory
upload_dir = 'U_DATA/'

# Ensure the upload directory exists
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

#print("\n\n==========I there! I'm PYTHON SCRIPT==========\n\n")

if len(sys.argv) > 1:
    post_data = sys.argv[1]
else:
    post_data = ""

# Extract file content from POST data
start_index = post_data.find('\r\n\r\n') + 4  # Find the end of headers
file_content = post_data[start_index:]



# Extract filename from POST data
filename = None
if 'filename="' in post_data:
    start_index = post_data.find('filename="') + len('filename="')
    end_index = post_data.find('"', start_index)
    filename = post_data[start_index:end_index]

if filename and file_content:
    # Save the file to the upload directory
    filename = os.path.join(upload_dir, filename)
    with open(filename, 'wb') as f:
        f.write(file_content)
        #f.write(file_content.encode())

    print("HTTP/1.1 200 OK\r\n")
    print("Content-Type: text/html\r\n")
    print("<!DOCTYPE html>")
    print("<html><body>")
    print("<h2>File uploaded successfully!</h2>")
    print("<p>Filename: %s</p>" % filename)
    print("</body></html>")
else:
    print("Content-Type: text/html\n")
    print("<html><body>")
    print("<h2>Error: No file received!</h2>")
    print("</body></html>")
