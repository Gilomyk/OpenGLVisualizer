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

static float Smooth(float prev, float curr, float alpha) {
    return prev + alpha * (curr - prev);
}

static float Clamp01(float v) {
    return clamp(v, 0.0f, 1.0f);
}

// algorytmy normalizacji

static float LogNormalize(float value, float minVal, float maxVal) {
    float logMin = std::log10(minVal + 1.0f);
    float logMax = std::log10(maxVal + 1.0f);
    float logVal = std::log10(value + 1.0f);
    return Clamp01((logVal - logMin) / (logMax - logMin));
}

static float NormalizeBandSoft(float value, float minVal, float maxVal, float gamma = 0.6f) {
    float norm = (value - minVal) / (maxVal - minVal);
    norm = clamp(norm, 0.0f, 1.0f);
    return std::pow(norm, gamma);
}

static float NormalizeBandHybrid(float value, float minVal, float maxVal, float blend = 0.01f) {
    float lin = clamp((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    float logMin = std::log10(minVal + 1.0f);
    float logMax = std::log10(maxVal + 1.0f);
    float logVal = std::log10(value + 1.0f);
    float logr = (logVal - logMin) / (logMax - logMin);
    return clamp(blend * logr + (1.0f - blend) * lin, 0.0f, 1.0f);
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

	const auto& s = j["stats"];
	if (s.contains("bands-sub_bass")) {
		const auto& bandStats = s["bands-sub_bass"];
		m_BandStats.sub_bass_min = bandStats.value("min", 0.0f);
		m_BandStats.sub_bass_max = bandStats.value("max", 1.0f);
	}
    if (s.contains("bands-bass")) {
        const auto& bandStats = s["bands-bass"];
        m_BandStats.bass_min = bandStats.value("min", 0.0f);
        m_BandStats.bass_max = bandStats.value("max", 1.0f);
    }
	if (s.contains("bands-low_mid")) {
		const auto& bandStats = s["bands-low_mid"];
		m_BandStats.low_mid_min = bandStats.value("min", 0.0f);
		m_BandStats.low_mid_max = bandStats.value("max", 1.0f);
	}
	if (s.contains("bands-mid")) {
		const auto& bandStats = s["bands-mid"];
		m_BandStats.mid_min = bandStats.value("min", 0.0f);
		m_BandStats.mid_max = bandStats.value("max", 1.0f);
	}
    if (s.contains("bands-high_mid")) {
        const auto& bandStats = s["bands-high_mid"];
        m_BandStats.high_mid_min = bandStats.value("min", 0.0f);
        m_BandStats.high_mid_max = bandStats.value("max", 1.0f);
    }
	if (s.contains("bands-presence")) {
		const auto& bandStats = s["bands-presence"];
		m_BandStats.presence_min = bandStats.value("min", 0.0f);
		m_BandStats.presence_max = bandStats.value("max", 1.0f);
	}
    if (s.contains("bands-brilliance")) {
        const auto& bandStats = s["bands-brilliance"];
        m_BandStats.brilliance_min = bandStats.value("min", 0.0f);
        m_BandStats.brilliance_max = bandStats.value("max", 1.0f);
    }
    if (s.contains("bands-air")) {
        const auto& bandStats = s["bands-air"];
        m_BandStats.air_min = bandStats.value("min", 0.0f);
        m_BandStats.air_max = bandStats.value("max", 1.0f);
    }
	printf("Zapisano statystyki pasma audio.\n");
    

    return true;
}


// -------------------- MAPOWANIA --------------------

float AudioMapper::MapValue(AudioVisualParam param, int frameIndex) const {
    const auto& f = m_Frames[frameIndex];
    switch (param) {
    case AudioVisualParam::SUN_EMISSION:
        return NormalizeRMS(f.rms);
    case AudioVisualParam::PLANET_SCALE:
        return NormalizeRMS(f.rms * 0.5f + f.bands.bass);
    case AudioVisualParam::PLANET_ROTATION:
        return NormalizeTempo(f.local_tempo);
    case AudioVisualParam::PLANET_COLOR:
        return NormalizeCentroid(f.spectral_centroid);
    case AudioVisualParam::ORBIT_RADIUS:
        return NormalizeBandwidth(f.spectral_bandwidth);
    case AudioVisualParam::SCENE_SATURATION:
        return NormalizeFlatness(f.spectral_flatness);
    }
    return 0.0f;
}

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
    return clamp(value, 0.0f, 1.0f);
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

// Uniwersalna funkcja logarytmicznej normalizacji energii
float AudioMapper::NormalizeBand(float value, float minVal, float maxVal) const {
    float logMin = std::log10(minVal + 1.0f);
    float logMax = std::log10(maxVal + 1.0f);
    float logVal = std::log10(value + 1.0f);
    float norm = (logVal - logMin) / (logMax - logMin);
    return clamp(norm, 0.0f, 1.0f);
}

// -------------------- WYGŁADZANIE --------------------

AudioFrame AudioMapper::GetSmoothedFrame(int frameIndex) {
    AudioFrame frame = GetFrame(frameIndex);
    AudioFrame result = frame; // kopia do modyfikacji

    // --- Normalizacja pasm na podstawie m_BandStats ---
    AudioBands norm;
    norm.sub_bass = LogNormalize(frame.bands.sub_bass, m_BandStats.sub_bass_min, m_BandStats.sub_bass_max);
    norm.bass = LogNormalize(frame.bands.bass, m_BandStats.bass_min, m_BandStats.bass_max);
    norm.low_mid = LogNormalize(frame.bands.low_mid, m_BandStats.low_mid_min, m_BandStats.low_mid_max);
    norm.mid = LogNormalize(frame.bands.mid, m_BandStats.mid_min, m_BandStats.mid_max);
    norm.high_mid = LogNormalize(frame.bands.high_mid, m_BandStats.high_mid_min, m_BandStats.high_mid_max);
    norm.presence = LogNormalize(frame.bands.presence, m_BandStats.presence_min, m_BandStats.presence_max);
    norm.brilliance = LogNormalize(frame.bands.brilliance, m_BandStats.brilliance_min, m_BandStats.brilliance_max);
    norm.air = LogNormalize(frame.bands.air, m_BandStats.air_min, m_BandStats.air_max);

    // --- Wygładzanie (exponential moving average) ---
    float alpha = m_SmoothAlpha;
    m_SmoothedBands.sub_bass = Smooth(m_SmoothedBands.sub_bass, norm.sub_bass, alpha);
    m_SmoothedBands.bass = Smooth(m_SmoothedBands.bass, norm.bass, alpha);
    m_SmoothedBands.low_mid = Smooth(m_SmoothedBands.low_mid, norm.low_mid, alpha);
    m_SmoothedBands.mid = Smooth(m_SmoothedBands.mid, norm.mid, alpha);
    m_SmoothedBands.high_mid = Smooth(m_SmoothedBands.high_mid, norm.high_mid, alpha);
    m_SmoothedBands.presence = Smooth(m_SmoothedBands.presence, norm.presence, alpha);
    m_SmoothedBands.brilliance = Smooth(m_SmoothedBands.brilliance, norm.brilliance, alpha);
    m_SmoothedBands.air = Smooth(m_SmoothedBands.air, norm.air, alpha);

    // --- Zapis do zwracanego frame’a ---
    result.bands = m_SmoothedBands;

    // Dodatkowo możesz też tu znormalizować RMS, centroid itp.:
    result.rms = NormalizeRMS(frame.rms);
    result.spectral_centroid = NormalizeCentroid(frame.spectral_centroid);
    result.spectral_bandwidth = NormalizeBandwidth(frame.spectral_bandwidth);

    return result;
}
