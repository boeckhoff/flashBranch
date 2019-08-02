/*
 * Be sure to run 
 * platformio run --target erase
 * once on any ESP that has been
 * connected to a wifi before
 */

#ifdef __INTELLISENSE__
#define MASTER
#endif

#ifdef MASTER

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "Arduino.h"
#include <SoftwareSerial.h>

#define PORT 43432

SoftwareSerial mySerial(D2, D1); //RX, TX

ESP8266WebServer server(PORT);

void handleTrigger() {
    Serial.println("handleTrigger started");

    if(!server.hasArg("ID")) {
        Serial.println("invalid request");
        server.send(404, "text/plain", "client did not provide ip or port");
        return;
    }

    String ID = server.arg("ID");
    mySerial.print('T');
    mySerial.println(ID);
    server.send(200, "text/plain", ID);
}

void handleAssignID() {

    Serial.println("handleAssignID started");

    if(!server.hasArg("ID")) {
        Serial.println("invalid request");
        server.send(404, "text/plain", "client did not provide ip or port");
        return;
    }

    String ID = server.arg("ID");
    mySerial.print('T');
    mySerial.println(ID);
    server.send(200, "text/plain", ID);
}

void handleRoot()
{
    Serial.println("handleRoot started");

    if (!server.hasArg("ip") || !server.hasArg("port"))
    {
        Serial.println("invalid request");
        server.send(404, "text/plain", "client did not provide ip or port");
        return;
    }

    server.send(200, "text/plain", "hola");
}

void setup()
{
    Serial.begin(9600);
    mySerial.begin(9600);
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);

    //Serial.println("Setting soft-AP ... ");
    boolean result = WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP");
    Serial.println(WiFi.softAPIP());
    if (result == true)
    {
        Serial.println("Ready");
    }
    else
    {
        Serial.println("Failed!");
    }

    server.on("/", handleRoot);
    server.on("/trigger", handleTrigger);
    server.begin();
}

void loop()
{
    server.handleClient();
    if (Serial.available())
    {
        mySerial.write(Serial.read());
    }
    if (mySerial.available())
    {
        Serial.write(mySerial.read());
    }
}

#endif