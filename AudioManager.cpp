#include "AudioManager.h"

#include "driver/adc.h"
#include <string.h>
#include <atomic>

#include "driver/timer.h"
#include <esp_intr_alloc.h>

#define TAG "AudioManager"

AudioManager::AudioManager() 
    : sampling_period_us(round(1000000 * (1.0 / SAMPLING_FREQUENCY))),
      FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY), sensitivity(1000) {
}

void AudioManager::initialize() {
    memset(peak, 0, sizeof(peak));
    memset(oldBarHeights, 0, sizeof(oldBarHeights));
    memset(bandValues, 0, sizeof(bandValues));
    memset(sendBandValues, 0, sizeof(sendBandValues));

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
    sendBars();
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
            // 18 bands
            if (i >= 1   && i <= 3  ) bandValues[0]  += (int)(vReal[i]*0.39);
            if (i > 3   && i <= 5  ) bandValues[1]  += (int)(vReal[i]*0.7);
            if (i > 5   && i <= 7  ) bandValues[2]  += (int)vReal[i];
            if (i > 7   && i <= 9  ) bandValues[3]  += (int)vReal[i];
            if (i > 9   && i <= 13 ) bandValues[4]  += (int)vReal[i];
            if (i > 13  && i <= 18 ) bandValues[5]  += (int)vReal[i];
            if (i > 18  && i <= 25 ) bandValues[6]  += (int)vReal[i];
            if (i > 25  && i <= 36 ) bandValues[7]  += (int)vReal[i];
            if (i > 36  && i <= 50 ) bandValues[8]  += (int)vReal[i];
            if (i > 50  && i <= 69 ) bandValues[9]  += (int)vReal[i];
            if (i > 69  && i <= 97 ) bandValues[10] += (int)vReal[i];
            if (i > 97  && i <= 135) bandValues[11] += (int)vReal[i];
            if (i > 135 && i <= 189) bandValues[12] += (int)vReal[i];
            if (i > 189 && i <= 220) bandValues[13] += (int)vReal[i];
            if (i > 220 && i <= 270) bandValues[14] += (int)vReal[i];
            if (i > 270 && i <= 320) bandValues[15] += (int)(vReal[i]*(0.82));
            if (i > 320 && i <= 360) bandValues[16] += (int)(vReal[i]*(0.82));
        
        
            if (i >= 2   && i <= 5  ) sendBandValues[0] += (int)vReal[i]/2;  // Low bass
            if (i > 5    && i <= 15 ) sendBandValues[1] += (int)vReal[i]/2;  // Mid bass
            if (i > 15   && i <= 35 ) sendBandValues[2] += (int)vReal[i];  // High bass
            if (i > 35   && i <= 70 ) sendBandValues[3] += (int)vReal[i];  // Low midrange
            if (i > 70   && i <= 140) sendBandValues[4] += (int)vReal[i];  // Mid midrange
            if (i > 140  && i <= 220) sendBandValues[5] += (int)vReal[i];  // Upper midrange
            if (i > 220  && i <= 300) sendBandValues[6] += (int)vReal[i];  // Lower treble
            if (i > 300  && i <= 350) sendBandValues[7] += (int)vReal[i];  // High treble
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

void AudioManager::sendBars() {
  const uint8_t HEADER = 0xAB;  
  Serial.write(HEADER);
  for (uint8_t i = 0; i < SEND_BANDS_COUNT; i++) {
    uint16_t v = sendBandValues[i];     // each is up to e.g. 0–1023 or more
    Serial.write(uint8_t(v & 0xFF));       // LSB
    Serial.write(uint8_t((v >> 8) & 0xFF)); // MSB
    sendBandValues[i] = 0;
  }
}
