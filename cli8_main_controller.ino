// LC includes
#include "AudioManager.h"
#include "DisplayManager.h"
#include "LightManager.h"

// Managers
AudioManager audioManager;
DisplayManager displayManager;
LightManager lightManager;

// Time
unsigned long currentMillis = 0;
unsigned long potMillis = 0;

// Analog controls
#define SENS_POT_PIN_MASTER 15
#define SENS_POT_PIN_SECONDARY 36
volatile int sens = 1000;


int currentLightIndex = 0;
unsigned long lightStartTime = 0;   
const int maxBrightness = 245;      
const unsigned long lightDuration = 5000;

void setup() 
{
    Serial.begin(115200);
    audioManager.initialize();
    displayManager.initialize();
    lightManager.initialize();
}

void loop() 
{
    currentMillis = millis();

    if (currentMillis - potMillis >= 200) {
        potMillis = currentMillis;
        readMasterPotentiometer();
    }

    audioManager.audioProcessingTask();

    for (int i = 0; i < LightManager::lightChannels.size(); i++)
    {
        lightManager.setLightBrightness(i, audioManager.oldBarHeights[i]);
    }
}

void readMasterPotentiometer()
{
    int potValue = analogRead(SENS_POT_PIN_MASTER); // Read potentiometer value (0 to 4095)

    // Normalize the potentiometer value to a 0.0 to 1.0 range
    float normalizedPot = (float)potValue / 4095.0f;

    // Apply an exponential function to create a logarithmic-like curve
    // Adjust the exponent to fine-tune the response curve
    float exponent = 4.0f; // Try values between 2.0 and 5.0
    float adjustedPot = powf(normalizedPot, exponent);

    // Map the adjusted potentiometer value to your desired sensitivity range
    int minSensitivity = 100;    // Minimum sensitivity value
    int maxSensitivity = 3000;   // Maximum sensitivity value
    int sensitivity = minSensitivity + (int)(adjustedPot * (maxSensitivity - minSensitivity));

    audioManager.sensitivity = sensitivity;
}

void readSecondaryPotentiometer()
{
    int potValue = analogRead(SENS_POT_PIN_SECONDARY); // Read secondary potentiometer value (0 to 4095)

    // Normalize the potentiometer value to a 0.0 to 1.0 range
    float normalizedPot = (float)potValue / 4095.0f;

    // Apply an exponential function to create a logarithmic-like curve
    // Adjust the exponent to fine-tune the response curve
    float exponent = 4.0f; // Try values between 2.0 and 5.0
    float adjustedPot = powf(normalizedPot, exponent);

    // Map the adjusted potentiometer value to your desired sensitivity range
    int minSensitivity = 100;    // Minimum sensitivity value
    int maxSensitivity = 3000;   // Maximum sensitivity value
    int sensitivity = minSensitivity + (int)(adjustedPot * (maxSensitivity - minSensitivity));
    
    // You can store this value wherever needed, similar to how you store the master sensitivity
    // audioManager.secondarySensitivity = sensitivity;
}

void turnOnAllChannels()
{

}

void drawBands()
{
    displayManager.clear();

    for (byte band = 0; band < BANDS_COUNT; band++) 
    {
        displayManager.drawBand(band, audioManager.oldBarHeights[band]/2);
    }

    displayManager.draw();
}

void runChase()
{
    // Calculate elapsed time
    unsigned long elapsedTime = currentMillis - lightStartTime;

    // Update the chase effect
    if (elapsedTime >= lightDuration) 
    {
        // Move to the next light
        currentLightIndex++;
        if (currentLightIndex >= LightManager::lightChannels.size()) 
        {
            currentLightIndex = 0; // Loop back to the first light
        }
        // Reset the start time for the new light
        lightStartTime = currentMillis;
        elapsedTime = 0; // Reset elapsedTime for the new light
    }

    for (int i = 0; i < LightManager::lightChannels.size(); i++)
    {
        if (i == currentLightIndex)
        {
            // Set the brightness for the current light
            lightManager.setLightBrightness(i, 160);
        }
        else
        {
            // Turn off other lights
            lightManager.setLightBrightness(i, 0);
        }
    }

    for (byte band = 0; band < BANDS_COUNT; band++) 
    {
        displayManager.drawBand(band, audioManager.oldBarHeights[band]/2);
    }
}
