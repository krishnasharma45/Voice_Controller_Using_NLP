#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
#define USE_SERIAL Serial
#include <Arduino.h>
#include <HTTPClient.h>

#define led_3 4
#define led_1 15
#define led_2 2

CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
  this->authentication = authentication;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  client.setCACert(root_ca);

  if (!client.connect(server, 443)) Serial.println("Connection failed!");
  digitalWrite(led_3, 1);  // Indicate connection success
}

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", "");
  client.print(enc);
  char** wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
    enc = base64::encode((byte*)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", "");
    client.print(enc);
  }
}

void CloudSpeechClient::Transcribe(Audio* audio) {
  // Preparing the HTTP body and headers
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-IN\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3;
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;  
  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey
               + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");

  client.print(HttpHeader);
  client.print(HttpBody1);
  PrintHttpBody2(audio);
  client.print(HttpBody3);

  // Wait for the server response
  String My_Answer = "";
  while (!client.available());
  while (client.available()) {
    char temp = client.read();
    My_Answer = My_Answer + temp;
  }

  // Parse the JSON response to extract the transcribed text
  int position = My_Answer.indexOf('{');
  String jsonResponse = My_Answer.substring(position);
  Serial.print("Received JSON data: ");

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject results_0 = doc["results"][0];
  const char* transcribed_text = results_0["alternatives"][0]["transcript"];
  
  // Output the transcribed text
  Serial.print("Transcribed Text: ");
  Serial.println(transcribed_text);

  // Indicate completion using LEDs
  digitalWrite(led_1, 1); // Success
  digitalWrite(led_2, 1); // Indicate transcription done
  delay(1000);  // Hold for a second before resetting LEDs
  digitalWrite(led_1, 0);
  digitalWrite(led_2, 0);
}
