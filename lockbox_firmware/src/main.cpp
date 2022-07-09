#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WiFiClient.h>
#include "ESP8266mDNS.h"

// Due to improper compatibility this must happen in this order:
#include <WiFiManager.h>
#define WEBSERVER_H
#include <ESPAsyncWebServer.h>

#define PINSERVO D4
#define MAX_PASSWORD_LENGTH 64
#define TCP_PORT 5000
#define INITSTRING "EKI_LOCKBOX"
#define CAM_INVERTED false
#define CAM_CLOSED 0
#define CAM_OPEN 180
#define EEPROM_STATE_ADDR 128
#define EEPROM_PASSWORD_ADDR EEPROM_STATE_ADDR + sizeof(EEPROMStateObject)
#define EEPROM_SIZE (EEPROM_STATE_ADDR + sizeof(EEPROMStateObject) + sizeof(EEPROMPasswordObject))

struct EEPROMStateObject
{
    char initstring[sizeof(INITSTRING)];
    bool locked;
};

struct EEPROMPasswordObject
{
    char password[MAX_PASSWORD_LENGTH];
};

Servo myservo;
WiFiManager wifiManager;
WiFiClient client;
AsyncWebServer server(TCP_PORT);

// EEPROM related functions

bool verify_eeprom_state_validity()
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    if (memcmp(state.initstring, INITSTRING, sizeof(INITSTRING)) != 0)
    {
        Serial.println("State is invalid! Initstring did not match");
        Serial.println(state.initstring);
        return false;
    }
    return true;
}

bool initialize_eeprom()
{
    EEPROMStateObject state;
    state.locked = false;
    strcpy(state.initstring, INITSTRING);
    EEPROM.put(EEPROM_STATE_ADDR, state);
    if (EEPROM.commit())
    {
        Serial.println("init eeprom committed");
        return true;
    }
    else
    {
        Serial.println("init eeprom commit failed!");
        return false;
    }
}

// State manipulation

bool set_software_locked(bool lock)
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    state.locked = lock;
    EEPROM.put(EEPROM_STATE_ADDR, state);
    if (EEPROM.commit())
    {
        Serial.println("set lock status eeprom committed");
        return true;
    }
    else
    {
        Serial.println("set lock status eeprom commit failed!");
        return false;
    }
}

bool get_password(char *buf)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(buf, password_object.password);
    return true;
}

bool set_password(const char *newPassword)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(password_object.password, newPassword);
    EEPROM.put(EEPROM_PASSWORD_ADDR, password_object);
    if (EEPROM.commit())
    {
        Serial.println("password eeprom committed");
        return true;
    }
    else
    {
        Serial.println("password eeprom commit failed!");
        return false;
    }
}

bool get_is_locked()
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    return state.locked; // TODO: investigate
}

void set_hardware_locked(bool lock)
{
    bool new_state = CAM_INVERTED ^ lock;
    if (new_state)
    {
        myservo.write(CAM_CLOSED);
    }
    else
    {
        myservo.write(CAM_OPEN);
    }
}

// Request handlers

void respond_json(AsyncWebServerRequest* request, int code, String jsonString)
{
    AsyncWebServerResponse *response = request->beginResponse(code, "application/json", jsonString);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void action_lock(AsyncWebServerRequest *request)
{
    if (get_is_locked())
    {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"alreadyLocked\"}");
    }
    String password;
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    } else {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"noPassword\"}");
        return;
    }
    // todo: check password length
    Serial.print("Locking with password: ");
    Serial.println(password);
    set_password(password.c_str());
    if (set_software_locked(true))
    {
        set_hardware_locked(true);
        respond_json(request, 200, "{\"result\":\"success\"}");
    }
    else
    {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"eepromError\"}");
    }
}

void action_unlock(AsyncWebServerRequest *request)
{
    String password;
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    } else {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"noPassword\"}");
        return;
    }
    Serial.print("Unlocking with password: ");
    Serial.println(password);
    char savedPassword[MAX_PASSWORD_LENGTH];
    get_password(savedPassword);
    if (strcmp(savedPassword, password.c_str()) == 0)
    {
        set_hardware_locked(false);
        if (set_software_locked(false))
        {
            respond_json(request, 200, "{\"result\":\"success\"}");
        }
        else
        {
            respond_json(request, 500, "{\"result\":\"error\", \"error\":\"eepromError\"}");
        }
    }
    else
    {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"wrongPassword\"}");
    }
}

void action_update(AsyncWebServerRequest *request)
{
    if (get_is_locked())
    {
        set_hardware_locked(true);
    }
    else
    {
        set_hardware_locked(false);
    }
    respond_json(request, 200, "{\"result\":\"success\"}");
}

void action_status(AsyncWebServerRequest *request)
{
    if (get_is_locked())
    {
        respond_json(request, 200, "{\"result\":\"success\", \"data\":\"locked\"}");
    }
    else
    {
        respond_json(request, 200, "{\"result\":\"success\", \"data\":\"unlocked\"}");
    }
}

void notFound(AsyncWebServerRequest *request) {
    respond_json(request, 404, "{\"result\":\"error\", \"error\":\"notFound\"}");
}

void setup()
{
    Serial.begin(9600);
    delay(500);
    EEPROM.begin(EEPROM_SIZE);
    myservo.attach(PINSERVO);

    if (!verify_eeprom_state_validity())
    {
        Serial.println("EEPROM not properly initialized. fixing.");
        if (!initialize_eeprom())
        {
            Serial.println("Could not initialize EEPROM!");
            delay(3000);
            ESP.restart();
            delay(5000);
        }
    }
    else
    {
        Serial.println("\n\nEEPROM valid");
    }

    if (get_is_locked())
    {
        Serial.println("State is locked");
        if (set_software_locked(true))
        {
            set_hardware_locked(true);
        }
        else
        {
            Serial.println("Could not set sw locked");
            set_hardware_locked(false);
        }
    }
    else
    {
        Serial.println("State is unlocked");
        set_hardware_locked(false);
    }

    Serial.print("Password: ");
    char pwd[MAX_PASSWORD_LENGTH];
    get_password(pwd);
    Serial.println(pwd);

    WiFi.softAPdisconnect(true);
    if (!wifiManager.autoConnect("EKI Lockbox"))
    {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        delay(5000);
    }

    const int mdns_name_len = 15;
    char mdns_name[mdns_name_len];
    sprintf(mdns_name, "lockbox_%6X", ESP.getChipId());
    if (!MDNS.begin(mdns_name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
    }
    MDNS.addService("ekilb", "tcp", TCP_PORT);

    server.onNotFound(notFound);
    server.on("/lock", HTTP_POST, action_lock);
    server.on("/unlock", HTTP_POST, action_unlock);
    server.on("/update", HTTP_POST, action_update);
    server.on("/status", HTTP_GET, action_status);
    server.begin();
}

void loop()
{
}
