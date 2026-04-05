#include <index_page.h>

String IndexPage::style() {
    String style;

    style += 
        R"(<style>
                body {
                    font-family: sans-serif;
                    margin: 0;
                    display: flex;
                    flex-direction: column;
                    height: 100vh;
                    background-color: #f0f2f5;
                }
                header {
                    background: #333;
                    color: white;
                    padding: 1rem;
                    text-align: center;
                }
                .container {
                    display: flex;
                    flex: 1; /* Ocupa o restante da tela */
                    gap: 10px;
                    padding: 10px;
                }
                .painel {
                    flex: 1;
                    display: flex;
                    flex-direction: column;
                    border: 1px solid #ccc;
                    border-radius: 8px;
                    background: white;
                    overflow: hidden;
                }
                .painel-header {
                    background: #eee;
                    padding: 10px;
                    font-weight: bold;
                    border-bottom: 1px solid #ccc;
                }
                iframe {
                    width: 100%;
                    height: 100%;
                    border: none;
                }
            </style>
        )";

    return style;
}

String IndexPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                "<title>Index Page - " + String(TITLE) + " - Univesp</title>"
                + this->style() +
            "</head>";

    String mode = DEBUG ? "[Debug Mode]" : "[Production Mode]";

    html += "<header><h1 style=\"text-align: center;\">" + mode + " Index Page</h1><h2>" + String(TITLE) + " - Univesp - PI V</h2></header>";

    return html;
}

String IndexPage::body() {
    String body;

    body +=
        R"(<body>"
                <div class="container">
                    <div class="painel">
                        <div class="painel-header">Route: /analysis</div>
                        <iframe src="/analysis" title="Analysis Data"></iframe>
                    </div>
                    <div class="painel">
                        <div class="painel-header">Route: /raw</div>
                        <iframe src="/raw" title="Raw Data"></iframe>
                    </div>"
                </div>"
            </body>
        )";

    return body;
}

String IndexPage::page() {
    String html;

    html += this->header();
    html += this->body();

    return html;
}