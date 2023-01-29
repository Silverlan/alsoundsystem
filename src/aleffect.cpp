/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
