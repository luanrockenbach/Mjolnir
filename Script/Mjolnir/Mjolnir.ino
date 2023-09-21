#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "a"
#define WIFI_PASSWORD "12345678"
// Defining the WiFi channel speeds up the connection:

WebServer server(80);

const int LED1 = 2;

bool led1State = false;

void sendHtml() {
  String response = R"(
    <!DOCTYPE html><html>
      <head>
        <title>Mjonir</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta charset="UTF-8">
        <meta http-equiv="refresh" content="1">
        <style>
          html { font-family: sans-serif; text-align: center;}
          body { display: inline-flex; flex-direction: column; }
          hr {display: block; margin-top: 0.5em; margin-bottom: 0.5em; margin-left: auto; margin-right: auto;
              border-style: inset;border-width: 1px;}
          h1 { text-align: center;} 
          .container{text-align: center;}
          .btn { background-color: #5B5; border: none; color: #fff; padding: 0.5em 1em;
                 font-size: 2em; text-decoration: none}
          .btn.OFF { background-color: #111; color: #fff; }
        </style>
      </head>
            
      <body>
        <h1>CST Eletrônica Industrial</h1>
        <hr>
        <h1>Martelo Eletromagnético</h1>

        <div class="container">
          <a href="/toggle/1" class="btn LED1_TEXT">LED1_TEXT</a>
        </div>
      </body>
    </html>
  )";
  response.replace("LED1_TEXT", led1State ? "ON" : "OFF");
  server.send(200, "text/html", response);
}

void setup(void) {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 1, false, 1);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  // Wait for connection
  while (WiFi.softAPgetStationNum() == 0) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", sendHtml);

  server.on(UriBraces("/toggle/{}"), []() {
    String led = server.pathArg(0);

    switch (led.toInt()) {
      case 1:
        led1State = !led1State;
        digitalWrite(LED1, led1State);
        break;
    }

    sendHtml();
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);
}