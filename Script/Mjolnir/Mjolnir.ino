#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <Arduino.h>
#include "HX711.h"
//#include "soc/rtc.h"

#define WIFI_SSID "aBcD"
#define WIFI_PASSWORD "12345678"

WebServer server(80);

const int LED1 = 2;
const int mosfet = 13;


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 4;
const int NUM_AMOSTRAS = 32;
long int medidas_forca[NUM_AMOSTRAS];
int medidas_forca_indice=0;

HX711 scale;
// Parâmetros para botar na tela o gráfico de força
const int FATOR_REDUCAO=1000;
const int LARGURA_QUADRO=300;
const int ALTURA_QUADRO=100;
const int PASSO_QUADRO=LARGURA_QUADRO/NUM_AMOSTRAS;
const int FATOR_AJUSTE_TELA=(ALTURA_QUADRO/6); // fator para fazer log10(1.000.000)=6 caber na tela

bool led1State = false;

void sendHtml() {
  
  Serial.println("Criando página WEB - inicio");
  char response [4096];
  const char PAGINAINICIO[] = R"(
    <!DOCTYPE html><html>
      <head>
        <title>Mjonir</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta charset="UTF-8">
        <meta http-equiv="refresh" content="1" />
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

        <h2>Quanto forte você é? Você é o escolhido? </h2>

        <canvas id="myCanvas" width="WWWWWWWWWW" height="HHHHHHHHHH" style="border:1px solid grey;"></canvas>

        <script>
        // Create a canvas:
        const canvas = document.getElementById("myCanvas");
        const ctx = canvas.getContext("2d");

        // Define a new path:
        ctx.beginPath();
        // Define a start point
        ctx.moveTo(0,0);
        // Define an end point
  )";
  const char PAGINAFINAL[] = R"(
        ctx.stroke();  
        </script>
      </body>
    </html>
      )";
  Serial.println("Criando página WEB - 1");
 strcpy (response,PAGINAINICIO);
  Serial.println("Criando página WEB - 2");
  Serial.println("Criando página WEB - ");
  int a=0,b;
  b=medidas_forca_indice;
  char buffer[11];
  itoa(LARGURA_QUADRO,buffer,10); 
  //response.replace("WWWWWWWWWW", buffer);
  Serial.println("Criando página WEB - 3");
  Serial.printf("\t\t %s\n",buffer);
  memcpy(strstr(response, "WWWWWWWWWW"), buffer, 10);
  Serial.println("Criando página WEB - 4");
  itoa(ALTURA_QUADRO,buffer,10);
  Serial.printf("\t\t %s\n",buffer);
  //response.replace("HHHHHHHHHH", buffer );  
  memcpy(strstr(response, "HHHHHHHHHH"), buffer, 10);
  Serial.println("Criando página WEB - 5");
        
  
  for(a=0;a<NUM_AMOSTRAS;a++)
  {
    char buffer[50];
    sprintf(buffer, "ctx.lineTo(%d,%d);\n", (PASSO_QUADRO*a), ALTURA_QUADRO-medidas_forca[b]); // geração do gráfico é apontando para baixo (sentido do eixo Y para baixo)
    //response.concat(buffer);
    strcat (response,buffer);
    b++;
    if(b==NUM_AMOSTRAS)
      b=0;  // volta ao inicio do vetor
  }
  strcat (response,PAGINAINICIO);
  //response.replace("LED1_TEXT", led1State ? "ON " : "OFF");
  memcpy(strstr(response, "LED1_TEXT"), led1State ? "ON " : "OFF", 9);
  Serial.println("Criando página WEB - fim");
  server.send(200, "text/html", response);
  Serial.print(response);
}

void setup(void) {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(mosfet, OUTPUT);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 1, false, 1);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  // Wait for connection
  while (WiFi.softAPgetStationNum() == 0) {
    delay(100);
    //Serial.print(".");
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
  //////////////////////////////////////////////////////
  //rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("HX711 em uso - medida de força - inicializando medida do ADC");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);                
  Serial.println("Calculando tara - zerando medida de força - espere");
  scale.tare(5);     // set the OFFSET value for tare weight; times = how many times to read the tare value
  Serial.println("Tara zerada");

  for(medidas_forca_indice=0;medidas_forca_indice<NUM_AMOSTRAS;medidas_forca_indice++)
      medidas_forca[medidas_forca_indice]=0;

  medidas_forca_indice=0;
}

void loop(void) {
  long int forca;
  server.handleClient();
  //delay(2);
  forca=scale.get_value(8);// média de x medidas de forca, já vai tomar o tempo equivalente do delay!
  // imprime para debug somente
  Serial.printf("%-8.0f    ->    ",(float)forca);   // print the average of X readings from the ADC
  medidas_forca[medidas_forca_indice]=FATOR_AJUSTE_TELA*((float)log10((float)abs(forca)));
  Serial.printf("%-8.0f\n",(float)medidas_forca[medidas_forca_indice]);   // print the average of X readings from the ADC
 
  medidas_forca_indice++;
  if(medidas_forca_indice==NUM_AMOSTRAS)
    medidas_forca_indice=0;  // volta ao inicio do vetor
  //Mutley, faça alguma coisa!!!
}