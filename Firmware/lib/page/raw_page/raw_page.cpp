#include <raw_page.h>

String RawPage::style() {
    String style;

    style += 
        R"(<style>
            .pagination {
                display: flex;
                list-style: none;
                padding: 0; gap: 5px;
                justify-content: center;
                font-family: sans-serif;
            }
            .pagination a { 
                text-decoration: none; 
                padding: 8px 12px;
                border: 1px solid #ccc;
                color: #333;
                border-radius: 4px;
            }
            .pagination a.active {
                background-color: #007bff;
                color: white; border-color: #007bff;
                font-weight: bold;
            }
            .pagination a:hover:not(.active) {
                background-color: #f0f0f0;
            }
            .disabled {
                color: #ccc !important;
                pointer-events: none;
                border-color: #eee !important;
            }
            .button {
                color: black;
                background-color: #e7e7e7;
                border: 1px solid #ccc;
                padding: 10px 20px;
                text-align: center;
                text-decoration: none;
                display: inline-block;
                border-radius: 4px;
                font-family: sans-serif;
                margin-right: 10px;
                font-weight: bold;
                box-shadow: 1px 1px 2px #888888;
            }
        </style>
    )";

    return style;
}

String RawPage::script() {
    String script;

    script += R"(
        <script>
        </script>
    )";

    return script;
}

String RawPage::table() {
    String html;

    html += R"(
            <table border='1' style='border-collapse: collapse; margin: auto; font-family: sans-serif; min-width: 65%; text-align: center;'>
                <tr>
                    <th>ID</th>
                    <th>Data/Hora</th>
                    <th>Tempo Amostragem (Minutos)</th>
                    <th>Entrada</th>
                    <th>Saída</th>
                    <th>Excluir</th>
                </tr>
            )";

    for (const auto& sample : this->samples) {
        html += "<tr>";
        html += "<td>" + String(sample.id) + "</td>";
        html += "<td>" + String(sample.timestamp.c_str()) + "</td>";
        html += "<td>" + String(sample.sampling_time) + "</td>";
        html += "<td>" + String(sample.in) + "</td>";
        html += "<td>" + String(sample.out) + "</td>";
        html += "<td>"
            "<a href='/delete?id=" + String(sample.id) + "' onclick='return confirm(\"Are you sure you want to remove the ID [" + String(sample.id) + "] ?\")'>"
                "🗑️"
            "</a>"
        "</td>";
        html += "</tr>";
    }

    html += "</table><br>";

    return html;
}

String RawPage::pagination() {
    String html;

    // Sistema de Paginação
    html += "<ul class='pagination'>";

    // Botão anterior
    if (currentPage > 1) {
        html += "<li><a href='?page=" + String(this->currentPage - 1) + "'>Anterior</a></li>";
    } else {
        html += "<li><a class='disabled'>Anterior</a></li>";
    }

    // Status
    if (this->totalPages != 0)
        html += "<li><a class='active'>" + String(this->currentPage) + " / " + String(this->totalPages) + "</a></li>";
    else
        html += "<li><a class='active'> - </a></li>";

    // Botão próximo
    if (currentPage < totalPages) {
        html += "<li><a href='?page=" + String(this->currentPage + 1) + "'>Próxima</a></li>";
    } else {
        html += "<li><a class='disabled'>Próxima</a></li>";
    }

    html += "</ul><br>";

    return html;
}

String RawPage::tools() {
    String html;

    html += "<div style='text-align: center; margin: 20px 0; width: 100%;'>";
    html += DEBUG ? "<a href='/simulate' class='button'>Simulate Flow</a>" : "";
    html += "<a href='/cleanup' class='button'>Cleanup Optimization Database (" + String(CLEANUP) + ")</a>";
    html += "<a href='/reset' class='button'>Reset Database</a>";
    html += "<a href='/download' class='button'>Download Database</a>";
    html += "</div>";

    return html;
}

String RawPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<title>Raw Data Page - " + String(TITLE) + " - Univesp</title>"
                + this->style()
                + this->script() +
            "</head>";

    return html;
}

String RawPage::body() {
    String html;

    html += "<body>";

    html += this->table();
    html += this->pagination();
    html += this->tools();

    return html;
}

String RawPage::footer() {
    return R"(
        </body>
        </html>
    )";
}

String RawPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}