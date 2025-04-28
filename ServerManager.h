#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "AudioManager.h"

class ServerManager
{
public:
    ServerManager(AudioManager& audioManager);
    void initialize();

private:
    AudioManager&    audioManager;
    AsyncWebServer   server;
    DNSServer        dns;
    Preferences      prefs;

    static const byte DNS_PORT = 53;
    static const char* AP_SSID;
    static const char* AP_PASSWORD;

    static void dnsTask(void* pv);
};

#endif // SERVERMANAGER_H
