/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_source.hpp"
#include "alsound_buffer.hpp"
#include "alsound_auxiliaryeffectslot.hpp"
#include "alsound_effect.hpp"
#include "alsoundsystem.hpp"
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/fmod/steam_audio_effects.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#endif
#endif

namespace al
{
	DEFINE_BASE_HANDLE(DLLALSYS,SoundSource,SoundSource);
};

al::SoundSource::SoundSource(SoundSystem &system,Decoder &decoder,InternalSource *source)
	: m_system(system),m_decoder(std::static_pointer_cast<Decoder>(decoder.shared_from_this())),m_source(source),m_handle(this)
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	,m_bSteamAudioSpatializerEnabled(system.GetSteamAudioSpatializerEnabled()),m_bSteamAudioReverbEnabled(system.GetSteamAudioReverbEnabled())
#endif
{
	OnReady();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	InitializeSteamAudio();
#endif
}

al::SoundSource::SoundSource(SoundSystem &system,SoundBuffer &buffer,InternalSource *source)
	: m_system(system),m_buffer(buffer.shared_from_this()),m_source(source),m_handle(this),m_bReady(buffer.IsReady())
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	,m_bSteamAudioSpatializerEnabled(system.GetSteamAudioSpatializerEnabled()),m_bSteamAudioReverbEnabled(system.GetSteamAudioReverbEnabled())
#endif
{
	if(m_bReady == true)
		OnReady();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	InitializeSteamAudio();
#endif
}
const al::InternalSource *al::SoundSource::GetInternalSource() const {return const_cast<al::SoundSource*>(this)->GetInternalSource();}
al::InternalSource *al::SoundSource::GetInternalSource() {return m_source;}

al::SoundSource::~SoundSource()
{
	RemoveEffects();

	m_handle.Invalidate();
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->destroy();
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	ClearSteamSoundEffects();
#endif
}

void al::SoundSource::OnReady()
{
	if(m_bSchedulePlay == true)
	{
		m_bSchedulePlay = false;
		Play();
	}
}

void al::SoundSource::Update()
{
	if(m_bReady == false && m_buffer.expired() == false && m_buffer.lock()->IsReady() == true)
	{
		m_bReady = true;
		OnReady();
	}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
		m_soundSourceData.offset = GetOffset();
#endif
}
bool al::SoundSource::IsIdle() const {return !IsPlaying();}

std::vector<al::Effect*> al::SoundSource::GetEffects()
{
	std::vector<al::Effect*> effects;
	effects.reserve(m_effects.size());
	for(auto &effect : m_effects)
		effects.push_back(effect.first.get());
	return effects;
}
void al::SoundSource::SetEffectParameters(Effect &effect,const Effect::Params &params)
{
	auto it = std::find_if(m_effects.begin(),m_effects.end(),[&effect](const std::pair<std::shared_ptr<Effect>,uint32_t> &pair) {
		return (pair.first.get() == &effect) ? true : false;
	});
	if(it == m_effects.end())
		return;
	SetEffectParameters(it->second,params);
}
void al::SoundSource::SetEffectParameters(uint32_t slotId,const Effect::Params &params)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setSendFilter(slotId,reinterpret_cast<const alure::FilterParams&>(params));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSource::SetEffectGain(Effect &effect,float gain)
{
	Effect::Params params {};
	params.gain = gain;
	SetEffectParameters(effect,params);
}
void al::SoundSource::SetEffectGain(uint32_t slotId,float gain)
{
	Effect::Params params {};
	params.gain = gain;
	SetEffectParameters(slotId,params);
}
const al::Effect::Params &al::SoundSource::GetDirectFilter() const {return m_directFilter;}
void al::SoundSource::SetDirectFilter(const Effect::Params &params)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setDirectFilter(reinterpret_cast<const alure::FilterParams&>(m_directFilter = params));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
bool al::SoundSource::AddEffect(Effect &effect,uint32_t &slotId,const Effect::Params &params)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	slotId = std::numeric_limits<uint32_t>::max();
	auto it = std::find_if(m_effects.begin(),m_effects.end(),[&effect](const std::pair<std::shared_ptr<Effect>,uint32_t> &pair) {
		return (pair.first.get() == &effect) ? true : false;
	});
	if(it != m_effects.end())
		return false;
	auto bNewId = false;
	if(m_freeAuxEffectIds.empty() == false)
		slotId = m_freeAuxEffectIds.front();
	else
	{
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

	m_effects.push_back({effect.shared_from_this(),slotId});
	m_source->setAuxiliarySendFilter(slot->GetALSlot(),slotId,reinterpret_cast<const alure::FilterParams&>(params));
	return true;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}
bool al::SoundSource::AddEffect(Effect &effect,const Effect::Params &params)
{
	uint32_t slotId;
	return AddEffect(effect,slotId,params);
}
bool al::SoundSource::AddEffect(Effect &effect,float gain)
{
	Effect::Params params {};
	params.gain = gain;
	return AddEffect(effect,params);
}
bool al::SoundSource::AddEffect(Effect &effect,uint32_t &slotId,float gain)
{
	Effect::Params params {};
	params.gain = gain;
	return AddEffect(effect,slotId,params);
}
void al::SoundSource::RemoveEffects()
{
	while(m_effects.empty() == false)
		RemoveInternalEffect(m_effects.begin());
}
void al::SoundSource::RemoveInternalEffect(decltype(m_effects)::iterator it)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto slotId = it->second;
	m_freeAuxEffectIds.push(slotId);
	auto peffect = it->first; // We need to keep a shared_ptr instance
	auto &effect = *peffect;
	m_effects.erase(it);
	m_source->setAuxiliarySend(nullptr,slotId);
	effect.DetachSource(*this);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSource::RemoveEffect(uint32_t slotId)
{
	auto it = std::find_if(m_effects.begin(),m_effects.end(),[slotId](const std::pair<std::shared_ptr<Effect>,uint32_t> &pair) {
		return (pair.second == slotId) ? true : false;
	});
	if(it == m_effects.end())
		return;
	RemoveInternalEffect(it);
}
void al::SoundSource::RemoveEffect(Effect &effect)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto it = std::find_if(m_effects.begin(),m_effects.end(),[&effect](const std::pair<std::shared_ptr<Effect>,uint32_t> &pair) {
		return (pair.first.get() == &effect) ? true : false;
	});
	if(it == m_effects.end())
		return;
	auto slotId = it->second;
	m_freeAuxEffectIds.push(slotId);
	m_effects.erase(it);
	m_source->setAuxiliarySend(nullptr,slotId);
	effect.DetachSource(*this);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

al::SoundSourceHandle al::SoundSource::GetHandle() const {return m_handle;}
al::SoundSystem &al::SoundSource::GetSoundSystem() const {return m_system;}

const al::SoundBuffer *al::SoundSource::GetBuffer() const {return const_cast<SoundSource*>(this)->GetBuffer();}
al::SoundBuffer *al::SoundSource::GetBuffer() {return m_buffer.lock().get();}
const al::Decoder *al::SoundSource::GetDecoder() const {return const_cast<SoundSource*>(this)->GetDecoder();}
al::Decoder *al::SoundSource::GetDecoder() {return m_decoder.get();}

float al::SoundSource::GetMaxAudibleDistance() const
{
	float rolloff = GetRolloffFactor();
	if(rolloff == 0.f)
		return std::numeric_limits<float>::max();
	return (GetMaxDistance() -GetReferenceDistance()) *(1.f /rolloff) +GetReferenceDistance();
}

al::impl::BufferBase *al::SoundSource::GetBaseBuffer() const
{
	if(m_decoder != nullptr)
		return static_cast<al::impl::BufferBase*>(m_decoder.get());
	if(m_buffer.expired() == true)
		return nullptr;
	return static_cast<al::impl::BufferBase*>(m_buffer.lock().get());
}

void al::SoundSource::SetFrameOffset(uint64_t offset)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setOffset(offset);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.offset = offset;
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPosition(offset,FMOD_TIMEUNIT_PCM));
#endif
}
uint64_t al::SoundSource::GetFrameOffset(uint64_t *latency) const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(latency != nullptr)
		*latency = m_source->getSampleOffsetLatency().first;
	return m_source->getSampleOffset();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		uint32_t pos;
		if(CheckResultAndUpdateValidity(m_source->getPosition(&pos,FMOD_TIMEUNIT_PCM)))
			return pos;
	}
	return m_soundSourceData.offset;
#endif
}
void al::SoundSource::SetOffset(float offset) {SetFrameOffset(offset *GetFrameLength());}
void al::SoundSource::SetTimeOffset(float offset)
{
	auto dur = GetDuration();
	if(dur == 0.f)
	{
		SetOffset(0.f);
		return;
	}
	SetOffset(offset /dur);
}
uint64_t al::SoundSource::GetFrameLength() const
{
	auto *buf = GetBaseBuffer();
	if(buf == nullptr)
		return 0;
	return buf->GetLength();
}
float al::SoundSource::GetOffset() const
{
	auto l = GetFrameLength();
	if(l == 0)
		return 0.f;
	return GetFrameOffset() /static_cast<float>(l);
}
float al::SoundSource::GetTimeOffset() const {return GetOffset() *GetDuration();}
void al::SoundSource::SetIdentifier(const std::string &identifier)
{
	m_identifier = identifier;
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	UpdateSteamAudioDSPEffects();
#endif
#endif
}
const std::string &al::SoundSource::GetIdentifier() const {return m_identifier;}
void al::SoundSource::Play()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(m_decoder != nullptr)
	{
		m_source->play(m_decoder->GetALDecoder(),m_system.GetAudioFrameSampleCount(),4);
		return;
	}
	if(m_buffer.expired() == true)
		return;
	auto buf = m_buffer.lock();
	if(buf->IsReady() == false)
	{
		m_bSchedulePlay = true;
		return;
	}
	m_source->play(*buf->GetALBuffer());
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	InitializeChannel();
	m_soundSourceData.offset = 0ull;
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPosition(0u,FMOD_TIMEUNIT_MS));
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPaused(false));
#endif
}

void al::SoundSource::Stop()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->stop();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->stop());
#endif
	m_bSchedulePlay = false;
}
void al::SoundSource::Pause()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->pause(); m_bSchedulePlay = false;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPaused(true));
#endif
}
void al::SoundSource::Resume()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->resume();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPaused(false));
#endif
}
bool al::SoundSource::IsPlaying() const
{
	if(m_bSchedulePlay == true)
		return true;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->isPlaying();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		auto r = false;
		if(CheckResultAndUpdateValidity(m_source->isPlaying(&r)))
			return r;
	}
	return false;
#endif
}
bool al::SoundSource::IsPaused() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->isPaused();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return !IsPlaying();
#endif
}
bool al::SoundSource::IsStopped() const {return (IsPlaying() == false && GetOffset() == 0.f) ? true : false;}

void al::SoundSource::SetPriority(uint32_t priority)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setPriority(priority);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.priority = priority;
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPriority(priority));
#endif
}
uint32_t al::SoundSource::GetPriority() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getPriority();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		int32_t priority;
		if(CheckResultAndUpdateValidity(m_source->getPriority(&priority)))
			return priority;
	}
	return m_soundSourceData.priority;
#endif
}

void al::SoundSource::SetLooping(bool bLoop)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setLooping(bLoop);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.looping = bLoop;
	if(m_source != nullptr)
	{
		FMOD_MODE mode;
		if(CheckResultAndUpdateValidity(m_source->getMode(&mode)))
		{
			mode &= ~(FMOD_LOOP_OFF | FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI);
			if(bLoop == false)
				mode |= FMOD_LOOP_OFF;
			else
				mode |= FMOD_LOOP_NORMAL;
			CheckResultAndUpdateValidity(m_source->setMode(mode));
		}
	}
#endif
}
bool al::SoundSource::IsLooping() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getLooping();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		FMOD_MODE mode;
		if(CheckResultAndUpdateValidity(m_source->getMode(&mode)))
			return !(mode &FMOD_LOOP_OFF) && (mode &(FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI)) != 0;
	}
	return m_soundSourceData.looping;
#endif
}

void al::SoundSource::SetPitch(float pitch)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setPitch(pitch);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.pitch = pitch;
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setPitch(pitch));
#endif
}
float al::SoundSource::GetPitch() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getPitch();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		auto pitch = 0.f;
		if(CheckResultAndUpdateValidity(m_source->getPitch(&pitch)))
			return pitch;
	}
	return m_soundSourceData.pitch;
#endif
}

void al::SoundSource::SetGain(float gain)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setGain(gain);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.gain = gain;
	if(m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->setVolume(gain));
#endif
}
float al::SoundSource::GetGain() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getGain();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(m_source != nullptr)
	{
		auto gain = 0.f;
		if(CheckResultAndUpdateValidity(m_source->getVolume(&gain)))
			return gain;
	}
	return m_soundSourceData.gain;
#endif
}

void al::SoundSource::SetGainRange(float minGain,float maxGain)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setGainRange(minGain,maxGain);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.minGain = minGain;
	m_soundSourceData.maxGain = maxGain;
	SetGain(umath::clamp(GetGain(),minGain,maxGain));
#endif
}
std::pair<float,float> al::SoundSource::GetGainRange() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getGainRange();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return {m_soundSourceData.minGain,m_soundSourceData.maxGain};
#endif
}
void al::SoundSource::SetMinGain(float gain) {SetGainRange(gain,GetMaxGain());}
float al::SoundSource::GetMinGain() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getMinGain();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return m_soundSourceData.minGain;
#endif
}
void al::SoundSource::SetMaxGain(float gain) {SetGainRange(GetMinGain(),gain);}
float al::SoundSource::GetMaxGain() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getMaxGain();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return m_soundSourceData.maxGain;
#endif
}

void al::SoundSource::SetDistanceRange(float refDist,float maxDist)
{
	refDist = umath::min(refDist,maxDist);
	auto refDistAudio = al::to_audio_distance(refDist);
	auto maxDistAudio = al::to_audio_distance(maxDist);
	if(maxDistAudio == std::numeric_limits<float>::infinity())
		maxDistAudio = std::numeric_limits<float>::max();
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setDistanceRange(refDistAudio,maxDistAudio);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.distanceRange = {refDist,maxDist};
	if(Is3D() && m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->set3DMinMaxDistance(refDistAudio,maxDistAudio));
#endif
}
std::pair<float,float> al::SoundSource::GetDistanceRange() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto distRange = m_source->getDistanceRange();
	return {al::to_game_distance(distRange.first),al::to_game_distance(distRange.second)};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(Is3D() && m_source != nullptr)
	{
		float minDist,maxDist;
		if(CheckResultAndUpdateValidity(m_source->get3DMinMaxDistance(&minDist,&maxDist)))
			return {al::to_game_distance(minDist),al::to_game_distance(maxDist)};
	}
	return m_soundSourceData.distanceRange;
#endif
}
void al::SoundSource::SetReferenceDistance(float dist) {SetDistanceRange(dist,GetMaxDistance());}
float al::SoundSource::GetReferenceDistance() const {return GetDistanceRange().first;}
void al::SoundSource::SetMaxDistance(float dist) {SetDistanceRange(GetReferenceDistance(),dist);}
float al::SoundSource::GetMaxDistance() const {return GetDistanceRange().second;}

void al::SoundSource::SetPosition(const Vector3 &pos)
{
	auto posAudio = al::to_audio_position(pos);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setPosition(&posAudio[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.position = pos;
	UpdateMode();
	if(Is3D() && m_source != nullptr)
	{
		auto fmPos = al::to_custom_vector<FMOD_VECTOR>(posAudio);
		CheckResultAndUpdateValidity(m_source->set3DAttributes(&fmPos,nullptr));
		return;
	}
#endif
}
Vector3 al::SoundSource::GetPosition() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto pos = m_source->getPosition();
	return al::to_game_position({pos[0],pos[1],pos[2]});
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(Is3D() && m_source != nullptr)
	{
		FMOD_VECTOR pos;
		if(CheckResultAndUpdateValidity(m_source->get3DAttributes(&pos,nullptr)))
			return al::to_game_position({pos.x,pos.y,pos.z});
	}
	return m_soundSourceData.position;
#endif
}

Vector3 al::SoundSource::GetWorldPosition() const
{
	if(IsRelative() == false)
		return GetPosition();
	auto &listener = m_system.GetListener();
	auto &lpos = listener.GetPosition();
	auto &lorientation = listener.GetOrientation();
	auto lrot = uquat::create(lorientation.first,uvec::cross(lorientation.first,lorientation.second),lorientation.second);
	auto pos = GetPosition();
	uvec::local_to_world(lpos,lrot,pos);
	return pos;
}

void al::SoundSource::SetVelocity(const Vector3 &vel)
{
	auto velAudio = al::to_audio_position(vel);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setVelocity(&velAudio[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.velocity = vel;
	UpdateMode();
	if(Is3D() && m_source != nullptr)
	{
		auto fmVel = al::to_custom_vector<FMOD_VECTOR>(velAudio);
		CheckResultAndUpdateValidity(m_source->set3DAttributes(nullptr,&fmVel));
		return;
	}
#endif
}
Vector3 al::SoundSource::GetVelocity() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto vel = m_source->getVelocity();
	return al::to_game_position({vel[0],vel[1],vel[2]});
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(Is3D() && m_source != nullptr)
	{
		FMOD_VECTOR vel;
		if(CheckResultAndUpdateValidity(m_source->get3DAttributes(nullptr,&vel)))
			return al::to_game_position({vel.x,vel.y,vel.z});
	}
	return m_soundSourceData.velocity;
#endif
}

void al::SoundSource::SetDirection(const Vector3 &dir)
{
	auto dirAudio = al::to_audio_direction(dir);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setDirection(&dirAudio[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
Vector3 al::SoundSource::GetDirection() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto dir = m_source->getDirection();
	return al::to_game_direction({dir[0],dir[1],dir[2]});
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {};
#endif
}

void al::SoundSource::SetOrientation(const Vector3 &at,const Vector3 &up)
{
	auto atAudio = al::to_audio_direction(at);
	auto atUp = al::to_audio_direction(up);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setOrientation(&atAudio[0],&atUp[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	//FMOD_VECTOR orientation {at.x,at.y,at.z};
	//al::fmod::check_result(m_source->set3DConeOrientation(&orientation));
#endif
}
std::pair<Vector3,Vector3> al::SoundSource::GetOrientation() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto orientation = m_source->getOrientation();
	return {al::to_game_direction({orientation.first[0],orientation.first[1],orientation.first[2]}),al::to_game_direction({orientation.second[0],orientation.second[1],orientation.second[2]})};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {{},{}};
#endif
}

void al::SoundSource::SetConeAngles(float inner,float outer)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setConeAngles(inner,outer);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.coneAngles = {inner,outer};
	UpdateMode();
	if(Is3D() && m_source != nullptr)
	{
		float t,volume;
		m_source->get3DConeSettings(&t,&t,&volume);
		CheckResultAndUpdateValidity(m_source->set3DConeSettings(inner,outer,volume));
	}
#endif
}
std::pair<float,float> al::SoundSource::GetConeAngles() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getConeAngles();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(Is3D() && m_source != nullptr)
	{
		float inner,outer;
		if(CheckResultAndUpdateValidity(m_source->get3DConeSettings(&inner,&outer,nullptr)))
			return {inner,outer};
	}
	return m_soundSourceData.coneAngles;
#endif
}
void al::SoundSource::SetInnerConeAngle(float inner) {SetConeAngles(inner,GetOuterConeAngle());}
float al::SoundSource::GetInnerConeAngle() const {return GetConeAngles().first;}
void al::SoundSource::SetOuterConeAngle(float outer) {SetConeAngles(GetInnerConeAngle(),outer);}
float al::SoundSource::GetOuterConeAngle() const {return GetConeAngles().second;}

void al::SoundSource::SetOuterConeGains(float gain,float gainHF)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setOuterConeGains(gain,gainHF);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSource::SetOuterConeGain(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gain,gains.second);
}
void al::SoundSource::SetOuterConeGainHF(float gain)
{
	auto gains = GetOuterConeGains();
	SetOuterConeGains(gains.first,gain);
}
std::pair<float,float> al::SoundSource::GetOuterConeGains() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getOuterConeGains();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {0.f,0.f};
#endif
}
float al::SoundSource::GetOuterConeGain() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getOuterConeGain();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0.f;
#endif
}
float al::SoundSource::GetOuterConeGainHF() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getOuterConeGainHF();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0.f;
#endif
}

void al::SoundSource::SetRolloffFactors(float factor,float roomFactor)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setRolloffFactors(factor,roomFactor);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSource::SetRoomRolloffFactor(float roomFactor) {SetRolloffFactors(GetRolloffFactor(),roomFactor);}
std::pair<float,float> al::SoundSource::GetRolloffFactors() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getRolloffFactors();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {0.f,0.f};
#endif
}
float al::SoundSource::GetRolloffFactor() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getRolloffFactor();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0.f;
#endif
}
float al::SoundSource::GetRoomRolloffFactor() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getRoomRolloffFactor();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0.f;
#endif
}

void al::SoundSource::SetDopplerFactor(float factor)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setDopplerFactor(factor);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.dopplerFactor = factor;
	if(Is3D() && m_source != nullptr)
		CheckResultAndUpdateValidity(m_source->set3DDopplerLevel(factor));
#endif
}
float al::SoundSource::GetDopplerFactor() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getDopplerFactor();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	if(Is3D() && m_source != nullptr)
	{
		auto factor = 0.f;
		if(CheckResultAndUpdateValidity(m_source->get3DDopplerLevel(&factor)))
			return factor;
	}
	return m_soundSourceData.dopplerFactor;
#endif
}

void al::SoundSource::SetRelative(bool bRelative)
{
	auto bRelativeOld = IsRelative();
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setRelative(bRelative);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	m_soundSourceData.relativeToListener = bRelative;
	UpdateMode();
#endif
	if(bRelative != bRelativeOld)
		CallCallbacks<void,bool>("OnRelativeChanged",bRelative);
}
bool al::SoundSource::IsRelative() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getRelative();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return m_soundSourceData.relativeToListener;
#endif
}

void al::SoundSource::SetRadius(float radius)
{
	auto radiusAudio = al::to_audio_distance(radius);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setRadius(radiusAudio);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	SetMaxDistance(radiusAudio);
#endif
}
float al::SoundSource::GetRadius() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return al::to_game_distance(m_source->getRadius());
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return al::to_game_distance(GetMaxDistance());
#endif
}

void al::SoundSource::SetStereoAngles(float leftAngle,float rightAngle)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setStereoAngles(leftAngle,rightAngle);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
std::pair<float,float> al::SoundSource::GetStereoAngles() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getStereoAngles();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {0.f,0.f};
#endif
}
void al::SoundSource::SetLeftStereoAngle(float ang) {SetStereoAngles(ang,GetRightStereoAngle());}
float al::SoundSource::GetLeftStereoAngle() const {return GetStereoAngles().first;}
void al::SoundSource::SetRightStereoAngle(float ang) {SetStereoAngles(GetLeftStereoAngle(),ang);}
float al::SoundSource::GetRightStereoAngle() const {return GetStereoAngles().second;}

void al::SoundSource::SetAirAbsorptionFactor(float factor)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setAirAbsorptionFactor(factor);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
float al::SoundSource::GetAirAbsorptionFactor() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getAirAbsorptionFactor();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0.f;
#endif
}

void al::SoundSource::SetGainAuto(bool directHF,bool send,bool sendHF)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_source->setGainAuto(directHF,send,sendHF);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
std::tuple<bool,bool,bool> al::SoundSource::GetGainAuto() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getGainAuto();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {false,false,false};
#endif
}
bool al::SoundSource::GetDirectGainHFAuto() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getDirectGainHFAuto();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}
bool al::SoundSource::GetSendGainAuto() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getSendGainAuto();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}
bool al::SoundSource::GetSendGainHFAuto() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_source->getSendGainHFAuto();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}

float al::SoundSource::GetDuration() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetDuration() : 0.f;
}
uint32_t al::SoundSource::GetFrequency() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetFrequency() : 0;
}
al::ChannelConfig al::SoundSource::GetChannelConfig() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetChannelConfig() : static_cast<ChannelConfig>(0);
}
al::SampleType al::SoundSource::GetSampleType() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetSampleType() : static_cast<SampleType>(0);
}
uint64_t al::SoundSource::GetLength() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetLength() : 0;
}
std::pair<uint64_t,uint64_t> al::SoundSource::GetLoopFramePoints() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetLoopFramePoints() : std::pair<uint64_t,uint64_t>(0,0);
}
std::pair<float,float> al::SoundSource::GetLoopTimePoints() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetLoopTimePoints() : std::pair<float,float>(0,0);
}

float al::SoundSource::GetInverseFrequency() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetInverseFrequency() : 0.f;
}
std::string al::SoundSource::GetChannelConfigName() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetChannelConfigName() : "";
}
std::string al::SoundSource::GetSampleTypeName() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->GetSampleTypeName() : "";
}
bool al::SoundSource::IsMono() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsMono() : false;
}
bool al::SoundSource::IsStereo() const
{
	auto *buf = GetBaseBuffer();
	return (buf != nullptr) ? buf->IsStereo() : false;
}

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
void al::SoundSource::SetFMOD3DAttributesEffective(bool b)
{
	m_b3DAttributesEffective = b;
	UpdateMode();
}
void al::SoundSource::UpdateMode()
{
	FMOD_MODE mode;
	if(CheckResultAndUpdateValidity(m_source->getMode(&mode)) == false)
		return;
	auto oldMode = mode;
	mode &= ~(FMOD_2D | FMOD_3D | FMOD_3D_HEADRELATIVE | FMOD_3D_WORLDRELATIVE);
	if(m_b3DAttributesEffective == false)
		mode |= FMOD_2D;
	else
	{
		if(IsRelative() == false)
			mode |= FMOD_3D | FMOD_3D_WORLDRELATIVE;
		else
		{
			auto coneAngles = GetConeAngles();
			if(uvec::length_sqr(m_soundSourceData.position) == 0.f && uvec::length_sqr(m_soundSourceData.velocity) == 0.f && m_soundSourceData.coneAngles.first >= 360.f && m_soundSourceData.coneAngles.second >= 360.f) // Note: UpdateMode() has to be called whenever one of these was changed
				mode |= FMOD_2D;
			else
				mode |= FMOD_3D | FMOD_3D_HEADRELATIVE;
		}
	}
	if(mode == oldMode)
		return;
	if((mode &FMOD_3D) == 0 || (oldMode &FMOD_3D) != 0) // No update required if new mode isn't 3D, or if old mode was already 3D
	{
		CheckResultAndUpdateValidity(m_source->setMode(mode));
		return;
	}
	// If this was previously a 2D sound, we have to re-set the 3D attributes
	// after the new mode has been applied
	auto distRange = GetDistanceRange();
	auto pos = GetPosition();
	auto vel = GetVelocity();
	auto coneAngles = GetConeAngles();
	auto dopplerFactor = GetDopplerFactor();
	if(CheckResultAndUpdateValidity(m_source->setMode(mode)) == false)
		return;
	SetDistanceRange(distRange.first,distRange.second);
	SetPosition(pos);
	SetVelocity(vel);
	SetConeAngles(coneAngles.first,coneAngles.second);
	SetDopplerFactor(dopplerFactor);
}
void al::SoundSource::InvalidateSource() const {m_source = nullptr;}
bool al::SoundSource::Is3D() const
{
	if(m_source != nullptr)
	{
		FMOD_MODE mode;
		if(CheckResultAndUpdateValidity(m_source->getMode(&mode)))
			return (mode &FMOD_3D) != 0;
	}
	return false;
}
bool al::SoundSource::Is2D() const {return !Is3D();}
void al::SoundSource::InitializeChannel()
{
	if(m_source != nullptr || m_buffer.expired())
		return;
	auto *sound = m_buffer.lock()->GetFMODSound();
	if(sound == nullptr || CheckResultAndUpdateValidity(m_system.GetFMODLowLevelSystem().playSound(sound,nullptr,true,&m_source)) == false)
		return;
	SetOffset(m_soundSourceData.offset);
	SetPriority(m_soundSourceData.priority);
	SetLooping(m_soundSourceData.looping);
	SetPitch(m_soundSourceData.pitch);
	SetGain(m_soundSourceData.gain);
	SetDistanceRange(m_soundSourceData.distanceRange.first,m_soundSourceData.distanceRange.second);
	SetPosition(m_soundSourceData.position);
	SetVelocity(m_soundSourceData.velocity);
	SetConeAngles(m_soundSourceData.coneAngles.first,m_soundSourceData.coneAngles.second);
	SetDopplerFactor(m_soundSourceData.dopplerFactor);
	SetRelative(m_soundSourceData.relativeToListener);
	SetChannelGroup(GetChannelGroup());
}
bool al::SoundSource::CheckResultAndUpdateValidity(uint32_t result) const
{
	if(result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN)
	{
		InvalidateSource();
		return false;
	}
	al::fmod::check_result(result);
	return result == FMOD_OK;
}
#endif
