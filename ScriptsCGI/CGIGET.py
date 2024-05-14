#!/usr/bin/env python3
import time
import cgi
import cgitb

# Ativa a exibição de erros no navegador (útil para depuração)
cgitb.enable()

# Função para gerar o código HTML para o efeito de texto digitado com data e hora, texto estático e cores personalizadas
def generate_typing_effect_with_datetime_and_static_text(text_color="#ff00ff", background_color="#000000", static_text_color="#0000ff"):
    # Obtém a data e a hora atuais
    current_datetime = time.strftime("%Y-%m-%d %H:%M:%S")

    # Gera o código HTML para o efeito de texto digitado com data e hora, texto estático e cores personalizadas
    html_code = f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Typing Effect with Date and Time</title>
        <style>
            /* CSS para o efeito de texto digitado */
            body {{
                background-color: {background_color};
                display: flex;
                flex-direction: column;
                justify-content: center;
                align-items: center;
                height: 100vh;
                overflow: hidden;
                color: {text_color};
                font-family: Arial, sans-serif;
                font-size: 36px; /* Tamanho da letra aumentado */
            }}
            
            .typing {{
                overflow: hidden;
                white-space: nowrap;
                border-right: .15em solid {text_color};
                animation: typing 2s steps(20) infinite;
            }}
            
            @keyframes typing {{
                from {{
                    width: 0
                }}
                to {{
                    width: 100%
                }}
            }}

            /* Estilos para o texto estático */
            .static-text {{
                margin-top: 20px;
                font-size: 18px;
                color: {static_text_color};
            }}
        </style>
    </head>
    <body>
        <!-- Gera o texto com o efeito de digitação -->
        <div class="typing">Hello, World! {current_datetime}</div>

        <!-- Texto estático -->
        <div class="static-text">Webserver - 42 Porto</div>
    </body>
    </html>
    """

    return html_code

# Função principal do script CGI
def main():
    # Imprime o cabeçalho HTTP necessário para o CGI
    print("Content-type: text/html\n")

    # Gera e imprime o código HTML
    print(generate_typing_effect_with_datetime_and_static_text(
        text_color="#ff00ff", 
        background_color="#000000", 
        static_text_color="#0000ff"
    ))

# Chama a função principal
if __name__ == "__main__":
    main()
