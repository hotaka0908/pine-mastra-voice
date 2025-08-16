/*
 * Utility functions for M5Stack CoreS3 Voice Recorder
 */

#ifndef UTILS_H
#define UTILS_H

#include "config.h"

// Debug printing macro
#if DEBUG_SERIAL
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(format, ...) Serial.printf(format, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(format, ...)
#endif

// LED Status indicators
enum LEDStatus {
  LED_OFF,
  LED_WIFI_CONNECTING,
  LED_WIFI_CONNECTED,
  LED_RECORDING,
  LED_SENDING,
  LED_SUCCESS,
  LED_ERROR
};

// Error codes
enum ErrorCode {
  ERROR_NONE = 0,
  ERROR_WIFI_FAILED = 1,
  ERROR_SPIFFS_FAILED = 2,
  ERROR_I2S_FAILED = 3,
  ERROR_MEMORY_FAILED = 4,
  ERROR_FILE_FAILED = 5,
  ERROR_SERVER_FAILED = 6,
  ERROR_NETWORK_FAILED = 7
};

// Function prototypes
void setLEDStatus(LEDStatus status);
void showError(ErrorCode error, const char* message = nullptr);
void playTone(int frequency, int duration);
void vibrate(int duration);
bool isValidAudioLevel(int16_t* buffer, size_t samples);
void printMemoryInfo();
String formatFileSize(size_t bytes);
String getErrorMessage(ErrorCode error);
void blinkBuiltinLED(int times, int delayMs = 200);

// LED Status control
void setLEDStatus(LEDStatus status) {
  #if ENABLE_LED_FEEDBACK
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  switch (status) {
    case LED_OFF:
      digitalWrite(LED_BUILTIN, LOW);
      break;
      
    case LED_WIFI_CONNECTING:
      // Slow blink
      if (millis() - lastBlink > 1000) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
        lastBlink = millis();
      }
      break;
      
    case LED_WIFI_CONNECTED:
      digitalWrite(LED_BUILTIN, HIGH);
      break;
      
    case LED_RECORDING:
      // Fast blink
      if (millis() - lastBlink > 200) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
        lastBlink = millis();
      }
      break;
      
    case LED_SENDING:
      // Double blink
      if (millis() - lastBlink > 100) {
        static int blinkCount = 0;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
        lastBlink = millis();
        blinkCount++;
        if (blinkCount >= 4) {
          blinkCount = 0;
          delay(300);
        }
      }
      break;
      
    case LED_SUCCESS:
      // Solid on for 2 seconds
      digitalWrite(LED_BUILTIN, HIGH);
      delay(2000);
      digitalWrite(LED_BUILTIN, LOW);
      break;
      
    case LED_ERROR:
      // Triple blink
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      break;
  }
  #endif
}

// Error handling
void showError(ErrorCode error, const char* message) {
  M5.Lcd.fillScreen(RED);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.println("ERROR!");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.printf("Code: %d", error);
  
  M5.Lcd.setCursor(10, 100);
  if (message) {
    M5.Lcd.println(message);
  } else {
    M5.Lcd.println(getErrorMessage(error));
  }
  
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.println("Press any button to continue");
  
  DEBUG_PRINTF("ERROR %d: %s\n", error, message ? message : getErrorMessage(error).c_str());
  
  setLEDStatus(LED_ERROR);
  
  #if ENABLE_VIBRATION
  vibrate(500);
  #endif
}

// Audio validation
bool isValidAudioLevel(int16_t* buffer, size_t samples) {
  int32_t sum = 0;
  int32_t maxLevel = 0;
  
  for (size_t i = 0; i < samples; i++) {
    int32_t level = abs(buffer[i]);
    sum += level;
    if (level > maxLevel) {
      maxLevel = level;
    }
  }
  
  int32_t average = sum / samples;
  
  #if DEBUG_AUDIO
  DEBUG_PRINTF("Audio levels - Avg: %d, Max: %d\n", average, maxLevel);
  #endif
  
  // Check if audio level is reasonable (not too quiet, not clipping)
  return (average > 50 && maxLevel < 32000);
}

// Memory information
void printMemoryInfo() {
  #if DEBUG_SERIAL
  DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
  DEBUG_PRINTF("Heap size: %d bytes\n", ESP.getHeapSize());
  DEBUG_PRINTF("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  DEBUG_PRINTF("PSRAM size: %d bytes\n", ESP.getPsramSize());
  #endif
}

// File size formatting
String formatFileSize(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < 1024 * 1024) {
    return String(bytes / 1024.0, 1) + "KB";
  } else {
    return String(bytes / (1024.0 * 1024.0), 1) + "MB";
  }
}

// Error message mapping
String getErrorMessage(ErrorCode error) {
  switch (error) {
    case ERROR_NONE: return "No error";
    case ERROR_WIFI_FAILED: return "WiFi connection failed";
    case ERROR_SPIFFS_FAILED: return "SPIFFS initialization failed";
    case ERROR_I2S_FAILED: return "I2S initialization failed";
    case ERROR_MEMORY_FAILED: return "Memory allocation failed";
    case ERROR_FILE_FAILED: return "File operation failed";
    case ERROR_SERVER_FAILED: return "Server communication failed";
    case ERROR_NETWORK_FAILED: return "Network connection failed";
    default: return "Unknown error";
  }
}

// Built-in LED blink
void blinkBuiltinLED(int times, int delayMs) {
  #if ENABLE_LED_FEEDBACK
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
  }
  #endif
}

// Tone generation (if speaker available)
void playTone(int frequency, int duration) {
  // M5Stack CoreS3 doesn't have built-in speaker
  // This can be implemented if external speaker is connected
  // For now, just use LED feedback
  blinkBuiltinLED(1, duration);
}

// Vibration feedback
void vibrate(int duration) {
  #if ENABLE_VIBRATION
  // M5Stack CoreS3 doesn't have built-in vibration motor
  // This can be implemented if external vibration motor is connected
  // For now, just use LED feedback
  blinkBuiltinLED(3, duration / 6);
  #endif
}

#endif // UTILS_H