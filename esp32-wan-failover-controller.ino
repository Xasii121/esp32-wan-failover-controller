#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Attempt to load private credentials
#if __has_include("config.h")
    #include "config.h"
#else
    #include "config.h.example"
    #warning "Using default configuration template. Rename config.h.example to config.h to use real keys."
#endif

int throttling_streak = 0;
WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PRIMARY_WAN, OUTPUT);
  pinMode(RELAY_BACKUP_WAN, OUTPUT);
  
  // Default State: Primary WAN Link active, Backup isolated
  digitalWrite(RELAY_PRIMARY_WAN, HIGH); 
  digitalWrite(RELAY_BACKUP_WAN, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[SYSTEM] Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[SYSTEM] Connected to Local Network Gateway.");
  
  client.setInsecure(); // Bypass strict SSL certificate validation for Telegram API
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[MONITOR] Sending ICMP echo to %s... ", PING_TARGET);
    
    bool success = Ping.ping(PING_TARGET, 5); 
    
    if (success) {
      float avg_time = Ping.averageTime();
      Serial.printf("Reply received. Avg Latency: %.2f ms\n", avg_time);

      if (avg_time >= LATENCY_THRESHOLD_MS) {
        throttling_streak++;
        Serial.printf("[WARNING] Bufferbloat/Throttling suspected. Streak: %d/%d\n", throttling_streak, MAX_FAIL_STREAK);
      } else {
        throttling_streak = 0; // Reset metrics on stable connectivity
      }
    } else {
      throttling_streak++;
      Serial.printf("[ALERT] Packet Drop / Link Down. Streak: %d/%d\n", throttling_streak, MAX_FAIL_STREAK);
    }

    if (throttling_streak >= MAX_FAIL_STREAK) {
      triggerWanFailover();
    }

  } else {
    Serial.println("[LINK] Wi-Fi lost. Attempting automated reconnection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  delay(60000); // Poll link state every 60 seconds
}

void triggerWanFailover() {
  Serial.println("[CRITICAL] Threshold broken. Quota completion assumed. Executing failover sequence...");
  
  String payload = "🚨 *WAN FAILOVER TRIGGERED*\n\nPrimary gateway data quota exceeded or heavily throttled. Flipping hardware switches to secondary backup server/router.";
  if (bot.sendMessage(TELEGRAM_CHAT_ID, payload, "Markdown")) {
    Serial.println("[TELEMETRY] Telegram notification dispatched.");
  } else {
    Serial.println("[ERROR] Telemetry dispatch failed.");
  }
  
  delay(3000); // Allow networking buffers to flush message before power down

  digitalWrite(RELAY_PRIMARY_WAN, LOW);  // Power down throttled interface
  delay(1500);
  digitalWrite(RELAY_BACKUP_WAN, HIGH);  // Energize backup line
  
  Serial.println("[SYSTEM] Failover complete. Halting core loop.");
  while (true) { 
    delay(10000); // Freeze execution state to avoid physical flapping loops
  }
}
