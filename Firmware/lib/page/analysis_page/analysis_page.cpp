#include <analysis_page.h>

String AnalysisPage::header() {
    String html;
    String mode = DEBUG ? "[Debug Mode]" : "[Production Mode]";
    
    mode += "<!DOCTYPE html><html lang=\"pt-br\"><head>";
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
                <button onclick='updateChart()'>GERAR RELATÓRIO</button>
            </div>
            <div class="chart-box">
                <canvas id="myChart"></canvas>
            </div>
        </div>
    )";
}

String AnalysisPage::footer() {
    // JavaScript para lógica de busca e plotagem
    return R"(
        <script>
            let myChart;
            async function updateChart() {
                const start = document.getElementById('t_start').value.replace('T', ' ') + ":00";
                const end = document.getElementById('t_end').value.replace('T', ' ') + ":00";
                const url = `/api/samples?t_start=${encodeURIComponent(start)}&t_end=${encodeURIComponent(end)}`;

                try {
                    const res = await fetch(url);
                    const json = await res.json();
                    let acc = 0;
                    const labels = [], dIn = [], dOut = [], dDiff = [];

                    json.data.forEach(s => {
                        labels.push(`+${acc}min`);
                        acc += s.sampling_time;
                        dIn.push(s.in);
                        dOut.push(s.out);
                        dDiff.push(s.out - s.in);
                    });

                    if(myChart) myChart.destroy();
                    myChart = new Chart(document.getElementById('myChart'), {
                        type: 'line',
                        data: {
                            labels: labels,
                            datasets: [
                                { label: 'Entrada', data: dIn, borderColor: '#2ecc71', tension: 0.4 },
                                { label: 'Saída', data: dOut, borderColor: '#e74c3c', tension: 0.4 },
                                { label: 'Diferença', data: dDiff, borderColor: '#3498db', borderDash: [5,5], tension: 0.4 }
                            ]
                        },
                        options: { responsive: true, maintainAspectRatio: false }
                    });
                } catch(e) { alert("Erro ao buscar dados do banco."); }
            }
            window.onload = () => {
                const now = new Date();
                document.getElementById('t_end').value = now.toISOString().slice(0, 16);
                now.setHours(now.getHours() - 1);
                document.getElementById('t_start').value = now.toISOString().slice(0, 16);
            };
        </script>
    </body></html>
    )";
}

String AnalysisPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}