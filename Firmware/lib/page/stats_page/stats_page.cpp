#include <stats_page.h>

String StatsPage::style() {
    String style;

    style += R"(
        <style>
            body { background-color: #f8f9fa; }
            .stats-card { transition: all 0.3s ease; }
            .value-text { font-family: 'Courier New', Courier, monospace; font-weight: bold; }
            .status-online { color: #28a745; }
        </style>
    )";

    return style;
}

String StatsPage::script() {
    String script;

    script += R"(
        <script>
        async function updateStats() {
            const url = `/api/stats`;

            try {
                const response = await fetch(url);
                const json = await response.json();

                // Atualiza RAM
                document.getElementById('ram-free').innerText = json.ram.free.toLocaleString();
                document.getElementById('ram-min').innerText = json.ram.min_ever.toLocaleString();
                document.getElementById('ram-block').innerText = json.ram.largest_block.toLocaleString();
                document.getElementById('ram-stable').innerHTML = json.ram.fragmentation_stable ? 
                    '<span class="text-success">Sim</span>' : '<span class="text-danger">Não (Crítico)</span>';

                // Atualiza Flash
                document.getElementById('flash-total').innerText = json.flash.total.toLocaleString();
                document.getElementById('flash-used').innerText = json.flash.used.toLocaleString();
                document.getElementById('flash-free').innerText = json.flash.free.toLocaleString();

                let perc = (json.flash.used / json.flash.total) * 100;
                document.getElementById('flash-bar').style.width = perc + '%';
                document.getElementById('flash-bar').className = `progress-bar ${perc > 90 ? 'bg-danger' : 'bg-success'}`;

                // Outros
                let seconds = Math.floor(json.uptime_ms / 1000);
                document.getElementById('uptime').innerText = seconds + 's';
                document.getElementById('last-update').innerText = new Date().toLocaleTimeString();
                document.getElementById('status').className = 'badge bg-success';
                document.getElementById('status').innerText = 'Online';

            } catch (e) {
                console.error("An error occurred while executing the stats script: ", e);
                document.getElementById('status').className = 'badge bg-danger';
                document.getElementById('status').innerText = 'Offline';
            }
        }

        // Atualiza a cada 5 segundos
        setInterval(updateStats, 5000);
        // Chama a primeira vez imediatamente
        updateStats();
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
                "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css\" rel=\"stylesheet\">"
                + this->style()
                + this->script() +
            "</head>";

    return html;
}

String StatsPage::body() {
    String body;

    body += R"(
        <body>
        <div class="container py-5">
        <div class="d-flex justify-content-between align-items-center mb-4">
            <h1>ESP32 Stats</h1>
            <span id="status" class="badge bg-success"> - </span>
        </div>

        <div class="row g-4">
            <div class="col-md-6">
                <div class="card h-100 shadow-sm stats-card">
                    <div class="card-header bg-primary text-white">Memória RAM</div>
                    <div class="card-body">
                        <table class="table table-borderless">
                            <tr><td>Livre:</td><td class="value-text text-end"><span id="ram-free">-</span> bytes</td></tr>
                            <tr><td>Mínima Histórica:</td><td class="value-text text-end"><span id="ram-min">-</span> bytes</td></tr>
                            <tr><td>Maior Bloco Contínuo:</td><td class="value-text text-end"><span id="ram-block">-</span> bytes</td></tr>
                            <tr><td>Estável:</td><td class="value-text text-end" id="ram-stable">-</td></tr>
                        </table>
                    </div>
                </div>
            </div>

            <div class="col-md-6">
                <div class="card h-100 shadow-sm stats-card">
                    <div class="card-header bg-dark text-white">Armazenamento (Flash/LittleFS)</div>
                    <div class="card-body">
                        <table class="table table-borderless">
                            <tr><td>Total:</td><td class="value-text text-end"><span id="flash-total">-</span> bytes</td></tr>
                            <tr><td>Usado:</td><td class="value-text text-end text-danger"><span id="flash-used">-</span> bytes</td></tr>
                            <tr><td>Disponível:</td><td class="value-text text-end text-success"><span id="flash-free">-</span> bytes</td></tr>
                        </table>
                        <div class="progress mt-3">
                            <div id="flash-bar" class="progress-bar" role="progressbar" style="width: 0%"></div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="col-12 text-center">
                <p class="text-muted">
                    Uptime: <span id="uptime" class="fw-bold">-</span> |
                    Última atualização: <span id="last-update">-</span>
                </p>
            </div>
        </div>
    </div>
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