#include "AudioManager.h"

#include "driver/adc.h"
#include <string.h>
#include <atomic>

#include "driver/timer.h"
#include <esp_intr_alloc.h>

#define TAG "AudioManager"

AudioManager::AudioManager() 
    : sampling_period_us(round(1000000 * (1.0 / SAMPLING_FREQUENCY))),
      FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY) {
}

void AudioManager::initialize() {
    memset(peak, 0, sizeof(peak));
    memset(oldBarHeights, 0, sizeof(oldBarHeights));
    memset(bandValues, 0, sizeof(bandValues));

    adc1_config_width(ADC_WIDTH_BIT_12); // Set the ADC width to 12-bit
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // Set attenuation for 0-3.3V range
}

void AudioManager::readAudioSamples() {
    for (int i = 0; i < SAMPLES; i++) {
        // newTime = micros();
        vReal[i] = adc1_get_raw(ADC1_CHANNEL_6);
        // unsigned long nextTime = micros();
        // Serial.println(nextTime - newTime);
        vImag[i] = 0;
        while ((micros() - newTime) < sampling_period_us) {
            // Wait until the next sampling period
        }
    }
}

void AudioManager::audioProcessingTask()
{
    readAudioSamples();
    processFFT();
    setFrequencyBars();
    averageBars();
}

void AudioManager::processFFT() {
    FFT.dcRemoval();
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
}

void AudioManager::setFrequencyBars() {
    for (int i = 2; i < (SAMPLES / 2); i++) {
        if (vReal[i] > this->noise) { // Noise filter
            // 8 bands - wider frequency distribution
            if (i >= 2   && i <= 5  ) bandValues[0] += (int)vReal[i];  // Low bass
            if (i > 5    && i <= 15 ) bandValues[1] += (int)vReal[i];  // Mid bass
            if (i > 15   && i <= 35 ) bandValues[2] += (int)vReal[i];  // High bass
            if (i > 35   && i <= 70 ) bandValues[3] += (int)vReal[i];  // Low midrange
            if (i > 70   && i <= 140) bandValues[4] += (int)vReal[i];  // Mid midrange
            if (i > 140  && i <= 220) bandValues[5] += (int)vReal[i];  // Upper midrange
            if (i > 220  && i <= 300) bandValues[6] += (int)vReal[i];  // Lower treble
            if (i > 300  && i <= 350) bandValues[7] += (int)vReal[i];  // High treble
        }
    }
}

void AudioManager::averageBars()
{
    for (byte band = 0; band < BANDS_COUNT; band++) {
        int newBarHeight = bandValues[band] / sensitivity;
        int smoothedBarHeight = (smoothFactor * newBarHeight) + ((1 - smoothFactor) * oldBarHeights[band]);

        if (smoothedBarHeight > peak[band]) {
            peak[band] = smoothedBarHeight;
        }

        oldBarHeights[band] = smoothedBarHeight;

        bandValues[band] = 0;
    }
}
