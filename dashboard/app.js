// DOM Elements
const connectBtn = document.getElementById('connectBtn');
const statusBadge = document.getElementById('connectionStatus');
const systemStateBadge = document.getElementById('systemStateBadge');
const riskValue = document.getElementById('riskValue');
const zScoreValue = document.getElementById('zScoreValue');
const flameStatus = document.getElementById('flameStatus');
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
            borderColor: '#c53030', // Red
            backgroundColor: 'rgba(197, 48, 48, 0.1)',
            data: [],
            fill: true,
            tension: 0.4
        }, {
            label: 'Gas Value (Scaled)',
            borderColor: '#3182ce', // Blue
            backgroundColor: 'rgba(49, 130, 206, 0.1)',
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
        }
    }
});

// Serial Connection Variables
let port;
let reader;
let keepReading = false;

// Connect Button Listener
connectBtn.addEventListener('click', async () => {
    if ('serial' in navigator) {
        try {
            port = await navigator.serial.requestPort();
            await port.open({ baudRate: 115200 });

            statusBadge.innerText = "Connected";
            statusBadge.classList.remove('warning', 'emergency');
            statusBadge.classList.add('safe');
            connectBtn.innerText = "Disconnect";

            keepReading = true;
            readSerialLoop();
        } catch (err) {
            console.error("Serial Connection Error:", err);
            alert("Failed to connect. Make sure your ESP32 is plugged in and no other app (like VS Code Monitor) is using the port.");
        }
    } else {
        alert("Web Serial API not supported in this browser. Please use Chrome or Edge.");
    }
});

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
            buffer = lines.pop(); // Keep incomplete line in buffer

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
    // Expected format: "Gas:1200,ZScore:0.5,Risk:2.5,Flame:0,State:SAFE"
    // Regex to extract values
    const regex = /Gas:([\d.]+),ZScore:([\d.-]+),Risk:([\d.]+),Flame:(\d),State:(\w+)/;
    const match = line.match(regex);

    if (match) {
        const gas = parseFloat(match[1]);
        const zScore = parseFloat(match[2]);
        const risk = parseFloat(match[3]);
        const flame = parseInt(match[4]);
        const state = match[5];

        updateDashboard(gas, zScore, risk, flame, state);
    }
}

function updateDashboard(gas, zScore, risk, flame, state) {
    // Update Text Values
    riskValue.innerText = risk.toFixed(1);
    zScoreValue.innerText = zScore.toFixed(2);

    // Update State Badge
    systemStateBadge.innerText = state;
    systemStateBadge.className = 'badge'; // Reset classes
    if (state === 'SAFE') systemStateBadge.classList.add('safe');
    else if (state === 'WARNING') systemStateBadge.classList.add('warning');
    else systemStateBadge.classList.add('emergency');

    // Update Flame Status
    if (flame === 1) { // 1 means NO FLAME in raw logic, wait... 
        // In my C++ code:
        // Serial.print(",Flame:"); Serial.print(flameDetected);
        // flameDetected = true if LOW. So 1 = FIRE.
        flameStatus.innerText = "FIRE DETECTED";
        flameStatus.style.color = "red";
    } else {
        flameStatus.innerText = "Safe";
        flameStatus.style.color = "var(--text-main)";
    }

    // Update Chart
    addDataToChart(liveChart, risk, gas / 40); // Scale gas down to ~0-100 range for visibility
}

function addDataToChart(chart, risk, gas) {
    const label = new Date().toLocaleTimeString();

    if (chart.data.labels.length > 50) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
        chart.data.datasets[1].data.shift();
    }

    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(risk);
    chart.data.datasets[1].data.push(gas);
    chart.update();
}
