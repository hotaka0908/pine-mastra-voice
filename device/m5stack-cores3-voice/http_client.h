/*
 * HTTP Client helper for M5Stack CoreS3 Voice Recorder
 * Handles multipart form data upload to Mastra server
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "config.h"
#include "utils.h"

class VoiceHTTPClient {
private:
  HTTPClient http;
  String boundary;
  
  // Generate random boundary string
  String generateBoundary() {
    String boundary = "----WebKitFormBoundary";
    for (int i = 0; i < 16; i++) {
      boundary += String(random(10));
    }
    return boundary;
  }
  
  // Create form data header
  String createFormDataHeader(const String& fieldName, const String& filename = "", const String& contentType = "") {
    String header = "--" + boundary + "\r\n";
    header += "Content-Disposition: form-data; name=\"" + fieldName + "\"";
    
    if (filename.length() > 0) {
      header += "; filename=\"" + filename + "\"";
    }
    
    header += "\r\n";
    
    if (contentType.length() > 0) {
      header += "Content-Type: " + contentType + "\r\n";
    }
    
    header += "\r\n";
    return header;
  }
  
  // Create form data footer
  String createFormDataFooter() {
    return "\r\n--" + boundary + "--\r\n";
  }

public:
  VoiceHTTPClient() {
    boundary = generateBoundary();
  }
  
  // Send audio file to server
  bool sendAudioFile(const String& filePath, const String& agentName, String& response) {
    if (!WiFi.isConnected()) {
      DEBUG_PRINTLN("WiFi not connected");
      return false;
    }
    
    File audioFile = SPIFFS.open(filePath, "r");
    if (!audioFile) {
      DEBUG_PRINTLN("Failed to open audio file");
      return false;
    }
    
    size_t fileSize = audioFile.size();
    DEBUG_PRINTF("Audio file size: %s\n", formatFileSize(fileSize).c_str());
    
    // Start HTTP client
    http.begin(serverURL);
    http.setTimeout(30000); // 30 second timeout
    
    // Prepare multipart form data
    String agentHeader = createFormDataHeader("agentName");
    String audioHeader = createFormDataHeader("audio", "audio.wav", "audio/wav");
    String footer = createFormDataFooter();
    
    size_t totalSize = agentHeader.length() + agentName.length() + 
                       audioHeader.length() + fileSize + footer.length();
    
    // Set headers
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);
    http.addHeader("Content-Length", String(totalSize));
    http.addHeader("User-Agent", "M5Stack-CoreS3-Voice/1.0");
    
    DEBUG_PRINTF("Total upload size: %s\n", formatFileSize(totalSize).c_str());
    DEBUG_PRINTF("Boundary: %s\n", boundary.c_str());
    
    // Start HTTP connection
    int httpResponseCode = http.POST("");
    
    if (httpResponseCode > 0) {
      // Get the response
      response = http.getString();
      DEBUG_PRINTF("HTTP Response Code: %d\n", httpResponseCode);
      DEBUG_PRINTF("Response: %s\n", response.c_str());
      
      audioFile.close();
      http.end();
      
      return (httpResponseCode == 200);
    } else {
      DEBUG_PRINTF("HTTP Error: %d\n", httpResponseCode);
      audioFile.close();
      http.end();
      return false;
    }
  }
  
  // Send audio file with streaming upload
  bool sendAudioFileStreaming(const String& filePath, const String& agentName, String& response) {
    if (!WiFi.isConnected()) {
      DEBUG_PRINTLN("WiFi not connected");
      return false;
    }
    
    File audioFile = SPIFFS.open(filePath, "r");
    if (!audioFile) {
      DEBUG_PRINTLN("Failed to open audio file");
      return false;
    }
    
    size_t fileSize = audioFile.size();
    DEBUG_PRINTF("Audio file size: %s\n", formatFileSize(fileSize).c_str());
    
    // Start HTTP client
    http.begin(serverURL);
    http.setTimeout(30000); // 30 second timeout
    
    // Prepare multipart form data
    String agentHeader = createFormDataHeader("agentName");
    String agentData = agentName + "\r\n";
    String audioHeader = createFormDataHeader("audio", "audio.wav", "audio/wav");
    String footer = createFormDataFooter();
    
    size_t totalSize = agentHeader.length() + agentData.length() + 
                       audioHeader.length() + fileSize + footer.length();
    
    // Set headers
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);
    http.addHeader("Content-Length", String(totalSize));
    http.addHeader("User-Agent", "M5Stack-CoreS3-Voice/1.0");
    
    DEBUG_PRINTF("Starting streaming upload, total size: %s\n", formatFileSize(totalSize).c_str());
    
    // Begin POST request
    WiFiClient* stream = http.getStreamPtr();
    http.POST("");
    
    // Send agent name part
    stream->print(agentHeader);
    stream->print(agentData);
    
    // Send audio header
    stream->print(audioHeader);
    
    // Send audio file in chunks
    uint8_t buffer[HTTP_CHUNK_SIZE];
    size_t totalSent = 0;
    
    while (audioFile.available()) {
      size_t bytesToRead = min((size_t)HTTP_CHUNK_SIZE, audioFile.available());
      size_t bytesRead = audioFile.read(buffer, bytesToRead);
      
      if (bytesRead > 0) {
        size_t bytesSent = stream->write(buffer, bytesRead);
        totalSent += bytesSent;
        
        // Update progress
        int progress = (totalSent * 100) / fileSize;
        DEBUG_PRINTF("Upload progress: %d%% (%s/%s)\n", 
                     progress, formatFileSize(totalSent).c_str(), formatFileSize(fileSize).c_str());
        
        // Update display with progress
        M5.Lcd.fillRect(10, 190, 300, 20, BLACK);
        M5.Lcd.setCursor(10, 190);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.printf("Progress: %d%%", progress);
        
        if (bytesSent != bytesRead) {
          DEBUG_PRINTLN("Write error during upload");
          break;
        }
      } else {
        DEBUG_PRINTLN("Read error from file");
        break;
      }
      
      yield(); // Allow other tasks to run
    }
    
    // Send footer
    stream->print(footer);
    
    // Get response
    int httpResponseCode = http.getResponseCode();
    
    if (httpResponseCode > 0) {
      response = http.getString();
      DEBUG_PRINTF("HTTP Response Code: %d\n", httpResponseCode);
      DEBUG_PRINTF("Response: %s\n", response.c_str());
    } else {
      DEBUG_PRINTF("HTTP Error: %d\n", httpResponseCode);
    }
    
    audioFile.close();
    http.end();
    
    return (httpResponseCode == 200);
  }
  
  // Test server connectivity
  bool testConnection() {
    if (!WiFi.isConnected()) {
      return false;
    }
    
    // Try to connect to server without sending data
    HTTPClient testHttp;
    testHttp.begin(serverURL);
    testHttp.setTimeout(5000); // 5 second timeout
    
    int responseCode = testHttp.GET();
    testHttp.end();
    
    DEBUG_PRINTF("Server test response: %d\n", responseCode);
    
    // Any response means server is reachable
    return (responseCode > 0);
  }
  
  // Get server status
  String getServerStatus() {
    if (!WiFi.isConnected()) {
      return "WiFi disconnected";
    }
    
    HTTPClient statusHttp;
    statusHttp.begin("http://" SERVER_HOST ":" STR(SERVER_PORT) "/api");
    statusHttp.setTimeout(5000);
    
    int responseCode = statusHttp.GET();
    String response = "";
    
    if (responseCode > 0) {
      response = statusHttp.getString();
      DEBUG_PRINTF("Server status: %d - %s\n", responseCode, response.c_str());
    } else {
      DEBUG_PRINTF("Server status error: %d\n", responseCode);
      response = "Server unreachable";
    }
    
    statusHttp.end();
    return response;
  }
};

#endif // HTTP_CLIENT_H