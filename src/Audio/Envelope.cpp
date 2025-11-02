#include "Envelope.h"
#include "imgui/imgui.h"

Envelope::Envelope(float attackTime, float decayTime, float sustainLevel, float releaseTime)
	: m_Attack(attackTime), m_Decay(decayTime), m_Sustain(sustainLevel), m_Release(releaseTime),
	m_Value(0.0f), m_State(State::Idle)
{
}

void Envelope::Trigger(bool on) {
    if (on && m_State != State::Attack && m_State != State::Decay && m_State != State::Sustain) {
        m_State = State::Attack;
    }
    else if (!on && (m_State == State::Attack || m_State == State::Sustain || m_State == State::Decay)) {
        m_State = State::Release;
    }
}

void Envelope::Update(float dt) {
    switch (m_State) {
    case State::Attack:
        m_Value += dt / std::max(m_Attack, 0.001f);
        if (m_Value >= 1.0f) {
            m_Value = 1.0f;
            m_State = (m_Decay > 0.0f) ? State::Decay : State::Sustain;
        }
        break;
    case State::Decay:
        m_Value -= dt * (1.0f - m_Sustain) / std::max(m_Decay, 0.001f);
        if (m_Value <= m_Sustain) {
            m_Value = m_Sustain;
            m_State = State::Sustain;
        }
        break;
    case State::Sustain:
        // wartoœæ utrzymuje siê — brak zmian
        break;
    case State::Release:
        m_Value -= dt / std::max(m_Release, 0.001f);
        if (m_Value <= 0.0f) {
            m_Value = 0.0f;
            m_State = State::Idle;
        }
        break;
    case State::Idle:
    default:
        break;
    }
}

void Envelope::SetParams(float attack, float decay, float sustain, float release) {
	m_Attack = attack;
	m_Decay = decay;
	m_Sustain = sustain;
	m_Release = release;
}

void Envelope::ImGuiControls(const std::string& name) {
	if (ImGui::CollapsingHeader(name.c_str())) {
		ImGui::SliderFloat("Attack", &m_Attack, 0.01f, 5.0f);
		ImGui::SliderFloat("Decay", &m_Decay, 0.01f, 5.0f);
		ImGui::SliderFloat("Sustain", &m_Sustain, 0.0f, 1.0f);
		ImGui::SliderFloat("Release", &m_Release, 0.01f, 5.0f);
	}
}