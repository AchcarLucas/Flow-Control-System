#include <raw_page.h>

String RawPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<title>Raw Data Page - " + String(TITLE) + " - Univesp</title>"
            "</head>";

    String mode = DEBUG ? "[Debug Mode]" : "[Production Mode]";

    html += "<h1 style=\"text-align: center;\">" + mode + " Raw Page</h1>";
    html += "<h2 style=\"text-align: center;\">\t\tDATABASE [" + this->database + "]</h1>";

    return html;
}

String RawPage::table() {
    String html;

    html += 
        "<table border='1' style='border-collapse: collapse; margin: auto; font-family: sans-serif; min-width: 65%; text-align: center;'>"
            "<tr>"
                "<th>ID</th>"
                "<th>Data/Hora</th>"
                "<th>Tempo Amostragem (Minutos)</th>"
                "<th>Entrada</th>"
                "<th>Saída</th>"
                "<th>Excluir</th>"
            "</tr>";

    for (const auto& sample : this->samples) {
        html += "<tr>";
        html += "<td>" + String(sample.id) + "</td>";
        html += "<td>" + String(sample.timestamp.c_str()) + "</td>";
        html += "<td>" + String(sample.sampling_time) + "</td>";
        html += "<td>" + String(sample.in) + "</td>";
        html += "<td>" + String(sample.out) + "</td>";
        html += "<td>"
            "<a href='/delete?id=" + String(sample.id) + "' onclick='return confirm(\"Are you sure you want to remove?\")'>"
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

    html += 
        "<style>"
            ".pagination { display: flex; list-style: none; padding: 0; gap: 5px; justify-content: center; font-family: sans-serif; }"
            ".pagination a { "
                "text-decoration: none; "
                "padding: 8px 12px; "
                "border: 1px solid #ccc; "
                "color: #333; "
                "border-radius: 4px; "
            "}"
            ".pagination a.active { background-color: #007bff; color: white; border-color: #007bff; font-weight: bold; }"
            ".pagination a:hover:not(.active) { background-color: #f0f0f0; }"
            ".disabled { color: #ccc !important; pointer-events: none; border-color: #eee !important; }"
        "</style>";

    // Sistema de Paginação
    html += "<ul class='pagination'>";

    // Botão anterior
    if (currentPage > 1) {
        html += "<li><a href='?page=" + String(this->currentPage - 1) + "'>Anterior</a></li>";
    } else {
        html += "<li><a class='disabled'>Anterior</a></li>";
    }

    // Status
    html += "<li><a class='active'>" + String(this->currentPage) + " / " + String(this->totalPages) + "</a></li>";

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

    // Estilo de botão real: cinza, com borda e sombra suave
    String style = 
                "color: black; background-color: #e7e7e7; border: 1px solid #ccc; "
                "padding: 10px 20px; text-align: center; text-decoration: none; "
                "display: inline-block; border-radius: 4px; font-family: sans-serif; "
                "margin-right: 10px; font-weight: bold; box-shadow: 1px 1px 2px #888888;";

    html += "<div style='text-align: center; margin: 20px 0; width: 100%;'>";

    if (DEBUG) {
        String simulateButton = "<a href='/simulate' style='%s'>Simulate Flow</a>";
        simulateButton.replace("%s", style);
        html += simulateButton;
    }

    String cleanupButton = "<a href='/cleanup' style='%s'>Cleanup Optimization Database</a>";
    cleanupButton.replace("%s", style);
    html += cleanupButton;

    String resetButton = "<a href='/reset' style='%s'>Reset Database</a>";
    resetButton.replace("%s", style);
    html += resetButton;

    String downloadButton = "<a href='/download' style='%s'>Download Database</a>";
    downloadButton.replace("%s", style);
    html += downloadButton;

    html += "</div>";

    return html;
}

String RawPage::page() {
    String html;

    html += this->header();
    html += this->table();
    html += this->pagination();
    html += this->tools();

    return html;
}