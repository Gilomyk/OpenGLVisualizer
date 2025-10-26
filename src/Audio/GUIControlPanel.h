// GUIControlPanel.h
#pragma once
#include <array>
#include <string>

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
        float sub_bass = 100.0f;
        float bass = 500.0f;
        float low_mid = 1000.0f;
        float mid = 2000.0f;
        float high_mid = 4000.0f;
        float presence = 6000.0f;
        float brilliance = 8000.0f;
        float air = 10000.0f;
    } bands;

    std::array<float, 5> fft_peak_freq{};
    std::array<float, 5> fft_peak_amp{};
    std::array<float, 7> spectral_contrast{};
    std::array<float, 12> chroma{};
    std::array<float, 13> mfcc{};
    std::array<float, 13> mfcc_delta{};

    GUIControlPanel()
    {
        fft_peak_freq.fill(0.0f);
        fft_peak_amp.fill(0.0f);
        spectral_contrast.fill(0.0f);
        chroma.fill(0.0f);
        mfcc.fill(0.0f);
        mfcc_delta.fill(0.0f);
    }

    void DrawImGUI();
};
