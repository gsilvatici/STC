#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <memory>
#include <vector>
#include <dimmable_light.h>

#define SYNC_PIN 35

class LightManager {
public:
    LightManager();
    void initialize();
    void updateBrightness();
    void setLightBrightness(unsigned int lightIndex, unsigned int brightness);
    void turnOnLight(unsigned int lightIndex);
    void turnOffLight(unsigned int lightIndex);

    static const std::vector<int> lightChannels;

private:
    std::vector<DimmableLight*> lights;
    const int period = 1;
};

#endif // LIGHT_MANAGER_H
