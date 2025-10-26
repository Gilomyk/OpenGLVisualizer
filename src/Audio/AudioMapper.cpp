#include "AudioMapper.h"
#include "imgui/imgui.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

template <typename T>
T clamp(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// -------------------- ŁADOWANIE DANYCH AUDIO --------------------

bool AudioMapper::LoadFromJSON(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku JSON: " << path << std::endl;
        return false;
    }

    json j;
    file >> j;
    
    const auto& frames = j["frames"];
    m_Frames.reserve(frames.size());

    for (const auto& f : frames) {
        AudioFrame frame;

        if (f.contains("time")) frame.time = f.value("time", 0.0f);
        if (f.contains("rms")) frame.rms = f.value("rms", 0.0f);
        if (f.contains("spectral_centroid")) frame.spectral_centroid = f.value("spectral_centroid", 0.0f);
        if (f.contains("spectral_bandwidth")) frame.spectral_bandwidth = f.value("spectral_bandwidth", 0.0f);
        if (f.contains("spectral_rolloff")) frame.spectral_rolloff = f.value("spectral_rolloff", 0.0f);
        if (f.contains("spectral_flatness")) frame.spectral_flatness = f.value("spectral_flatness", 0.0f);
        if (f.contains("local_tempo")) frame.local_tempo = f.value("local_tempo", 0.0f);
        if (f.contains("is_onset")) frame.is_onset = f.value("is_onset", false);
        if (f.contains("is_beat")) frame.is_beat = f.value("is_beat", false);

        if (f.contains("bands")) {
            const auto& b = f["bands"];
            frame.bands.sub_bass = b.value("sub_bass", 0.0f);
            frame.bands.bass = b.value("bass", 0.0f);
            frame.bands.low_mid = b.value("low_mid", 0.0f);
            frame.bands.mid = b.value("mid", 0.0f);
            frame.bands.high_mid = b.value("high_mid", 0.0f);
            frame.bands.presence = b.value("presence", 0.0f);
            frame.bands.brilliance = b.value("brilliance", 0.0f);
            frame.bands.air = b.value("air", 0.0f);
        }

        if (f.contains("fft_peaks")) {
            const auto& peaks = f["fft_peaks"];
            for (size_t i = 0; i < 5 && i < peaks.size(); ++i) {
                frame.fft_peak_freq[i] = peaks[i][0].get<float>(); // częstotliwość
                frame.fft_peak_amp[i] = peaks[i][1].get<float>(); // amplituda
            }
        }

        if (f.contains("spectral_contrast"))
            frame.spectral_contrast = f["spectral_contrast"].get<std::array<float, 7>>();

        if (f.contains("chroma"))
            frame.chroma = f["chroma"].get<std::array<float, 12>>();

		if (f.contains("mfcc"))
			frame.mfcc = f["mfcc"].get<std::array<float, 13>>();
        
		if (f.contains("mfcc_delta"))
			frame.mfcc_delta = f["mfcc_delta"].get<std::array<float, 13>>();

        m_Frames.push_back(frame);
    }

    std::cout << "Załadowano " << m_Frames.size() << " ramek audio." << std::endl;
    return true;
}


// -------------------- MAPOWANIA --------------------

glm::vec3 AudioMapper::MapColor(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return glm::vec3(0.2f);

    const auto& f = m_Frames[frameIndex];
    float t = NormalizeCentroid(f.spectral_centroid);

    // zimne barwy (niski centroid) → ciepłe barwy (wysoki centroid)
    glm::vec3 cold(0.1f, 0.2f, 0.8f);
    glm::vec3 warm(1.0f, 0.7f, 0.2f);

    return glm::mix(cold, warm, t);
}

float AudioMapper::MapEmission(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    return NormalizeRMS(f.rms);
}

float AudioMapper::MapOrbitRadius(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    float tempo = NormalizeTempo(f.local_tempo);
    return glm::mix(50.0f, 300.0f, tempo); // promień orbity
}

float AudioMapper::MapRotationSpeed(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    float contrast = NormalizeContrast(f.spectral_contrast[0]);
    return glm::mix(0.1f, 2.5f, contrast); // szybkość obrotu (rad/s)
}

bool AudioMapper::IsBeat(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return false;

    return m_Frames[frameIndex].is_beat;
}

// -------------------- NORMALIZACJE --------------------

// RMS zwykle w zakresie 0.0 – 0.1
float AudioMapper::NormalizeRMS(float value) const {
    return clamp(value / 0.1f, 0.0f, 1.0f);
}

// Spektralny centroid (typowo 0 – 11000 Hz)
float AudioMapper::NormalizeCentroid(float value) const {
    return clamp(value / 11000.0f, 0.0f, 1.0f);
}

// Spektralna szerokość pasma (typowo 0 – 7000 Hz)
float AudioMapper::NormalizeBandwidth(float value) const {
    return clamp(value / 7000.0f, 0.0f, 1.0f);
}

// Rolloff (często ~0–20000 Hz)
float AudioMapper::NormalizeRolloff(float value) const {
    return clamp(value / 20000.0f, 0.0f, 1.0f);
}

// Flatness (0.0 – 1.0)
float AudioMapper::NormalizeFlatness(float value) const {
    return clamp(value, 0.0f, 1.0f);
}

// Tempo lokalne (np. 0.0 – 1.0 jeśli już znormalizowane, inaczej /200)
float AudioMapper::NormalizeTempo(float value) const {
    return clamp(value / 200.0f, 0.0f, 1.0f);
}

// Kontrast spektralny (0 – 50 dB)
float AudioMapper::NormalizeContrast(float value) const {
    return clamp(value / 50.0f, 0.0f, 1.0f);
}

// Chroma (0 – 1)
float AudioMapper::NormalizeChroma(float value) const {
    return clamp(value, 0.0f, 1.0f);
}

// MFCC (np. -600 – 220)
float AudioMapper::NormalizeMFCC(float value) const {
    float norm = (value + 600.0f) / (220.0f + 600.0f);
    return clamp(norm, 0.0f, 1.0f);
}
