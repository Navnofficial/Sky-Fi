#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = "poco";
const char* password = "12345678";

// Motor control pins (PWM-capable)
const int gpLm = 5;   // Left Motor GPIO
const int gpRm = 43;  // Right Motor GPIO

// PWM settings
const int pwmFreq       = 5000; // Hz
const int pwmResolution = 8;    // 0-255

WebServer server(80);

// HTML UI - Manual Steering with Vertical Throttle
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<title>FPV RC Plane Control</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<style>
:root {
  --bg-color: #2c3e50;
  --widget-bg: #34495e;
  --accent-color: #3498db;
  --text-color: #ecf0f1;
  --button-color: #3498db;
  --button-active-color: #2980b9;
}
body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background-color: var(--bg-color);
  color: var(--text-color);
  margin: 0;
  overflow: hidden;
  -webkit-user-select: none;
  -ms-user-select: none;
  user-select: none;
}
.main-container {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 100vh;
  padding: 20px;
  box-sizing: border-box;
}
/* --- Throttle (Left Side) --- */
.throttle-container {
  width: 35%;
  height: 80%;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  background-color: var(--widget-bg);
  border-radius: 20px;
}
#throttleSlider {
  -webkit-appearance: none;
  appearance: none;
  width: 80%;
  height: 25px;
  background: #2c3e50;
  outline: none;
  border-radius: 15px;
  cursor: pointer;
  transform: rotate(-90deg);
}
#throttleSlider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 50px;
  height: 50px;
  background: var(--accent-color);
  border-radius: 50%;
  border: 4px solid var(--widget-bg);
}
/* --- Steering (Right Side) --- */
.steering-container {
  width: 55%;
  height: 80%;
  display: flex;
  justify-content: space-around;
  align-items: center;
  flex-wrap: wrap;
}
.steer-button {
  width: 120px;
  height: 120px;
  border-radius: 50%;
  border: 5px solid var(--button-color);
  background-color: var(--widget-bg);
  color: var(--text-color);
  font-size: 3rem;
  font-weight: bold;
  display: flex;
  justify-content: center;
  align-items: center;
  cursor: pointer;
  touch-action: manipulation;
}
.steer-button:active {
  background-color: var(--button-active-color);
  transform: scale(0.95);
}
/* --- Debug Info (Top Center) --- */
.debug-info {
  position: absolute;
  top: 10px;
  left: 50%;
  transform: translateX(-50%);
  font-family: 'Courier New', Courier, monospace;
  background-color: rgba(0,0,0,0.3);
  padding: 5px 15px;
  border-radius: 10px;
  font-size: 0.9rem;
}
</style>
</head>
<body>
<div class="debug-info" id="debugInfo">L: 0 | R: 0</div>
<div class="main-container">
  <div class="throttle-container">
    <input type="range" min="0" max="255" value="0" id="throttleSlider">
  </div>
  <div class="steering-container">
    <div class="steer-button" id="leftButton">&#9664;</div>
    <div class="steer-button" id="rightButton">&#9654;</div>
  </div>
</div>
<script>
const throttleSlider = document.getElementById('throttleSlider');
const leftButton     = document.getElementById('leftButton');
const rightButton    = document.getElementById('rightButton');
const debugInfo      = document.getElementById('debugInfo');

// --- Configuration ---
const UPDATE_INTERVAL_MS    = 50;  // Send updates every 50ms
const STEERING_SENSITIVITY  = 0.6; // 60% power difference when steering

let throttle      = 0;
let steerDirection = 0; // -1 = left, 0 = straight, 1 = right

// Update throttle from slider
throttleSlider.addEventListener('input', (event) => {
  throttle = parseInt(event.target.value, 10);
});

// --- Steering Button Event Listeners ---
// Left Button
leftButton.addEventListener('mousedown',  () => { steerDirection = -1; });
leftButton.addEventListener('touchstart', (e) => { e.preventDefault(); steerDirection = -1; });
leftButton.addEventListener('mouseup',    () => { steerDirection = 0; });
leftButton.addEventListener('touchend',   () => { steerDirection = 0; });
leftButton.addEventListener('mouseleave', () => { steerDirection = 0; }); // Failsafe

// Right Button
rightButton.addEventListener('mousedown',  () => { steerDirection = 1; });
rightButton.addEventListener('touchstart', (e) => { e.preventDefault(); steerDirection = 1; });
rightButton.addEventListener('mouseup',    () => { steerDirection = 0; });
rightButton.addEventListener('touchend',   () => { steerDirection = 0; });
rightButton.addEventListener('mouseleave', () => { steerDirection = 0; }); // Failsafe

// --- Main Control Loop ---
setInterval(() => {
  let leftMotor  = throttle;
  let rightMotor = throttle;

  if (steerDirection !== 0) {
    const steerEffect = throttle * STEERING_SENSITIVITY;
    if (steerDirection === -1) {       // Turning Left
      leftMotor  -= steerEffect;
      rightMotor += steerEffect;
    } else if (steerDirection === 1) { // Turning Right
      rightMotor -= steerEffect;
      leftMotor  += steerEffect;
    }
  }

  // Clamp values to 0-255
  leftMotor  = Math.max(0, Math.min(255, Math.round(leftMotor)));
  rightMotor = Math.max(0, Math.min(255, Math.round(rightMotor)));

  // Send commands to ESP32
  fetch(`/set?motor=left&value=${leftMotor}`);
  fetch(`/set?motor=right&value=${rightMotor}`);

  // Update debug display
  debugInfo.innerText = `L: ${leftMotor} | R: ${rightMotor}`;
}, UPDATE_INTERVAL_MS);
</script>
</body>
</html>
)rawliteral";

// Handle root page
void handleRoot() {
  server.send_P(200, "text/html", htmlPage);
}

// Handle motor value update
void handleSetMotor() {
  if (server.hasArg("motor") && server.hasArg("value")) {
    String motor = server.arg("motor");
    int value    = server.arg("value").toInt();
    value        = constrain(value, 0, 255);

    if (motor == "left") {
      ledcWrite(gpLm, value);
    } else if (motor == "right") {
      ledcWrite(gpRm, value);
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

void setup() {
  Serial.begin(115200);

  // Attach motors to PWM channels
  ledcAttach(gpLm, pwmFreq, pwmResolution);
  ledcAttach(gpRm, pwmFreq, pwmResolution);

  // Stop both motors at startup
  ledcWrite(gpLm, 0);
  ledcWrite(gpRm, 0);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Register routes and start web server
  server.on("/",    handleRoot);
  server.on("/set", handleSetMotor);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
