// =========================================================
// STEPHEN ACTIVITY 4
// ESP32 DHT11 FIREBASE MONITOR
// FUNCTIONALITY BASED ON KIMBERLY ACTIVITY 4
// STEPHEN COLOR / VISUAL THEME
// =========================================================

import { initializeApp } from
    "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";

import {
    getDatabase,
    ref,
    onValue
} from
    "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

// =========================================================
// FIREBASE CONFIG
// =========================================================

const firebaseConfig = {
    apiKey: "AIzaSyCa5RQoQVKLq02hLKvrrRv4dkxmqz3kKAk",
    authDomain: "stephen-d3eaa.firebaseapp.com",
    databaseURL: "https://stephen-d3eaa-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "stephen-d3eaa",
    storageBucket: "stephen-d3eaa.firebasestorage.app",
    messagingSenderId: "1006111660999",
    appId: "1:1006111660999:web:5adfcb3a79c3bda05597e2",
    measurementId: "G-ERHCH0F9R1"
};

// =========================================================
// INITIALIZE FIREBASE
// =========================================================

const firebaseApp = initializeApp(firebaseConfig);
const database = getDatabase(firebaseApp);

// IMPORTANT:
// ESP32 should write:
// /ESP32_Data/YYYY-MM-DD/HH:MM:SS/temperature
// /ESP32_Data/YYYY-MM-DD/HH:MM:SS/humidity
const dataRef = ref(database, "ESP32_Data");

// =========================================================
// GLOBAL VARIABLES
// =========================================================

let allSensorData = {};
let selectedDate = "";
let sensorChart = null;

// =========================================================
// DOM ELEMENTS
// =========================================================

const statusElement = document.getElementById("firebaseStatus");
const currentTemperature = document.getElementById("currentTemperature");
const currentHumidity = document.getElementById("currentHumidity");
const graphDate = document.getElementById("graphDate");
const historyDate = document.getElementById("historyDate");
const historyBody = document.getElementById("historyTableBody");
const recordCount = document.getElementById("recordCount");
const toggleHistory = document.getElementById("toggleHistory");
const historyContent = document.getElementById("historyContent");

// =========================================================
// STATUS
// =========================================================

function setStatus(text, connected) {
    if (!statusElement) return;

    statusElement.textContent = "Firebase Status: " + text;

    statusElement.classList.toggle("status-connected", connected);
    statusElement.classList.toggle("status-error", !connected);
}

// =========================================================
// NUMBER HELPERS
// =========================================================

function toNumber(value) {
    const number = Number(value);
    return Number.isFinite(number) ? number : null;
}

function formatNumber(value) {
    if (value === null || value === undefined) return "--";
    return Number(value).toFixed(1);
}

// =========================================================
// GET LATEST READING
// =========================================================

function getLatestReading(data) {
    let latest = null;

    const dates = Object.keys(data || {}).sort();

    for (const date of dates) {
        const times = Object.keys(data[date] || {}).sort();

        for (const time of times) {
            const reading = data[date][time];

            if (!reading || typeof reading !== "object") continue;

            const temperature = toNumber(reading.temperature);
            const humidity = toNumber(reading.humidity);

            if (temperature === null && humidity === null) continue;

            latest = {
                date,
                time,
                temperature,
                humidity
            };
        }
    }

    return latest;
}

// =========================================================
// GET DATE LIST
// =========================================================

function getDateList(data) {
    return Object.keys(data || {})
        .filter(key =>
            data[key] &&
            typeof data[key] === "object"
        )
        .sort()
        .reverse();
}

// =========================================================
// POPULATE DATE SELECTS
// =========================================================

function populateDateSelects() {
    const dates = getDateList(allSensorData);

    [graphDate, historyDate].forEach(select => {
        if (!select) return;

        select.innerHTML = "";

        if (dates.length === 0) {
            const option = document.createElement("option");
            option.value = "";
            option.textContent = "No dates available";
            select.appendChild(option);
            return;
        }

        dates.forEach(date => {
            const option = document.createElement("option");
            option.value = date;
            option.textContent = date;
            select.appendChild(option);
        });

        select.value = selectedDate || dates[0];
    });
}

// =========================================================
// UPDATE CURRENT READING
// =========================================================

function updateCurrentReading() {
    const latest = getLatestReading(allSensorData);

    if (!latest) {
        if (currentTemperature) currentTemperature.textContent = "-- °C";
        if (currentHumidity) currentHumidity.textContent = "-- %";
        return;
    }

    if (currentTemperature) {
        currentTemperature.textContent =
            formatNumber(latest.temperature) + " °C";
    }

    if (currentHumidity) {
        currentHumidity.textContent =
            formatNumber(latest.humidity) + " %";
    }
}

// =========================================================
// GET READINGS FOR DATE
// =========================================================

function getReadingsForDate(date) {
    const result = [];

    if (!date) return result;

    const dayData = allSensorData[date];

    if (!dayData || typeof dayData !== "object") {
        return result;
    }

    Object.keys(dayData)
        .filter(time =>
            dayData[time] &&
            typeof dayData[time] === "object"
        )
        .sort()
        .forEach(time => {
            const reading = dayData[time];

            const temperature = toNumber(reading.temperature);
            const humidity = toNumber(reading.humidity);

            if (temperature === null && humidity === null) return;

            result.push({
                time,
                temperature,
                humidity
            });
        });

    return result;
}

// =========================================================
// UPDATE GRAPH
// =========================================================

function updateChart() {
    if (typeof Chart === "undefined") {
        console.error("Chart.js is not loaded.");
        return;
    }

    const date = graphDate ? graphDate.value : selectedDate;
    const readings = getReadingsForDate(date);

    const labels = readings.map(item => item.time);
    const temperatures = readings.map(item => item.temperature);
    const humidities = readings.map(item => item.humidity);

    const canvas = document.getElementById("sensorChart");

    if (!canvas) return;

    if (sensorChart) {
        sensorChart.destroy();
        sensorChart = null;
    }

    sensorChart = new Chart(canvas, {
        type: "line",
        data: {
            labels,
            datasets: [
                {
                    label: "Temperature (°C)",
                    data: temperatures,
                    yAxisID: "temperature",
                    tension: 0.3,
                    borderWidth: 3,
                    pointRadius: 3,
                    borderColor: "#9ee870",
                    backgroundColor: "rgba(158,232,112,0.12)",
                    pointBackgroundColor: "#9ee870",
                    pointBorderColor: "#07090b"
                },
                {
                    label: "Humidity (%)",
                    data: humidities,
                    yAxisID: "humidity",
                    tension: 0.3,
                    borderWidth: 3,
                    pointRadius: 3,
                    borderColor: "#d69b58",
                    backgroundColor: "rgba(214,155,88,0.12)",
                    pointBackgroundColor: "#d69b58",
                    pointBorderColor: "#07090b"
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: {
                duration: 250
            },
            interaction: {
                mode: "index",
                intersect: false
            },
            plugins: {
                legend: {
                    labels: {
                        color: "#b8c2c7",
                        font: {
                            family: "JetBrains Mono"
                        }
                    }
                }
            },
            scales: {
                x: {
                    ticks: {
                        color: "#68747b",
                        maxRotation: 45,
                        minRotation: 0
                    },
                    grid: {
                        color: "#1b2328"
                    }
                },
                temperature: {
                    type: "linear",
                    position: "left",
                    title: {
                        display: true,
                        text: "Temperature (°C)",
                        color: "#9ee870"
                    },
                    ticks: {
                        color: "#9ee870"
                    },
                    grid: {
                        color: "#222b32"
                    }
                },
                humidity: {
                    type: "linear",
                    position: "right",
                    title: {
                        display: true,
                        text: "Humidity (%)",
                        color: "#d69b58"
                    },
                    ticks: {
                        color: "#d69b58"
                    },
                    grid: {
                        drawOnChartArea: false
                    }
                }
            }
        }
    });
}

// =========================================================
// UPDATE HISTORY
// =========================================================

function updateHistory() {
    if (!historyBody) return;

    const date = historyDate ? historyDate.value : selectedDate;
    const readings = getReadingsForDate(date);

    historyBody.innerHTML = "";

    if (readings.length === 0) {
        const row = document.createElement("tr");
        const cell = document.createElement("td");

        cell.colSpan = 3;
        cell.className = "loading";
        cell.textContent = "No sensor data available.";

        row.appendChild(cell);
        historyBody.appendChild(row);

        if (recordCount) recordCount.textContent = "0 records";
        return;
    }

    readings.slice().reverse().forEach(reading => {
        const row = document.createElement("tr");

        const timeCell = document.createElement("td");
        const temperatureCell = document.createElement("td");
        const humidityCell = document.createElement("td");

        timeCell.textContent = reading.time;
        temperatureCell.textContent =
            formatNumber(reading.temperature) + " °C";
        humidityCell.textContent =
            formatNumber(reading.humidity) + " %";

        row.appendChild(timeCell);
        row.appendChild(temperatureCell);
        row.appendChild(humidityCell);

        historyBody.appendChild(row);
    });

    if (recordCount) {
        recordCount.textContent =
            readings.length +
            (readings.length === 1 ? " record" : " records");
    }
}

// =========================================================
// UPDATE DASHBOARD
// =========================================================

function updateDashboard() {
    const dates = getDateList(allSensorData);

    if (dates.length === 0) {
        if (currentTemperature) currentTemperature.textContent = "-- °C";
        if (currentHumidity) currentHumidity.textContent = "-- %";

        populateDateSelects();
        updateHistory();
        updateChart();
        return;
    }

    if (!selectedDate || !dates.includes(selectedDate)) {
        selectedDate = dates[0];
    }

    populateDateSelects();
    updateCurrentReading();
    updateChart();
    updateHistory();
}

// =========================================================
// FIREBASE LISTENER
// =========================================================

console.log("=================================");
console.log("STEPHEN ACTIVITY 4");
console.log("CONNECTING TO FIREBASE");
console.log("Database path: /ESP32_Data");
console.log("=================================");

setStatus("Connecting...", false);

onValue(
    dataRef,
    snapshot => {
        const value = snapshot.val();

        allSensorData = value || {};

        setStatus("Connected", true);
        updateDashboard();

        console.log("Firebase data received.");
    },
    error => {
        console.error("FIREBASE READ ERROR:", error);

        setStatus("Error", false);
    }
);

// =========================================================
// GRAPH DATE CHANGE
// =========================================================

if (graphDate) {
    graphDate.addEventListener("change", function () {
        selectedDate = this.value;

        if (historyDate) {
            historyDate.value = selectedDate;
        }

        updateChart();
        updateHistory();
    });
}

// =========================================================
// HISTORY DATE CHANGE
// =========================================================

if (historyDate) {
    historyDate.addEventListener("change", function () {
        selectedDate = this.value;

        if (graphDate) {
            graphDate.value = selectedDate;
        }

        updateHistory();
        updateChart();
    });
}

// =========================================================
// SHOW / HIDE HISTORY
// =========================================================

if (toggleHistory && historyContent) {
    toggleHistory.addEventListener("click", function () {
        const hidden = historyContent.classList.contains("hidden");

        if (hidden) {
            historyContent.classList.remove("hidden");
            toggleHistory.textContent = "Hide History";
            updateHistory();
        } else {
            historyContent.classList.add("hidden");
            toggleHistory.textContent = "Show History";
        }
    });
}

// =========================================================
// INITIAL STATUS
// =========================================================

setStatus("Connecting...", false);
