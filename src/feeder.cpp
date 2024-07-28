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
  maxMotorRotations = defaultPortionsCount;
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

ArRequestHandlerFunction engineRequestHandler = [](AsyncWebServerRequest *request)
{
  if (!request->hasParam("state"))
  {
    request->send(500);
  }

  AsyncWebParameter *state = request->getParam("state");

  state->value().equals("on") ? engineOn() : engineOff();
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

ArRequestHandlerFunction putConfigRequestHandler = [](AsyncWebServerRequest *request)
{
  String output;
  serializeJson(config, output);
  request->send(200, "application/json", output);
};

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

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid())
  {
    if (!wasError("setup IsDateTimeValid"))
    {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    if (!wasError("setup GetIsRunning"))
    {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
    }
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (!wasError("setup GetDateTime"))
  {
    if (now < compiled)
    {
      Serial.println("RTC is older than compile time, updating DateTime");
      Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
      Serial.println("RTC is newer than compile time, this is expected");
    }
    else if (now == compiled)
    {
      Serial.println("RTC is the same as compile time, while not expected all is still fine");
    }
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });
  server.serveStatic("/", LittleFS, "/");
  server.on("/led", HTTP_GET, ledRequestHandler);
  server.on("/engine", HTTP_GET, engineRequestHandler);
  server.on("/config", HTTP_GET, getConfigRequestHandler);
  server.on("/config", HTTP_PUT, putConfigRequestHandler);

  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000); });

  server.addHandler(&events);
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

    for (JsonVariant scheduleItem : schedule)
    {
      char *nowTime = printTime(now);

      if (nowTime == scheduleItem["time"])
      {
        engineOn(scheduleItem["portions"].as<uint8_t>());
      }
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
