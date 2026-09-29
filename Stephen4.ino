#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <time.h>

/* =====================================================
   DHT11
   ===================================================== */

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

/* =====================================================
   STEPHEN FIREBASE
   ===================================================== */

#define API_KEY "AIzaSyCa5RQoQVKLq02hLKvrrRv4dkxmqz3kKAk"

#define DATABASE_URL "https://stephen-d3eaa-default-rtdb.europe-west1.firebasedatabase.app"

/*
   IMPORTANT:
   Replace these two placeholders with the Firebase
   Authentication email/password used by Stephen's
   Firebase project.

   Do NOT send the password here in chat.
*/
#define USER_EMAIL "stephenpanizal138@gmail.com"
#define USER_PASSWORD "edgarpanizal"

/* =====================================================
   FIREBASE OBJECTS
   ===================================================== */

UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD,
    3000
);

FirebaseApp app;

WiFiClientSecure ssl_client;

AsyncClientClass aClient(ssl_client);

RealtimeDatabase Database;

/* =====================================================
   WEB SERVER
   ===================================================== */

AsyncWebServer server(80);

/* =====================================================
   WIFI MANAGER
   ===================================================== */

const char *AP_SSID = "ESP-WIFI-MANAGER";

bool wifiManagerMode = false;

/* =====================================================
   SENSOR TIMER
   ===================================================== */

unsigned long lastSensorRead = 0;

const unsigned long SENSOR_INTERVAL = 10000;

/* =====================================================
   FUNCTION DECLARATIONS
   ===================================================== */

void processFirebase(AsyncResult &aResult);

bool connectToSavedWiFi();

void startWiFiManager();

void startMainWebServer();

void setupFirebase();

void sendSensorData();

String readFile(const char *path);

bool writeFile(const char *path, const String &data);

void deleteWiFiFiles();

String getDateString();

String getTimeString();

/* =====================================================
   WIFI MANAGER HTML
   ===================================================== */

const char WIFI_MANAGER_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
<meta charset="UTF-8">

<meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
>

<title>STEPHEN // ESP32 NETWORK</title>

<style>

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

:root {
    --bg: #07090b;
    --panel: #0d1115;
    --line: #222b32;
    --line-light: #2c373f;
    --text: #e8edf0;
    --muted: #7c878e;
    --dim: #4d585f;
    --accent: #9ee870;
    --accent-dark: #4f813e;
    --orange: #d69b58;
}

body {
    min-height: 100vh;
    background: var(--bg);
    color: var(--text);
    font-family: Arial, sans-serif;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 20px;
}

.wifi-card {
    width: 100%;
    max-width: 560px;
    background: linear-gradient(135deg, #10161a, #0b0f12);
    border: 1px solid var(--line);
    padding: 35px;
    position: relative;
    box-shadow: 0 20px 60px rgba(0,0,0,.45);
}

.wifi-card::before {
    content: "";
    position: absolute;
    left: 0;
    top: 0;
    width: 140px;
    height: 2px;
    background: var(--accent);
}

.topline {
    color: var(--dim);
    font-size: 8px;
    letter-spacing: 2px;
    margin-bottom: 30px;
}

.icon {
    width: 58px;
    height: 58px;
    display: grid;
    place-items: center;
    background: var(--accent);
    color: #091008;
    font-size: 24px;
    font-weight: 900;
    margin-bottom: 18px;
}

.label {
    color: var(--accent);
    font-size: 8px;
    font-weight: bold;
    letter-spacing: 2px;
    margin-bottom: 10px;
}

h1 {
    color: #f1f4f5;
    font-size: 34px;
    margin-bottom: 10px;
}

.description {
    color: var(--muted);
    font-size: 10px;
    line-height: 1.8;
    margin-bottom: 28px;
}

.group {
    margin-bottom: 18px;
}

.group label {
    display: block;
    color: #68747b;
    font-size: 8px;
    font-weight: bold;
    letter-spacing: 1.5px;
    margin-bottom: 8px;
}

.group input {
    width: 100%;
    height: 52px;
    padding: 0 15px;
    border: 1px solid var(--line-light);
    outline: none;
    background: #080b0d;
    color: var(--accent);
    font-size: 14px;
}

.group input:focus {
    border-color: var(--accent-dark);
}

.button {
    width: 100%;
    height: 54px;
    border: 1px solid var(--accent);
    background: var(--accent);
    color: #081008;
    cursor: pointer;
    font-size: 11px;
    font-weight: bold;
    letter-spacing: 1px;
}

.note {
    margin-top: 22px;
    padding: 15px;
    border: 1px solid var(--line);
    background: #0a0f12;
    color: var(--muted);
    font-size: 8px;
    line-height: 1.8;
}

.note strong {
    color: var(--orange);
}

.specs {
    margin-top: 18px;
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    border: 1px solid var(--line);
}

.spec {
    padding: 12px;
    border-right: 1px solid var(--line);
}

.spec:last-child {
    border-right: 0;
}

.spec span {
    display: block;
    color: var(--dim);
    font-size: 6px;
    margin-bottom: 6px;
}

.spec b {
    color: #aab4b9;
    font-size: 7px;
    word-break: break-word;
}

@media(max-width:600px) {
    .wifi-card {
        padding: 25px 20px;
    }

    h1 {
        font-size: 28px;
    }

    .specs {
        grid-template-columns: 1fr;
    }

    .spec {
        border-right: 0;
        border-bottom: 1px solid var(--line);
    }

    .spec:last-child {
        border-bottom: 0;
    }
}

</style>
</head>

<body>

<div class="wifi-card">

    <div class="topline">
        STEPHEN / ESP32 NETWORK CONFIGURATION
    </div>

    <div class="icon">W</div>

    <div class="label">
        SECURE FIELD LINK
    </div>

    <h1>Wi-Fi Manager</h1>

    <div class="description">
        Configure the network used by the ESP32 DHT11 sensor node.
    </div>

    <form action="/savewifi" method="POST">

        <div class="group">

            <label for="ssid">
                WI-FI SSID
            </label>

            <input
                type="text"
                id="ssid"
                name="ssid"
                placeholder="Enter Wi-Fi name"
                required
            >

        </div>

        <div class="group">

            <label for="password">
                WI-FI PASSWORD
            </label>

            <input
                type="password"
                id="password"
                name="password"
                placeholder="Enter Wi-Fi password"
            >

        </div>

        <button
            type="submit"
            class="button"
        >
            [ SAVE & CONNECT ]
        </button>

    </form>

    <div class="note">

        <strong>NETWORK NOTE</strong>

        <br><br>

        Credentials are saved in the ESP32 LittleFS memory
        and reused on the next boot.

    </div>

    <div class="specs">

        <div class="spec">
            <span>AP SSID</span>
            <b>ESP-WIFI-MANAGER</b>
        </div>

        <div class="spec">
            <span>SECURITY</span>
            <b>OPEN NETWORK</b>
        </div>

        <div class="spec">
            <span>MANAGER IP</span>
            <b>192.168.4.1</b>
        </div>

    </div>

</div>

</body>
</html>
)rawliteral";

/* =====================================================
   READ FILE
   ===================================================== */

String readFile(const char *path)
{
    if (!LittleFS.exists(path))
    {
        return "";
    }

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        return "";
    }

    String data = file.readString();

    file.close();

    data.trim();

    return data;
}

/* =====================================================
   WRITE FILE
   ===================================================== */

bool writeFile(const char *path, const String &data)
{
    File file = LittleFS.open(path, "w");

    if (!file)
    {
        Serial.print("Failed to open file: ");
        Serial.println(path);
        return false;
    }

    file.print(data);
    file.close();

    return true;
}

/* =====================================================
   DELETE WIFI FILES
   ===================================================== */

void deleteWiFiFiles()
{
    LittleFS.remove("/ssid.txt");
    LittleFS.remove("/pass.txt");
    LittleFS.remove("/ip.txt");
    LittleFS.remove("/gateway.txt");

    Serial.println("WiFi settings deleted.");
}

/* =====================================================
   CONNECT SAVED WIFI
   ===================================================== */

bool connectToSavedWiFi()
{
    String ssid = readFile("/ssid.txt");
    String pass = readFile("/pass.txt");
    String ip = readFile("/ip.txt");
    String gateway = readFile("/gateway.txt");

    if (ssid.length() == 0)
    {
        Serial.println();
        Serial.println("No saved WiFi credentials.");
        return false;
    }

    Serial.println();
    Serial.println("=================================");
    Serial.println("       SAVED WIFI FOUND");
    Serial.println("=================================");

    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    delay(500);

    if (ip.length() > 0 && gateway.length() > 0)
    {
        IPAddress local_IP;
        IPAddress gateway_IP;

        if (
            local_IP.fromString(ip) &&
            gateway_IP.fromString(gateway)
        )
        {
            IPAddress subnet(
                255, 255, 255, 0
            );

            if (
                WiFi.config(
                    local_IP,
                    gateway_IP,
                    subnet
                )
            )
            {
                Serial.println(
                    "Static IP configured."
                );
            }
        }
    }

    WiFi.begin(
        ssid.c_str(),
        pass.c_str()
    );

    Serial.print("Connecting to WiFi");

    unsigned long startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("=================================");
        Serial.println("       WIFI CONNECTED");
        Serial.println("=================================");

        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());

        Serial.println();

        return true;
    }

    Serial.println(
        "Failed to connect to saved WiFi."
    );

    WiFi.disconnect(true);
    delay(1000);

    return false;
}

/* =====================================================
   START WIFI MANAGER
   ===================================================== */

void startWiFiManager()
{
    wifiManagerMode = true;

    Serial.println();
    Serial.println("=================================");
    Serial.println("       WIFI MANAGER MODE");
    Serial.println("=================================");

    WiFi.mode(WIFI_AP);
    delay(500);

    bool apStarted = WiFi.softAP(AP_SSID);

    if (!apStarted)
    {
        Serial.println(
            "ERROR: Failed to start WiFi Manager AP!"
        );
    }

    delay(1000);

    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);

    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",
                WIFI_MANAGER_HTML
            );
        }
    );

    server.on(
        "/savewifi",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            String ssid = "";
            String pass = "";

            if (
                request->hasParam(
                    "ssid",
                    true
                )
            )
            {
                ssid =
                    request
                    ->getParam(
                        "ssid",
                        true
                    )
                    ->value();
            }

            if (
                request->hasParam(
                    "password",
                    true
                )
            )
            {
                pass =
                    request
                    ->getParam(
                        "password",
                        true
                    )
                    ->value();
            }

            ssid.trim();
            pass.trim();

            if (ssid.length() == 0)
            {
                request->send(
                    400,
                    "text/plain",
                    "Wi-Fi SSID is required."
                );

                return;
            }

            writeFile(
                "/ssid.txt",
                ssid
            );

            writeFile(
                "/pass.txt",
                pass
            );

            writeFile(
                "/ip.txt",
                ""
            );

            writeFile(
                "/gateway.txt",
                ""
            );

            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#07090b;"
                "color:#e8edf0;'>"

                "<div style='background:#0d1115;"
                "border:1px solid #222b32;"
                "padding:30px;"
                "border-radius:4px;"
                "max-width:500px;"
                "margin:auto;'>"

                "<h1 style='color:#9ee870;'>"
                "Wi-Fi Saved"
                "</h1>"

                "<p>"
                "The ESP32 will restart and connect "
                "to the saved Wi-Fi."
                "</p>"

                "<p>"
                "Please wait..."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );

            delay(1500);

            ESP.restart();
        }
    );

    server.begin();

    Serial.println();
    Serial.println("=================================");
    Serial.println(" WIFI MANAGER READY");
    Serial.println("=================================");

    Serial.print(
        "Connect to WiFi: "
    );

    Serial.println(AP_SSID);

    Serial.print(
        "Then open: http://"
    );

    Serial.println(
        WiFi.softAPIP()
    );

    Serial.println();
}

/* =====================================================
   START MAIN WEB SERVER
   ===================================================== */

void startMainWebServer()
{
    wifiManagerMode = false;

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            if (
                LittleFS.exists(
                    "/index.html"
                )
            )
            {
                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );
            }
            else
            {
                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );
            }
        }
    );

    server.on(
        "/change-wifi",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#07090b;"
                "color:#e8edf0;'>"

                "<div style='background:#0d1115;"
                "border:1px solid #222b32;"
                "padding:30px;"
                "border-radius:4px;"
                "max-width:500px;"
                "margin:auto;'>"

                "<h1 style='color:#9ee870;'>"
                "Changing Wi-Fi..."
                "</h1>"

                "<p>"
                "Wi-Fi settings will be cleared."
                "</p>"

                "<p>"
                "The ESP32 will restart."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );

            delay(1000);

            deleteWiFiFiles();

            ESP.restart();
        }
    );

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );

    server.begin();

    Serial.println();
    Serial.println("=================================");
    Serial.println("      MAIN WEB SERVER READY");
    Serial.println("=================================");

    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.println();
}

/* =====================================================
   FIREBASE CALLBACK
   ===================================================== */

void processFirebase(AsyncResult &aResult)
{
    if (!aResult.isResult())
    {
        return;
    }

    if (aResult.isEvent())
    {
        Firebase.printf(
            "Firebase Event - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.eventLog().message().c_str(),
            aResult.eventLog().code()
        );
    }

    if (aResult.isDebug())
    {
        Firebase.printf(
            "Firebase Debug - task: %s, msg: %s\n",
            aResult.uid().c_str(),
            aResult.debug().c_str()
        );
    }

    if (aResult.isError())
    {
        Firebase.printf(
            "Firebase Error - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.error().message().c_str(),
            aResult.error().code()
        );
    }

    if (aResult.available())
    {
        Firebase.printf(
            "Firebase Payload - task: %s, payload: %s\n",
            aResult.uid().c_str(),
            aResult.c_str()
        );
    }
}

/* =====================================================
   FIREBASE SETUP
   ===================================================== */

void setupFirebase()
{
    Serial.println();
    Serial.println("=================================");
    Serial.println("       FIREBASE SETUP");
    Serial.println("=================================");

    ssl_client.setInsecure();

    Serial.println(
        "Initializing Firebase..."
    );

    initializeApp(
        aClient,
        app,
        getAuth(user_auth),
        processFirebase,
        "authTask"
    );

    app.getApp<RealtimeDatabase>(
        Database
    );

    Database.url(
        DATABASE_URL
    );

    Serial.println(
        "Firebase initialization started."
    );

    Serial.println();
}

/* =====================================================
   GET DATE
   ===================================================== */

String getDateString()
{
    struct tm timeinfo;

    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {
        return "1970-01-01";
    }

    char buffer[20];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );

    return String(buffer);
}

/* =====================================================
   GET TIME
   ===================================================== */

String getTimeString()
{
    struct tm timeinfo;

    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {
        return "00:00:00";
    }

    char buffer[20];

    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );

    return String(buffer);
}

/* =====================================================
   SEND SENSOR DATA
   ===================================================== */

void sendSensorData()
{
    if (!app.ready())
    {
        Serial.println(
            "Firebase not ready yet..."
        );

        return;
    }

    float humidity =
        dht.readHumidity();

    float temperature =
        dht.readTemperature();

    if (
        isnan(humidity) ||
        isnan(temperature)
    )
    {
        Serial.println(
            "ERROR: Failed to read DHT11"
        );

        return;
    }

    String date =
        getDateString();

    String time =
        getTimeString();

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;

    String temperaturePath =
        basePath +
        "/temperature";

    String humidityPath =
        basePath +
        "/humidity";

    Serial.println();
    Serial.println("=================================");
    Serial.println("       DHT11 SENSOR READING");
    Serial.println("=================================");

    Serial.print("Temperature: ");
    Serial.print(
        temperature,
        1
    );
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(
        humidity,
        1
    );
    Serial.println(" %");

    Serial.print("Date: ");
    Serial.println(date);

    Serial.print("Time: ");
    Serial.println(time);

    Serial.print("Firebase base path: ");
    Serial.println(basePath);

    Database.set<float>(
        aClient,
        temperaturePath,
        temperature,
        processFirebase,
        "temperatureTask"
    );

    Database.set<float>(
        aClient,
        humidityPath,
        humidity,
        processFirebase,
        "humidityTask"
    );

    Serial.println(
        "Temperature write task sent."
    );

    Serial.println(
        "Humidity write task sent."
    );

    Serial.println(
        "================================="
    );
}

/* =====================================================
   SETUP
   ===================================================== */

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println(
        "================================="
    );

    Serial.println(
        "   STEPHEN DHT11 FIREBASE MONITOR"
    );

    Serial.println(
        "================================="
    );

    /* =================================================
       LITTLEFS
       ================================================= */

    Serial.println(
        "Starting LittleFS..."
    );

    if (!LittleFS.begin(true))
    {
        Serial.println(
            "LittleFS mount failed!"
        );

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println(
        "LittleFS ready."
    );

    /* =================================================
       DHT11
       ================================================= */

    dht.begin();

    Serial.println(
        "DHT11 initialized on GPIO4."
    );

    /* =================================================
       WIFI
       ================================================= */

    bool connected =
        connectToSavedWiFi();

    if (!connected)
    {
        startWiFiManager();
        return;
    }

    /* =================================================
       NTP
       ================================================= */

    Serial.println(
        "Starting NTP time..."
    );

    configTime(
        8 * 3600,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );

    Serial.print(
        "Waiting for time"
    );

    struct tm timeinfo;

    int retry = 0;

    while (
        !getLocalTime(
            &timeinfo
        ) &&
        retry < 20
    )
    {
        Serial.print(".");
        delay(500);
        retry++;
    }

    Serial.println();

    if (
        getLocalTime(
            &timeinfo
        )
    )
    {
        Serial.println(
            "Time synchronized."
        );

        Serial.print(
            "Date: "
        );

        Serial.println(
            getDateString()
        );

        Serial.print(
            "Time: "
        );

        Serial.println(
            getTimeString()
        );
    }
    else
    {
        Serial.println(
            "WARNING: Time synchronization failed."
        );
    }

    /* =================================================
       FIREBASE
       ================================================= */

    setupFirebase();

    /* =================================================
       WEB SERVER
       ================================================= */

    startMainWebServer();

    /* =================================================
       READY
       ================================================= */

    Serial.println();
    Serial.println(
        "================================="
    );

    Serial.println(
        "         SYSTEM READY"
    );

    Serial.println(
        "================================="
    );

    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.println();
}

/* =====================================================
   LOOP
   ===================================================== */

void loop()
{
    if (!wifiManagerMode)
    {
        app.loop();
    }

    if (
        !wifiManagerMode &&
        millis() - lastSensorRead >=
        SENSOR_INTERVAL
    )
    {
        lastSensorRead = millis();

        sendSensorData();
    }

    delay(10);
}
