// GUIControlPanel.h
#pragma once
#include <array>
#include <string>
#include "AudioMapper.h"

struct GUIControlPanel
{
    float rms = 0.05f;
    float spectral_centroid = 4000.0f;
    float spectral_bandwidth = 3000.0f;
    float spectral_rolloff = 8000.0f;
    float spectral_flatness = 0.3f;
    float local_tempo = 0.5f;
    bool is_onset = false;
    bool is_beat = false;

    struct Bands {
        float sub_bass = 0.0f;
        float bass = 0.0f;
        float low_mid = 0.0f;
        float mid = 0.0f;
        float high_mid = 0.0f;
        float presence = 0.0f;
        float brilliance = 0.0f;
        float air = 0.0f;
    } bands;

    struct BandRanges {
        float sub_bass_min, sub_bass_max;
        float bass_min, bass_max;
        float low_mid_min, low_mid_max;
        float mid_min, mid_max;
        float high_mid_min, high_mid_max;
        float presence_min, presence_max;
        float brilliance_min, brilliance_max;
        float air_min, air_max;
    } bandRanges;

    std::array<float, 5> fft_peak_freq{};
    std::array<float, 5> fft_peak_amp{};
    std::array<float, 7> spectral_contrast{};
    std::array<float, 12> chroma{};
    std::array<float, 13> mfcc{};
    std::array<float, 13> mfcc_delta{};

    GUIControlPanel()
    {
		bandRanges = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
					  0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
        fft_peak_freq.fill(0.0f);
        fft_peak_amp.fill(0.0f);
        spectral_contrast.fill(0.0f);
        chroma.fill(0.0f);
        mfcc.fill(0.0f);
        mfcc_delta.fill(0.0f);
    }

    void setBandRanges(AudioBandsMaxMin stats);
    void DrawImGUI();
};
