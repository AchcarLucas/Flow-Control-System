#include <index_page.h>

String IndexPage::header() {
    String html;

    html += "<!DOCTYPE html>"
            "<html lang=\"pt-br\">"
            "<head>"
                "<meta charset=\"UTF-8\">"
                "<title>Index Page - Monitor de Fluxo - Univesp</title>"
            "</head>";

    String title = DEBUG ? "[Debug Mode]" : "[Production Mode]";

    html += "<h1 style=\"text-align: center;\">" + title + " Index Page - Monitor de Fluxo - Univesp</h1>";

    return html;
}

String IndexPage::page() {
    String html;

    html += this->header();

    return html;
}