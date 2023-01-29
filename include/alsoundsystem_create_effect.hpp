/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUNDSYSTEM_CREATE_EFFECT_HPP__
#define __ALSOUNDSYSTEM_CREATE_EFFECT_HPP__

#include "alsoundsystem.hpp"
#include "alsound_effect.hpp"

template<class TEfxProperties>
al::PEffect al::ISoundSystem::CreateEffect(const TEfxProperties &props)
{
	auto effect = CreateEffect();
	if(effect == nullptr)
		return nullptr;
	effect->SetProperties(props);
	m_effects.push_back(effect->GetHandle());
	return effect;
}

#endif
