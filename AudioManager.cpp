#include "AudioManager.h"

#include "driver/adc.h"
// #include "esp_adc/adc_continuous.h"
// #include <esp_log.h>
#include <string.h>
#include <atomic>

#include "driver/timer.h"
#include <esp_intr_alloc.h>

#define TAG "AudioManager"

// static adc_continuous_handle_t adc_handle;
// std::atomic_uint32_t sampleCount;
// #define DMA_BUFFER_SIZE 4096
// #define ESP_OK_CHECK(x) do { esp_err_t __err_rc = (x); if (__err_rc != ESP_OK) { ESP_LOGE(TAG, "ESP_ERROR %d at %s:%d", __err_rc, __FILE__, __LINE__); } } while(0)

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

    // adc_power_acquire();

    // // ADC continuous mode driver initial configurations
    // adc_continuous_handle_cfg_t handle_cfg = {
    //     .max_store_buf_size = DMA_BUFFER_SIZE, // Max length of the conversion results buffer, in bytes
    //     .conv_frame_size = DMA_BUFFER_SIZE / 2 // Conversion frame size, in bytes
    // };
    // ESP_OK_CHECK(adc_continuous_new_handle(&handle_cfg, &adc_handle));

    // adc_digi_pattern_config_t channel[1] = {
    //     {
    //         .atten = ADC_ATTEN_DB_11,     // Attenuation setting for 0-3.3V range
    //         .channel = ADC1_CHANNEL_6,    // Channel number (GPIO 32)
    //         .unit = ADC_UNIT_1,           // Use ADC unit 1
    //         .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH // 12-bit resolution (Max supported)
    //     }
    // };

    // adc_continuous_config_t continuous_cfg = {
    //     .pattern_num = 1,                // Number of ADC channels/patterns
    //     .adc_pattern = channel,          // Set the conversion pattern
    //     .sample_freq_hz = SAMPLING_FREQUENCY,   // Desired sampling frequency
    //     .conv_mode = ADC_CONV_SINGLE_UNIT_1, // Single ADC unit
    //     .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1 // ADC output format
    // };
    // ESP_OK_CHECK(adc_continuous_config(adc_handle, &continuous_cfg));
    // ESP_OK_CHECK(adc_continuous_start(adc_handle));
}

// void AudioManager::readAudioSamples() {
//     sampleIndex = 0;

//     // Start the timer
//     timer_start(TIMER_GROUP_0, TIMER_0);

//     // Wait until all samples are collected
//     while (sampleIndex < SAMPLES) {
//         // Optional: Add a small delay or yield to prevent watchdog resets
//         // vTaskDelay(1);  // Uncomment if necessary
//     }

//     // Timer is paused in ISR when sampling is complete
// }

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

// void AudioManager::readAudioSamples() {
//     uint8_t dma_buffer[DMA_BUFFER_SIZE]; // Allocate buffer for DMA read as uint8_t
//     uint32_t bytes_read = 0; // Correct type for 'out_length'
//     int samples_accumulated = 0; // Counter for accumulated samples

//     // Loop until we collect exactly SAMPLES (1024) samples
//     while (samples_accumulated < SAMPLES) {
//         // Read samples from the ADC DMA buffer
//         esp_err_t err = adc_continuous_read(adc_handle, dma_buffer, DMA_BUFFER_SIZE, &bytes_read, portMAX_DELAY);

//         if (err == ESP_OK && bytes_read > 0) {
//             // Process the read samples
//             adc_digi_output_data_t* data = (adc_digi_output_data_t*)dma_buffer;
//             int samples_read = bytes_read / sizeof(adc_digi_output_data_t);

//             // Store the samples into the FFT buffer
//             for (int i = 0; i < samples_read; ++i) {
//                 if (samples_accumulated < SAMPLES) {
//                     vReal[samples_accumulated] = data[i].type1.data; // Store ADC samples in vReal
//                     vImag[samples_accumulated] = 0; // Imaginary part for FFT
//                     samples_accumulated++;
//                 } else {
//                     // Break out of the loop if we have enough samples
//                     break;
//                 }
//             }
//         } else {
//             ESP_LOGE(TAG, "Failed to read from ADC DMA buffer: %s", esp_err_to_name(err));
//         }
//     }

//     // Increment sample count for monitoring purposes (optional)
//     sampleCount += samples_accumulated;
// }

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

// void AudioManager::setFrequencyBars() {
//     for (int i = 2; i < (SAMPLES / 2); i++) {
//         if (vReal[i] > this->noise) { // Noise filter
//             // 24 bands
//             if      (i >= 2    && i <= 3   ) bandValues[0]  += (int)vReal[i];
//             else if (i >= 4    && i <= 5   ) bandValues[1]  += (int)vReal[i];
//             else if (i >= 6    && i <= 7   ) bandValues[2]  += (int)vReal[i];
//             else if (i >= 8    && i <= 9   ) bandValues[3]  += (int)vReal[i];
//             else if (i >= 10   && i <= 11  ) bandValues[4]  += (int)vReal[i];
//             else if (i >= 12   && i <= 13  ) bandValues[5]  += (int)vReal[i];
//             else if (i >= 14   && i <= 15  ) bandValues[6]  += (int)vReal[i];
//             else if (i >= 16   && i <= 17  ) bandValues[7]  += (int)vReal[i];
//             else if (i >= 18   && i <= 19  ) bandValues[8]  += (int)vReal[i];
//             else if (i >= 20   && i <= 21  ) bandValues[9]  += (int)vReal[i];
//             else if (i >= 22   && i <= 23  ) bandValues[10] += (int)vReal[i];
//             else if (i >= 24   && i <= 25  ) bandValues[11] += (int)vReal[i];
//             else if (i >= 26   && i <= 27  ) bandValues[12] += (int)vReal[i];
//             else if (i >= 28   && i <= 31  ) bandValues[13] += (int)vReal[i];
//             else if (i >= 32   && i <= 35  ) bandValues[14] += (int)vReal[i];
//             else if (i >= 36   && i <= 39  ) bandValues[15] += (int)vReal[i];
//             else if (i >= 40   && i <= 45  ) bandValues[16] += (int)vReal[i];
//             else if (i >= 46   && i <= 51  ) bandValues[17] += (int)vReal[i];
//             else if (i >= 52   && i <= 59  ) bandValues[18] += (int)vReal[i];
//             else if (i >= 60   && i <= 69  ) bandValues[19] += (int)vReal[i];
//             else if (i >= 70   && i <= 81  ) bandValues[20] += (int)vReal[i];
//             else if (i >= 82   && i <= 92  ) bandValues[21] += (int)vReal[i];
//             else if (i >= 93   && i <= 108 ) bandValues[22] += (int)vReal[i];
//             else if (i >= 116               ) bandValues[23] += (int)vReal[i];
//         }
//     }
// }
void AudioManager::setFrequencyBars() {
    for (int i = 2; i < (SAMPLES / 2); i++) {
        if (vReal[i] > this->noise) { // Noise filter
            // 18 bands
            if (i >= 2   && i <= 3  ) bandValues[0]  += (int)vReal[i]/2;
            if (i > 3   && i <= 5  ) bandValues[1]  += (int)vReal[i]/2;
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
            if (i > 270 && i <= 310) bandValues[15] += (int)vReal[i];
            if (i > 310 && i <= 350) bandValues[16] += (int)vReal[i];
        }
    }
}

void AudioManager::averageBars()
{
    // for (byte iteration = 0; iteration < 10; iteration++) 
    // {
    //     int minVal = oldBarHeights[0]; // Initialize with the first value
    //     int maxVal = oldBarHeights[0];
    //     byte minIndex = 0;
    //     byte maxIndex = 0;

    //     // Find the min and max values and their indices
    //     for (byte band = 1; band < BANDS_COUNT; band++) 
    //     {
    //         if (oldBarHeights[band] < minVal) 
    //         {
    //             minVal = oldBarHeights[band];
    //             minIndex = band;
    //         }
    //         if (oldBarHeights[band] > maxVal) 
    //         {
    //             maxVal = oldBarHeights[band];
    //             maxIndex = band;
    //         }
    //     }

    //     // Calculate the midpoint
    //     int midpoint = (maxVal + minVal) / 2;

    //     // Adjust values to reduce the gap
    //     for (byte band = 0; band < BANDS_COUNT; band++) 
    //     {
    //         if (band == maxIndex) 
    //         {
    //             // Bring the max closer to the midpoint
    //             oldBarHeights[band] -= (oldBarHeights[band] - midpoint) / 8;
    //         } else if (band == minIndex) 
    //         {
    //             // Bring the min closer to the midpoint
    //             oldBarHeights[band] += (midpoint - oldBarHeights[band]) / 8;
    //         }
    //     }
    // }

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
