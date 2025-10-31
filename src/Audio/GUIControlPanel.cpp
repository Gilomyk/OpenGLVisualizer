// GUIControlPanel.cpp
#include "GUIControlPanel.h"
#include "imgui/imgui.h"

void GUIControlPanel::setBandRanges(AudioBandsMaxMin stats) {
    bandRanges.sub_bass_min = stats.sub_bass_min;
    bandRanges.sub_bass_max = stats.sub_bass_max;
	bandRanges.bass_min = stats.bass_min;
	bandRanges.bass_max = stats.bass_max;
	bandRanges.low_mid_min = stats.low_mid_min;
	bandRanges.low_mid_max = stats.low_mid_max;
	bandRanges.mid_min = stats.mid_min;
	bandRanges.mid_max = stats.mid_max;
	bandRanges.high_mid_min = stats.high_mid_min;
	bandRanges.high_mid_max = stats.high_mid_max;
	bandRanges.presence_min = stats.presence_min;
	bandRanges.presence_max = stats.presence_max;
	bandRanges.brilliance_min = stats.brilliance_min;
	bandRanges.brilliance_max = stats.brilliance_max;
	bandRanges.air_min = stats.air_min;
	bandRanges.air_max = stats.air_max;
}

void GUIControlPanel::DrawImGUI()
{
    static float globalScale = 1.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(1400, 900));

    ImGui::Begin("Audio Control Panel");
    ImGui::SliderFloat("Global Scale", &globalScale, 0.5f, 2.0f);
    ImGui::SetWindowFontScale(globalScale);

    if (ImGui::BeginTabBar("AudioTabs")) {

        if (ImGui::BeginTabItem("General")) {
            ImGui::SliderFloat("RMS", &rms, 0.0f, 1.0f);
            ImGui::SliderFloat("Spectral Centroid", &spectral_centroid, 1000.0f, 11000.0f);
            ImGui::SliderFloat("Spectral Bandwidth", &spectral_bandwidth, 1300.0f, 6500.0f);
            ImGui::SliderFloat("Spectral Rolloff", &spectral_rolloff, 1500.0f, 19000.0f);
            ImGui::SliderFloat("Spectral Flatness", &spectral_flatness, 0.0f, 0.6f);
            ImGui::SliderFloat("Local Tempo", &local_tempo, 0.0f, 1.0f);
            ImGui::Checkbox("Onset", &is_onset);
            ImGui::Checkbox("Beat", &is_beat);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Bands")) {
            ImGui::Text("Frequency Bands (Hz)");
            ImGui::Separator();
            ImGui::SliderFloat("Sub Bass", &bands.sub_bass, bandRanges.sub_bass_min, bandRanges.sub_bass_max);
            ImGui::SliderFloat("Bass", &bands.bass, bandRanges.bass_min, bandRanges.bass_max);
            ImGui::SliderFloat("Low Mid", &bands.low_mid, bandRanges.low_mid_min, bandRanges.low_mid_max);
            ImGui::SliderFloat("Mid", &bands.mid, bandRanges.mid_min, bandRanges.mid_max);
            ImGui::SliderFloat("High Mid", &bands.high_mid, bandRanges.high_mid_min, bandRanges.high_mid_max);
            ImGui::SliderFloat("Presence", &bands.presence, bandRanges.presence_min, bandRanges.presence_max);
            ImGui::SliderFloat("Brilliance", &bands.brilliance, bandRanges.brilliance_min, bandRanges.brilliance_max);
            ImGui::SliderFloat("Air", &bands.air, bandRanges.air_min, bandRanges.air_max);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("FFT Peaks")) {
            for (int i = 0; i < 5; ++i) {
                ImGui::SliderFloat(("Peak Freq " + std::to_string(i)).c_str(), &fft_peak_freq[i], 0.0f, 22050.0f);
                ImGui::SliderFloat(("Peak Amp " + std::to_string(i)).c_str(), &fft_peak_amp[i], 0.0f, 200.0f);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Spectral Contrast")) {
            for (int i = 0; i < 7; ++i)
                ImGui::SliderFloat(("Contrast " + std::to_string(i)).c_str(), &spectral_contrast[i], 0.0f, 50.0f);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Chroma")) {
            for (int i = 0; i < 12; ++i)
                ImGui::SliderFloat(("Chroma " + std::to_string(i)).c_str(), &chroma[i], 0.0f, 1.0f);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("MFCC")) {
            for (int i = 0; i < 13; ++i)
                ImGui::SliderFloat(("MFCC " + std::to_string(i)).c_str(), &mfcc[i], -600.0f, 220.0f);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("MFCC Δ")) {
            for (int i = 0; i < 13; ++i)
                ImGui::SliderFloat(("ΔMFCC " + std::to_string(i)).c_str(), &mfcc_delta[i], -40.0f, 65.0f);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
