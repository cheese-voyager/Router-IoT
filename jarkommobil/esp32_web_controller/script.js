const topicInput = document.getElementById('mqttTopic');
const connectBtn = document.getElementById('connectBtn');
const statusDot = document.getElementById('statusDot');
const statusText = document.getElementById('statusText');
const speedSlider = document.getElementById('speedSlider');
const speedValue = document.getElementById('speedValue');
const padBtns = document.querySelectorAll('.pad-btn');

let client;
let isConnected = false;
let currentTopic = "";

// HiveMQ Public Broker WebSocket URL
const brokerUrl = 'wss://broker.hivemq.com:8884/mqtt';

// Coba load Topic terakhir dari localStorage
const savedTopic = localStorage.getItem('mqtt_topic');
if (savedTopic) {
    topicInput.value = savedTopic;
}

function initMQTT(topic) {
    if (client) {
        client.end();
    }

    statusText.textContent = "Connecting to HiveMQ...";
    statusDot.className = "status-dot";
    connectBtn.textContent = "Connecting";
    connectBtn.disabled = true;

    currentTopic = topic;

    // Hubungkan ke HiveMQ MQTT Broker
    client = mqtt.connect(brokerUrl);

    client.on('connect', function () {
        isConnected = true;
        statusText.textContent = "Connected (MQTT)";
        statusDot.className = "status-dot connected";
        connectBtn.textContent = "Disconnect";
        connectBtn.style.background = "var(--danger)";
        connectBtn.style.boxShadow = "0 4px 15px rgba(239, 68, 68, 0.5)";
        connectBtn.disabled = false;

        // Simpan topic ke LocalStorage
        localStorage.setItem('mqtt_topic', topic);

        console.log("Connected to HiveMQ. Ready to publish to: " + topic);

        // Kirim kecepatan awal saat konek
        sendSpeed();
    });

    client.on('error', function (error) {
        console.error("MQTT Error: ", error);
        statusText.textContent = "Error";
        connectBtn.disabled = false;
    });

    client.on('close', function () {
        isConnected = false;
        statusText.textContent = "Disconnected";
        statusDot.className = "status-dot disconnected";
        connectBtn.textContent = "Connect";
        connectBtn.style.background = "var(--primary)";
        connectBtn.style.boxShadow = "0 4px 15px var(--primary-glow)";
        connectBtn.disabled = false;
    });
}

connectBtn.addEventListener('click', () => {
    if (!isConnected) {
        const topic = topicInput.value.trim();
        if (topic) {
            initMQTT(topic);
        } else {
            alert("Harap masukkan Topic MQTT terlebih dahulu!");
        }
    } else {
        client.end();
    }
});

// Fungsi mengirim perintah teks ke Broker MQTT
function sendCommand(cmd) {
    if (isConnected && client) {
        client.publish(currentTopic, cmd);
        console.log("Published [" + currentTopic + "] : " + cmd);
    }
}

// Fungsi update slider kecepatan
function sendSpeed() {
    const val = speedSlider.value;
    speedValue.textContent = val;
    // Kirim format "V:255"
    sendCommand("V:" + val);
}

// Event listener saat slider digeser
speedSlider.addEventListener('input', sendSpeed);

// Event Handler untuk D-Pad (Mouse & Touch) sama seperti sebelumnya
padBtns.forEach(btn => {
    const cmd = btn.getAttribute('data-cmd');

    const handlePress = (e) => {
        btn.classList.add('active');
        sendCommand(cmd);
    };

    const handleRelease = (e) => {
        btn.classList.remove('active');
        if (cmd !== 'S') {
            sendCommand('S'); // S = Stop otomatis saat dilepas
        }
    };

    btn.addEventListener('mousedown', handlePress);
    btn.addEventListener('mouseup', handleRelease);
    btn.addEventListener('mouseleave', handleRelease);

    btn.addEventListener('touchstart', (e) => { e.preventDefault(); handlePress(e); }, { passive: false });
    btn.addEventListener('touchend', (e) => { e.preventDefault(); handleRelease(e); }, { passive: false });
    btn.addEventListener('touchcancel', (e) => { e.preventDefault(); handleRelease(e); }, { passive: false });
});

// Keyboard Support
document.addEventListener('keydown', (e) => {
    if (!isConnected) return;
    if (e.repeat) return;

    switch (e.key.toLowerCase()) {
        case 'w': case 'arrowup': document.getElementById('btnF').dispatchEvent(new Event('mousedown')); break;
        case 's': case 'arrowdown': document.getElementById('btnB').dispatchEvent(new Event('mousedown')); break;
        case 'a': case 'arrowleft': document.getElementById('btnL').dispatchEvent(new Event('mousedown')); break;
        case 'd': case 'arrowright': document.getElementById('btnR').dispatchEvent(new Event('mousedown')); break;
        case ' ': document.getElementById('btnS').dispatchEvent(new Event('mousedown')); break;
    }
});

document.addEventListener('keyup', (e) => {
    if (!isConnected) return;

    switch (e.key.toLowerCase()) {
        case 'w': case 'arrowup': document.getElementById('btnF').dispatchEvent(new Event('mouseup')); break;
        case 's': case 'arrowdown': document.getElementById('btnB').dispatchEvent(new Event('mouseup')); break;
        case 'a': case 'arrowleft': document.getElementById('btnL').dispatchEvent(new Event('mouseup')); break;
        case 'd': case 'arrowright': document.getElementById('btnR').dispatchEvent(new Event('mouseup')); break;
        case ' ': document.getElementById('btnS').dispatchEvent(new Event('mouseup')); break;
    }
});
