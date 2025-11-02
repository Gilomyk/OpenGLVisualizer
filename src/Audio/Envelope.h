#pragma once
#include <algorithm>
#include <string>

class Envelope {
public:
	enum class State {
		Idle,
		Attack,
		Decay,
		Sustain,
		Release
	};

	Envelope(float attackTime = 0.1f, float decayTime = 0.1f, float sustainLevel = 0.7f, float releaseTime = 0.2f);

	void Trigger(bool on);

	void Update(float dt);

	float GetValue() const { return m_Value; }

	State GetState() const { return m_State; }

	void SetParams(float attack, float decay, float sustain, float release);

	void ImGuiControls(const std::string& name);

private:
	float m_Attack, m_Decay, m_Sustain, m_Release;
	float m_Value;
	State m_State;
};

