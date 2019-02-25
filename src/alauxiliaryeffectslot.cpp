/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_auxiliaryeffectslot.hpp"
#include "alsound_effect.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#endif

al::AuxiliaryEffectSlot::AuxiliaryEffectSlot(alure::AuxiliaryEffectSlot *slot)
	: m_slot(slot)
{}

al::AuxiliaryEffectSlot::~AuxiliaryEffectSlot()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_slot->destroy();
#endif
}

const alure::AuxiliaryEffectSlot &al::AuxiliaryEffectSlot::GetALSlot() const {return const_cast<AuxiliaryEffectSlot*>(this)->GetALSlot();}
alure::AuxiliaryEffectSlot &al::AuxiliaryEffectSlot::GetALSlot() {return *m_slot;}

void al::AuxiliaryEffectSlot::SetGain(float gain)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_slot->setGain(m_gain = umath::clamp(gain,0.f,1.f));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
float al::AuxiliaryEffectSlot::GetGain() const {return m_gain;}

void al::AuxiliaryEffectSlot::SetSendAuto(bool bAuto)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_slot->setSendAuto(m_bSendAuto = bAuto);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
bool al::AuxiliaryEffectSlot::GetSendAuto() const {return m_bSendAuto;}

void al::AuxiliaryEffectSlot::ApplyEffect(const Effect &effect)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_slot->applyEffect(effect.GetALEffect());
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

bool al::AuxiliaryEffectSlot::IsInUse() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_slot->getUseCount() > 0;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return true;
#endif
}
