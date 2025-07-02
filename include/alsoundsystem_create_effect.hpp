// SPDX-FileCopyrightText: © 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
