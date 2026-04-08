#include <stats_page.h>

String StatsPage::style() {
    String style;

    style += R"(
        <style>
        </style>
    )";

    return style;
}

String StatsPage::script() {
    String script;

    script += R"(
        <script>
        </script>
    )";

    return script;
}

String StatsPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<title>Stats Data Page - " + String(TITLE) + " - Univesp</title>"
                + this->style()
                + this->script() +
            "</head>";

    return html;
}

String StatsPage::body() {
    String body;

    body += R"(
        <body>
    )";

    return body;
}

String StatsPage::footer() {
    return R"(
        </body>
        </html>
    )";
}

String StatsPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}