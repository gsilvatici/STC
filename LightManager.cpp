#include "LightManager.h"

using namespace std;                          //                                   2              2  2   2   2
                                              // 1   2   3   4   5   6   7   8  9  10 11  12  13  14 15  16  17
// const vector<int> LightManager::lightChannels = {13, 25, 14, 27, 2, 26, 33, 12, 0, 4, 32, 16, 5, 18, 17, 23, 19};

// 10

const vector<int> LightManager::lightChannels = {13, 14, 27, 12, 16, 5, 18, 17};

// SECCION A
                                                //gr ve am  ve  az  r   vi  r
// const vector<int> LightManager::lightChannels = {12, 2, 26, 23, 19, 32, 4, 5};


LightManager::LightManager() {
}

void LightManager::initialize() {
    DimmableLight::setSyncPin(SYNC_PIN);
    DimmableLight::begin();
    
    for (int pin : lightChannels) {
        lights.push_back(new DimmableLight(pin));
    }
}

void LightManager::updateBrightness() {
    for (int i = 50; i < 255; i++) {
        for (auto& light : lights) {
            light->setBrightness(i);
        }
        delay(period);
    }
    for (int i = 255; i >= 50; i--) {
        for (auto& light : lights) {
            light->setBrightness(i);
        }
        delay(period);
    }
}

void LightManager::setLightBrightness(unsigned int lightIndex, unsigned int brightness) {
    if (brightness < 55)
        brightness = 55;

    // if (brightness < 150)
    //     brightness = 150;


    if (brightness > 240)
        brightness = 240;

    lights[lightIndex]->setBrightness(brightness);
}

void LightManager::turnOnLight(unsigned int lightIndex) {
    lights[lightIndex]->turnOn();
}

void LightManager::turnOffLight(unsigned int lightIndex) {
    lights[lightIndex]->turnOff();
}
