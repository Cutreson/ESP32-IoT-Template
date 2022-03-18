#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Arduino.h>
#include <SocketIoClient.h>
#include <string.h>
#include <ArduinoJson.h>

const char* host = "esp32";
const char* ssid = "Cutreson";
const char* password = "28031997";

WebServer server(80);
//////////////////////////////////////////////////////////
#define ledPin 2
const char *socketServer = "esp32-iot-template.herokuapp.com";
const int socketPort = 80;

SocketIoClient socket;
char messageJsonEmit[300]; //message socket io emit len server
//chuyen data json nhan ve thanh json object
JsonObject& dataReceiver(const char *data){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& parsed = jsonBuffer.parseObject(data); //Parse message
  Serial.printf("Server send data :\n");
  Serial.printf(parsed["name"]);
  Serial.printf("\n");
  Serial.printf(parsed["address"]);
  Serial.printf("\n");
  Serial.printf(parsed["value"]);
  Serial.printf("\n");
  return parsed;
  }
//tao chuoi json gui len server
void dataSender(const char *client_name, const char *address, const char *value){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& jsonEncoder = jsonBuffer.createObject();
  jsonEncoder["name"] = client_name;
  jsonEncoder["address"] = address;
  jsonEncoder["value"] = value;
  char JSONmessageBuffer[300];
  jsonEncoder.prettyPrintTo(messageJsonEmit, sizeof(messageJsonEmit));
  //Serial.println("Client send data :\n");
  //Serial.println(JSONmessageBuffer);
  }
//function trong socket.on("server-send-data", server_send_data);
void server_send_data(const char *data, size_t length){ 
  if(strcmp(dataReceiver(data)["value"], "on") == 0){
      digitalWrite(ledPin, HIGH);
      dataSender("ESP client", "led", "on"); //tao chuoi json luu tai messageJsonEmit
      socket.emit("client-send-data", messageJsonEmit);
  }
  else{
      digitalWrite(ledPin, LOW);
      dataSender("ESP client", "led", "off");
      socket.emit("client-send-data", messageJsonEmit );
    }
}
//////////////////////////////////////////////////////////
/*
 * Login page
 */

const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);

  // Connect to WiFi network
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
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  ////////////////////////////////////////////////////////////////////////////
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  
  socket.begin(socketServer, socketPort);
  socket.on("server-send-data", server_send_data);
  ////////////////////////////////////////////////////////////////////////////
}

void loop(void) {
  server.handleClient();
  delay(1);
  ////////////////////////////////////////////////////////////////////////////
  socket.loop();
  ////////////////////////////////////////////////////////////////////////////
}
