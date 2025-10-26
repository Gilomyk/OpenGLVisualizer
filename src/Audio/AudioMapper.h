#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <json/json.hpp>

// Struktura pasm audio
struct AudioBands {
    float sub_bass = 0.0f;
    float bass = 0.0f;
    float low_mid = 0.0f;
    float mid = 0.0f;
    float high_mid = 0.0f;
    float presence = 0.0f;
    float brilliance = 0.0f;
    float air = 0.0f;
};


// Struktura przechowuj¹ca dane jednej klatki audio
struct AudioFrame {
    float time = 0.0f;
    float rms = 0.0f;
    AudioBands bands;
    std::array<float, 5> fft_peak_freq{};
    std::array<float, 5> fft_peak_amp{};
    float spectral_centroid = 0.0f;
    float spectral_bandwidth = 0.0f;
    float spectral_rolloff = 0.0f;
    float spectral_flatness = 0.0f;
    float local_tempo = 0.0f;
    bool is_onset = false;
    bool is_beat = false;
    std::array<float, 7> spectral_contrast{};
    std::array<float, 12> chroma{};
    std::array<float, 13> mfcc{};
    std::array<float, 13> mfcc_delta{};

    AudioFrame()
    {
        fft_peak_freq.fill(0.0f);
        fft_peak_amp.fill(0.0f);
        spectral_contrast.fill(0.0f);
        chroma.fill(0.0f);
        mfcc.fill(0.0f);
        mfcc_delta.fill(0.0f);
    }
};

// Klasa mapuj¹ca dane audio na parametry wizualne
class AudioMapper {
public:
    AudioMapper() = default;

    bool LoadFromJSON(const std::string& path);

    // Zwraca czy wyst¹pi³o uderzenie dŸwiêku (beat)
    bool IsBeat(int frameIndex) const;

    // Liczba za³adowanych ramek
    size_t GetFrameCount() const { return m_Frames.size(); }

	// Pobiera ramkê audio
	AudioFrame GetFrame(int frameIndex) {
		if (frameIndex < 0 || frameIndex >= m_Frames.size())
			return AudioFrame();
		return m_Frames[frameIndex];
	}

    // Zwraca sta³¹ referencjê (do przegl¹dania)
    const std::vector<AudioFrame>& GetFrames() const { return m_Frames; }

    // --- Mapowania ---
    glm::vec3 MapColor(int frameIndex) const;
    float MapEmission(int frameIndex) const;
    float MapOrbitRadius(int frameIndex) const;
    float MapRotationSpeed(int frameIndex) const;

private:
    std::vector<AudioFrame> m_Frames;

    // --- Normalizacje ---
    float NormalizeRMS(float value) const;
    float NormalizeCentroid(float value) const;
    float NormalizeBandwidth(float value) const;
    float NormalizeRolloff(float value) const;
    float NormalizeFlatness(float value) const;
    float NormalizeTempo(float value) const;
    float NormalizeContrast(float value) const;
    float NormalizeChroma(float value) const;
    float NormalizeMFCC(float value) const;
};
