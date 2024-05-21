
<!-- deve ser rodado na pagina resources/website -->
<!-- php -S localhost:8080 -->

<?php
// Verifique se o formulário foi submetido
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Verifique se "key1" e "key2" foram enviados no POST
    if (isset($_POST['key1']) && isset($_POST['key2'])) {
        $key1 = $_POST['key1'];
        $key2 = $_POST['key2'];

        // Agora você pode usar $key1 e $key2 para o que você precisa
        echo "Chave 1: " . $key1 . "<br>";
        echo "Chave 2: " . $key2;
    }
}
?>