#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <arduinoFFT.h>
#include "DisplayManager.h"

#define BANDS_COUNT 17
#define SEND_BANDS_COUNT 8
#define SAMPLES 1024
#define SAMPLING_FREQUENCY 44100
#define AVERAGE_WINDOW_SIZE 5
// #define AUDIO_PIN 13

class AudioManager {
public:
    int bandValues[BANDS_COUNT];
    int oldBarHeights[BANDS_COUNT];
    int sendBandValues[SEND_BANDS_COUNT];
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
    void sendBars();

private:
    const unsigned int noise = 1400;
    unsigned int sampling_period_us;
    unsigned long newTime;
    double vReal[SAMPLES];
    double vImag[SAMPLES];
    ArduinoFFT<double> FFT;
    byte peak[BANDS_COUNT];
    const float smoothFactor = 0.32;

    float agcGain   = 1.0f;                  // running gain
    static constexpr float AGC_TARGET  = 5000.0f;
    static constexpr float AGC_ATTACK  = 0.02f;   // faster =  larger
    static constexpr float AGC_RELEASE = 0.005f;  // faster =  larger

};

#endif // AUDIOMANAGER_H
