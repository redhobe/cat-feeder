#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FTPServer.h>
#include <Arduino_JSON.h>
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
uint8_t ledIO = 5;
uint8_t motorClockwiseIO = 18;
uint8_t motorAntiClockwiseIO = 17;
uint8_t sleepIO = 19;
uint8_t sensorsIO = 33;
uint8_t motorSensorIO = 27;
uint8_t feedSensorIO = 14;
uint8_t speakerIO = 16;
uint8_t sdaIO = 15;
uint8_t sclIO = 4;

uint8_t feedSensorState = HIGH;
uint8_t motorSensorState = LOW;
bool isEngineOn = false;
bool isFoodPouredOut = false;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelaySchedule = 60000;

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
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

void engineOn()
{
  isEngineOn = true;
  motorSensorState = HIGH;
  isFoodPouredOut = false;

  digitalWrite(sensorsIO, HIGH);
  digitalWrite(motorClockwiseIO, HIGH);
  digitalWrite(sleepIO, HIGH);
}

void engineOff()
{
  isEngineOn = false;
  motorSensorState = HIGH;

  digitalWrite(sensorsIO, LOW);
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
}

void checkFoodPouredOut()
{
  if (!isFoodPouredOut)
  {
    isFoodPouredOut = !!digitalRead(feedSensorIO);
  }
}

void checkMotorSensor()
{
  if (!digitalRead(motorSensorIO))
  {
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
    lastTime = millis();
  }

  if (isEngineOn)
  {
    checkFoodPouredOut();
    checkMotorSensor();
  }

  ftpSrv.handleFTP();
  ArduinoOTA.handle();
}
