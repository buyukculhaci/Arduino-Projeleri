#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <Servo.h>

/* Put your SSID & Password */
const char* ssid = "Kuzu";  // Enter SSID here
const char* password = "12345678";  //Enter Password here
static const int servoPin = 4;

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

Servo servo1;

String saat = "", sonYemSaat = "", sonrakiYemSaat = "";
int otomatikSelector = 0, bugunVerilenSn = 0;
String yemSaatleri[] = {"", "", "", "", ""};
int yemSureleri[] = {0, 0, 0, 0, 0};

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/yemVerBTN", handle_yemVerBTN);
  server.on("/ayarlar", handle_ayarlar);
  server.on("/ayarlariUygula", handle_ayarlariUygula);
  server.on("/setTime", HTTP_POST, handle_setTime);
  server.onNotFound(handle_NotFound);

  server.begin();

  struct timeval tv;
  tv.tv_sec =   0;
  settimeofday(&tv, NULL);

  time_t now;
  struct tm timeDetails;
  time(&now);
  localtime_r(&now, &timeDetails);
  Serial.println(&timeDetails, "%A, %B %d %Y %H:%M:%S");

  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();

  time_t now;
  struct tm timeDetails;
  time(&now);
  localtime_r(&now, &timeDetails);

//----****----
  char saatC[50] = {0};
  strftime(saatC,sizeof(saatC),"%H:%M", &timeDetails);
  saat = saatC;
//----****----

  if(saat==yemSaatleri[otomatikSelector]){
    servo1.write(60);
    delay(yemSureleri[otomatikSelector]*1000);
    servo1.write(20);
    Serial.println(String(otomatikSelector+1) + ". yem verildi");
    sonYemSaat = saat;
    bugunVerilenSn += yemSureleri[otomatikSelector];
    otomatikSelector += 1;
    if(yemSaatleri[otomatikSelector] == ""){otomatikSelector+=1;}
    if(otomatikSelector == 5){
      otomatikSelector = 0;
      bugunVerilenSn = 0;
    }
    sonrakiYemSaat[otomatikSelector];
  }

}

void layout() {

  //String dene = ctime(&now); //***************************
  Serial.println(saat);
  server.send(200, "text/html", SendHTML(saat, sonYemSaat, sonrakiYemSaat, String(bugunVerilenSn)));
}

void handle_OnConnect() {
  Serial.println("Bağlantı kuruldu");
  layout();
}

void handle_yemVerBTN() {
  Serial.println("Plan dışı yem verildi");
  sonYemSaat = saat;//elle yem verince sonraki yemlemeyi atlayacak mı?
  layout();
}

void handle_ayarlar() {
  Serial.println("--Ayarlar--");
  server.send(200, "text/html", ayarHTML());
}

void handle_ayarlariUygula() {
  for (int i = 0; i < 5; i++) {
    yemSaatleri[i] = server.arg("h" + String(i + 1));
    yemSureleri[i] = server.arg("s" + String(i + 1)).toInt();
  }
  //burada selector ayarlanacak
  otomatikSelector = 0;
  for(int i = 0; i < 5; i++){
    if((saat.substring(0,2).toInt()*60+saat.substring(3,5).toInt()) < (yemSaatleri[i].substring(0,2).toInt()*60 + yemSaatleri[i].substring(3,5).toInt())){
      otomatikSelector = i;
      sonrakiYemSaat = yemSaatleri[otomatikSelector];
      break;
    }
  }
  if(otomatikSelector == 5){otomatikSelector = 0;}
  for(int i = 0; i<6; i++){
    Serial.println(yemSaatleri[i]);
  }
  Serial.print("Selected time: ");
  Serial.println(yemSaatleri[otomatikSelector]);
  layout();
}

void handle_setTime() {
  Serial.println(server.arg("zaman"));
  struct timeval tv;

  timeSetter(server.arg("zaman"));

  time_t now;
  struct tm timeDetails;
  time(&now);
  localtime_r(&now, &timeDetails);

}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
void timeSetter(String t) {
  struct timeval tv;
  tv.tv_sec =   0;

  for (int i = 0; i < t.length(); i++) {
    tv.tv_sec += t.substring(i, i + 1).toInt() * pow(10, (t.length() - i) - 1);
  }
  settimeofday(&tv, NULL);
}

String ayarHTML() {
  String ptr = "<!DOCTYPE html><html lang=\"tr\"><head>    <meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Otomatik yemleme</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "td{text-align: center;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<form action=\"/ayarlariUygula\" method=\"GET\">";
  ptr += "<table style=\"margin: auto; padding-bottom: 10px;\">\n";
  ptr += "<tr><th></th><th>Saat</th><th>Süre (sn)</th></tr>\n";

  for (int i = 1; i < 6; i++) {
    if(yemSaatleri[i-1] != ""){
      ptr += "<tr><td>" + String(i) + ".</td><td><input type=\"time\" id=\"h" + String(i) + "\" name=\"h" + String(i) + "\" value=\"" + yemSaatleri[i-1] + "\" ></td>\n";
    }
    else{
      ptr += "<tr><td>" + String(i) + ".</td><td><input type=\"time\" id=\"h" + String(i) + "\" name=\"h" + String(i) + "\" ></td>\n";
    }
    ptr += "<td><select name=\"s" + String(i) + "\" id=\"s" + String(i) + "\">\n";
    for(int j = 1; j<6; j++){
      ptr += "<option ";
      if(yemSureleri[i-1] == j){
        ptr += "selected value=\""+ String(j) +"\">"+ String(j) +"</option>";
      }
      else{
        ptr += "value=\""+ String(j) +"\">"+ String(j) +"</option>";
      }
    }
    ptr += "</select></td></tr>\n";
  }
  ptr += "</table>\n";  
  ptr += "<button class=\"button button-on\" style=\"width:140px;\" type=\"submit\">Kaydet</button>";

  ptr += "</body>\n";
  ptr += "</html>\n";

  return ptr;
}
String SendHTML(String saat, String sonYem, String sonrakiYem, String bugunKacSn) {
  String ptr = "<!DOCTYPE html><html lang=\"tr\"><head>    <meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "    <title>Cumartesi Tarım</title>\n";
  ptr += "    <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "    .button {display: block;width: 100px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += "    .button-on {background-color: #3498db;} .button-on:active {background-color: #2980b9;} .button-off {background-color: #34495e;} .button-off:active {background-color: #2c3e50;}\n";
  ptr += "    td{text-align: center;}\n";
  ptr += "    p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "    ul{list-style-type: none;padding: 0;}\n";
  ptr += "    </style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "    <h1>Cumartesi Tarım</h1>\n";
  ptr += "    <h3>Otomatik yemleme</h3>\n";
  ptr += "    <p>Bugün " + bugunKacSn + " sn yem verildi</p><a class=\"button button-on\" href=\"/yemVerBTN\">Yem ver</a>\n";
  ptr += "    <ul><li><h1>" + saat + "</h1></li><li>Son yemleme: <b>" + sonYem + "</b></li><li>Sonraki yemleme: <b>" + sonrakiYem + "</b></li><li></li></ul>\n";
  ptr += "    <a class=\"button button-off\" href=\"/ayarlar\">Ayarlar</a>\n";
  ptr += "</body>\n";
  ptr += "<script>window.onload = function () {var simdi = Math.floor(new Date().getTime()/1000.0+10800); console.log(simdi); let request = new XMLHttpRequest(); var params = 'zaman='+simdi; url = 'setTime?'+params; request.open('POST', url, true); request.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); request.send(params); }</script>\n";
  ptr += "</html>\n";
  return ptr;
}
