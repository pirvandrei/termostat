/* Set these to your desired softAP credentials. They are not configurable at runtime */
const char *softAP_ssid;
const char *softAP_password = "";
extern "C" {
#include "user_interface.h"
}

// DNS server
const byte DNS_PORT = 53;
char *myHostname;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;
#include <FS.h>
/** Last time I tried to connect to WLAN */
long lastConnectTry = 0;

/** Current WLAN status */
int status = WL_IDLE_STATUS;

/** Handle root or redirect to captive portal */
void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin ( ssid, password );
 /* int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );*/
}
void loopWifi()
{
  webSocket.loop();
  if (connect) {
    Serial.println ( "Connect requested" );
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000) ) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
    //  connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print ( "Status: " );
      Serial.println ( s );
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( ssid );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
          MDNS.addService("ws", "tcp", 81);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}

void setupWifi()
{
  SPIFFS.begin();

  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);


  String tmpId = name + getUniqueId();
  myHostname = (char*)malloc(tmpId.length() + 1);
  tmpId.toCharArray(myHostname, tmpId.length() + 1);
  WiFi.softAP(myHostname, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Wifi name: ");
  Serial.println(myHostname);
  Serial.print("Hostname: ");
  Serial.print(myHostname);
  Serial.println(".local");

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/css", handleCSS);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID

  //Websocket setup


  //Serial.setDebugOutput(true);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void handleCSS()
{
  server.sendHeader("Cache-Control",  "public, max-age=31536000");
  // server.sendHeader("Expires" , "Wed, 02 Dec 3015 21:37:50 GMT");
  //server.sendHeader("Pragma", "cache");
  handleFileRead("/mini.css");

}

void sendScript(String _server)
{
  server.sendContent(
    "\
<script language = 'javascript' type = 'text/javascript'>\
var wsUri = 'ws://" + _server + ":81/';\
  var output;\
\
  function init() {\
      doWebSocket();\
  }\
\
  function doWebSocket() {\
      websocket = new WebSocket(wsUri);\
      websocket.onopen = function(evt) {\
          console.log('CONNECTED');\
      };\
      websocket.onclose = function(evt) {\
          console.log('DISCONNECTED');\
          doWebSocket();\
      };\
      websocket.onmessage = function(evt) {\
        console.log('Got message:' + evt.data)\
      };\
      websocket.onerror = function(evt) {\
          console.log('ERROR: ' + evt.data);\
      };\
  }\
\
  window.addEventListener('load', init, false);\
    </script>\
  ");


}

void sendTop(String _name)
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<html> <head><style>body{ font-family: -apple-system, BlinkMacSystemFont,'Segoe UI','Roboto', 'Droid Sans','Helvetica Neue', Helvetica, Arial, sans-serif;line-height: 1.5; background-color:black; color:white} a{color:white} a.button{display: inline-block;background: rgba(208,208,208,0.75);color: #212121;border: 0;border-radius: 2px;padding: 0.5rem 0.75rem;margin: .5rem;text-decoration: none;transition: background 0.3s;cursor: pointer;text-shadow: 0 0 black;}</style></head><body>");
  server.sendContent("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  server.sendContent("<header class=\"sticky\"><a href=\"#\" class=\"logo\" style=\"margin-left: 1rem;\">" + _name + "</a></header>");
  server.sendContent("<div class=\"container\"><div class=\"row\"><div class=\"col-sm\">");
}

void sendBottom()
{

  server.sendContent("</div></div></div>");
  server.sendContent("</body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}

void sendButton(String _name, String _cmd)
{
  server.sendContent("<a href='#' onClick='websocket.send(\"" + _cmd + "\"); return false'; class='button'>" + _name  + "</a>");
}

void sendSlider(String _name, int _id)
{//
  server.sendContent(String("<div>" +_name + ":<br>" +
   "<input type='range' onChange='websocket.send(\"S" + _id +  "\"+this.value);' id='" + _name + "' min='0' value='" + sliderValues[_id] + "' max='255'></div>" +

   "<script language = 'javascript' type = 'text/javascript'>document.getElementById('"+_name+"').addEventListener('input',  function(){websocket.send(\"S" + _id +  "\"+this.value)}, false);</script>"));
  
}

void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }

  sendTop("ioGlow: Fablab RUC");
  if (server.client().localIP() == apIP)
  {
    sendScript(IpAddress2String(WiFi.softAPIP()));
  }
  else
  {
    sendScript(IpAddress2String(WiFi.localIP()));
  }
  if (server.arg("pattern").length() > 0)
  {
    server.sendContent("<h1>You picked:" + server.arg("pattern") + "</h1>");
  }

  server.sendContent("<h1>Choose your pattern:</h1>" );
  for (int i = 0; i < lightPatternsAssigned; i++)
  {
    sendButton(lightPatternNames[i], String("B") + i);
  }
  sendSlider("Param0",0);
  sendSlider("Param1",1);
  sendSlider("Param2",2);

  
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
  } else {
    server.sendContent(String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  server.sendContent("<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>");
  //server.sendContent("<p><a onClick='window.close();' class='button' href='#'>Close Window</a></p>");
  sendBottom();
}
//captive.apple.com
/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && !(server.hostHeader().equalsIgnoreCase((String(myHostname) + ".local")))) {
    Serial.println("Request redirected to captive portal");
    Serial.println(":" + server.hostHeader() + ":");
    Serial.println(":" + String(myHostname) + ".local");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent(
    "<html><head></head><body>"
    "<h1>Wifi config</h1>"
  );
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
  } else {
    server.sendContent(String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  server.sendContent(
    "\r\n<br />"
    "<table><tr><th align='left'>SoftAP config</th></tr>"
  );
  server.sendContent(String() + "<tr><td>SSID " + String(softAP_ssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.softAPIP()) + "</td></tr>");
  server.sendContent(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN config</th></tr>"
  );
  server.sendContent(String() + "<tr><td>SSID " + String(ssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.localIP()) + "</td></tr>");
  server.sendContent(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
  );
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      server.sendContent(String() + "\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *") + " (" + WiFi.RSSI(i) + ")</td></tr>");
    }
  } else {
    server.sendContent(String() + "<tr><td>No WLAN found</td></tr>");
  }
  server.sendContent(
    "</table>"
    "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
    "<input type='text' placeholder='network' name='n'/>"
    "<br /><input type='password' placeholder='password' name='p'/>"
    "<br /><input type='submit' value='Connect/Disconnect'/></form>"
    "<p>You may want to <a href='/'>return to the home page</a>.</p>"
    "</body></html>"
  );
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    Serial.println ("done");
    return true;

  }
  server.send ( 404, "text/plain", "Could not send file" );
  Serial.println ("error");
  return false;
}


String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}


String getUniqueId()
{
  uint8_t MAC_array[6];
  String tmpId = "";
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    tmpId = tmpId + (char)(MAC_array[i] % 23 + 97);
  }
  return tmpId;
}

//based on https://github.com/Links2004/arduinoWebSockets/tree/master/examples
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Websocket Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Websocket Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, " Connected");
      }
      break;
    case WStype_TEXT:
      //Serial.printf("[%u] websocket get Text: %s\n", num, payload);

      if (payload[0] == 'B')
      {
        lightPatternSelected = String((char *)payload).substring(1).toInt();
        Serial.println(String("pattern selected:") + lightPatternSelected);
      }
      if (payload[0] == 'S')
      {
        sliderValues[String((char *)payload).substring(1,2).toInt()] = String((char *)payload).substring(2).toInt();
       // Serial.println(String("slider value:") + sliderValues[String((char *)payload).substring(1,1).toInt()]);
       // Serial.println(String("heap") + system_get_free_heap_size());
      }



      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[%u] websocket get binary length: %u\n", num, length);
      hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }

}

