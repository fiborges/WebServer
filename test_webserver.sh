#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RESET='\033[0m'

# Define sleep duration in seconds
x=5

function print_test_case {
    echo -e "\n"
    echo -e "========================================================================="
    echo -e "\n"
    echo -e "${CYAN}Testing: ${1}${RESET}"
    echo -e "${YELLOW}Expected Result: ${2}${RESET}"
    sleep $x
}


# Basic GET Requests
print_test_case "GET request to root directory" "Should return the root directory contents or index file."
curl -X GET http://localhost:8080/


print_test_case "GET request to uploads directory" "Should return the contents of the uploads directory."
curl -X GET http://localhost:8080/uploads/


# POST Requests
print_test_case "POST request to upload a file" "Should successfully upload a file."
curl -X POST -F "file=@./resources/website/upload.html" http://localhost:8080/uploads/
#curl -X POST -F "file=@/home/firibeir/Desktop/WebServer/resources/website/upload.html" http://localhost:8080/uploads/


# DELETE Requests
print_test_case "DELETE request to remove a file" "..."
curl -X DELETE http://localhost:8080/uploads/CR.cpp


# CGI Script Execution
print_test_case "GET request to execute a .py CGI script" "Should execute the Python CGI script and return its output."
curl -X GET http://localhost:8080/cgi-bin/ExampleGET.py


print_test_case "POST request to execute a .py CGI script" "Should execute the Python CGI script with POST data and return its output."
curl -X POST -H "Content-Type: application/x-www-form-urlencoded" -d "num1=10&num2=20&num3=30" http://localhost:8080/cgi-bin/ExampleGET.py
echo -e "========================================================================="

# # Error Pages
# print_test_case "PUT request to root (Method Not Allowed)" "Should return 405 Method Not Allowed error page."
# curl -X PUT http://localhost:8080/

# print_test_case "POST request to CGI script causing error" "Should return 500 Internal Server Error page."
# curl -X POST -d "trigger=error" http://localhost:8080/cgi-bin/error_script.py

# # Autoindex
# print_test_case "GET request to autoindexed directory" "Should return the directory listing."
# curl -X GET http://localhost:8080/uploads/

# # Client Body Size Limit
# print_test_case "POST request with large file exceeding client body size limit" "Should return an error indicating the file is too large."
# dd if=/dev/zero of=largefile.txt bs=1024 count=200
# curl -X POST -F "file=@largefile.txt" http://localhost:8080/uploads/

# # Custom Headers
# print_test_case "GET request with custom header" "Should return the response with the custom header included."
# curl -X GET -H "X-Custom-Header: value" http://localhost:8080/

# # Other HTTP Methods
# print_test_case "HEAD request to root directory" "Should return headers only."
# curl -I http://localhost:8080/

# print_test_case "OPTIONS request to root directory" "Should return allowed HTTP methods."
# curl -X OPTIONS http://localhost:8080/

# # Testing Redirections
# print_test_case "GET request to a redirected URL" "Should return the new location of the resource."
# curl -L http://localhost:8080/redirect

# # Testing different file types
# print_test_case "GET request for an HTML file" "Should return the HTML content."
# curl -X GET http://localhost:8080/index.html

# print_test_case "GET request for a CSS file" "Should return the CSS content."
# curl -X GET http://localhost:8080/style.css

# print_test_case "GET request for a JavaScript file" "Should return the JavaScript content."
# curl -X GET http://localhost:8080/script.js

# print_test_case "GET request for an image file" "Should return the image content."
# curl -X GET http://localhost:8080/image.png

# # Testing JSON response
# print_test_case "GET request for a JSON response" "Should return JSON data."
# curl -X GET http://localhost:8080/data.json

# # Testing XML response
# print_test_case "GET request for an XML response" "Should return XML data."
# curl -X GET http://localhost:8080/data.xml

# # Clean up large test file
# rm -f largefile.txt

echo -e "\n"
echo -e "${GREEN}All tests completed.${RESET}"
