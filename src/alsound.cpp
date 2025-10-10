// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "alsound_source.hpp"
#include "alsound_buffer.hpp"
#include "alsound_listener.hpp"
#include "alsound_auxiliaryeffectslot.hpp"
#include "alsound_effect.hpp"
#include "alsoundsystem.hpp"
#include "alsound_coordinate_system.hpp"

al::ISoundChannel::ISoundChannel(ISoundSystem &system, Decoder &decoder)
    : m_system(system), m_decoder(std::static_pointer_cast<Decoder>(decoder.shared_from_this()))
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
      ,
      m_bSteamAudioSpatializerEnabled(system.GetSteamAudioSpatializerEnabled()), m_bSteamAudioReverbEnabled(system.GetSteamAudioReverbEnabled())
#endif
{
	OnReady();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	InitializeSteamAudio();
#endif
}

al::ISoundChannel::ISoundChannel(ISoundSystem &system, ISoundBuffer &buffer)
    : m_system(system), m_buffer(buffer.shared_from_this()), m_bReady(buffer.IsReady())
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
      ,
      m_bSteamAudioSpatializerEnabled(system.GetSteamAudioSpatializerEnabled()), m_bSteamAudioReverbEnabled(system.GetSteamAudioReverbEnabled())
#endif
{
	if(m_bReady == true)
		OnReady();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	InitializeSteamAudio();
#endif
}

al::ISoundChannel::~ISoundChannel()
{
	RemoveEffects();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	ClearSteamSoundEffects();
#endif
}

void al::ISoundChannel::OnReady()
{
	if(m_bSchedulePlay == true) {
		m_bSchedulePlay = false;
		Play();
	}
}

void al::ISoundChannel::Update()
{
	if(m_bReady == false && m_buffer.expired() == false && m_buffer.lock()->IsReady() == true) {
		m_bReady = true;
		OnReady();
	}
}
bool al::ISoundChannel::IsIdle() const { return !IsPlaying(); }

std::vector<al::IEffect *> al::ISoundChannel::GetEffects()
{
	std::vector<al::IEffect *> effects;
	effects.reserve(m_effects.size());
	for(auto &effect : m_effects)
		effects.push_back(effect.first.get());
	return effects;
}
void al::ISoundChannel::SetEffectParameters(IEffect &effect, const EffectParams &params)
{
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [&effect](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.first.get() == &effect) ? true : false; });
	if(it == m_effects.end())
		return;
	SetEffectParameters(it->second, params);
}
void al::ISoundChannel::SetEffectGain(IEffect &effect, float gain)
{
	EffectParams params {};
	params.gain = gain;
	SetEffectParameters(effect, params);
}
void al::ISoundChannel::SetEffectGain(uint32_t slotId, float gain)
{
	EffectParams params {};
	params.gain = gain;
	SetEffectParameters(slotId, params);
}
const al::EffectParams &al::ISoundChannel::GetDirectFilter() const { return m_directFilter; }
bool al::ISoundChannel::AddEffect(IEffect &effect, uint32_t &slotId, const EffectParams &params)
{
	slotId = std::numeric_limits<uint32_t>::max();
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [&effect](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.first.get() == &effect) ? true : false; });
	if(it != m_effects.end())
		return false;
	auto bNewId = false;
	if(m_freeAuxEffectIds.empty() == false)
		slotId = m_freeAuxEffectIds.front();
	else {
		slotId = m_nextAuxSlot;
		bNewId = true;
		if(slotId >= m_system.GetMaxAuxiliaryEffectsPerSource())
			return false; // The source already has the maximum amount of aux effects applied
	}
	auto *slot = effect.AttachSource(*this);
	if(slot == nullptr)
		return false;
	if(bNewId == false)
		m_freeAuxEffectIds.pop();
	else
		++m_nextAuxSlot;

	m_effects.push_back({effect.shared_from_this(), slotId});
	DoAddEffect(*slot, slotId, params);
	return true;
}
bool al::ISoundChannel::AddEffect(IEffect &effect, const EffectParams &params)
{
	uint32_t slotId;
	return AddEffect(effect, slotId, params);
}
bool al::ISoundChannel::AddEffect(IEffect &effect, float gain)
{
	EffectParams params {};
	params.gain = gain;
	return AddEffect(effect, params);
}
bool al::ISoundChannel::AddEffect(IEffect &effect, uint32_t &slotId, float gain)
{
	EffectParams params {};
	params.gain = gain;
	return AddEffect(effect, slotId, params);
}
void al::ISoundChannel::RemoveEffects()
{
	while(m_effects.empty() == false)
		RemoveInternalEffect(m_effects.begin());
}
void al::ISoundChannel::RemoveInternalEffect(decltype(m_effects)::iterator it)
{
	auto slotId = it->second;
	m_freeAuxEffectIds.push(slotId);
	auto peffect = it->first; // We need to keep a shared_ptr instance
	auto &effect = *peffect;
	m_effects.erase(it);
	DoRemoveInternalEffect(slotId);
	effect.DetachSource(*this);
}
void al::ISoundChannel::RemoveEffect(uint32_t slotId)
{
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [slotId](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.second == slotId) ? true : false; });
	if(it == m_effects.end())
		return;
	RemoveInternalEffect(it);
}
void al::ISoundChannel::RemoveEffect(IEffect &effect)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [&effect](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.first.get() == &effect) ? true : false; });
	if(it == m_effects.end())
		return;
	auto slotId = it->second;
	m_freeAuxEffectIds.push(slotId);
	m_effects.erase(it);
	DoRemoveEffect(slotId);
	effect.DetachSource(*this);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

al::ISoundSystem &al::ISoundChannel::GetSoundSystem() const { return m_system; }

const al::ISoundBuffer *al::ISoundChannel::GetBuffer() const { return const_cast<ISoundChannel *>(this)->GetBuffer(); }
al::ISoundBuffer *al::ISoundChannel::GetBuffer() { return m_buffer.lock().get(); }
const al::Decoder *al::ISoundChannel::GetDecoder() const { return const_cast<ISoundChannel *>(this)->GetDecoder(); }
al::Decoder *al::ISoundChannel::GetDecoder() { return m_decoder.get(); }

float al::ISoundChannel::GetMaxAudibleDistance() const
{
	float rolloff = GetRolloffFactor();
	if(rolloff == 0.f)
		return std::numeric_limits<float>::max();
	return (GetMaxDistance() - GetReferenceDistance()) * (1.f / rolloff) + GetReferenceDistance();
}

al::impl::BufferBase *al::ISoundChannel::GetBaseBuffer() const
{
	if(m_decoder != nullptr)
		return static_cast<al::impl::BufferBase *>(m_decoder.get());
	if(m_buffer.expired() == true)
		return nullptr;
	return static_cast<al::impl::BufferBase *>(m_buffer.lock().get());
}

void al::ISoundChannel::SetIdentifier(const std::string &identifier)
{
	m_identifier = identifier;
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	UpdateSteamAudioDSPEffects();
#endif
#endif
}
const std::string &al::ISoundChannel::GetIdentifier() const { return m_identifier; }
bool al::ISoundChannel::IsStopped() const { return (IsPlaying() == false && GetOffset() == 0.f) ? true : false; }

void al::ISoundChannel::SetMinGain(float gain) { SetGainRange(gain, GetMaxGain()); }
void al::ISoundChannel::SetMaxGain(float gain) { SetGainRange(GetMinGain(), gain); }

void al::ISoundChannel::SetReferenceDistance(float dist) { SetDistanceRange(dist, GetMaxDistance()); }
float al::ISoundChannel::GetReferenceDistance() const { return GetDistanceRange().first; }
void al::ISoundChannel::SetMaxDistance(float dist) { SetDistanceRange(GetReferenceDistance(), dist); }
float al::ISoundChannel::GetMaxDistance() const { return GetDistanceRange().second; }

Vector3 al::ISoundChannel::GetWorldPosition() const
{
	if(IsRelative() == false)
		return GetPosition();
	auto &listener = m_system.GetListener();
	auto &lpos = listener.GetPosition();
	auto &lorientation = listener.GetOrientation();
	auto lrot = uquat::create(lorientation.first, uvec::cross(lorientation.first, lorientation.second), lorientation.second);
	auto pos = GetPosition();
	uvec::local_to_world(lpos, lrot, pos);
	return pos;
}

void al::ISoundChannel::SetInnerConeAngle(float inner) { SetConeAngles(inner, GetOuterConeAngle()); }
float al::ISoundChannel::GetInnerConeAngle() const { return GetConeAngles().first; }
void al::ISoundChannel::SetOuterConeAngle(float outer) { SetConeAngles(GetInnerConeAngle(), outer); }
float al::ISoundChannel::GetOuterConeAngle() const { return GetConeAngles().second; }

void al::ISoundChannel::SetOuterConeGain(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gain, gains.second);
}
void al::ISoundChannel::SetOuterConeGainHF(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gains.first, gain);
}

void al::ISoundChannel::SetRoomRolloffFactor(float roomFactor) { SetRolloffFactors(GetRolloffFactor(), roomFactor); }

void al::ISoundChannel::SetLeftStereoAngle(float ang) { SetStereoAngles(ang, GetRightStereoAngle()); }
float al::ISoundChannel::GetLeftStereoAngle() const { return GetStereoAngles().first; }
void al::ISoundChannel::SetRightStereoAngle(float ang) { SetStereoAngles(GetLeftStereoAngle(), ang); }
float al::ISoundChannel::GetRightStereoAngle() const { return GetStereoAngles().second; }

float al::ISoundChannel::GetDuration() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetDuration() : 0.f;
}

bool al::ISoundChannel::IsMono() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsMono() : false;
}
bool al::ISoundChannel::IsStereo() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsStereo() : false;
}

////////

al::SoundSource::SoundSource(const std::shared_ptr<ISoundChannel> &channel) : m_channel {channel}, m_handle {this} {}
al::SoundSource::~SoundSource() { m_handle.Invalidate(); }
al::SoundSourceHandle al::SoundSource::GetHandle() const { return m_handle; }

void al::SoundSource::OnRelease() { m_channel->GetSoundSystem().OnSoundRelease(*this); }
