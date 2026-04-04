#include <analysis_page.h>

String AnalysisPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<title>Analysis Page - Monitor de Fluxo - Univesp</title>"
            "</head>";

    String title = DEBUG ? "[Debug Mode]" : "[Production Mode]";

    html += "<h1 style=\"text-align: center;\">" + title + " Analysis Page - Monitor de Fluxo - Univesp</h1>";
    html += "<h2 style=\"text-align: center;\">\t\tDATABASE [" + this->database + "]</h1>";

    return html;
}

String AnalysisPage::page() {
    String html;

    html += this->header();

    return html;
}