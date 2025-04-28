#include "ServerManager.h"

// AP credentials
const char* ServerManager::AP_SSID     = "CLI-8";
const char* ServerManager::AP_PASSWORD = "123456789";

ServerManager::ServerManager(AudioManager& audioManager)
: audioManager(audioManager)
, server(80)
, dns()
, prefs()
{
}

void ServerManager::initialize()
{
    // Serial.begin(115200);

    // 1) Mount LittleFS
    if (! LittleFS.begin())
    {
        // Serial.println("❌ LittleFS mount failed");
    }

    // 2) Pure‑AP setup: ensure AP‑only, static IP, then start
    WiFi.mode(WIFI_AP);
    IPAddress local_ip(192,168,4,1),
              gateway(192,168,4,1),
              subnet(255,255,255,0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    // 3) Brief pause for LWIP
    delay(100);

    IPAddress apIP = WiFi.softAPIP();
    // Serial.printf("🔌 AP '%s' @ %s\n", AP_SSID, apIP.toString().c_str());

    // 4) Start captive‑portal DNS on Core 0
    xTaskCreatePinnedToCore(
        dnsTask,          // entry
        "dnsTask",        // name
        2048,             // stack
        &dns,             // pvParam
        1,                // priority
        nullptr,          // handle
        0                 // core 0
    );

    // 5) HTTP routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
    {
        req->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *req)
    {
        req->send(LittleFS, "/style.css", "text/css");
    });
    server.on("/setSens", HTTP_GET, [this](AsyncWebServerRequest *req)
    {
        if (auto* p = req->getParam("value"))
        {
            auto sensitivitySlider = p->value().toInt();
            if (sensitivitySlider > 0)
                audioManager.sensitivity = sensitivitySlider;
        }
        req->send(200, "text/plain", "OK");
    });

    // 6) Portal + probe redirects
    String portal = String("http://") + apIP.toString() + "/";
    server.onNotFound([portal](AsyncWebServerRequest *req)
    {
        req->redirect(portal);
    });
    server.on("/connecttest.txt",     HTTP_GET, [portal](AsyncWebServerRequest *req)
    {
        req->redirect(portal);
    });
    server.on("/generate_204",        HTTP_GET, [portal](AsyncWebServerRequest *req)
    {
        req->redirect(portal);
    });
    server.on("/hotspot-detect.html", HTTP_GET, [portal](AsyncWebServerRequest *req)
    {
        req->redirect(portal);
    });

    // 7) Launch HTTP server
    server.begin();
    // Serial.println("🚀 HTTP server started");
}

void ServerManager::dnsTask(void* pv)
{
    auto* dnsPtr = static_cast<DNSServer*>(pv);

    // Make sure Wi‑Fi/AP is up
    vTaskDelay(pdMS_TO_TICKS(500));

    dnsPtr->start(DNS_PORT, "*", WiFi.softAPIP());
    // Serial.println("🌐 Captive portal DNS started");

    for (;;)
    {
        dnsPtr->processNextRequest();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
