#include <index_page.h>

String IndexPage::style() {
    String style;

    style += 
        R"(<style>
                body {
                    min-width: 720px;
                    min-height: 100vh;
                    height: auto;
                    font-family: sans-serif;
                    margin: 0;
                    display: flex;
                    flex-direction: column;
                    background-color: #f0f2f5;
                    overflow: auto;
                }
                header {
                    background: #007bff;
                    color: white;
                    padding: 1rem;
                    text-align: center;
                }
                .container {
                    padding: 20px;
                    width: 100%;
                    box-sizing: border-box;
                }
                .painel {
                    /* flex: 1; */
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
                    flex: 1;
                    width: 100%;
                    border: none;
                }
            </style>
        )";

    return style;
}

String IndexPage::script() {
    String script;

    script += R"(
        <script>
        </script>
    )";

    return script;
}

String IndexPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<meta name=\"viewport\" content=\"width=720\">"
                "<title>Index Page - " + String(TITLE) + " - Univesp</title>"
                + this->style()
                + this->script() +
            "</head>";

    String mode = DEBUG ? "[MODE] Debug" : "[MODE] Production";
    String buildVersion = "[BUILD VERSION] " + String(BUILD_VERSION);

    html += "<header><h2>" + String(TITLE) + " - Univesp - PI V</h2><h5>" + mode + " - " + buildVersion + "</h5></header>";

    return html;
}

String IndexPage::body() {
    String body;

    body += R"(
        <body>
            <div class="container">
                <div class="painel" style="min-height: 750px">
                    <div class="painel-header">Route: /analysis</div>
                    <iframe src="/analysis" title="Analysis Data"></iframe>
                </div>
                <div class="painel" style="min-height: 475px;">
                    <div class="painel-header">Route: /raw</div>
                    <iframe src="/raw" title="Raw Data"></iframe>
                </div>
                <div class="painel" style="min-height: 550px;">
                    <div class="painel-header">Route: /stats</div>
                    <iframe src="/stats" title="Raw Data"></iframe>
                </div>
            </div>
    )";

    return body;
}

String IndexPage::footer() {
    return R"(
        </body>
        </html>
    )";
}

String IndexPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}