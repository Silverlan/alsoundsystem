// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.alsoundsystem;

import :source;
import :system;

al::ISoundSystem::GlobalEffect::SoundInfo::~SoundInfo()
{
	if(relativeCallback.IsValid())
		relativeCallback.Remove();
}

void al::ISoundSystem::ApplyGlobalEffect(SoundSource &source, GlobalEffect &globalEffect)
{
	auto bApply = false;
	if(source->IsRelative() == true) {
		if(((globalEffect.flags & GlobalEffectFlag::RelativeSounds)) != GlobalEffectFlag::None)
			bApply = true;
	}
	else if((globalEffect.flags & GlobalEffectFlag::WorldSounds) != GlobalEffectFlag::None)
		bApply = true;

	auto slotId = std::numeric_limits<uint32_t>::max();
	if(bApply == true)
		source->AddEffect(*globalEffect.effect, slotId, globalEffect.params);
	globalEffect.sourceInfo.push_back(std::shared_ptr<GlobalEffect::SoundInfo>(new GlobalEffect::SoundInfo()));
	auto &info = globalEffect.sourceInfo.back();
	info->source = source.GetHandle();
	auto allSoundFlags = GlobalEffectFlag::RelativeSounds | GlobalEffectFlag::WorldSounds;
	if((globalEffect.flags & allSoundFlags) != allSoundFlags) {
		// Need to update the effect if the sound changes its relative state
		info->relativeCallback = source->AddCallback("OnRelativeChanged", FunctionCallback<void, bool>::Create([this, &globalEffect, &source](bool bRelative) {
			auto it = std::find_if(globalEffect.sourceInfo.begin(), globalEffect.sourceInfo.end(), [&source](const std::shared_ptr<GlobalEffect::SoundInfo> &info) { return (info->source.get() == &source) ? true : false; });
			if(it != globalEffect.sourceInfo.end()) {
				source->RemoveEffect(*globalEffect.effect);
				globalEffect.sourceInfo.erase(it);
			}
			ApplyGlobalEffect(source, globalEffect);
		}));
	}
	info->slotId = slotId;
}
void al::ISoundSystem::ApplyGlobalEffects(SoundSource &source)
{
	for(auto &pair : m_globalEffects) {
		auto &globalEffect = pair.second;
		ApplyGlobalEffect(source, globalEffect);
	}
}
uint32_t al::ISoundSystem::AddGlobalEffect(IEffect &effect, GlobalEffectFlag flags, const EffectParams &params)
{
	if((flags & GlobalEffectFlag::RelativeSounds) == GlobalEffectFlag::None && (flags & GlobalEffectFlag::WorldSounds) == GlobalEffectFlag::None)
		return std::numeric_limits<uint32_t>::max();

	auto slotId = 0u;
	if(m_freeGlobalEffectIds.empty() == false) {
		slotId = m_freeGlobalEffectIds.front();
		m_freeGlobalEffectIds.pop();
	}
	else
		slotId = m_nextGlobalEffectId++;

	auto &globalEffect = m_globalEffects[slotId] = {};
	globalEffect.effect = &effect;
	globalEffect.params = params;
	globalEffect.flags = flags;
	globalEffect.sourceInfo.reserve(m_sources.size());
	for(auto &hSrc : m_sources)
		ApplyGlobalEffect(*hSrc.get(), globalEffect);
	return slotId;
}
void al::ISoundSystem::RemoveGlobalEffect(GlobalEffect &globalEffect)
{
	for(auto &info : globalEffect.sourceInfo) {
		auto &hSnd = info->source;
		if(hSnd.IsValid() == false || info->slotId == std::numeric_limits<uint32_t>::max())
			continue;
		(*hSnd.get())->RemoveEffect(info->slotId);
	}
}
void al::ISoundSystem::RemoveGlobalEffect(IEffect &effect)
{
	auto it = std::find_if(m_globalEffects.begin(), m_globalEffects.end(), [&effect](const std::pair<uint32_t, GlobalEffect> &pair) { return (pair.second.effect == &effect) ? true : false; });
	if(it == m_globalEffects.end())
		return;
	m_freeGlobalEffectIds.push(it->first);
	RemoveGlobalEffect(it->second);
	m_globalEffects.erase(it);
}
void al::ISoundSystem::RemoveGlobalEffect(uint32_t slotId)
{
	auto it = m_globalEffects.find(slotId);
	if(it == m_globalEffects.end())
		return;
	m_freeGlobalEffectIds.push(it->first);
	RemoveGlobalEffect(it->second);
	m_globalEffects.erase(it);
}
void al::ISoundSystem::SetGlobalEffectParameters(GlobalEffect &globalEffect, const EffectParams &params)
{
	globalEffect.params = params;
	for(auto &info : globalEffect.sourceInfo) {
		auto &hSnd = info->source;
		if(hSnd.IsValid() == false || info->slotId == std::numeric_limits<uint32_t>::max())
			continue;
		(*hSnd.get())->SetEffectParameters(info->slotId, params);
	}
}
void al::ISoundSystem::SetGlobalEffectParameters(IEffect &effect, const EffectParams &params)
{
	auto it = std::find_if(m_globalEffects.begin(), m_globalEffects.end(), [&effect](const std::pair<uint32_t, GlobalEffect> &pair) { return (pair.second.effect == &effect) ? true : false; });
	if(it == m_globalEffects.end())
		return;
	SetGlobalEffectParameters(it->second, params);
}
void al::ISoundSystem::SetGlobalEffectParameters(uint32_t slotId, const EffectParams &params)
{
	auto it = m_globalEffects.find(slotId);
	if(it == m_globalEffects.end())
		return;
	SetGlobalEffectParameters(it->second, params);
}
