#include <analysis_page.h>

String AnalysisPage::style() {
    String style;

    style += R"(
            <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
            <style>
                body {
                    font-family: 'Segoe UI',
                    Tahoma, Geneva, Verdana, sans-serif;
                    margin: 0;
                    background: #f0f2f5;
                    color: #333;
                }
                .container {
                    max-width: 1200px;
                    margin: 20px auto;
                    padding: 0 20px;
                }
                .controls { 
                    background: white;
                    padding: 20px;
                    border-radius: 8px;
                    display: flex;
                    gap: 15px;
                    align-items: center;
                    justify-content: center;
                    margin-bottom: 20px;
                }
                .chart-box {
                    background: white;
                    padding: 20px;
                    border-radius: 8px;
                    height: 500px;
                    box-shadow: 0 2px 5px rgba(0,0,0,0.05);
                }
                input, button {
                    padding: 10px;
                    border: 1px solid #ddd;
                    border-radius: 4px;
                }
                button {
                    background: #007bff;
                    color: white;
                    border: none;
                    cursor: pointer;
                    font-weight: bold;
                }
                button:hover {
                    background: #1557b0;
                }
            </style>
    )";

    return style;
}

String AnalysisPage::script() {
    String script;

    script += R"(
        <script>
            let myChart;
            async function updateChart() {
                // Tempo base e o range em minutos para a construção do gráfico
                const tBaseInput = document.getElementById('t_base').value;
                const rangeMinutes = parseInt(document.getElementById('range_select').value);

                // Step em minutos do eixo X
                const STEP = 10; 

                if (!tBaseInput) return alert("Please select a datetime base.");

                // Definir Janela de Tempo (conversão Local para GMT/UTC)
                const dateStart = new Date(tBaseInput);
                const dateEnd = new Date(dateStart.getTime() + (rangeMinutes * 60000));

                // Formatação ISO para API (garante que o banco receba em GMT)
                const startGMT = dateStart.toISOString().replace('T', ' ').slice(0, 19);
                const endGMT = dateEnd.toISOString().replace('T', ' ').slice(0, 19);

                const url = `/api/samples?t_start=${encodeURIComponent(startGMT)}&t_end=${encodeURIComponent(endGMT)}`;

                console.log(`API URL: ${url}`);

                try {
                    const res = await fetch(url);
                    const json = await res.json();

                    const dataMap = {};
                    const absoluteStartMs = dateStart.getTime();

                    const dIn = [], dOut = [], dDiff = [];

                    let sumIn = 0;
                    let sumOut = 0;

                    json.data.forEach(s => {
                        const p = s.timestamp.split(' ');
                        const d = p[0].split('/');
                        const iso = `${d[2]}-${d[1]}-${d[0]}T${p[1]}`; 
                        const sampleDate = new Date(iso);
                        
                        // Cálculo do minuto absoluto
                        const minAbs = (sampleDate.getTime() - absoluteStartMs) / 60000;

                        if (minAbs >= 0 && minAbs <= rangeMinutes) {
                            dIn.push({ x: minAbs, y: s.in, dt: `${d[2]}:${d[1]}:${d[0]} ${p[1]}` });
                            dOut.push({ x: minAbs, y: s.out, dt: `${d[2]}:${d[1]}:${d[0]} ${p[1]}` });
                            dDiff.push({ x: minAbs, y: s.out - s.in, dt: `${d[2]}:${d[1]}:${d[0]} ${p[1]}` });

                            sumIn += parseFloat(s.in || 0);
                            sumOut += parseFloat(s.out || 0);
                        }
                    });

                    // Criamos as labels de STEP em STEP apenas para o eixo X
                    const labels = [];
                    for (let m = 0; m <= rangeMinutes; m += STEP) {
                        let label = m >= 60 ? `${Math.floor(m/60)}h${m%60}m` : `${m}min`;
                        labels.push(`+${label}`);
                    }

                    if(myChart) myChart.destroy();

                    myChart = new Chart(document.getElementById('myChart'), {
                        type: 'line',
                        data: {
                            // Labels servem como os "marcos" fixos no eixo X
                            labels: labels,
                            datasets: [
                                { 
                                    label: 'Entrada', 
                                    data: dIn, // Array de objetos {x, y}
                                    borderColor: '#2ecc71', 
                                    tension: 0.4,
                                    pointRadius: 4,
                                    showLine: true 
                                },
                                { 
                                    label: 'Saída', 
                                    data: dOut, 
                                    borderColor: '#e74c3c', 
                                    tension: 0.4,
                                    pointRadius: 4,
                                    showLine: true
                                },
                                { 
                                    label: 'Diferença', 
                                    data: dDiff, 
                                    borderColor: '#3498db', 
                                    borderDash: [5, 5],
                                    tension: 0.4,
                                    pointRadius: 4
                                }
                            ]
                        },
                        options: {
                            responsive: true,
                            maintainAspectRatio: false,
                            scales: {
                                x: {
                                    type: 'linear',
                                    min: 0,
                                    max: rangeMinutes,
                                    ticks: {
                                        stepSize: STEP, // Força o STEP
                                        callback: function(value) {
                                            if (value % 10 === 0) {
                                                let h = Math.floor(value / 60);
                                                let m = value % 60;
                                                return h > 0 ? `${h}h${m}m` : `${m}min`;
                                            }
                                        }
                                    }
                                },
                                y: { beginAtZero: true }
                            },
                            plugins: {
                                legend: {
                                    position: 'top'
                                },
                                subtitle: {
                                    display: true,
                                    text: `Total Entrada: ${sumIn.toLocaleString()} | Total Saída: ${sumOut.toLocaleString()}`,
                                    color: '#555',
                                    font: {
                                        size: 14,
                                        weight: 'bold'
                                    },
                                    padding: {
                                        bottom: 10
                                    }
                                },
                                tooltip: {
                                    callbacks: {
                                        title: (items) => {
                                            const min = items[0].parsed.x;
                                            const dt = items[0].raw.dt;

                                            const h = Math.floor(min / 60);
                                            const m = Math.floor(min % 60);
                                            const s = Math.round((min % 1) * 60);

                                            const relativeTime = `+${h}h ${m}m ${s}s`;

                                            return [
                                                `Data: ${dt}`,
                                                `Relativo: ${relativeTime}`
                                            ];
                                        }
                                    }
                                }
                            }
                        }
                    });

                } catch(e) {
                    console.error("An error occurred while executing the graph display script: ", e);
                }
            }

            window.onload = () => {
                const now = new Date();

                // Ajusta o fuso horário manualmente para compensar o deslocamento (offset)
                const offset = now.getTimezoneOffset() * 60000; // converte minutos para milissegundos
                const localISOTime = new Date(now - offset).toISOString().slice(0, 16);

                document.getElementById('t_base').value = localISOTime;
            };
        </script>
    )";

    return script;
}

String AnalysisPage::header() {
    String html;

    html += 
        "<!DOCTYPE html>"
        "<html lang=\"pt-br\">"
        "<head>"
            "<meta charset=\"UTF-8\">"
            "<title>Analysis Data Page - " + String(TITLE) + "</title>"
            + this->style()
            + this->script() +
        "</head>";

    return html;
}

String AnalysisPage::body() {
    return R"(
        <body>
        <div class="container">
            <div class="controls">
                <!--<label>Início:</label> <input type="datetime-local" id="t_start">
                <label>Fim:</label> <input type="datetime-local" id="t_end">-->
                <input type="datetime-local" id="t_base">
                <select id="range_select">
                    <option value="60" selected>+1h</option>
                    <option value="120">+2h</option>
                    <option value="180">+3h</option>
                    <option value="360">+6h</option>
                    <option value="540">+9h</option>
                    <option value="720">+12h</option>
                    <option value="960">+16h</option>
                    <option value="1080">+18h</option>
                    <option value="1440">+24h</option>
                </select>
                <button onclick='updateChart()'>GERAR RELATÓRIO</button>
            </div>
            <div class="chart-box">
                <canvas id="myChart"></canvas>
            </div>
        </div>
    )";
}

String AnalysisPage::footer() {
    return R"(
        </body>
        </html>
    )";
}

String AnalysisPage::page() {
    String html;

    html += this->header();
    html += this->body();
    html += this->footer();

    return html;
}