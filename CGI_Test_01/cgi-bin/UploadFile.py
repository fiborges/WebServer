# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    UploadFile.py                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/03 13:43:36 by brolivei          #+#    #+#              #
#    Updated: 2024/05/03 14:44:07 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import cgi # provides support for handling CGI requests in python

#---------------------------------------------------------------

import os # Using operating system-dependent functionality in Python.
# Used to checking if a directory exists and creating directories

#-----------------------------------------------------------

# Set the directory where uploaded files will be stored

# 	Defines a variable named "UPLOAD_DIR" and assigns it the path to the directory
# where uploaed files will be saved.

UPLOAD_DIR = "../DATA_UPLOAD"

#---------------------------------------------------------------

# Create the upload directory if it does not exist

# 	This lines check if the upload directory specified by "UPLOAD_DIR" exists.
# If it does not exist os.path.exists(UPLOAD_DIR) returns False, and the script
# creates it using "os.makedirs(UPLOAD_DIR)".

if not os.path.exists(UPLOAD_DIR):
	os.makedirs(UPLOAD_DIR)

#---------------------------------------------------------------

# Set the directory to save uploaded files

# 	This is equivalent to "cd"command in bash, to change the current directory to
# the upload directory.

os.chdir(UPLOAD_DIR)

#---------------------------------------------------------------

# Create instance of FieldStorage

# 	This line creates a "FieldStorage" object named "form".
# The "FieldStorage" object represents the form data submitted in the HTTP request.
# It allows to access the values of form fields, including uploaded files.

form = cgi.FieldStorage()

#---------------------------------------------------------------

# Get the file data from the form

# 	This line retrieves the value of the form field named "filename" from the "form"
# object. We need to adjust this line if the name of the file input field in the HTML
# form is different.

fileitem = form['filename']

#---------------------------------------------------------------

# Check if the file was uploaded
# 	This line checks if a file was uploaded by examining the "filename" attribute
# of the "fileitem" object. If a file was uploaded, in other words, the file name is
# not an empty string, the condition is True.
if fileitem.filename:

#---------------------------------------------------------------

	# Create a new file in the upload directory
	
	filename = os.path.join(UPLOAD_DIR, os.path.basename(fileitem.filename))
	with open(filename, 'wb') as f:
		# Write the file data to the new file
		f.write(fileitem.file.read())
	message = 'The file "' + filename + ' " was uploaded successfully!'
else:
	message = 'No file was uploaded.'

# HTML response
print("Content-type: text/html\n")
print("<html>")
print("<head>")
print("<title>File Upload</title>")
print("</head>")
print("<body>")
print("<h1>File Upload</h1>")
print("<p>" + message + "</p>")
print("</body>")
print("</html>")
