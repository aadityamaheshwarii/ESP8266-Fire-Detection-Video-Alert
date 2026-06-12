#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

ESP8266WebServer server(80);

int flamePin = D5;

bool fireDetected = false;
unsigned long lastFireTime = 0;

void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Fire Alert</title>
</head>
<body>

<h1 id="status" style="color:green;">No Fire</h1>
<button onclick="enableSound()">Enable Sound</button>

<iframe id="video"
  src="https://player.vimeo.com/video/1181698022?loop=1&autopause=0&muted=1"
  width="100%" height="400">
</iframe>

<script>
let video = document.getElementById("video");
let statusText = document.getElementById("status");
let playerReady = false;

// Wait a bit for iframe to load
setTimeout(() => {
  playerReady = true;
}, 2000);

function playVideo() {
  video.contentWindow.postMessage('{"method":"play"}', '*');
}

function pauseVideo() {
  video.contentWindow.postMessage('{"method":"pause"}', '*');
}

function enableSound() {
  video.contentWindow.postMessage('{"method":"setVolume","value":1}', '*');
  video.contentWindow.postMessage('{"method":"play"}', '*');
}

function checkFire() {
  fetch('/status')
    .then(res => res.text())
    .then(data => {

      if (data == "1") {
        statusText.innerHTML = "🔥 FIRE DETECTED";
        statusText.style.color = "red";

        if (playerReady) playVideo();

      } else {
        statusText.innerHTML = "No Fire";
        statusText.style.color = "green";

        if (playerReady) pauseVideo();
      }
    });
}

setInterval(checkFire, 1000);
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleStatus() {
  server.send(200, "text/plain", fireDetected ? "1" : "0");
}

void setup() {
  Serial.begin(115200);
  pinMode(flamePin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  int flameState = digitalRead(flamePin);

  if (flameState == LOW) {
    fireDetected = true;
    lastFireTime = millis();
  }

  // 🔥 HOLD for 20 seconds after detection
  if (millis() - lastFireTime > 20000) {
    fireDetected = false;
  }

  server.handleClient();
}