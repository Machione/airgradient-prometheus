#include <AirGradient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define WIFI_CONNECT_RETRY_MS 10000          /** ms */
#define DISP_UPDATE_INTERVAL 30000            /** ms */
#define SENSOR_CO2_CALIB_COUNTDOWN_MAX 5     /** sec */
#define SENSOR_CO2_UPDATE_INTERVAL 10000      /** ms */
#define SENSOR_PM_UPDATE_INTERVAL 10000       /** ms */
#define SENSOR_TEMP_HUM_UPDATE_INTERVAL 10000 /** ms */
#define DISPLAY_DELAY_SHOW_CONTENT_MS 2000   /** ms */
const char* ssid = "CHANGEME";
const char* password = "CHANGEME";
const char* deviceId = "CHANGEME";


/**
 * @brief Schedule handle with timing period
 *
 */
class AgSchedule {
public:
  AgSchedule(int period, void (*handler)(void))
      : period(period), handler(handler) {}
  void run(void) {
    uint32_t ms = (uint32_t)(millis() - count);
    if (ms >= period) {
      handler();
      count = millis();
    }
  }

private:
  void (*handler)(void);
  int period;
  int count;
};


/** Create airgradient instance for 'DIY_BASIC' board */
AirGradient ag = AirGradient(DIY_BASIC);
ESP8266WebServer server(9100);

static int co2Ppm = -1;
static int pm25 = -1;
static float temp = -1001;
static int hum = -1;
static long val;
static bool wifiHasConfig = false; /** */

static void boardInit(void);
static void failedHandler(String msg);
static void co2Calibration(void);
static void co2Update(void);
static void pmUpdate(void);
static void tempHumUpdate(void);
static void dispHandler(void);
static void updateWiFiConnect(void);

bool hasSensorS8 = true;
bool hasSensorPMS = true;
bool hasSensorSHT = true;
int pmFailCount = 0;
int getCO2FailCount = 0;

AgSchedule dispSchedule(DISP_UPDATE_INTERVAL, dispHandler);
AgSchedule co2Schedule(SENSOR_CO2_UPDATE_INTERVAL, co2Update);
AgSchedule pmsSchedule(SENSOR_PM_UPDATE_INTERVAL, pmUpdate);
AgSchedule tempHumSchedule(SENSOR_TEMP_HUM_UPDATE_INTERVAL, tempHumUpdate);

void setup() {
  Serial.begin(115200);

  /** Init I2C */
  Wire.begin(ag.getI2cSdaPin(), ag.getI2cSclPin());
  delay(1000);

  /** Board init */
  boardInit();

  /** Show boot display */
  displayShowText("DIY basic", "Lib:" + ag.getVersion(), "");
  delay(2000);

  /** WiFi connect */
  WiFi.mode(WIFI_STA);

  // Configure Hostname
  if ((deviceId != NULL) && (deviceId[0] == '\0')) {
    Serial.println("No Device ID is Defined, Defaulting to board defaults");
  } else {
    wifi_station_set_hostname(deviceId);
    WiFi.setHostname(deviceId);
  }

  // Setup and wait for WiFi.
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    displayShowText("Connecting to ", String(ssid), "");
  }
  wifiHasConfig = true;
  
  server.on("/", HandleRoot);
  server.on("/metrics", HandleRoot);
  server.onNotFound(HandleNotFound);

  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString() + ":9100");
  
  displayShowText("Warming up", deviceId, "");
  delay(5000);
}

void loop() {
  dispSchedule.run();
  if (hasSensorS8) {
    co2Schedule.run();
  }
  if (hasSensorPMS) {
    pmsSchedule.run();
  }
  if (hasSensorSHT) {
    tempHumSchedule.run();
  }

  updateWiFiConnect();

  /** Read PMS on loop */
  ag.pms5003.handle();
  
  server.handleClient();
}

void displayShowText(String ln1, String ln2, String ln3) {
  char buf[9];
  ag.display.clear();

  ag.display.setCursor(1, 1);
  ag.display.setText(ln1);
  ag.display.setCursor(1, 19);
  ag.display.setText(ln2);
  ag.display.setCursor(1, 37);
  ag.display.setText(ln3);

  ag.display.show();
  delay(100);
}


static void boardInit(void) {
  /** Init SHT sensor */
  if (ag.sht.begin(Wire) == false) {
    hasSensorSHT = false;
    Serial.println("SHT sensor not found");
  }

  /** CO2 init */
  if (ag.s8.begin(&Serial) == false) {
    Serial.println("CO2 S8 snsor not found");
    hasSensorS8 = false;
  }

  /** PMS init */
  if (ag.pms5003.begin(&Serial) == false) {
    Serial.println("PMS sensor not found");
    hasSensorPMS = false;
  }

  /** Display init */
  ag.display.begin(Wire);
  ag.display.setTextColor(1);
  ag.display.clear();
  ag.display.show();
  delay(100);
}

static void failedHandler(String msg) {
  while (true) {
    Serial.println(msg);
    delay(1000);
  }
}

static void co2Calibration(void) {
  /** Count down for co2CalibCountdown secs */
  for (int i = 0; i < SENSOR_CO2_CALIB_COUNTDOWN_MAX; i++) {
    displayShowText("CO2 calib", "after",
                    String(SENSOR_CO2_CALIB_COUNTDOWN_MAX - i) + " sec");
    delay(1000);
  }

  if (ag.s8.setBaselineCalibration()) {
    displayShowText("Calib", "success", "");
    delay(1000);
    displayShowText("Wait for", "finish", "...");
    int count = 0;
    while (ag.s8.isBaseLineCalibrationDone() == false) {
      delay(1000);
      count++;
    }
    displayShowText("Finish", "after", String(count) + " sec");
    delay(DISPLAY_DELAY_SHOW_CONTENT_MS);
  } else {
    displayShowText("Calib", "failure!!!", "");
    delay(DISPLAY_DELAY_SHOW_CONTENT_MS);
  }
}

static void co2Update() {
  int value = ag.s8.getCo2();
  if (value >= 0) {
    co2Ppm = value;
    getCO2FailCount = 0;
    Serial.printf("CO2 index: %d\r\n", co2Ppm);
  } else {
    getCO2FailCount++;
    Serial.printf("Get CO2 failed: %d\r\n", getCO2FailCount);
    if (getCO2FailCount >= 3) {
      co2Ppm = -1;
    }
  }
}

void pmUpdate() {
  if (ag.pms5003.isFailed() == false) {
    pm25 = ag.pms5003.getPm25Ae();
    Serial.printf("PMS2.5: %d\r\n", pm25);
    pmFailCount = 0;
  } else {
    Serial.printf("PM read failed, %d", pmFailCount);
    pmFailCount++;
    if (pmFailCount >= 3) {
      pm25 = -1;
    }
  }
}

static void tempHumUpdate() {
  if (ag.sht.measure()) {
    temp = ag.sht.getTemperature();
    hum = ag.sht.getRelativeHumidity();
    Serial.printf("Temperature: %0.2f\r\n", temp);
    Serial.printf("   Humidity: %d\r\n", hum);
  } else {
    Serial.println("Meaure SHT failed");
  }
}

static void dispHandler() {
  String ln1 = "";
  String ln2 = "";
  String ln3 = "";

  if (pm25 < 0) {
    ln1 = "PM :- ug";

  } else {
    ln1 = "PM :" + String(pm25) + " ug";
  }
  
  if (co2Ppm > -1001) {
    ln2 = "CO2:" + String(co2Ppm);
  } else {
    ln2 = "CO2: -";
  }

  String _hum = "-";
  if (hum > 0) {
    _hum = String(hum);
  }

  String _temp = "-";

  if (temp > -1001) {
    _temp = String(temp).substring(0, 4);
  }
  ln3 = _temp + " " + _hum + "%";
  
  displayShowText(ln1, ln2, ln3);
}


/**
 * @brief WiFi reconnect handler
 */
static void updateWiFiConnect(void) {
  static uint32_t lastRetry;
  if (wifiHasConfig == false) {
    return;
  }
  if (WiFi.isConnected()) {
    lastRetry = millis();
    return;
  }
  uint32_t ms = (uint32_t)(millis() - lastRetry);
  if (ms >= WIFI_CONNECT_RETRY_MS) {
    lastRetry = millis();
    WiFi.reconnect();

    Serial.printf("Re-Connect WiFi\r\n");
  }
}


void HandleRoot() {
  server.send(200, "text/plain", GeneratePrometheusMetrics());
}

void HandleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", message);
}


String GeneratePrometheusMetrics() {
  String message = "";
  String idString = "{id=\"" + String(deviceId) + "\",mac=\"" + WiFi.macAddress().c_str() + "\"}";
  
  pmUpdate();
  co2Update();
  tempHumUpdate();
  
  if (co2Ppm > -1001) {
    message += "# HELP pm02 Particulate Matter PM2.5 value, in μg/m3 \n";
    message += "# TYPE pm02 gauge\n";
    message += "pm02";
    message += idString;
    message += String(pm25);
    message += "\n";
  }
  if (co2Ppm > -1001) {
    message += "# HELP rco2 CO2 value, in ppm\n";
    message += "# TYPE rco2 gauge\n";
    message += "rco2";
    message += idString;
    message += String(co2Ppm);
    message += "\n";
  }
  if (temp > -1001) {
    message += "# HELP atmp Temperature, in degrees Celsius\n";
    message += "# TYPE atmp gauge\n";
    message += "atmp";
    message += idString;
    message += String(temp);
    message += "\n";
  }
  if (hum > 0) {
    message += "# HELP rhum Relative humidity, in percent\n";
    message += "# TYPE rhum gauge\n";
    message += "rhum";
    message += idString;
    message += String(hum);
    message += "\n";
  }
  return message;
}