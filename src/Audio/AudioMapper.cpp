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




// -------------------- ŁADOWANIE DANYCH AUDIO --------------------

bool AudioMapper::LoadFromJSON(const std::string& path, bool onlyStats)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku JSON: " << path << std::endl;
        return false;
    }

    json j;
    file >> j;

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

	if (onlyStats)
		return true;
    
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


// -------------------- MAPOWANIE DANYCH AUDIO NA PARAMETRY WIZUALNE --------------------

float AudioMapper::MapValue(AudioVisualParam param, int frameIndex) const {
    switch (param) {
    case AudioVisualParam::SUN_EMISSION:
        return MapSunEmission(frameIndex);
	case AudioVisualParam::PLANET_SCALE:
		return MapPlanetScale(frameIndex);
	case AudioVisualParam::ORBIT_SHAKE:
		return MapOrbitShake(frameIndex);
	case AudioVisualParam::PLANET_COLOR:
		return MapPlanetColorShift(frameIndex);
	case AudioVisualParam::SPECULAR_INTENSITY:
		return MapSpecularIntensity(frameIndex);
	case AudioVisualParam::NOISE_AMOUNT:
		return MapNoiseAmount(frameIndex);
	case AudioVisualParam::ATMOSPHERE_ALPHA:
		return MapAtmosphereAlpha(frameIndex);
	case AudioVisualParam::ORBIT_RADIUS:
		return MapOrbitRadius(frameIndex);
	case AudioVisualParam::ROTATION_SPEED:
		return MapRotationSpeed(frameIndex);
	default:
		break;
	}
    return 0.0f;
}

// --- Mapowania ---

float AudioMapper::MapSunEmission(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float norm = NormalizeRMS(f.rms);
    float normPrev = NormalizeRMS(fprev.rms);
    float rms = Smooth(normPrev, norm, 0.15f);

    return glm::mix(0.5f, 6.0f, pow(rms, 1.0f)); // bloom intensity
}

float AudioMapper::MapPlanetScale(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size()) return 1.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float norm = NormalizeBand(f.bands.bass, m_BandStats.bass_min, m_BandStats.bass_max);
    float normPrev = NormalizeBand(f.bands.bass, m_BandStats.bass_min, m_BandStats.bass_max);

    float bass = Smooth(normPrev, norm, m_SmoothAlpha);
    return glm::mix(1.0f, 1.8f, norm); // scale multiplier
}

float AudioMapper::MapOrbitShake(int frameIndex) const {
    if (frameIndex <= 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float normPrev = NormalizeBand(fprev.bands.sub_bass, m_BandStats.sub_bass_min, m_BandStats.sub_bass_max);
    float norm = NormalizeBand(f.bands.sub_bass, m_BandStats.sub_bass_min, m_BandStats.sub_bass_max);
    float sub = Smooth(normPrev, norm, m_SmoothAlpha);

    return glm::mix(0.0f, 0.5f, sub); // orbit shake amplitude
}

float AudioMapper::MapPlanetColorShift(int frameIndex) const {
    if (frameIndex <= 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float normPrev = NormalizeBand(fprev.bands.mid, m_BandStats.mid_min, m_BandStats.mid_max);
    float norm = NormalizeBand(f.bands.mid, m_BandStats.mid_min, m_BandStats.mid_max);
    float mid = Smooth(normPrev, norm, m_SmoothAlpha);

    return mid; // used for color / hue modulation
}

float AudioMapper::MapSpecularIntensity(int frameIndex) const {
    if (frameIndex <= 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float normPrev = NormalizeBand(fprev.bands.high_mid, m_BandStats.high_mid_min, m_BandStats.high_mid_max);
    float norm = NormalizeBand(f.bands.high_mid, m_BandStats.high_mid_min, m_BandStats.high_mid_max);
    float highMid = Smooth(normPrev, norm, m_SmoothAlpha);

    return glm::mix(0.1f, 2.0f, highMid); // rim / specular highlight
}

float AudioMapper::MapNoiseAmount(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size()) return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float normPrev = NormalizeBand(fprev.bands.presence, m_BandStats.presence_min, m_BandStats.presence_max);
    float norm = NormalizeBand(f.bands.presence, m_BandStats.presence_min, m_BandStats.presence_max);
	float presence = Smooth(normPrev, norm, m_SmoothAlpha);
    return glm::mix(0.0f, 1.0f, presence);
}

float AudioMapper::MapAtmosphereAlpha(int frameIndex) const {
    if (frameIndex <= 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
    const auto& fprev = (frameIndex == 0) ? f : m_Frames[frameIndex - 1];

    float normPrev = NormalizeBand(fprev.bands.air, m_BandStats.air_min, m_BandStats.air_max);
    float norm = NormalizeBand(f.bands.air, m_BandStats.air_min, m_BandStats.air_max);
    float air = Smooth(normPrev, norm, m_SmoothAlpha);

    return glm::mix(0.2f, 1.0f, air); // atmosphere alpha / halo
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

bool AudioMapper::IsBeat(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return false;

    return m_Frames[frameIndex].is_beat;
}

float AudioMapper::MapBandForPlanet(int frameIndex, int bandIndex) const {
    if (frameIndex < 0 || frameIndex >= m_Frames.size())
        return 0.0f;

    const auto& f = m_Frames[frameIndex];
	Band band = static_cast<Band>(bandIndex);

    return GetSmoothedBandByType(band);
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

// MFCC Delta (typowo -40 – 70)
float AudioMapper::NormalizeMFCCDelta(float value) const {
    // przesuń i przeskaluj do zakresu 0–1
    float norm = (value + 40.0f) / (70.0f + 40.0f); // przesunięcie o 40 w górę, zakres 110
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

float AudioMapper::GetSmoothedBandByType(Band band) const {
    switch (band) {
    case Band::SUB_BASS:
        return m_SmoothedBands.sub_bass;
	case Band::BASS:
		return m_SmoothedBands.bass;
	case Band::LOW_MID:
		return m_SmoothedBands.low_mid;
	case Band::MID:
		return m_SmoothedBands.mid;
	case Band::HIGH_MID:
		return m_SmoothedBands.high_mid;
	case Band::PRESENCE:
		return m_SmoothedBands.presence;
	case Band::BRILLIANCE:
		return m_SmoothedBands.brilliance;
	case Band::AIR:
		return m_SmoothedBands.air;
    default:
        return 0.0f;
    }
}

// wersja ogólna
//float AudioMapper::GetSmoothedBand(Band band) const {
//    const float* ptr = reinterpret_cast<const float*>(&m_SmoothedBands);
//    return ptr[static_cast<int>(band)];
//}

// -------------------- WYGŁADZANIE --------------------

void AudioMapper::UpdateSmoothedBands(int frameIndex) {
    const auto& f = m_Frames[frameIndex];
    float alpha = m_SmoothAlpha;

    auto smooth = [&](float prev, float cur, float min, float max) {
        float norm = NormalizeBand(cur, min, max);
        return Smooth(prev, norm, alpha);
        };

    m_SmoothedBands.sub_bass = smooth(m_SmoothedBands.sub_bass, f.bands.sub_bass, m_BandStats.sub_bass_min, m_BandStats.sub_bass_max);
    m_SmoothedBands.bass = smooth(m_SmoothedBands.bass, f.bands.bass, m_BandStats.bass_min, m_BandStats.bass_max);
    m_SmoothedBands.low_mid = smooth(m_SmoothedBands.low_mid, f.bands.low_mid, m_BandStats.low_mid_min, m_BandStats.low_mid_max);
    m_SmoothedBands.mid = smooth(m_SmoothedBands.mid, f.bands.mid, m_BandStats.mid_min, m_BandStats.mid_max);
    m_SmoothedBands.high_mid = smooth(m_SmoothedBands.high_mid, f.bands.high_mid, m_BandStats.high_mid_min, m_BandStats.high_mid_max);
    m_SmoothedBands.presence = smooth(m_SmoothedBands.presence, f.bands.presence, m_BandStats.presence_min, m_BandStats.presence_max);
    m_SmoothedBands.brilliance = smooth(m_SmoothedBands.brilliance, f.bands.brilliance, m_BandStats.brilliance_min, m_BandStats.brilliance_max);
    m_SmoothedBands.air = smooth(m_SmoothedBands.air, f.bands.air, m_BandStats.air_min, m_BandStats.air_max);
}
