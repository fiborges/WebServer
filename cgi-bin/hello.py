import os
import cgi
import cgitb

cgitb.enable()

form = cgi.FieldStorage()

print("<h1>Form Parameters</h1>")
print("<ul>")
for field in form:
    print("<li><b>{}</b>: {}</li>".format(field, form.getvalue(field)))
print("</ul>")

if "Content-Type" not in form:
	print("<H1>Error</H1>")
	print("You didn't upload nothing.")
else:
	print("<p>name:", form["name"].value)
	print("<p>addr:", form["addr"].value)

#print("Content-type: text/html")
#print()
#print("<TITLE>CGI script output</TITLE>")
#print("<H1>THis is my first CGI script</H1>")
#print("Hello, world! From Python")
