// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

import :effect;
import :source;

pragma::audio::EffectParams::EffectParams(float pgain, float pgainHF, float pgainLF) : gain(pgain), gainHF(pgainHF), gainLF(pgainLF) {}

/////////////////////

pragma::audio::IEffect::IEffect(ISoundSystem &soundSys) : m_soundSystem {soundSys} {}
pragma::audio::IEffect::~IEffect()
{
	if(m_handle.IsValid())
		m_handle.Invalidate();
}

pragma::audio::EffectHandle pragma::audio::IEffect::GetHandle() const { return m_handle; }
void pragma::audio::IEffect::Release()
{
	while(m_attachedSources.empty() == false)
		DetachSource(m_attachedSources.front()->GetChannel());
	delete this;
}
