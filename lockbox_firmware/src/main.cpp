#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WiFiClient.h>
#include <WiFiManager.h>

#include "ESP8266mDNS.h"

#define PINDEBUGLEDPRIMARY D1
#define PINSERVO D4

#define MAX_PASSWORD_LENGTH 64
#define MAX_INCOMING_DATA_LENGTH 256
#define TCP_PORT 5000
#define INITSTRING "EKI_LOCKBOX"
#define EEPROM_STATE_ADDR 128
#define EEPROM_PASSWORD_ADDR EEPROM_STATE_ADDR + sizeof(EEPROMStateObject)
#define EEPROM_SIZE (EEPROM_STATE_ADDR + sizeof(EEPROMStateObject) + sizeof(EEPROMPasswordObject))
#define MAX_BOX_NAME_LENGTH 32

struct EEPROMStateObject
{
    char initstring[sizeof(INITSTRING)];
    bool locked;
};

struct EEPROMPasswordObject
{
    char password[MAX_PASSWORD_LENGTH];
};

struct EEPROMSettingsObject
{
    int camClosedPosition;
    int camOpenPosition;
    char lockboxName[MAX_BOX_NAME_LENGTH];
};

char incomingData[MAX_INCOMING_DATA_LENGTH] = {0};
uint8_t incomingDataIndex = 0;
Servo myservo;
WiFiManager wifiManager;
WiFiClient client;
WiFiServer server(TCP_PORT);
StaticJsonDocument<MAX_INCOMING_DATA_LENGTH> jsonInput;

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
    return state.locked; // TODO: investigate "may be used uninitialized"
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

char *handleCommand(char *data)
{
    DeserializationError error = deserializeJson(jsonInput, data);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "{\"result\":\"failure\", \"error\":\"deserializationError\"}\n";
    }

    const char *command = jsonInput["command"];

    if (strcmp(command, "lock") == 0)
    {
        if (get_is_locked())
        {
            return "{\"result\":\"failure\", \"error\":\"alreadyLocked\"}\n";
        }
        const char *password = jsonInput["password"];
        Serial.print("Locking with password: ");
        Serial.println(password);
        set_password(password);
        if (set_software_locked(true))
        {
            set_hardware_locked(true);
            return "{\"result\": \"success\"}\n";
        }
        else
        {
            return "{\"result\":\"failure\", \"error\":\"EEPROM error\"}\n";
        }
    }
    else if (strcmp(command, "unlock") == 0)
    {
        const char *password = jsonInput["password"];
        Serial.print("Unlocking with password: ");
        Serial.println(password);
        char savedPassword[MAX_PASSWORD_LENGTH];
        get_password(savedPassword);
        if (strcmp(savedPassword, password) == 0)
        {
            set_hardware_locked(false);
            if (set_software_locked(false))
            {
                return "{\"result\":\"success\"}\n";
            }
            else
            {
                return "{\"result\":\"failure\", \"error\":\"EEPROM error\"}\n";
            }
        }
        else
        {
            return "{\"result\":\"failure\", \"error\":\"wrongPassword\"}\n";
        }
    }
    else if (strcmp(command, "update") == 0)
    {
        if (get_is_locked())
        {
            set_hardware_locked(true);
        }
        else
        {
            set_hardware_locked(false);
        }
        return "{\"result\":\"success\"}\n";
    }
    else if (strcmp(command, "status") == 0)
    {
        if (get_is_locked())
        {
            return "{\"result\":\"success\", \"status\":\"locked\"}\n";
        }
        else
        {
            return "{\"result\":\"success\", \"status\":\"unlocked\"}\n";
        }
    }
    return "{\"result\":\"failure\", \"error\":\"invalidCommand\"}\n";
}

void setup()
{
    Serial.begin(9600);
    delay(500);
    EEPROM.begin(EEPROM_SIZE);
    myservo.attach(PINSERVO);
    pinMode(PINDEBUGLEDPRIMARY, OUTPUT);
    digitalWrite(PINDEBUGLEDPRIMARY, LOW);

    if (!verify_eeprom_state_validity())
    {
        Serial.println("EEPROM not properly initialized. Initializing now...");
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
        Serial.println("EEPROM valid");
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
            Serial.println("Could not set lock in software");
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
    server.begin();
    digitalWrite(PINDEBUGLEDPRIMARY, HIGH);

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
}

void loop()
{

    if (!client.connected())
    {
        client = server.available();
    }
    else
    {
        if (client.available() > 0)
        {
            if (client.peek() == '\n')
            {
                client.read();
                incomingData[incomingDataIndex] = '\0';
                client.write((handleCommand(incomingData)));
                incomingDataIndex = 0;
            }
            else
            {
                incomingData[incomingDataIndex] = client.read();
                incomingDataIndex++;
                if (incomingDataIndex >= MAX_INCOMING_DATA_LENGTH - 1)
                {
                    incomingDataIndex = 0;
                }
            }
        }
    }
}
