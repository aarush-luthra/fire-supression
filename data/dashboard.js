// DOM Elements
const connectBtn = document.getElementById('connectBtn');
const statusBadge = document.getElementById('connectionStatus');
const systemStateBadge = document.getElementById('systemStateBadge');
const riskValue = document.getElementById('riskValue');
const zScoreValue = document.getElementById('zScoreValue');
const flameStatus = document.getElementById('flameStatus');
const trendArrow = document.getElementById('trendArrow');
const trendValue = document.getElementById('trendValue');
const trendLabel = document.getElementById('trendLabel');
const rawGasValue = document.getElementById('rawGasValue');
const rawGasLabel = document.getElementById('rawGasLabel');
const flamePersistValue = document.getElementById('flamePersistValue');
const flamePersistLabel = document.getElementById('flamePersistLabel');
const logList = document.getElementById('eventLog');
const currentDate = document.getElementById('currentDate');

// Update Date
const now = new Date();
currentDate.innerText = now.toLocaleDateString('en-US', { weekday: 'long', month: 'short', day: 'numeric' });

// Chart Configuration
const ctx = document.getElementById('liveChart').getContext('2d');
const liveChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Risk Score',
            borderColor: '#c53030',
            backgroundColor: 'rgba(197, 48, 48, 0.1)',
            data: [],
            fill: true,
            tension: 0.4
        }, {
            label: 'Gas (Scaled)',
            borderColor: '#3182ce',
            backgroundColor: 'rgba(49, 130, 206, 0.1)',
            data: [],
            fill: true,
            tension: 0.4
        }, {
            label: 'Trend (Scaled)',
            borderColor: '#d69e2e',
            backgroundColor: 'rgba(214, 158, 46, 0.1)',
            data: [],
            fill: true,
            tension: 0.4
        }]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        scales: {
            x: { display: false },
            y: { beginAtZero: true, max: 100 }
        },
        plugins: {
            legend: {
                labels: { font: { family: 'Outfit', size: 11 } }
            }
        }
    }
});

// Serial Connection Variables
let port;
let reader;
let keepReading = false;
let eventSource;
let lastState = '';

// Helper to log to the UI
function logToDashboard(message) {
    const li = document.createElement('li');
    li.className = 'log-item';
    li.innerHTML = `<strong>${new Date().toLocaleTimeString()}</strong>: ${message}`;
    logList.prepend(li);
    if (logList.children.length > 50) {
        logList.removeChild(logList.lastChild);
    }
}

// Connect Button Listener
connectBtn.addEventListener('click', async () => {
    if (window.location.protocol.startsWith('http')) {
        logToDashboard("Manual reconnection attempt...");
        if (eventSource && eventSource.readyState !== EventSource.CLOSED) {
            eventSource.close();
        }
        startSSE();
        return;
    }

    if ('serial' in navigator) {
        try {
            port = await navigator.serial.requestPort();
            await port.open({ baudRate: 115200 });

            statusBadge.innerText = "Connected (USB)";
            statusBadge.className = 'badge safe';
            connectBtn.innerText = "Disconnect";
            logToDashboard("Connected via USB Serial");

            keepReading = true;
            readSerialLoop();
        } catch (err) {
            console.error("Serial Connection Error:", err);
            logToDashboard(`Serial Error: ${err.message}`);
            alert("Failed to connect via USB. See logs.");
        }
    } else {
        alert("Web Serial API not supported. Please use Chrome/Edge or connect via WiFi.");
    }
});

// Auto-connect if hosted on ESP32 (Wi-Fi)
if (window.location.hostname && window.location.protocol.startsWith('http')) {
    console.log("Hosted on device, attempting SSE connection...");
    connectBtn.innerText = "Reconnect WiFi";
    startSSE();
} else {
    logToDashboard("Ready. Connect via USB or load from Device IP.");
}

function startSSE() {
    logToDashboard("Connecting to Event Stream at /events...");
    statusBadge.innerText = "Connecting...";
    statusBadge.className = 'badge warning';

    if (eventSource) eventSource.close();

    eventSource = new EventSource('/events');

    eventSource.onopen = function () {
        console.log("SSE Connected");
        logToDashboard("WiFi Connected successfully.");
        statusBadge.innerText = "Connected (Wi-Fi)";
        statusBadge.className = 'badge safe';
    };

    eventSource.onerror = function (e) {
        console.log("SSE Error", e);
        if (eventSource.readyState != EventSource.OPEN) {
            statusBadge.innerText = "Reconnecting...";
            statusBadge.className = 'badge warning';
        }
    };

    eventSource.onmessage = function (e) {
        parseData(e.data);
    };
}

async function readSerialLoop() {
    const textDecoder = new TextDecoderStream();
    const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
    const reader = textDecoder.readable.getReader();

    let buffer = "";

    try {
        while (true) {
            const { value, done } = await reader.read();
            if (done) break;

            buffer += value;
            const lines = buffer.split('\n');
            buffer = lines.pop();

            for (const line of lines) {
                parseData(line.trim());
            }
        }
    } catch (error) {
        console.error("Read Error:", error);
    } finally {
        reader.releaseLock();
    }
}

function parseData(line) {
    // New format: Gas:1200,ZScore:0.5,Trend:1.2,Risk:2.5,Flame:0,FlamePersist:0,State:SAFE
    const regex = /Gas:([\d.]+),ZScore:([\d.-]+),Trend:([\d.-]+),Risk:([\d.]+),Flame:(\d),FlamePersist:(\d+),State:(\w+)/;
    const match = line.match(regex);

    if (match) {
        const gas = parseFloat(match[1]);
        const zScore = parseFloat(match[2]);
        const trend = parseFloat(match[3]);
        const risk = parseFloat(match[4]);
        const flame = parseInt(match[5]);
        const flamePersist = parseInt(match[6]);
        const state = match[7];

        updateDashboard(gas, zScore, trend, risk, flame, flamePersist, state);
    }
}

function updateDashboard(gas, zScore, trend, risk, flame, flamePersist, state) {
    // Risk Score
    riskValue.innerText = risk.toFixed(1);

    // Z-Score
    zScoreValue.innerText = zScore.toFixed(2);

    // System State Badge
    systemStateBadge.innerText = state;
    systemStateBadge.className = 'badge';
    if (state === 'SAFE') systemStateBadge.classList.add('safe');
    else if (state === 'WARNING') systemStateBadge.classList.add('warning');
    else systemStateBadge.classList.add('emergency');

    // Log state changes
    if (state !== lastState && lastState !== '') {
        logToDashboard(`State changed: ${lastState} → ${state}`);
    }
    lastState = state;

    // Flame Status
    if (flame === 1) {
        flameStatus.innerText = "FIRE DETECTED";
        flameStatus.style.color = "var(--status-danger-text)";
    } else {
        flameStatus.innerText = "Safe";
        flameStatus.style.color = "var(--text-main)";
    }

    // Gas Trend
    trendValue.innerText = Math.abs(trend).toFixed(1);
    if (trend > 2.0) {
        trendArrow.innerText = "↑";
        trendArrow.className = "trend-arrow trend-rising";
        trendLabel.innerText = "Rising — elevated risk";
    } else if (trend < -2.0) {
        trendArrow.innerText = "↓";
        trendArrow.className = "trend-arrow trend-falling";
        trendLabel.innerText = "Falling — improving";
    } else {
        trendArrow.innerText = "—";
        trendArrow.className = "trend-arrow trend-stable";
        trendLabel.innerText = "Stable";
    }

    // Raw Gas Level
    rawGasValue.innerText = gas.toFixed(0);
    if (gas >= 2500) {
        rawGasLabel.innerText = "DANGER — very high";
        rawGasLabel.style.color = "var(--status-danger-text)";
    } else if (gas >= 800) {
        rawGasLabel.innerText = "Elevated";
        rawGasLabel.style.color = "var(--status-warning-text)";
    } else {
        rawGasLabel.innerText = "Normal range";
        rawGasLabel.style.color = "var(--text-secondary)";
    }

    // Flame Persistence
    flamePersistValue.innerText = flamePersist;
    if (flamePersist >= 3) {
        flamePersistLabel.innerText = "Confirmed fire";
        flamePersistLabel.style.color = "var(--status-danger-text)";
    } else if (flamePersist > 0) {
        flamePersistLabel.innerText = "Flicker detected";
        flamePersistLabel.style.color = "var(--status-warning-text)";
    } else {
        flamePersistLabel.innerText = "No flame detected";
        flamePersistLabel.style.color = "var(--text-secondary)";
    }

    // Update Chart
    addDataToChart(liveChart, risk, gas / 40, Math.max(0, trend * 2));
}

function addDataToChart(chart, risk, gas, trend) {
    const label = new Date().toLocaleTimeString();

    if (chart.data.labels.length > 50) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
        chart.data.datasets[1].data.shift();
        chart.data.datasets[2].data.shift();
    }

    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(risk);
    chart.data.datasets[1].data.push(gas);
    chart.data.datasets[2].data.push(trend);
    chart.update();
}
