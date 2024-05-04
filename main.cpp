/* Library */
#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ESP32_OTA.h>
#include <Arduino_ConnectionHandler.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>
/* Library OTA */
#include <WiFi.h>
// #include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
/* Config for web Arduino IOT Cloud*/
const char THING_ID[]           = "18538d68-17a5-479d-a139-631868dce7af";
const char DEVICE_LOGIN_NAME[]  = "0a5191a4-be2f-4ddd-9dc7-565da86d45ac";
const char DEVICE_KEY[]         = "y095zvIZPST6rNdExt#HonPdk";

const char SSID[]               = "QUANGHUY";
const char PASS[]               = "12121213";
/* State of button on web */
#define button_web_state_on     0x1  
#define button_web_state_off    0x0   
/* State of Button */
#define ON        0x0
#define OFF       0x1
/* Hàng và cột để set thông tin */
#define col_param 56
#define row_param 10
/* Define GPIO */
#define Device1   33
#define Device2   32
#define Button1   13
#define Button2   12
#define DHT_pin   16
#define wifiLed   2
#define IR_pin    18
/* Mã hexa nhận được từ điều khiển Remote */
#define IR_SW1        0xF20DFF00
#define IR_SW2        0xE718FF00
#define IR_ON_ALL     0xB847FF00
#define IR_OFF_ALL    0xBA45FF00
/* Cài đặt thông số cho màn hình oled */
#define SCREEN_WIDTH    128   // OLED display width, in pixels
#define SCREEN_HEIGHT   64    // OLED display height, in pixels
#define OLED_RESET      -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHT_pin, DHT11);
IRrecv irrecv(IR_pin);

WebServer server(80);

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

/* Biến lưu trạng thái của các thiết bị */ 
bool device1State = false;
bool device2State = false;
/* Biến lưu giá trị cảm biến  */
float humidity_, temperature_;
/*
  button1, button2: trạng thái hiển thị trên web
  giá trị cảm biến hiển thị trên web
*/
CloudSwitch button1;
CloudSwitch button2;
CloudTemperatureSensor temperature;
CloudPercentage humidity;

const unsigned char logo [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfd, 0xe3, 0x7f, 0xff, 0xfe, 0xff, 0xff, 0xd9, 0xe3, 
	0x6f, 0xff, 0xfe, 0xff, 0xff, 0xdb, 0xff, 0xf3, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0x01, 0xf3, 0x7f, 
	0xfe, 0xff, 0xf3, 0xf0, 0x00, 0x1e, 0x3f, 0xfe, 0xff, 0xf7, 0xdc, 0x00, 0x07, 0xef, 0xfe, 0xff, 
	0xdf, 0x1f, 0x00, 0x01, 0xdf, 0xfe, 0xff, 0xee, 0x1f, 0x80, 0x00, 0xff, 0xfe, 0xff, 0xdc, 0x3f, 
	0xfc, 0x00, 0x73, 0xfe, 0xff, 0x38, 0x7f, 0xff, 0xc0, 0x3d, 0xfe, 0xfe, 0x70, 0x7f, 0x81, 0xf0, 
	0x1d, 0xfe, 0xfd, 0x60, 0xff, 0x00, 0x1c, 0x06, 0xfe, 0xfb, 0xc0, 0xff, 0x00, 0x0c, 0x07, 0x7e, 
	0xff, 0x80, 0xff, 0x00, 0x03, 0x03, 0xbe, 0xff, 0x81, 0xff, 0x00, 0x03, 0x03, 0xbe, 0xf3, 0x03, 
	0xff, 0x00, 0x01, 0x81, 0x9e, 0xff, 0x07, 0x3f, 0x00, 0x00, 0xc1, 0x9e, 0xfa, 0x06, 0x1f, 0x80, 
	0x00, 0xe0, 0x9e, 0xfe, 0x0c, 0x0f, 0xc0, 0x00, 0x60, 0xfe, 0xfe, 0x0c, 0x07, 0xe0, 0x00, 0x60, 
	0xfe, 0xfe, 0x0c, 0x01, 0xf0, 0x00, 0x30, 0xfe, 0xfe, 0x18, 0x00, 0xf0, 0x00, 0x30, 0x7e, 0xfc, 
	0x18, 0x00, 0x78, 0x00, 0x30, 0x7e, 0xfc, 0x18, 0x00, 0x3c, 0x00, 0x30, 0x7e, 0xfc, 0x18, 0x00, 
	0x3e, 0x00, 0x30, 0x7e, 0xf4, 0x19, 0xff, 0xde, 0x00, 0x30, 0x7e, 0xfc, 0x18, 0xfc, 0xfc, 0x00, 
	0x30, 0x7e, 0xfe, 0x18, 0x47, 0xf8, 0x00, 0x30, 0xde, 0xe6, 0x08, 0x3f, 0xf0, 0x00, 0x30, 0xee, 
	0xee, 0x0c, 0x1f, 0xe0, 0x00, 0x60, 0xfe, 0xee, 0x0c, 0x0f, 0x80, 0x00, 0x60, 0xfe, 0xf6, 0x06, 
	0x07, 0x80, 0x00, 0x60, 0xce, 0xff, 0x0f, 0xff, 0x80, 0x00, 0xc1, 0xde, 0xf7, 0x07, 0xff, 0xef, 
	0xff, 0xc1, 0xbe, 0xf9, 0x87, 0xff, 0xff, 0xff, 0xc3, 0xfe, 0xfd, 0x80, 0xc0, 0x38, 0x0f, 0x03, 
	0xfe, 0xfe, 0xc0, 0xe0, 0x10, 0x0e, 0x06, 0xfe, 0xfe, 0xc0, 0x70, 0x00, 0x1c, 0x06, 0xfe, 0xff, 
	0x70, 0x1e, 0x00, 0xf0, 0x1f, 0xfe, 0xff, 0xb8, 0x07, 0xff, 0xe0, 0x1d, 0xfe, 0xff, 0xfc, 0x00, 
	0xfe, 0x00, 0x77, 0xfe, 0xff, 0xfe, 0x00, 0x00, 0x00, 0xf7, 0xfe, 0xff, 0xdf, 0x00, 0x00, 0x01, 
	0xef, 0xfe, 0xff, 0xf7, 0xc0, 0x00, 0x07, 0x1f, 0xfe, 0xff, 0xfe, 0x70, 0x00, 0x1e, 0x1f, 0xfe, 
	0xff, 0xff, 0x3f, 0x01, 0xfd, 0x7f, 0xfe, 0xff, 0xff, 0xe3, 0xff, 0xa7, 0xff, 0xfe, 0xff, 0xff, 
	0xe2, 0x3e, 0x0f, 0xff, 0xfe, 0xff, 0xff, 0xfa, 0x0a, 0x5f, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfe
};

const char* host = "esp32";
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
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
void initProperties();
void GPIO_Init();
void Connected();
void doThisOnSync();
void Disconnected();
void onButton1Change();
void onButton2Change();
void manual_Control();
void display_state();
void display_logo();
void IR_remote_control();
void readSensor_dht11();
void Device_State(bool state);
void Device_oNoFF(uint8_t num);
void OTA_Init();
void OTA_Working();

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(true); // Don't proceed, loop forever
  }
  display.clearDisplay();

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, Connected);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::SYNC, doThisOnSync);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, Disconnected);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  GPIO_Init();
}

void loop() {
  OTA_Working();
  ArduinoCloud.update();

  IR_remote_control();
  manual_Control();
  readSensor_dht11();

  display_state();
  display_logo();

}
void display_logo()
{
  display.drawBitmap(0,0, logo, 55, 55, WHITE);
  display.display();
}
void display_state()
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  // Hiển thị thông tin thiết bị 1
  display.setCursor(col_param, row_param);
  display.print("LAMP 1:");
  if (device1State == true)
  {
    display.setTextColor(WHITE, BLACK);
    display.print(" ON ");
  }
  else
  {
    display.setTextColor(WHITE, BLACK);
    display.print(" OFF");
  }
    
  // Hiển thị thông tin thiết bị 2
  display.setCursor(col_param, row_param+8);
  display.print("LAMP 2:");
  if (device2State == true)
    display.print(" ON ");
  else
    display.print(" OFF");

  // Hiển thị giá trị
  display.setCursor(col_param, row_param+16);
  display.print("Temp:");
  display.setTextColor(WHITE, BLACK);
  display.print(temperature_);
  display.print("oC");

  // Hiển thị giá trị
  display.setCursor(col_param, row_param+24);
  display.print("Humi:");
  display.setTextColor(WHITE, BLACK);
  display.print(humidity_);
  display.print("%");

  // Cập nhật màn hình
  display.display();
}
void readSensor_dht11(){
  humidity_ = dht.readHumidity();
  temperature_ = dht.readTemperature();
  humidity = humidity_;
  temperature = temperature_;
}
/* Chế độ điều khiển bằng nút bấm trên mạch */
void manual_Control(){
  if(digitalRead(Button1) == LOW && device1State == false){
    digitalWrite(Device1, ON);    
    device1State = true;
    button1 = button_web_state_on;
    while(digitalRead(Button1) == LOW){
      delay(20);
    }
  }
  if(digitalRead(Button1) == LOW && device1State == true){
    digitalWrite(Device1, OFF);   
    device1State = false;
    button1 = button_web_state_off;
    while(digitalRead(Button1) == LOW){
      delay(20);
    }
  }

  if(digitalRead(Button2) == LOW && device2State == false){
    digitalWrite(Device2, ON);    // Loai Relay kich muc cao
    device2State = true;
    button2 = button_web_state_on;
    while(digitalRead(Button2) == LOW){
      delay(20);
    }
  }
  if(digitalRead(Button2) == LOW && device2State == true){
    digitalWrite(Device2, OFF);    // Loai Relay kich muc cao
    device2State = false;
    button2 = button_web_state_off;
    while(digitalRead(Button2) == LOW){
      delay(20);
    }
  }
}
/* @brief Hàm set trạng thái cho tất cả thiết bị 
   @param state: Trạng thái được set
*/
void Device_State(bool state){
  if(state){
    digitalWrite(Device1, ON);
    digitalWrite(Device2, ON);
    device1State = true;
    device2State = true;
    button1 = button_web_state_on;
    button2 = button_web_state_on;
  }
  else{
    device1State = false;
    device2State = false;
    digitalWrite(Device1, OFF);
    digitalWrite(Device2, OFF);
    button1 = button_web_state_off;
    button2 = button_web_state_off;
  }
}
/* @brief Hàm set trạng thái cho từng thiết bị 
   @param num: Thiết bị được điều khiển
*/
void Device_oNoFF(uint8_t num){
  switch(num){
    case 1:
      if(device1State == false){
        digitalWrite(Device1, ON);
        device1State = true;
        button1 = button_web_state_on;
      }
      else{
        digitalWrite(Device1, OFF);
        device1State = false;
        button1 = button_web_state_off;
      }
    break;

    case 2:
      if(device2State == false){
        digitalWrite(Device2, ON);
        device2State = true;
        button2 = button_web_state_on;
      }
      else{
        digitalWrite(Device2, OFF);
        device2State = false;
        button2 = button_web_state_off;
      }
    break;

    default:

    break;
  }
}
/* Chế độ điều khiển thiết bị bằng điều khiển remote */
void IR_remote_control(){
  if (irrecv.decode()){
    switch (irrecv.decodedIRData.decodedRawData){
      case IR_OFF_ALL:
        Device_State(ON);
        Serial.println("Turn off all relay by Remote");
      break;

      case IR_ON_ALL:
        Device_State(OFF);
        Serial.println("turn on all relay by Remote");
      break;

      case IR_SW1:
        Device_oNoFF(1);
        if(device1State == true) Serial.println("Device 1 on by Remote");
        else Serial.println("Device 1 off by Remote");
      break;

      case IR_SW2:
        Device_oNoFF(2);
        if(device2State == true) Serial.println("Device 2 on by Remote");
        else Serial.println("Device 2 off by Remote");
      break;

      default:
        break;
    }
    delay(200);
    irrecv.resume();
  }
}

/**
 * Kiểm tra trạng thái thay đổi của nút bấm lên web
*/
void onButton1Change()  {
  // Add your code here to act upon Button1 change
  if(button1 == button_web_state_on){
    digitalWrite(Device1, ON);    
    Serial.println("Device1 ON");
    device1State = true;
  }
  else{
    digitalWrite(Device1, OFF);    
    Serial.println("Device1 OFF");
    device1State = false;
  }
}

void onButton2Change()  {
  // Add your code here to act upon Button2 change
  if(button2 == button_web_state_on){
    digitalWrite(Device2, ON);    // Loai Relay kich muc cao
    Serial.println("Device2 ON");
    device2State = true;
  }
  else{
    digitalWrite(Device2, OFF);    // Loai Relay kich muc cao
    Serial.println("Device2 OFF");
    device2State = false;
  }
}
void GPIO_Init(){

  pinMode(DHT_pin, INPUT);
  dht.begin();

  irrecv.enableIRIn();

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Device1, OUTPUT);
  pinMode(Device2, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  digitalWrite(Device1, OFF);
  digitalWrite(Device2, OFF);
  device1State = false;
  device2State = false;
}
/* Hàm khởi tạo kết nối web  */
void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(button1, READWRITE, ON_CHANGE, onButton1Change);
  ArduinoCloud.addProperty(button2, READWRITE, ON_CHANGE, onButton2Change);
  ArduinoCloud.addProperty(temperature, READ, 1 * SECONDS, NULL);
  ArduinoCloud.addProperty(humidity, READ, 1 * SECONDS, NULL);
}
/*Check for connect wifi*/
void Connected() {
  Serial.println("Board successfully connected to Arduino IoT Cloud");
  OTA_Init();
  digitalWrite(wifiLed, HIGH);
}
void doThisOnSync() {
  Serial.println("Thing Properties synchronised");
}
void Disconnected() {
  Serial.println("Board disconnected from Arduino IoT Cloud");
  digitalWrite(wifiLed, LOW);
}
void OTA_Init(){
  // Connect to WiFi network
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
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
}

void OTA_Working(){
  server.handleClient();
}