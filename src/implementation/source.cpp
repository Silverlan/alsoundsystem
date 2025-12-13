// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

import :buffer;
import :source;
import :system;

pragma::audio::ISoundChannel::ISoundChannel(ISoundSystem &system, Decoder &decoder)
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

pragma::audio::ISoundChannel::ISoundChannel(ISoundSystem &system, ISoundBuffer &buffer)
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

pragma::audio::ISoundChannel::~ISoundChannel()
{
	RemoveEffects();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	ClearSteamSoundEffects();
#endif
}

void pragma::audio::ISoundChannel::OnReady()
{
	if(m_bSchedulePlay == true) {
		m_bSchedulePlay = false;
		Play();
	}
}

void pragma::audio::ISoundChannel::Update()
{
	if(m_bReady == false && m_buffer.expired() == false && m_buffer.lock()->IsReady() == true) {
		m_bReady = true;
		OnReady();
	}
}
bool pragma::audio::ISoundChannel::IsIdle() const { return !IsPlaying(); }

std::vector<pragma::audio::IEffect *> pragma::audio::ISoundChannel::GetEffects()
{
	std::vector<IEffect *> effects;
	effects.reserve(m_effects.size());
	for(auto &effect : m_effects)
		effects.push_back(effect.first.get());
	return effects;
}
void pragma::audio::ISoundChannel::SetEffectParameters(IEffect &effect, const EffectParams &params)
{
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [&effect](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.first.get() == &effect) ? true : false; });
	if(it == m_effects.end())
		return;
	SetEffectParameters(it->second, params);
}
void pragma::audio::ISoundChannel::SetEffectGain(IEffect &effect, float gain)
{
	EffectParams params {};
	params.gain = gain;
	SetEffectParameters(effect, params);
}
void pragma::audio::ISoundChannel::SetEffectGain(uint32_t slotId, float gain)
{
	EffectParams params {};
	params.gain = gain;
	SetEffectParameters(slotId, params);
}
const pragma::audio::EffectParams &pragma::audio::ISoundChannel::GetDirectFilter() const { return m_directFilter; }
bool pragma::audio::ISoundChannel::AddEffect(IEffect &effect, uint32_t &slotId, const EffectParams &params)
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
bool pragma::audio::ISoundChannel::AddEffect(IEffect &effect, const EffectParams &params)
{
	uint32_t slotId;
	return AddEffect(effect, slotId, params);
}
bool pragma::audio::ISoundChannel::AddEffect(IEffect &effect, float gain)
{
	EffectParams params {};
	params.gain = gain;
	return AddEffect(effect, params);
}
bool pragma::audio::ISoundChannel::AddEffect(IEffect &effect, uint32_t &slotId, float gain)
{
	EffectParams params {};
	params.gain = gain;
	return AddEffect(effect, slotId, params);
}
void pragma::audio::ISoundChannel::RemoveEffects()
{
	while(m_effects.empty() == false)
		RemoveInternalEffect(m_effects.begin());
}
void pragma::audio::ISoundChannel::RemoveInternalEffect(decltype(m_effects)::iterator it)
{
	auto slotId = it->second;
	m_freeAuxEffectIds.push(slotId);
	auto peffect = it->first; // We need to keep a shared_ptr instance
	auto &effect = *peffect;
	m_effects.erase(it);
	DoRemoveInternalEffect(slotId);
	effect.DetachSource(*this);
}
void pragma::audio::ISoundChannel::RemoveEffect(uint32_t slotId)
{
	auto it = std::find_if(m_effects.begin(), m_effects.end(), [slotId](const std::pair<std::shared_ptr<IEffect>, uint32_t> &pair) { return (pair.second == slotId) ? true : false; });
	if(it == m_effects.end())
		return;
	RemoveInternalEffect(it);
}
void pragma::audio::ISoundChannel::RemoveEffect(IEffect &effect)
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

pragma::audio::ISoundSystem &pragma::audio::ISoundChannel::GetSoundSystem() const { return m_system; }

const pragma::audio::ISoundBuffer *pragma::audio::ISoundChannel::GetBuffer() const { return const_cast<ISoundChannel *>(this)->GetBuffer(); }
pragma::audio::ISoundBuffer *pragma::audio::ISoundChannel::GetBuffer() { return m_buffer.lock().get(); }
const pragma::audio::Decoder *pragma::audio::ISoundChannel::GetDecoder() const { return const_cast<ISoundChannel *>(this)->GetDecoder(); }
pragma::audio::Decoder *pragma::audio::ISoundChannel::GetDecoder() { return m_decoder.get(); }

float pragma::audio::ISoundChannel::GetMaxAudibleDistance() const
{
	float rolloff = GetRolloffFactor();
	if(rolloff == 0.f)
		return std::numeric_limits<float>::max();
	return (GetMaxDistance() - GetReferenceDistance()) * (1.f / rolloff) + GetReferenceDistance();
}

pragma::audio::impl::BufferBase *pragma::audio::ISoundChannel::GetBaseBuffer() const
{
	if(m_decoder != nullptr)
		return static_cast<impl::BufferBase *>(m_decoder.get());
	if(m_buffer.expired() == true)
		return nullptr;
	return static_cast<impl::BufferBase *>(m_buffer.lock().get());
}

void pragma::audio::ISoundChannel::SetIdentifier(const std::string &identifier)
{
	m_identifier = identifier;
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	UpdateSteamAudioDSPEffects();
#endif
#endif
}
const std::string &pragma::audio::ISoundChannel::GetIdentifier() const { return m_identifier; }
bool pragma::audio::ISoundChannel::IsStopped() const { return (IsPlaying() == false && GetOffset() == 0.f) ? true : false; }

void pragma::audio::ISoundChannel::SetMinGain(float gain) { SetGainRange(gain, GetMaxGain()); }
void pragma::audio::ISoundChannel::SetMaxGain(float gain) { SetGainRange(GetMinGain(), gain); }

void pragma::audio::ISoundChannel::SetReferenceDistance(float dist) { SetDistanceRange(dist, GetMaxDistance()); }
float pragma::audio::ISoundChannel::GetReferenceDistance() const { return GetDistanceRange().first; }
void pragma::audio::ISoundChannel::SetMaxDistance(float dist) { SetDistanceRange(GetReferenceDistance(), dist); }
float pragma::audio::ISoundChannel::GetMaxDistance() const { return GetDistanceRange().second; }

Vector3 pragma::audio::ISoundChannel::GetWorldPosition() const
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

void pragma::audio::ISoundChannel::SetInnerConeAngle(float inner) { SetConeAngles(inner, GetOuterConeAngle()); }
float pragma::audio::ISoundChannel::GetInnerConeAngle() const { return GetConeAngles().first; }
void pragma::audio::ISoundChannel::SetOuterConeAngle(float outer) { SetConeAngles(GetInnerConeAngle(), outer); }
float pragma::audio::ISoundChannel::GetOuterConeAngle() const { return GetConeAngles().second; }

void pragma::audio::ISoundChannel::SetOuterConeGain(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gain, gains.second);
}
void pragma::audio::ISoundChannel::SetOuterConeGainHF(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gains.first, gain);
}

void pragma::audio::ISoundChannel::SetRoomRolloffFactor(float roomFactor) { SetRolloffFactors(GetRolloffFactor(), roomFactor); }

void pragma::audio::ISoundChannel::SetLeftStereoAngle(float ang) { SetStereoAngles(ang, GetRightStereoAngle()); }
float pragma::audio::ISoundChannel::GetLeftStereoAngle() const { return GetStereoAngles().first; }
void pragma::audio::ISoundChannel::SetRightStereoAngle(float ang) { SetStereoAngles(GetLeftStereoAngle(), ang); }
float pragma::audio::ISoundChannel::GetRightStereoAngle() const { return GetStereoAngles().second; }

float pragma::audio::ISoundChannel::GetDuration() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetDuration() : 0.f;
}

bool pragma::audio::ISoundChannel::IsMono() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsMono() : false;
}
bool pragma::audio::ISoundChannel::IsStereo() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsStereo() : false;
}

////////

std::shared_ptr<pragma::audio::SoundSource> pragma::audio::SoundSource::Create(const std::shared_ptr<ISoundChannel> &channel) {
	auto snd = std::shared_ptr<SoundSource> {new SoundSource {channel}, [](SoundSource *snd) {
		snd->OnRelease();
		delete snd;
	}};
	return snd;
}
pragma::audio::SoundSource::SoundSource(const std::shared_ptr<ISoundChannel> &channel) : m_channel {channel} {}
pragma::audio::SoundSource::~SoundSource() {}
void pragma::audio::SoundSource::InitializeHandle(const pragma::util::TSharedHandle<SoundSource> &ptr) {
	m_handle = ptr;
}
pragma::audio::SoundSourceHandle pragma::audio::SoundSource::GetHandle() const { return pragma::util::claim_shared_handle_ownership(m_handle); }

void pragma::audio::SoundSource::OnRelease() { m_channel->GetSoundSystem().OnSoundRelease(*this); }
