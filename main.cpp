#include <Arduino.h>

#include <WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;
#define AI2 34
const char *ssid = "xxxxxxx";
const char *password = "xxxxx";

//const char *websockets_server_host = "ws://localhost"; //Enter server adress
const char *websockets_server_host = "ws://127.0.0.1"; //Enter server adress
const uint16_t websockets_server_port = 8080; // Enter server port

WebsocketsClient websocket_client;

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" connected");
    server.begin();
    Serial.printf("Web server started, open %s in a web browser\n",
                  WiFi.localIP().toString().c_str());
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
    esp_sleep_enable_timer_wakeup(5000000);

    // run callback when messages are received
    websocket_client.onMessage(onMessageCallback);
}

String prepareHtmlPage() {
    String htmlPage =
            String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +
            "Refresh: 0.5\r\n" +
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<html>" +
            "Analog input: " + String(analogRead(AI2)) +
            "</html>" +
            "\r\n";
    return htmlPage;
}

void loop() {

//delay(2000);
    WiFiClient client_wifi = server.available();

    if (client_wifi) {
        bool connected = websocket_client.connect(websockets_server_host, websockets_server_port, "/");
        if (connected) {
            Serial.println("Connected to websocket!");
            websocket_client.send("My name is JohnTravolta");
        } else {
            Serial.println("Not Connected!");
        }

        Serial.println("\n[Client connected]");
        while (client_wifi.connected()) {
            if (client_wifi.available()) {
                String line = client_wifi.readStringUntil('\r');
                Serial.print(line);

                if (line.length() == 1 && line[0] == '\n') {
                    client_wifi.println(prepareHtmlPage());
                    break;
                }
            }
        }
        delay(1);

        client_wifi.stop();
        Serial.println("[Client disonnected]");
    }

    esp_deep_sleep_start();
}
