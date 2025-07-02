// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "alsound_effect.hpp"
#include "alsound_source.hpp"
#include "alsound_auxiliaryeffectslot.hpp"
#include "alsoundsystem.hpp"

namespace al {
	DEFINE_BASE_HANDLE(DLLALSYS, IEffect, Effect);
};

al::EffectParams::EffectParams(float pgain, float pgainHF, float pgainLF) : gain(pgain), gainHF(pgainHF), gainLF(pgainLF) {}

/////////////////////

al::IEffect::IEffect(ISoundSystem &soundSys) : m_soundSystem {soundSys} {}
al::IEffect::~IEffect()
{
	if(m_handle.IsValid())
		m_handle.Invalidate();
}

al::EffectHandle al::IEffect::GetHandle() const { return m_handle; }
void al::IEffect::Release()
{
	while(m_attachedSources.empty() == false)
		DetachSource(**m_attachedSources.front().get());
	delete this;
}
