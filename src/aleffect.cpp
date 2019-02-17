/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_effect.hpp"
#include "alsound_source.hpp"
#include "alsound_auxiliaryeffectslot.hpp"
#include "alsoundsystem.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#endif

namespace al
{
	DEFINE_BASE_HANDLE(DLLALSYS,Effect,Effect);
};

al::Effect::Params::Params(float pgain,float pgainHF,float pgainLF)
	: gain(pgain),gainHF(pgainHF),gainLF(pgainLF)
{}

/////////////////////

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
al::Effect::Effect(SoundSystem &soundSys,alure::Effect *effect)
	: m_handle(this),m_soundSystem(soundSys),m_effect(effect)
{}
const alure::Effect &al::Effect::GetALEffect() const {return const_cast<Effect*>(this)->GetALEffect();}
alure::Effect &al::Effect::GetALEffect() {return *m_effect;}
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
const std::shared_ptr<FMOD::DSP> &al::Effect::GetFMODDsp() const {return const_cast<al::Effect*>(this)->GetFMODDsp();}
std::shared_ptr<FMOD::DSP> &al::Effect::GetFMODDsp() {return m_fmDsp;}
al::Effect::Effect(SoundSystem &soundSys,const std::shared_ptr<FMOD::DSP> &dsp)
	: m_handle(this),m_soundSystem(soundSys),m_fmDsp(dsp)
{}
#endif

al::Effect::~Effect()
{
	if(m_handle.IsValid())
		m_handle.Invalidate();
	while(m_attachedSources.empty() == false)
		DetachSource(*m_attachedSources.front().get());
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_effect->destroy();
#endif
}

void al::Effect::SetProperties(EfxEaxReverbProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_effect->setReverbProperties(reinterpret_cast<EFXEAXREVERBPROPERTIES&>(props));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

al::AuxiliaryEffectSlot *al::Effect::AttachSource(SoundSource &source)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto it = std::find(m_attachedSources.begin(),m_attachedSources.end(),&source);
	if(it != m_attachedSources.end())
		return nullptr;
	if(m_slot == nullptr)
	{
		auto *slot = m_soundSystem.CreateAuxiliaryEffectSlot();
		if(slot == nullptr)
			return nullptr;
		m_slot = slot;
		m_slot->ApplyEffect(*this);
	}
	m_attachedSources.push_back(&source);
	return m_slot;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}
void al::Effect::DetachSource(SoundSource &source)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto it = std::find(m_attachedSources.begin(),m_attachedSources.end(),&source);
	if(it == m_attachedSources.end())
		return;
	m_attachedSources.erase(it);
	if(m_attachedSources.empty() == true && m_slot != nullptr)
	{
		m_soundSystem.FreeAuxiliaryEffectSlot(m_slot);
		m_slot = nullptr;
	}
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
al::EffectHandle al::Effect::GetHandle() const {return m_handle;}
void al::Effect::Release() {delete this;}
