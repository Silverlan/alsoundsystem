/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_SOURCE_HPP__
#define __ALSOUND_SOURCE_HPP__

#include "alsound_definitions.hpp"
#include "alsound_enums.hpp"
#include "alsound_decoder.hpp"
#include "alsound_effect.hpp"
#include "alsound_settings.hpp"
#include "impl_alsound_source_handle.hpp"
#include <sharedutils/callback_handler.h>
#include <sharedutils/util_virtual_shared_from_this.hpp>
#include <sharedutils/util_overridable.hpp>
#include <memory>
#include <mathutil/uvec.h>
#include <queue>

#pragma warning(push)
#pragma warning(disable:4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {class Source;};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include "steam_audio/alsound_steam_audio_properties.hpp"
namespace FMOD {class Channel; class ChannelGroup; class DSP;};
#endif
namespace al
{
	class SoundBuffer;
	class SoundSourceFactory;
	class SoundSystem;
	class DLLALSYS SoundSource
		: virtual public CallbackHandler,
		virtual public util::inheritable_enable_shared_from_this<SoundSource>
	{
	public:
		virtual ~SoundSource();

		enum class DSPEffectSlot : uint32_t
		{
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
			Spatializer,
			Reverb,
#endif
#endif
			Count
		};

		SoundSource(SoundSystem &system,SoundBuffer &buffer,InternalSource *source);
		SoundSource(SoundSystem &system,Decoder &decoder,InternalSource *source);
		const InternalSource *GetInternalSource() const;
		InternalSource *GetInternalSource();
		
		const SoundBuffer *GetBuffer() const;
		SoundBuffer *GetBuffer();
		const Decoder *GetDecoder() const;
		Decoder *GetDecoder();
		SoundSourceHandle GetHandle() const;
		SoundSystem &GetSoundSystem() const;

		uint32_t GetFrequency() const;
		ChannelConfig GetChannelConfig() const;
		SampleType GetSampleType() const;
		uint64_t GetLength() const;
		std::pair<uint64_t,uint64_t> GetLoopFramePoints() const;
		std::pair<float,float> GetLoopTimePoints() const;

		float GetDuration() const;
		float GetInverseFrequency() const;
		std::string GetChannelConfigName() const;
		std::string GetSampleTypeName() const;
		bool IsMono() const;
		bool IsStereo() const;

		void Play();
		void Stop();
		void Pause();
		void Resume();
		bool IsPlaying() const;
		bool IsPaused() const;
		bool IsStopped() const;

		void SetPriority(uint32_t priority);
		uint32_t GetPriority() const;

		uint64_t GetFrameLength() const;
		void SetFrameOffset(uint64_t offset);
		uint64_t GetFrameOffset(uint64_t *latency=nullptr) const;

		void SetOffset(float offset);
		float GetOffset() const;

		void SetTimeOffset(float offset);
		float GetTimeOffset() const;

		void SetLooping(bool bLoop);
		bool IsLooping() const;

		void SetPitch(float pitch);
		float GetPitch() const;

		void SetGain(float gain);
		float GetGain() const;

		void SetGainRange(float minGain,float maxGain);
		std::pair<float,float> GetGainRange() const;
		void SetMinGain(float gain);
		float GetMinGain() const;
		void SetMaxGain(float gain);
		float GetMaxGain() const;

		void SetDistanceRange(float refDist,float maxDist);
		std::pair<float,float> GetDistanceRange() const;
		void SetReferenceDistance(float dist);
		float GetReferenceDistance() const;
		void SetMaxDistance(float dist);
		float GetMaxDistance() const;

		void SetPosition(const Vector3 &pos);
		Vector3 GetPosition() const;

		Vector3 GetWorldPosition() const;

		void SetVelocity(const Vector3 &vel);
		Vector3 GetVelocity() const;

		void SetDirection(const Vector3 &dir);
		Vector3 GetDirection() const;

		void SetOrientation(const Vector3 &at,const Vector3 &up);
		std::pair<Vector3,Vector3> GetOrientation() const;

		void SetConeAngles(float inner,float outer);
		std::pair<float,float> GetConeAngles() const;
		void SetInnerConeAngle(float inner);
		float GetInnerConeAngle() const;
		void SetOuterConeAngle(float outer);
		float GetOuterConeAngle() const;

		void SetOuterConeGains(float gain,float gainHF=1.f);
		void SetOuterConeGain(float gain);
		void SetOuterConeGainHF(float gain);
		std::pair<float,float> GetOuterConeGains() const;
		float GetOuterConeGain() const;
		float GetOuterConeGainHF() const;

		void SetRolloffFactors(float factor,float roomFactor=0.f);
		void SetRoomRolloffFactor(float roomFactor);
		std::pair<float,float> GetRolloffFactors() const;
		float GetRolloffFactor() const;
		float GetRoomRolloffFactor() const;

		void SetDopplerFactor(float factor);
		float GetDopplerFactor() const;

		void SetRelative(bool bRelative);
		bool IsRelative() const;

		void SetRadius(float radius);
		float GetRadius() const;

		void SetStereoAngles(float leftAngle,float rightAngle);
		std::pair<float,float> GetStereoAngles() const;
		void SetLeftStereoAngle(float ang);
		float GetLeftStereoAngle() const;
		void SetRightStereoAngle(float ang);
		float GetRightStereoAngle() const;

		void SetAirAbsorptionFactor(float factor);
		float GetAirAbsorptionFactor() const;

		void SetGainAuto(bool directHF,bool send,bool sendHF);
		std::tuple<bool,bool,bool> GetGainAuto() const;
		bool GetDirectGainHFAuto() const;
		bool GetSendGainAuto() const;
		bool GetSendGainHFAuto() const;

		float GetMaxAudibleDistance() const;

		void SetDirectFilter(const Effect::Params &params);
		const Effect::Params &GetDirectFilter() const;
		bool AddEffect(Effect &effect,const Effect::Params &params=Effect::Params());
		bool AddEffect(Effect &effect,uint32_t &slotId,const Effect::Params &params=Effect::Params());
		bool AddEffect(Effect &effect,float gain);
		bool AddEffect(Effect &effect,uint32_t &slotId,float gain);
		void RemoveEffect(Effect &effect);
		void RemoveEffect(uint32_t slotId);
		void RemoveEffects();
		void SetEffectParameters(Effect &effect,const Effect::Params &params);
		void SetEffectParameters(uint32_t slotId,const Effect::Params &params);
		void SetEffectGain(Effect &effect,float gain);
		void SetEffectGain(uint32_t slotId,float gain);
		std::vector<Effect*> GetEffects();

		virtual void Update();
		virtual bool IsIdle() const; // If true is returned, and no references to the sound exist anymore, the sound will be released by the sound system

		void SetIdentifier(const std::string &identifier);
		const std::string &GetIdentifier() const;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		void InitializeSteamAudio();
		bool InitializeConvolutionEffect(const std::string &name);
		void SetSteamAudioEffectsEnabled(bool b);
		void ClearSteamSoundEffects();
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	public:
		void UpdateSteamAudioDSPEffects();
	private:
		void SetSteamAudioSpatializerDSPEnabled(bool b,bool bForceReload=false);
		void SetSteamAudioReverbDSPEnabled(bool b,bool bForceReload=false);
		void ClearSteamAudioSpatializerDSP();
		void ClearSteamAudioReverbDSP();
		FMOD::ChannelGroup &GetChannelGroup() const;
		void SetChannelGroup(FMOD::ChannelGroup &group);
		FMOD::ChannelGroup &InitializeChannelGroup();

		void UpdateSteamAudioAttributes();
		void UpdateSteamAudioIdentifier();
		void ApplySteamProperties();

		struct SteamAudioData
		{
			SteamAudioData(steam_audio::Properties &parent)
				: properties(parent)
			{}
			SteamAudioData(const SteamAudioData&)=delete;
			SteamAudioData &operator=(const SteamAudioData&)=delete;
			std::shared_ptr<FMOD::DSP> dspSpatializer = nullptr;
			std::shared_ptr<FMOD::DSP> dspReverb = nullptr;
			steam_audio::Properties properties = {};
		};
		std::unique_ptr<SteamAudioData> m_steamAudioData = nullptr;
		util::Overridable<bool> m_bSteamAudioSpatializerEnabled = false;
		util::Overridable<bool> m_bSteamAudioReverbEnabled = false;
		bool m_bCustomChannelGroup = false;
		mutable FMOD::ChannelGroup *m_channelGroup = nullptr;
#endif
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	public:
		void SetFMOD3DAttributesEffective(bool b);
	private:
		struct SoundSourceData
		{
			uint64_t offset = 0ull;
			uint32_t priority = 0u;
			bool looping = false;
			float pitch = 1.f;
			float gain = 1.f;
			std::pair<float,float> distanceRange = {1.f,10'000.f};
			Vector3 position = {};
			Vector3 velocity = {};
			std::pair<float,float> coneAngles = {360.f,360.f};
			float dopplerFactor = 1.f;
			bool relativeToListener = true;
		};
		void UpdateMode();
		bool Is3D() const;
		bool Is2D() const;
		bool CheckResultAndUpdateValidity(uint32_t result) const;
		void InvalidateSource() const;
		void InitializeChannel();
		SoundSourceData m_soundSourceData = {};
#endif
	private:
		SoundSystem &m_system;
		mutable InternalSource *m_source = nullptr;
		bool m_b3DAttributesEffective = true;
		std::weak_ptr<SoundBuffer> m_buffer = {};
		Effect::Params m_directFilter = {};
		std::string m_identifier = {};
		bool m_bReady = true;
		bool m_bSchedulePlay = false;
		std::vector<std::pair<std::shared_ptr<Effect>,uint32_t>> m_effects;
		PDecoder m_decoder = nullptr;
		mutable SoundSourceHandle m_handle = {};

		uint32_t m_nextAuxSlot = 0;
		std::queue<uint32_t> m_freeAuxEffectIds;

		virtual void OnReady();
		impl::BufferBase *GetBaseBuffer() const;
		void RemoveInternalEffect(decltype(m_effects)::iterator it);
	};
	using PSoundSource = std::shared_ptr<SoundSource>;
	using WPSoundSource = std::weak_ptr<SoundSource>;
};
#pragma warning(pop)

#endif
