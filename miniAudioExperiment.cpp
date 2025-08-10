#include <iostream>
#include "miniaudio.h"
#include <Windows.h>
#include <vector>


#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

using namespace std;

struct UserData {
    ma_waveform waves[2];
    ma_waveform wave2;
    ma_encoder encoder;
    bool isPlaying = false;

    int waveIndex = 0;
    bool wavIndexes[3];

};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    UserData* ud = (UserData*)pDevice->pUserData;

    //ma_waveform_read_pcm_frames((ma_waveform*)pDevice->pUserData, pOutput, frameCount, NULL);
    if(ud->isPlaying)
    {
        ma_uint32 channels = pDevice->playback.channels;
        float* buffer1 = new float[frameCount * channels];
        float* buffer2 = new float[frameCount * channels];
        float* output = new float[frameCount * channels];

        if(ud->wavIndexes[0] == true){  }
        if (ud->wavIndexes[1] == true) {  }

        ma_waveform_read_pcm_frames(&ud->waves[0], buffer1, frameCount, NULL);
        ma_waveform_read_pcm_frames(&ud->waves[1], buffer2, frameCount, NULL);

        
        for(ma_uint32 i = 0; i<frameCount * channels; i++)
        {
            float mixed = 0.0f;
            
            if (ud->wavIndexes[0] == true) { mixed += buffer1[i]; }
            if (ud->wavIndexes[1] == true) { mixed += buffer2[i]; }
           
            output[i] = mixed;
        }

        memcpy(pOutput, output, frameCount * channels * sizeof(float));

        delete[] buffer1;
        delete[] buffer2;
        delete[] output;
    }
    else
    {
        memset(pOutput, 0, ma_get_bytes_per_frame(DEVICE_FORMAT, DEVICE_CHANNELS) * frameCount);
    }

    ma_encoder_write_pcm_frames(&ud->encoder, pOutput, frameCount, NULL);
    cout << "Index0: " << ud->wavIndexes[0] << endl;
    cout << "Index1: " << ud->wavIndexes[1] << endl;

    (void)pInput;   /* Unused. */
}

int main()
{
    /*
     ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS)
    {
        return -1;
    }

    ma_engine_play_sound(&engine, "Horror.wav", NULL);

    printf("Press Enter to quit...");
    getchar();

    ma_engine_uninit(&engine);

    return 0;
    */

    UserData ud;
    ma_waveform sineWave;
    ma_device_config deviceConfig;
    ma_device device;
    //ma_waveform_config sineWaveConfig;

    ma_encoder_config encConfig = ma_encoder_config_init(ma_encoding_format_wav,
        DEVICE_FORMAT,
        DEVICE_CHANNELS,
        DEVICE_SAMPLE_RATE);

    if (ma_encoder_init_file("MySound.wav", &encConfig, &ud.encoder) != MA_SUCCESS) {
        std::cout << "Failed to create WAV file." << std::endl;
        return -1;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &ud;


    if(ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }

    printf("Device Name: %s\n", device.playback.name);

    ma_waveform_config sineWaveConfig = ma_waveform_config_init(
        DEVICE_FORMAT,
        DEVICE_CHANNELS,
        DEVICE_SAMPLE_RATE,
        ma_waveform_type_square,
        0.1,
        120
    );
    ma_waveform_init(&sineWaveConfig, &ud.waves[0]);


    ma_waveform_config sineWaveConfig2 = ma_waveform_config_init(
        DEVICE_FORMAT,
        DEVICE_CHANNELS,
        DEVICE_SAMPLE_RATE,
        ma_waveform_type_triangle,
        0.1,
        220
    );
    ma_waveform_init(&sineWaveConfig2, &ud.waves[1]);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }

    bool running = true;
    while (running)
    {

        ud.wavIndexes[0] = (GetKeyState('A') & 0x8000);
        ud.wavIndexes[1] = (GetKeyState('D') & 0x8000);
        ud.isPlaying = ud.wavIndexes[0] || ud.wavIndexes[1];
        if (GetKeyState('B') & 0x8000)
        {
            running = false;
        }
    }

    ma_device_uninit(&device);
    ma_encoder_uninit(&ud.encoder); // <<< ganz wichtig!
}

