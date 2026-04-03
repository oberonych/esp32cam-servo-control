#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include "esp_camera.h"

// ========== WiFi Configuration ==========
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ========== Camera Pins (CAMERA_MODEL_AI_THINKER) ==========
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ========== Servo Pins ==========
#define SERVO1_PIN 12
#define SERVO2_PIN 13
#define SERVO3_PIN 15

// ========== Server Setup ==========
WebServer server(80);
Servo servo1, servo2, servo3;

int servo1Angle = 90;
int servo2Angle = 90;
int servo3Angle = 90;

// ========== Camera Initialization ==========
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d7 = Y7_GPIO_NUM;
  config.pin_d6 = Y6_GPIO_NUM;
  config.pin_d5 = Y5_GPIO_NUM;
  config.pin_d4 = Y4_GPIO_NUM;
  config.pin_d3 = Y3_GPIO_NUM;
  config.pin_d2 = Y2_GPIO_NUM;
  config.pin_d1 = Y9_GPIO_NUM;
  config.pin_d0 = Y8_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  Serial.println("Camera initialized successfully");
}

// ========== Web Pages ==========
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32-CAM Control</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    
    .container {
      background: white;
      border-radius: 10px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.3);
      max-width: 900px;
      width: 100%;
      padding: 30px;
    }
    
    h1 {
      color: #333;
      text-align: center;
      margin-bottom: 30px;
      font-size: 28px;
    }
    
    .content {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 30px;
    }
    
    .video-section {
      text-align: center;
    }
    
    #mjpeg-stream {
      max-width: 100%;
      height: auto;
      border: 3px solid #667eea;
      border-radius: 8px;
      background: #000;
    }
    
    .control-section {
      display: flex;
      flex-direction: column;
      gap: 25px;
    }
    
    .servo-control {
      background: #f8f9fa;
      padding: 20px;
      border-radius: 8px;
      border-left: 4px solid #667eea;
    }
    
    .servo-control h3 {
      color: #333;
      margin-bottom: 15px;
      font-size: 16px;
    }
    
    .slider-container {
      display: flex;
      flex-direction: column;
      gap: 10px;
    }
    
    input[type="range"] {
      width: 100%;
      height: 8px;
      cursor: pointer;
      accent-color: #667eea;
    }
    
    .angle-display {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-weight: bold;
      color: #667eea;
      font-size: 18px;
    }
    
    .button-group {
      display: flex;
      gap: 10px;
      margin-top: 10px;
    }
    
    button {
      flex: 1;
      padding: 10px;
      background: #667eea;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-weight: bold;
      transition: background 0.3s;
    }
    
    button:hover {
      background: #764ba2;
    }
    
    button.reset {
      background: #6c757d;
    }
    
    button.reset:hover {
      background: #5a6268;
    }
    
    @media (max-width: 768px) {
      .content {
        grid-template-columns: 1fr;
      }
      
      h1 {
        font-size: 20px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎥 ESP32-CAM + Servo Control</h1>
    
    <div class="content">
      <div class="video-section">
        <img id="mjpeg-stream" src="/stream" alt="Camera Stream">
      </div>
      
      <div class="control-section">
        <!-- Servo 1 -->
        <div class="servo-control">
          <h3>🎯 Servo 1</h3>
          <div class="slider-container">
            <input type="range" id="servo1" min="0" max="180" value="90">
            <div class="angle-display">
              <span>Кут:</span>
              <span id="servo1-value">90°</span>
            </div>
          </div>
          <div class="button-group">
            <button onclick="setServo(1, 0)">0°</button>
            <button onclick="setServo(1, 90)">90°</button>
            <button onclick="setServo(1, 180)">180°</button>
          </div>
        </div>
        
        <!-- Servo 2 -->
        <div class="servo-control">
          <h3>🎯 Servo 2</h3>
          <div class="slider-container">
            <input type="range" id="servo2" min="0" max="180" value="90">
            <div class="angle-display">
              <span>Кут:</span>
              <span id="servo2-value">90°</span>
            </div>
          </div>
          <div class="button-group">
            <button onclick="setServo(2, 0)">0°</button>
            <button onclick="setServo(2, 90)">90°</button>
            <button onclick="setServo(2, 180)">180°</button>
          </div>
        </div>
        
        <!-- Servo 3 -->
        <div class="servo-control">
          <h3>🎯 Servo 3</h3>
          <div class="slider-container">
            <input type="range" id="servo3" min="0" max="180" value="90">
            <div class="angle-display">
              <span>Кут:</span>
              <span id="servo3-value">90°</span>
            </div>
          </div>
          <div class="button-group">
            <button onclick="setServo(3, 0)">0°</button>
            <button onclick="setServo(3, 90)">90°</button>
            <button onclick="setServo(3, 180)">180°</button>
          </div>
        </div>
        
        <!-- Reset All -->
        <button class="reset" onclick="resetAll()">🔄 Повернути в центр</button>
      </div>
    </div>
  </div>

  <script>
    // Update servo on slider change
    document.getElementById('servo1').addEventListener('input', (e) => {
      const angle = e.target.value;
      document.getElementById('servo1-value').textContent = angle + '°';
      fetch(`/servo?id=1&angle=${angle}`);
    });
    
    document.getElementById('servo2').addEventListener('input', (e) => {
      const angle = e.target.value;
      document.getElementById('servo2-value').textContent = angle + '°';
      fetch(`/servo?id=2&angle=${angle}`);
    });
    
    document.getElementById('servo3').addEventListener('input', (e) => {
      const angle = e.target.value;
      document.getElementById('servo3-value').textContent = angle + '°';
      fetch(`/servo?id=3&angle=${angle}`);
    });
    
    function setServo(id, angle) {
      document.getElementById(`servo${id}`).value = angle;
      document.getElementById(`servo${id}-value`).textContent = angle + '°';
      fetch(`/servo?id=${id}&angle=${angle}`);
    }
    
    function resetAll() {
      setServo(1, 90);
      setServo(2, 90);
      setServo(3, 90);
    }
  </script>
</body>
</html>
)rawliteral";

// ========== Web Server Handlers ==========
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleServo() {
  if (!server.hasArg("id") || !server.hasArg("angle")) {
    server.send(400, "text/plain", "Missing parameters");
    return;
  }
  
  int servoId = server.arg("id").toInt();
  int angle = server.arg("angle").toInt();
  
  angle = constrain(angle, 0, 180);
  
  switch(servoId) {
    case 1:
      servo1.write(angle);
      servo1Angle = angle;
      break;
    case 2:
      servo2.write(angle);
      servo2Angle = angle;
      break;
    case 3:
      servo3.write(angle);
      servo3Angle = angle;
      break;
  }
  
  Serial.printf("Servo %d set to %d degrees\n", servoId, angle);
  server.send(200, "text/plain", "OK");
}

void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);
  
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }
    
    String part = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ";
    part += fb->len;
    part += "\r\n\r\n";
    
    server.sendContent(part);
    server.sendContent((const char *)fb->buf, fb->len);
    server.sendContent("\r\n");
    
    esp_camera_fb_return(fb);
    delay(30); // ~33 FPS
  }
}

// ========== WiFi Setup ==========
void connectWiFi() {
  Serial.println("\nStarting WiFi connection...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32-CAM Servo Control Starting...");
  
  // Initialize camera
  initCamera();
  
  // Initialize servos
  servo1.attach(SERVO1_PIN, 1000, 2000);
  servo2.attach(SERVO2_PIN, 1000, 2000);
  servo3.attach(SERVO3_PIN, 1000, 2000);
  
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  
  Serial.println("Servos initialized");
  
  // Connect to WiFi
  connectWiFi();
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/servo", handleServo);
  
  server.begin();
  Serial.println("Web server started");
}

// ========== Main Loop ==========
void loop() {
  server.handleClient();
}
