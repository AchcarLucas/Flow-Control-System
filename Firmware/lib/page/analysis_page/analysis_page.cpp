#include <analysis_page.h>

String AnalysisPage::header() {
    String mode = DEBUG ? "[Debug Mode]" : "[Production Mode]";
    
    String html = "<!DOCTYPE html><html lang=\"pt-br\"><head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<title>Analysis Data Page - " + String(TITLE) + "</title>";
    
    // Injeção de CSS via Raw String para manter o C++ limpo
    html += R"(
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
            <style>
                body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f0f2f5; color: #333; }
                .header-section { background: #1a73e8; color: white; padding: 20px; text-align: center; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
                .container { max-width: 1200px; margin: 20px auto; padding: 0 20px; }
                .controls { background: white; padding: 20px; border-radius: 8px; display: flex; gap: 15px; align-items: center; justify-content: center; margin-bottom: 20px; }
                .chart-box { background: white; padding: 20px; border-radius: 8px; height: 500px; box-shadow: 0 2px 5px rgba(0,0,0,0.05); }
                input, button { padding: 10px; border: 1px solid #ddd; border-radius: 4px; }
                button { background: #1a73e8; color: white; border: none; cursor: pointer; font-weight: bold; }
                button:hover { background: #1557b0; }
            </style>
        )";
    
    html += "</head><body>";
    html += "<div class=\"header-section\">";
    html += "<h1>" + mode + " Analysis Page</h1>";
    html += "<h2>DATABASE [" + this->database + "]</h2>";
    html += "</div>";
    
    return html;
}

String AnalysisPage::body() {
    return R"(
        <div class="container">
            <div class="controls">
                <label>Início:</label> <input type="datetime-local" id="t_start">
                <label>Fim:</label> <input type="datetime-local" id="t_end">
                <button onclick=\"updateChart()\">GERAR RELATÓRIO</button>
            </div>
            <div class="chart-box">
                <canvas id="myChart"></canvas>
            </div>
        </div>
    )";
}

String AnalysisPage::page() {
    String html;

    //html += this->header();

    return html;
}