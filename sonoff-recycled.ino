#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP_Mail_Client.h>

// ------------------ PIN ------------------
#define LED_RED      13
#define LED_GREEN    14
#define BUTTON_PIN   0

// ------------------ EEPROM ------------------
#define EEPROM_SIZE 1024
#define EEPROM_FLAG 1000

// ------------------ Config ------------------
char ssid[32]          = "";
char wifiPassword[32]  = "";

char smtpHost[64]      = "smtp.gmail.com";
char smtpPort[8]       = "465";
char emailSender[64]   = "";
char emailPassword[64] = "";
char emailTo[64]       = "";

char deviceLocation[96] = "";

char emailSubject[80] = "Pulsante emergenza pressato";
char emailBody[220]   = "Qualcuno ha pressato sul bottone emergenza del dispositivo numero 001 che si trova:";

char repeatEnabled[4] = "0";   // 0 = no, 1 = sì
char repeatMinMin[8]  = "5";
char repeatMaxMin[8]  = "10";

// ------------------ Web portal ------------------
ESP8266WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// IP fisso del portale quando il WiFi non si connette
IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

SMTPSession smtp;

// ------------------ Stati ------------------
bool emergencyActive = false;
bool lastButtonState = HIGH;

unsigned long lastButtonTime = 0;
unsigned long lastEmailTime = 0;
unsigned long nextRepeatDelay = 0;

const unsigned long buttonDebounce = 500;

// ------------------ Utility ------------------
String htmlEscape(String s) {
  s.replace("&", "&amp;");
  s.replace("\"", "&quot;");
  s.replace("<", "&lt;");
  s.replace(">", "&gt;");
  return s;
}

void blinkGreen3Times() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GREEN, LOW);
    delay(200);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
  }
}

String getDeviceIpText() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return WiFi.softAPIP().toString();
}

String getDeviceWebAddress() {
  return String("http://") + getDeviceIpText() + "/";
}

// ------------------ EEPROM ------------------
void saveConfig() {
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.put(0, ssid);
  EEPROM.put(32, wifiPassword);

  EEPROM.put(64, smtpHost);
  EEPROM.put(128, smtpPort);
  EEPROM.put(136, emailSender);
  EEPROM.put(200, emailPassword);
  EEPROM.put(264, emailTo);
  EEPROM.put(328, deviceLocation);
  EEPROM.put(424, emailSubject);
  EEPROM.put(504, emailBody);
  EEPROM.put(724, repeatEnabled);
  EEPROM.put(728, repeatMinMin);
  EEPROM.put(736, repeatMaxMin);

  byte flag = 0xAA;
  EEPROM.put(EEPROM_FLAG, flag);

  EEPROM.commit();
}

bool loadConfig() {
  EEPROM.begin(EEPROM_SIZE);

  byte flag = 0;
  EEPROM.get(EEPROM_FLAG, flag);

  if (flag != 0xAA) {
    strcpy(ssid, "");
    strcpy(wifiPassword, "");

    strcpy(smtpHost, "smtp.gmail.com");
    strcpy(smtpPort, "465");
    strcpy(emailSender, "");
    strcpy(emailPassword, "");
    strcpy(emailTo, "");
    strcpy(deviceLocation, "");

    strcpy(emailSubject, "Pulsante emergenza pressato");
    strcpy(emailBody, "Qualcuno ha pressato sul bottone emergenza del dispositivo numero 001 che si trova:");

    strcpy(repeatEnabled, "0");
    strcpy(repeatMinMin, "5");
    strcpy(repeatMaxMin, "10");

    return false;
  }

  EEPROM.get(0, ssid);
  EEPROM.get(32, wifiPassword);

  EEPROM.get(64, smtpHost);
  EEPROM.get(128, smtpPort);
  EEPROM.get(136, emailSender);
  EEPROM.get(200, emailPassword);
  EEPROM.get(264, emailTo);
  EEPROM.get(328, deviceLocation);
  EEPROM.get(424, emailSubject);
  EEPROM.get(504, emailBody);
  EEPROM.get(724, repeatEnabled);
  EEPROM.get(728, repeatMinMin);
  EEPROM.get(736, repeatMaxMin);

  return true;
}

void resetConfig() {
  EEPROM.begin(EEPROM_SIZE);
  byte flag = 0;
  EEPROM.put(EEPROM_FLAG, flag);
  EEPROM.commit();
  delay(500);
  ESP.restart();
}

// ------------------ EMAIL ------------------
bool sendEmergencyEmail() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connesso, email non inviata");
    return false;
  }

  if (strlen(emailSender) == 0 || strlen(emailPassword) == 0 || strlen(emailTo) == 0) {
    Serial.println("Configurazione email incompleta");
    return false;
  }

  ESP_Mail_Session session;

  session.server.host_name = smtpHost;
  session.server.port = atoi(smtpPort);
  session.login.email = emailSender;
  session.login.password = emailPassword;
  session.login.user_domain = "";

  SMTP_Message message;

  message.sender.name = "Dispositivo Emergenza 001";
  message.sender.email = emailSender;
  message.subject = emailSubject;

  message.addRecipient("Destinatario", emailTo);

  String finalBody = String(emailBody);
  finalBody += "\n\nLuogo dispositivo: ";
  finalBody += String(deviceLocation);
  finalBody += "\n\nDispositivo numero: 001";
  finalBody += "\nIP dispositivo: ";
  finalBody += getDeviceIpText();
  finalBody += "\nPagina configurazione: ";
  finalBody += getDeviceWebAddress();

  message.text.content = finalBody.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  smtp.debug(1);

  if (!smtp.connect(&session)) {
    Serial.println("Errore connessione SMTP");
    return false;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.print("Errore invio email: ");
    Serial.println(smtp.errorReason());
    smtp.closeSession();
    return false;
  }

  smtp.closeSession();

  Serial.println("Email emergenza inviata");
  return true;
}

bool sendIpEmail() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connesso, email IP non inviata");
    return false;
  }

  if (strlen(emailSender) == 0 || strlen(emailPassword) == 0 || strlen(emailTo) == 0) {
    Serial.println("Configurazione email incompleta, email IP non inviata");
    return false;
  }

  ESP_Mail_Session session;

  session.server.host_name = smtpHost;
  session.server.port = atoi(smtpPort);
  session.login.email = emailSender;
  session.login.password = emailPassword;
  session.login.user_domain = "";

  SMTP_Message message;

  message.sender.name = "Dispositivo Emergenza 001";
  message.sender.email = emailSender;
  message.subject = "IP dispositivo emergenza";

  message.addRecipient("Destinatario", emailTo);

  String body = "Il dispositivo emergenza si e' connesso al WiFi.";
  body += "\n\nSSID: ";
  body += String(ssid);
  body += "\nIP dispositivo: ";
  body += getDeviceIpText();
  body += "\nPagina configurazione: ";
  body += getDeviceWebAddress();
  body += "\n\nApri questo indirizzo da un telefono o PC connesso alla stessa rete WiFi.";

  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  smtp.debug(1);

  if (!smtp.connect(&session)) {
    Serial.println("Errore connessione SMTP per email IP");
    return false;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.print("Errore invio email IP: ");
    Serial.println(smtp.errorReason());
    smtp.closeSession();
    return false;
  }

  smtp.closeSession();

  Serial.println("Email con IP inviata");
  return true;
}

void scheduleNextRepeat() {
  int minM = atoi(repeatMinMin);
  int maxM = atoi(repeatMaxMin);

  if (minM < 1) minM = 1;
  if (maxM < minM) maxM = minM;

  long randomMinutes = random(minM, maxM + 1);
  nextRepeatDelay = randomMinutes * 60000UL;

  Serial.print("Prossima email tra minuti: ");
  Serial.println(randomMinutes);
}

void triggerEmergency() {
  emergencyActive = true;

  blinkGreen3Times();

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  }

  sendEmergencyEmail();

  lastEmailTime = millis();

  if (String(repeatEnabled) == "1") {
    scheduleNextRepeat();
  }
}

// ------------------ WEB SERVER ------------------
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Emergency Email Config</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";

  html += "<style>";
  html += "body{background:#000;color:#fff;font-family:Arial;margin:0;padding:20px;}";
  html += "h2,h3{color:#FFA500;text-align:center;}";
  html += "form{display:flex;flex-direction:column;width:100%;max-width:500px;margin:auto;}";
  html += "input,textarea,select{margin:8px 0;padding:12px;border-radius:5px;border:none;font-size:16px;}";
  html += "textarea{min-height:90px;}";
  html += "input[type=submit],button{margin-top:12px;padding:12px;border:none;border-radius:5px;background:#FFA500;color:#000;font-weight:bold;cursor:pointer;font-size:16px;}";
  html += "a{color:#FFA500;text-align:center;display:block;margin-top:20px;}";
  html += "</style></head><body>";

  html += "<h2>Emergency Email Config</h2>";
  html += "<div style='max-width:500px;margin:10px auto;padding:12px;border:1px solid #FFA500;border-radius:5px;'>";
  html += "<b>Stato rete:</b> ";
  html += (WiFi.status() == WL_CONNECTED ? "Connesso al WiFi" : "Modalita AP configurazione");
  if (WiFi.status() != WL_CONNECTED) {
    html += "<br><b>Rete da cercare:</b> EmergencyEmail001";
    html += "<br><b>Indirizzo fisso AP:</b> http://192.168.4.1/";
  }
  html += "<br><b>IP:</b> " + getDeviceIpText();
  html += "<br><b>Pagina:</b> <a style='display:inline;margin:0;' href='" + getDeviceWebAddress() + "'>" + getDeviceWebAddress() + "</a>";
  html += "</div>";

  html += "<form action='/save' method='POST'>";

  html += "<h3>WiFi</h3>";
  html += "<input type='text' name='ssid' placeholder='WiFi SSID' value='" + htmlEscape(String(ssid)) + "'>";
  html += "<input type='password' name='wifiPassword' placeholder='WiFi Password' value='" + htmlEscape(String(wifiPassword)) + "'>";

  html += "<h3>Email</h3>";
  html += "<input type='text' name='deviceLocation' placeholder='Luogo dove si trova il dispositivo' value='" + htmlEscape(String(deviceLocation)) + "'>";

  html += "<input type='text' name='smtpHost' placeholder='SMTP Host es. smtp.gmail.com' value='" + htmlEscape(String(smtpHost)) + "'>";
  html += "<input type='text' name='smtpPort' placeholder='SMTP Port es. 465' value='" + htmlEscape(String(smtpPort)) + "'>";

  html += "<input type='text' name='emailSender' placeholder='Email mittente' value='" + htmlEscape(String(emailSender)) + "'>";
  html += "<input type='password' name='emailPassword' placeholder='Password email o App Password' value='" + htmlEscape(String(emailPassword)) + "'>";
  html += "<input type='text' name='emailTo' placeholder='Email destinatario' value='" + htmlEscape(String(emailTo)) + "'>";

  html += "<input type='text' name='emailSubject' placeholder='Oggetto email' value='" + htmlEscape(String(emailSubject)) + "'>";
  html += "<textarea name='emailBody' placeholder='Messaggio email'>" + htmlEscape(String(emailBody)) + "</textarea>";

  html += "<h3>Ripetizione email</h3>";
  html += "<select name='repeatEnabled'>";

  html += "<option value='0'";
  if (String(repeatEnabled) == "0") html += " selected";
  html += ">Non ripetere</option>";

  html += "<option value='1'";
  if (String(repeatEnabled) == "1") html += " selected";
  html += ">Ripeti email</option>";

  html += "</select>";

  html += "<input type='text' name='repeatMinMin' placeholder='Minimo minuti' value='" + htmlEscape(String(repeatMinMin)) + "'>";
  html += "<input type='text' name='repeatMaxMin' placeholder='Massimo minuti' value='" + htmlEscape(String(repeatMaxMin)) + "'>";

  html += "<input type='submit' value='Salva e riavvia'>";
  html += "</form>";

  html += "<a href='/reset'>Reset impostazioni dispositivo</a>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleStatus() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Stato dispositivo</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{background:#000;color:#fff;font-family:Arial;text-align:center;padding:30px;}h2{color:#FFA500;}a{color:#FFA500;}</style>";
  html += "</head><body>";
  html += "<h2>Stato dispositivo</h2>";
  html += "<p><b>Stato WiFi:</b> ";
  html += (WiFi.status() == WL_CONNECTED ? "Connesso" : "Non connesso / AP Mode");
  if (WiFi.status() != WL_CONNECTED) {
    html += "</p><p><b>Rete WiFi ESP:</b> EmergencyEmail001</p>";
    html += "<p><b>Indirizzo AP:</b> http://192.168.4.1/</p>";
  }
  html += "</p><p><b>IP:</b> " + getDeviceIpText() + "</p>";
  html += "<p><b>Pagina configurazione:</b><br><a href='" + getDeviceWebAddress() + "'>" + getDeviceWebAddress() + "</a></p>";
  html += "<p><a href='/'>Torna alla configurazione</a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("ssid")) server.arg("ssid").toCharArray(ssid, 32);
  if (server.hasArg("wifiPassword")) server.arg("wifiPassword").toCharArray(wifiPassword, 32);

  if (server.hasArg("smtpHost")) server.arg("smtpHost").toCharArray(smtpHost, 64);
  if (server.hasArg("smtpPort")) server.arg("smtpPort").toCharArray(smtpPort, 8);
  if (server.hasArg("emailSender")) server.arg("emailSender").toCharArray(emailSender, 64);
  if (server.hasArg("emailPassword")) server.arg("emailPassword").toCharArray(emailPassword, 64);
  if (server.hasArg("emailTo")) server.arg("emailTo").toCharArray(emailTo, 64);
  if (server.hasArg("deviceLocation")) server.arg("deviceLocation").toCharArray(deviceLocation, 96);
  if (server.hasArg("emailSubject")) server.arg("emailSubject").toCharArray(emailSubject, 80);
  if (server.hasArg("emailBody")) server.arg("emailBody").toCharArray(emailBody, 220);

  if (server.hasArg("repeatEnabled")) server.arg("repeatEnabled").toCharArray(repeatEnabled, 4);
  if (server.hasArg("repeatMinMin")) server.arg("repeatMinMin").toCharArray(repeatMinMin, 8);
  if (server.hasArg("repeatMaxMin")) server.arg("repeatMaxMin").toCharArray(repeatMaxMin, 8);

  saveConfig();

  server.send(200, "text/html", "Salvato! Riavvio ESP...");
  delay(2000);
  ESP.restart();
}

void handleResetPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{background:#000;color:#fff;font-family:Arial;text-align:center;padding:30px;}a,button{padding:15px;margin:10px;background:#FFA500;color:#000;border:none;border-radius:5px;font-weight:bold;text-decoration:none;display:inline-block;}</style>";
  html += "</head><body>";
  html += "<h2>Reset impostazioni</h2>";
  html += "<p>Se confermi, tutte le impostazioni WiFi ed email verranno cancellate.</p>";
  html += "<form action='/doreset' method='POST'>";
  html += "<button type='submit'>Conferma reset</button>";
  html += "</form>";
  html += "<a href='/'>Annulla</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleDoReset() {
  server.send(200, "text/html", "Reset impostazioni in corso...");
  delay(1000);
  resetConfig();
}

void handleNotFound() {
  if (WiFi.getMode() == WIFI_AP) {
    server.sendHeader("Location", getDeviceWebAddress(), true);
    server.send(302, "text/plain", "");
  } else {
    server.send(404, "text/plain", "Pagina non trovata. Apri: " + getDeviceWebAddress());
  }
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reset", HTTP_GET, handleResetPage);
  server.on("/doreset", HTTP_POST, handleDoReset);
  server.onNotFound(handleNotFound);
  server.begin();
}

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  loadConfig();

  randomSeed(ESP.getChipId() + millis());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifiPassword);

  int wifiCounter = 0;

  while (WiFi.status() != WL_CONNECTED && wifiCounter < 20) {
    delay(500);
    wifiCounter++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Se non riesce a collegarsi al WiFi salvato, apre una rete propria.
    // Collegati alla rete "EmergencyEmail001" e apri http://192.168.4.1/
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    WiFi.softAP("EmergencyEmail001");

    dnsServer.start(DNS_PORT, "*", apIP);
    startWebServer();

    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);

    Serial.println("WiFi non connesso: AP Mode - Config portal");
    Serial.println("Connettiti alla rete WiFi: EmergencyEmail001");
    Serial.println("Apri nel browser: http://192.168.4.1/");
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    startWebServer();

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);

    Serial.println("WiFi connesso");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Pagina configurazione: ");
    Serial.println(getDeviceWebAddress());

    sendIpEmail();
  }
}

// ------------------ LOOP ------------------
void loop() {
  server.handleClient();

  if (WiFi.getMode() == WIFI_AP) {
    dnsServer.processNextRequest();
  }

  unsigned long now = millis();

  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    if (now - lastButtonTime > buttonDebounce) {
      Serial.println("Pulsante emergenza premuto");
      triggerEmergency();
      lastButtonTime = now;
    }
  }

  lastButtonState = buttonState;

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  } else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  }

  if (emergencyActive && String(repeatEnabled) == "1") {
    if (nextRepeatDelay > 0 && now - lastEmailTime >= nextRepeatDelay) {
      Serial.println("Ripetizione email emergenza");
      sendEmergencyEmail();
      lastEmailTime = now;
      scheduleNextRepeat();
    }
  }
}
