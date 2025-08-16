/*
 * Configuration file for M5Stack CoreS3 Voice Recorder
 * 
 * Update these settings according to your environment
 */

#ifndef CONFIG_H
#define CONFIG_H

// ================================
// WiFi Configuration
// ================================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT_MS 10000  // 10 seconds

// ================================
// Mastra Server Configuration
// ================================
#define SERVER_HOST "192.168.1.100"  // Replace with your server IP
#define SERVER_PORT 4114
#define SERVER_ENDPOINT "/voice-to-agent"
#define DEFAULT_AGENT "generalAgent"  // or "weatherAgent"

// Full server URL (constructed from above)
#define SERVER_URL "http://" SERVER_HOST ":" STR(SERVER_PORT) SERVER_ENDPOINT
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// ================================
// Audio Configuration
// ================================
#define AUDIO_SAMPLE_RATE 16000     // 16kHz
#define AUDIO_BITS_PER_SAMPLE 16    // 16-bit
#define AUDIO_CHANNELS 1            // Mono
#define MAX_RECORD_TIME_SEC 5       // Maximum recording time
#define MIN_RECORD_TIME_SEC 1       // Minimum recording time

// ================================
// I2S Pin Configuration for CoreS3
// ================================
#define I2S_WS_PIN 42    // PDM CLK pin
#define I2S_SD_PIN 41    // PDM DATA pin
#define I2S_SCK_PIN -1   // Not used for PDM

// ================================
// Hardware Configuration
// ================================
#define BUTTON_RECORD M5.BtnA       // Record button
#define LED_BUILTIN 2               // Built-in LED pin
#define VIBRATION_PIN 4             // Vibration motor pin (if available)

// ================================
// Memory Configuration
// ================================
#define AUDIO_BUFFER_SIZE (AUDIO_SAMPLE_RATE * MAX_RECORD_TIME_SEC * sizeof(int16_t))
#define I2S_DMA_BUF_COUNT 8
#define I2S_DMA_BUF_LEN 1024
#define HTTP_CHUNK_SIZE 1024

// ================================
// Display Configuration
// ================================
#define DISPLAY_BRIGHTNESS 100      // LCD brightness (0-255)
#define STATUS_UPDATE_INTERVAL 100  // Status update interval in ms
#define DISPLAY_TIMEOUT_MS 30000    // Auto turn off display after 30 seconds

// ================================
// Debug Configuration
// ================================
#define DEBUG_SERIAL true           // Enable serial debug output
#define DEBUG_BAUD_RATE 115200      // Serial baud rate
#define DEBUG_AUDIO false           // Enable audio debug (prints audio levels)

// ================================
// Error Handling
// ================================
#define MAX_RETRY_ATTEMPTS 3        // Max retry for network operations
#define RETRY_DELAY_MS 1000         // Delay between retries
#define ERROR_DISPLAY_TIME_MS 3000  // How long to show error messages

// ================================
// Feature Flags
// ================================
#define ENABLE_SPIFFS true          // Use SPIFFS for temporary file storage
#define ENABLE_VIBRATION false      // Enable vibration feedback
#define ENABLE_LED_FEEDBACK true    // Enable LED status indication
#define ENABLE_AUTO_GAIN true       // Enable automatic gain control

#endif // CONFIG_H