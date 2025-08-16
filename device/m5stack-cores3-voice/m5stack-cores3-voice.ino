/*
 * M5Stack CoreS3 Voice Recorder for Mastra Server
 * 
 * Features:
 * - Record audio using built-in PDM microphone
 * - Convert to WAV format (16kHz, 16bit, mono)
 * - Send to Mastra server via HTTP POST
 * - WiFi connectivity
 * - Button control for recording
 */

#include <M5CoreS3.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include <SPIFFS.h>
#include "config.h"

// WiFi Configuration
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Mastra Server Configuration
const char* serverURL = SERVER_URL;
const char* agentName = DEFAULT_AGENT;

// Audio Configuration
#define SAMPLE_RATE AUDIO_SAMPLE_RATE
#define BITS_PER_SAMPLE AUDIO_BITS_PER_SAMPLE
#define CHANNELS AUDIO_CHANNELS
#define RECORD_TIME_SECONDS MAX_RECORD_TIME_SEC
#define BUFFER_SIZE (SAMPLE_RATE * BITS_PER_SAMPLE / 8 * CHANNELS)

// I2S Configuration for CoreS3 PDM microphone
#define I2S_PORT I2S_NUM_0

// Recording state
bool isRecording = false;
bool isWiFiConnected = false;
unsigned long recordStartTime = 0;
int16_t* audioBuffer = nullptr;
size_t audioBufferSize = 0;

void setup() {
    M5.begin();
    Serial.begin(115200);
    
    // Initialize SPIFFS for temporary file storage
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        M5.Lcd.println("SPIFFS Error!");
        return;
    }
    
    // Initialize display
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("M5Stack Voice Recorder");
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.println("Connecting to WiFi...");
    
    // Connect to WiFi
    connectToWiFi();
    
    // Initialize I2S for PDM microphone
    initializeI2S();
    
    // Allocate audio buffer
    audioBufferSize = SAMPLE_RATE * RECORD_TIME_SECONDS * sizeof(int16_t);
    audioBuffer = (int16_t*)malloc(audioBufferSize);
    if (!audioBuffer) {
        Serial.println("Failed to allocate audio buffer!");
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setCursor(10, 100);
        M5.Lcd.println("Memory Error!");
        return;
    }
    
    updateDisplay();
}

void loop() {
    M5.update();
    
    // Button A: Start/Stop recording
    if (M5.BtnA.wasPressed()) {
        if (!isRecording) {
            startRecording();
        } else {
            stopRecording();
        }
    }
    
    // Auto-stop recording after 5 seconds
    if (isRecording && (millis() - recordStartTime > (RECORD_TIME_SECONDS * 1000))) {
        stopRecording();
    }
    
    delay(50);
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        M5.Lcd.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isWiFiConnected = true;
        Serial.println("");
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.println("WiFi Connected!");
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.print("IP: ");
        M5.Lcd.println(WiFi.localIP());
    } else {
        isWiFiConnected = false;
        Serial.println("WiFi connection failed!");
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setCursor(10, 100);
        M5.Lcd.println("WiFi Failed!");
    }
    
    delay(2000);
}

void initializeI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("I2S driver install failed: %d\n", result);
        return;
    }
    
    result = i2s_set_pin(I2S_PORT, &pin_config);
    if (result != ESP_OK) {
        Serial.printf("I2S set pin failed: %d\n", result);
        return;
    }
    
    Serial.println("I2S initialized successfully");
}

void startRecording() {
    if (!isWiFiConnected) {
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setCursor(10, 100);
        M5.Lcd.println("No WiFi Connection!");
        delay(2000);
        updateDisplay();
        return;
    }
    
    Serial.println("Starting recording...");
    isRecording = true;
    recordStartTime = millis();
    
    // Clear audio buffer
    memset(audioBuffer, 0, audioBufferSize);
    
    // Start I2S
    i2s_start(I2S_PORT);
    
    updateDisplay();
    
    // Record audio in chunks
    size_t totalBytesRead = 0;
    size_t bytesToRead = BUFFER_SIZE;
    
    while (isRecording && totalBytesRead < audioBufferSize - bytesToRead) {
        size_t bytesRead = 0;
        esp_err_t result = i2s_read(I2S_PORT, 
                                   (uint8_t*)audioBuffer + totalBytesRead, 
                                   bytesToRead, 
                                   &bytesRead, 
                                   portMAX_DELAY);
        
        if (result == ESP_OK) {
            totalBytesRead += bytesRead;
        }
        
        // Check for stop conditions
        M5.update();
        if (M5.BtnA.wasPressed() || (millis() - recordStartTime > (RECORD_TIME_SECONDS * 1000))) {
            break;
        }
    }
    
    i2s_stop(I2S_PORT);
    Serial.printf("Recording completed. Bytes recorded: %d\n", totalBytesRead);
}

void stopRecording() {
    if (!isRecording) return;
    
    isRecording = false;
    i2s_stop(I2S_PORT);
    
    Serial.println("Recording stopped. Converting to WAV...");
    updateDisplay();
    
    // Convert to WAV and send to server
    if (createWAVFile()) {
        sendAudioToServer();
    }
    
    updateDisplay();
}

bool createWAVFile() {
    File file = SPIFFS.open("/audio.wav", "w");
    if (!file) {
        Serial.println("Failed to create WAV file");
        return false;
    }
    
    // WAV header
    uint32_t dataSize = audioBufferSize;
    uint32_t fileSize = dataSize + 36;
    
    // RIFF header
    file.write((uint8_t*)"RIFF", 4);
    file.write((uint8_t*)&fileSize, 4);
    file.write((uint8_t*)"WAVE", 4);
    
    // fmt chunk
    file.write((uint8_t*)"fmt ", 4);
    uint32_t fmtSize = 16;
    file.write((uint8_t*)&fmtSize, 4);
    uint16_t audioFormat = 1; // PCM
    file.write((uint8_t*)&audioFormat, 2);
    uint16_t numChannels = CHANNELS;
    file.write((uint8_t*)&numChannels, 2);
    uint32_t sampleRate = SAMPLE_RATE;
    file.write((uint8_t*)&sampleRate, 4);
    uint32_t byteRate = SAMPLE_RATE * CHANNELS * BITS_PER_SAMPLE / 8;
    file.write((uint8_t*)&byteRate, 4);
    uint16_t blockAlign = CHANNELS * BITS_PER_SAMPLE / 8;
    file.write((uint8_t*)&blockAlign, 2);
    uint16_t bitsPerSample = BITS_PER_SAMPLE;
    file.write((uint8_t*)&bitsPerSample, 2);
    
    // data chunk
    file.write((uint8_t*)"data", 4);
    file.write((uint8_t*)&dataSize, 4);
    
    // Write audio data
    file.write((uint8_t*)audioBuffer, audioBufferSize);
    
    file.close();
    Serial.println("WAV file created successfully");
    return true;
}

void sendAudioToServer() {
    if (!isWiFiConnected) {
        Serial.println("No WiFi connection");
        return;
    }
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("Sending to server...");
    
    HTTPClient http;
    http.begin(serverURL);
    
    // Read WAV file
    File file = SPIFFS.open("/audio.wav", "r");
    if (!file) {
        Serial.println("Failed to open WAV file");
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setCursor(10, 100);
        M5.Lcd.println("File Error!");
        delay(2000);
        return;
    }
    
    size_t fileSize = file.size();
    
    // Create multipart form data
    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String contentType = "multipart/form-data; boundary=" + boundary;
    
    String formData = "";
    formData += "--" + boundary + "\r\n";
    formData += "Content-Disposition: form-data; name=\"agentName\"\r\n\r\n";
    formData += agentName;
    formData += "\r\n--" + boundary + "\r\n";
    formData += "Content-Disposition: form-data; name=\"audio\"; filename=\"audio.wav\"\r\n";
    formData += "Content-Type: audio/wav\r\n\r\n";
    
    String footer = "\r\n--" + boundary + "--\r\n";
    
    http.addHeader("Content-Type", contentType);
    http.addHeader("Content-Length", String(formData.length() + fileSize + footer.length()));
    
    // Send POST request
    WiFiClient* client = http.getStreamPtr();
    client->print(formData);
    
    // Send file data in chunks
    uint8_t buffer[1024];
    while (file.available()) {
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        client->write(buffer, bytesRead);
    }
    
    client->print(footer);
    file.close();
    
    int httpResponseCode = http.GET(); // Actually sends the POST
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 50);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response: %d\n", httpResponseCode);
        Serial.println("Response: " + response);
        
        if (httpResponseCode == 200) {
            M5.Lcd.setTextColor(GREEN);
            M5.Lcd.println("Success!");
            M5.Lcd.setCursor(10, 80);
            M5.Lcd.setTextSize(1);
            M5.Lcd.println("Check server response");
        } else {
            M5.Lcd.setTextColor(RED);
            M5.Lcd.println("Server Error!");
            M5.Lcd.setCursor(10, 80);
            M5.Lcd.printf("Code: %d", httpResponseCode);
        }
    } else {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Connection Failed!");
        Serial.printf("HTTP Error: %d\n", httpResponseCode);
    }
    
    http.end();
    delay(3000);
}

void updateDisplay() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Voice Recorder");
    
    M5.Lcd.setCursor(10, 50);
    if (!isWiFiConnected) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("WiFi: Disconnected");
    } else {
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println("WiFi: Connected");
    }
    
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.setTextColor(WHITE);
    if (isRecording) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Recording...");
        
        // Show countdown
        unsigned long elapsed = (millis() - recordStartTime) / 1000;
        unsigned long remaining = RECORD_TIME_SECONDS - elapsed;
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.printf("Time: %lu sec", remaining);
    } else {
        M5.Lcd.println("Press A to Record");
    }
    
    M5.Lcd.setCursor(10, 150);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("Server: %s", serverURL);
    M5.Lcd.setCursor(10, 170);
    M5.Lcd.printf("Agent: %s", agentName);
}