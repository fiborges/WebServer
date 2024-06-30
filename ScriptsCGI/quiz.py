#!/usr/bin/env python3
import cgi
import cgitb

cgitb.enable()  # Enable debugging

print("Content-Type: text/html\n")

form = cgi.FieldStorage()

# Fetching the answers from the form
q1_answer = form.getvalue('q1', 'No Answer Provided')
q2_answer = form.getvalue('q2', 'No Answer Provided')
q3_answer = form.getvalue('q3', 'No Answer Provided')

# Correct answers
correct_answers = {'q1': 'B', 'q2': 'D', 'q3': 'C'}
user_answers = {'q1': q1_answer, 'q2': q2_answer, 'q3': q3_answer}

# Calculate score
score = sum(1 for question, answer in user_answers.items() if answer == correct_answers[question])

# Generate the response
print(f"""
<html>
<head>
    <title>Quiz Results</title>
    <style>
        body {{
            background-color: black;
            color: white;
            font-family: 'Arial', sans-serif;
            font-size: 24px; /* Increase font size */
            text-align: center; /* Center text */
            margin-top: 100px; /* Add some space at the top */
        }}
    </style>
</head>
<body>
    <h1>Results of Your Quiz</h1>
    <div>
        {f"<div style='color: green;'>Congratulations! You scored {score} out of 3!</div>" if score == 3 else f"<div style='color: red;'>You scored {score} out of 3. Better luck next time!</div>"}
    </div>
</body>
</html>
""")

