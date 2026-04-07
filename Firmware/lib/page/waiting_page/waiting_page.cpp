#include <waiting_page.h>

String WaitingPage::style() {
    return String("<style>body { font-family: sans-serif; text-align: center; padding-top: 50px; }</style>");
}

String WaitingPage::script() {
    return String("");
}

String WaitingPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                + this->style()
                + this->script() +
            "</head>";

    return html;
}

String WaitingPage::body() {
    String body;

    body += R"(
        <body>
            <h2>Aguarde...</h2>
            <p>{TEXT}</p>
            <p>Você será redirecionado ao final do processo...</p>
            <script>
                setTimeout(function() { window.location.href = '{URL}'; }, {TIMEOUT});
            </script>
    )";

    body.replace("{TEXT}", this->text);
    body.replace("{URL}", this->url);
    body.replace("{TIMEOUT}", String(this->timeout));

    return body;
}

String WaitingPage::footer() {
    return R"(
        </body>
        </html>
    )";
}

String WaitingPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}