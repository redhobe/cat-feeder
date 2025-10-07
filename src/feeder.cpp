#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FTPServer.h>
#include <ArduinoJSON.h>
#include <Wire.h>
#include <RtcPCF8563.h>
#include "pitches.h"
#include "ota.h"

// Инициализируем FTP сервер с littleFS
FTPServer ftpSrv(LittleFS);

// Инициализируем работу с PCF8563 по I2C
RtcPCF8563<TwoWire> Rtc(Wire);

const char *ssid = "Home";
const char *password = "89189569599";

// Инициализируем AsyncWebServer с портом 80
AsyncWebServer server(80);

// Инициализируем источник SSE на /events
AsyncEventSource events("/events");

// GPIO
const uint8_t ledIO = 5;
const uint8_t motorClockwiseIO = 18;
const uint8_t motorAntiClockwiseIO = 17;
const uint8_t sleepIO = 19;
const uint8_t sensorsIO = 33;
const uint8_t motorSensorIO = 27;
const uint8_t feedSensorIO = 14;
const uint8_t speakerIO = 16;
const uint8_t sdaIO = 15;
const uint8_t sclIO = 4;
const uint8_t feedButtonIO = 34;
const uint8_t pairButtonIO = 0;

uint8_t feedButtonState = HIGH;
uint8_t feedSensorState = HIGH;
uint8_t motorSensorPrevState = LOW;

bool isEngineOn = false;
bool isFoodPouredOut = false;
uint8_t motorRotationCount = 0;
uint8_t maxMotorRotations = 1;
uint8_t defaultPortionsCount = 1;

unsigned long lastTime = 0;
unsigned long timerDelaySchedule = 60000;

JsonDocument config;
JsonArray schedule;

void logger(const char *message)
{
  events.send(message, NULL, millis());
  Serial.print(message);
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  logger("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  logger(WiFi.localIP().toString().c_str());
}

void setTimeFromNTP()
{
  const int GMT_OFFSET_SEC = 3 * 60 * 60;
  const int DAYLIGHT_OFFSET_SEC = 0;

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    RtcDateTime ntpdt = RtcDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Rtc.SetDateTime(ntpdt);
  }
}

void printDateTime(const RtcDateTime &dt)
{
  char datestring[26];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Day(),
             dt.Month(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  events.send(datestring, NULL, millis());
}

char *printTime(const RtcDateTime &dt)
{
  static char datestring[6]; // Use static to avoid memory allocation on each call

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u"),
             dt.Hour(),
             dt.Minute());
  return datestring; // Return the pointer to the formatted string
}

bool wasError(const char *errorTopic = "")
{
  uint8_t error = Rtc.LastError();
  if (error != 0)
  {
    Serial.print("[");
    Serial.print(errorTopic);
    Serial.print("] WIRE communications error (");
    Serial.print(error);
    Serial.print(") : ");

    switch (error)
    {
    case Rtc_Wire_Error_None:
      Serial.println("(none?!)");
      break;
    case Rtc_Wire_Error_TxBufferOverflow:
      Serial.println("transmit buffer overflow");
      break;
    case Rtc_Wire_Error_NoAddressableDevice:
      Serial.println("no device responded");
      break;
    case Rtc_Wire_Error_UnsupportedRequest:
      Serial.println("device doesn't support request");
      break;
    case Rtc_Wire_Error_Unspecific:
      Serial.println("unspecified error");
      break;
    case Rtc_Wire_Error_CommunicationTimeout:
      Serial.println("communications timed out");
      break;
    }
    return true;
  }
  return false;
}

void engineOn(uint8_t rotations = defaultPortionsCount)
{
  isEngineOn = true;
  motorSensorPrevState = LOW;
  isFoodPouredOut = false;
  motorRotationCount = 0;
  maxMotorRotations = rotations;
  digitalWrite(sensorsIO, HIGH);
  digitalWrite(motorClockwiseIO, HIGH);
  digitalWrite(sleepIO, HIGH);
}

void engineOff()
{
  isEngineOn = false;
  digitalWrite(sensorsIO, LOW);
  digitalWrite(motorAntiClockwiseIO, LOW);
  digitalWrite(motorClockwiseIO, LOW);
  digitalWrite(sleepIO, LOW);
}
void notFound(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    request->send(404, "application/json", "{\"message\":\"Not found\"}");
  }
}

ArRequestHandlerFunction timeRequestHandler = [](AsyncWebServerRequest *request)
{
  RtcDateTime now = Rtc.GetDateTime();
  char *nowTime = printTime(now);

  request->send(200, "text/plain", nowTime);
};

ArRequestHandlerFunction feedRequestHandler = [](AsyncWebServerRequest *request)
{
  if (!request->hasParam("portions"))
  {
    request->send(500);
  }

  AsyncWebParameter *state = request->getParam("portions");

  engineOn(state->value().toInt());
  request->send(200);
};

ArRequestHandlerFunction ledRequestHandler = [](AsyncWebServerRequest *request)
{
  if (!request->hasParam("state"))
  {
    request->send(500);
  }

  AsyncWebParameter *state = request->getParam("state");

  state->value().equals("on") ? digitalWrite(ledIO, HIGH) : digitalWrite(ledIO, LOW);
  request->send(200);
};

ArRequestHandlerFunction getConfigRequestHandler = [](AsyncWebServerRequest *request)
{
  String output;
  serializeJson(config, output);
  request->send(200, "application/json", output);
};

bool saveConfig(const JsonVariantConst json)
{
  // Открываем файл для записи
  File dataFile = LittleFS.open("/config.json", "w");
  if (!dataFile)
  {
    logger("Failed to open data file for writing");
    return false;
  }

  // Сериализуем JSON в файл
  if (serializeJson(json, dataFile) == 0)
  {
    logger("Failed to serialize JSON to file");
    dataFile.close();
    return false;
  }

  // Закрываем файл
  dataFile.close();
  return true;
}

bool loadConfig()
{
  File dataFile = LittleFS.open("/config.json", "r");
  if (!dataFile)
  {
    logger("Failed to open data file");
    return false;
  }

  size_t size = dataFile.size();
  if (size > 1024)
  {
    logger("Data file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  dataFile.readBytes(buf.get(), size);

  auto error = deserializeJson(config, buf.get());
  if (error)
  {
    logger("Failed to parse config file");
    return false;
  }

  defaultPortionsCount = config["manualPortions"].as<uint8_t>();
  schedule = config["schedule"].as<JsonArray>();

  return true;
}

ArRequestHandlerFunction reloadConfigRequestHandler = [](AsyncWebServerRequest *request)
{
  loadConfig();
  request->send(200);
};

ArRequestHandlerFunction putConfigRequestHandler = [](AsyncWebServerRequest *request)
{
  // Проверяем наличие параметра "body"
  if (request->hasParam("config"))
  {
    AsyncWebParameter *state = request->getParam("config");
    String bodyValue = state->value(); // Получаем значение параметра

    // Создаем JSON-документ
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, bodyValue);

    // Проверяем успешность десериализации
    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      request->send(400, "text/plain", "Invalid JSON");
      return;
    }

    // Преобразуем в JsonVariantConst
    JsonVariantConst jsonVariant = doc.as<JsonVariantConst>();

    // Передаем JsonVariantConst в функцию saveConfig
    saveConfig(jsonVariant);
    loadConfig();
    // Отправляем успешный ответ
    request->send(200, "text/plain", "Config saved");
  }
  else
  {
    request->send(400, "text/plain", "Parameter 'body' not found");
  }
};

void setup()
{
  Serial.begin(115200);

  pinMode(ledIO, OUTPUT);
  pinMode(motorClockwiseIO, OUTPUT);
  pinMode(motorAntiClockwiseIO, OUTPUT);
  pinMode(sleepIO, OUTPUT);
  pinMode(sensorsIO, OUTPUT);
  pinMode(motorSensorIO, INPUT);
  pinMode(feedSensorIO, INPUT);
  pinMode(feedButtonIO, INPUT);

  initWiFi();
  Wire.begin(sdaIO, sclIO);
  Rtc.Begin();

  bool isFSok = LittleFS.begin();
  Serial.printf_P(PSTR("FS init: %s\n"), isFSok ? PSTR("ok") : PSTR("fail!"));

  if (!Rtc.GetIsRunning())
  {
    if (!wasError("setup GetIsRunning"))
    {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
    }
  }

  setTimeFromNTP();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });
  server.serveStatic("/", LittleFS, "/");
  server.on("/led", HTTP_GET, ledRequestHandler);
  server.on("/time", HTTP_GET, timeRequestHandler);
  server.on("/feed", HTTP_GET, feedRequestHandler);
  server.on("/reload-config", HTTP_GET, reloadConfigRequestHandler);
  server.on("/config", HTTP_GET, getConfigRequestHandler);
  server.on("/config", HTTP_PUT, putConfigRequestHandler);
  server.onNotFound(notFound);
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000); });

  server.addHandler(&events);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.begin();
  OTABegin();
  ftpSrv.begin(F("ftp"), F("ftp"));
  loadConfig();
  digitalWrite(ledIO, HIGH);
}

void checkFoodPouredOut()
{
  if (!isFoodPouredOut)
  {
    isFoodPouredOut = digitalRead(feedSensorIO) == HIGH;
  }
}

void checkMotorSensor()
{
  const int sensorState = digitalRead(motorSensorIO);
  if (sensorState == LOW && motorSensorPrevState == HIGH)
  {
    motorRotationCount++;
  }

  motorSensorPrevState = sensorState;

  if (motorRotationCount >= maxMotorRotations)
  {
    // Задержка, чтобы окошко для датчика в задающем диске успело сдвинуться перед выключением моторчика
    delay(50);
    engineOff();
    if (!isFoodPouredOut)
    {
      playMelody(speakerIO);
    }
  }
}

void loop()
{
  if ((millis() - lastTime) > timerDelaySchedule)
  {
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    char *nowTime = printTime(now);

    for (JsonVariant scheduleItem : schedule)
    {
      if (nowTime == scheduleItem["time"])
      {
        engineOn(scheduleItem["portions"].as<uint8_t>());
      }
    }

    if (nowTime == "00:00")
    {
      setTimeFromNTP();
    }

    lastTime = millis();
  }

  feedButtonState = digitalRead(feedButtonIO);

  if (feedButtonState == LOW)
  {
    engineOn();
  }

  if (isEngineOn)
  {
    checkMotorSensor();
    checkFoodPouredOut();
  }

  ftpSrv.handleFTP();
  ArduinoOTA.handle();
}
