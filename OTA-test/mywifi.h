#ifndef MYWIFI_H
#define MYWIFI_H
void wifiStart()
{
 // Connect to WiFi network
 Serial.printf("Starting WiFi with SSID = [%s], password = [%s]\n",ssid, password);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.printf("OTA loader at http://%s.local or the IP address above.\n", host);
}
#endif
