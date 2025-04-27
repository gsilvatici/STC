#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <arduinoFFT.h>
#include "DisplayManager.h"

#define BANDS_COUNT 8
#define SAMPLES 1024
#define SAMPLING_FREQUENCY 44100
#define AVERAGE_WINDOW_SIZE 5
// #define AUDIO_PIN 13

class AudioManager {
public:
    int bandValues[BANDS_COUNT];
    int oldBarHeights[BANDS_COUNT];
    unsigned int sensitivity;

    int barHeightBuffer[BANDS_COUNT][AVERAGE_WINDOW_SIZE] = {0};
    int bufferIndex[BANDS_COUNT] = {0}; // Track the index for each band

    AudioManager();
    void initialize();
    
    void audioProcessingTask();
    void readAudioSamples();
    void processFFT();
    void setFrequencyBars();
    void averageBars();

private:
    const unsigned int noise = 1450;
    unsigned int sampling_period_us;
    unsigned long newTime;
    double vReal[SAMPLES];
    double vImag[SAMPLES];
    ArduinoFFT<double> FFT;
    byte peak[BANDS_COUNT];
    const float smoothFactor = 0.32;

};

#endif // AUDIOMANAGER_H
