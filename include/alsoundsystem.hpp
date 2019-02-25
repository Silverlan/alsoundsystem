/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUNDSYSTEM_HPP__
#define __ALSOUNDSYSTEM_HPP__

#include "alsound_definitions.hpp"
#include "alsound_enums.hpp"
#include "alsound_effect.hpp"
#include "alsound_auxiliaryeffectslot.hpp"
#include "alsound_source.hpp"
#include "alsound_buffer.hpp"
#include "alsound_listener.hpp"
#include "alsound_decoder.hpp"
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable:4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure
{
	class Device;
	class Context;
	class Buffer;
	class Source;
	class Listener;
	class Effect;
};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/alsound_steam_audio_properties.hpp"
#endif
namespace FMOD
{
	class System;
	class Channel;
	namespace Studio
	{
		class System;
	};
};
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
namespace ipl {class Context; class Scene; struct AudioDataBuffer;};
#endif
namespace al
{
	class SoundSystem;
	class DLLALSYS SoundSourceFactory
	{
	public:
		virtual SoundSource *CreateSoundSource(SoundSystem &system,SoundBuffer &buffer,InternalSource *source)=0;
		virtual SoundSource *CreateSoundSource(SoundSystem &system,Decoder &decoder,InternalSource *source)=0;
	};
	class DLLALSYS SoundSystem
	{
	public:
		enum class GlobalEffectFlag : uint32_t
		{
			None = 0,
			RelativeSounds = 1,
			WorldSounds = RelativeSounds<<1,
			All = RelativeSounds | WorldSounds
		};
		enum class Type : uint32_t
		{
			Buffer = 0,
			Decoder = 1
		};
		
		static std::shared_ptr<SoundSystem> Create(const std::string &deviceName,float metersPerUnit=1.f);
		static std::shared_ptr<SoundSystem> Create(float metersPerUnit=1.f);
		~SoundSystem();

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	public:
		const alure::Device *GetALDevice() const;
		alure::Device *GetALDevice();
		const alure::Context *GetALContext() const;
		alure::Context *GetALContext();
	private:
		SoundSystem(alure::Device *device,alure::Context *context,float metersPerUnit);
		alure::Device *m_device = nullptr;
		alure::Context *m_context = nullptr;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	public:
		const FMOD::Studio::System &GetFMODSystem() const;
		FMOD::Studio::System &GetFMODSystem();
		const FMOD::System &GetFMODLowLevelSystem() const;
		FMOD::System &GetFMODLowLevelSystem();
	private:
		SoundSystem(const std::shared_ptr<FMOD::Studio::System> &fmSystem,FMOD::System &lowLevelSystem,float metersPerUnit);
		std::shared_ptr<FMOD::Studio::System> m_fmSystem = nullptr;
		FMOD::System &m_fmLowLevelSystem;
#endif
	public:
		template<class TEfxProperties>
			PEffect CreateEffect(const TEfxProperties &props);
		AuxiliaryEffectSlot *CreateAuxiliaryEffectSlot();
		void FreeAuxiliaryEffectSlot(AuxiliaryEffectSlot *slot);

		// It's the caller's responsibility to destroy all shared pointer instances before the sound system is destroyed
		PSoundSource CreateSource(SoundBuffer &buffer);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
		PSoundSource CreateSource(Decoder &decoder);
#endif
		PSoundSource CreateSource(const std::string &name,bool bStereo,Type type=Type::Buffer);
		const std::vector<PSoundSource> &GetSources() const;
		std::vector<PSoundSource> &GetSources();
		void StopSounds();

		std::vector<SoundBuffer*> GetBuffers() const;

		const Listener &GetListener() const;
		Listener &GetListener();

		SoundBuffer *LoadSound(const std::string &path,bool bConvertToMono=false,bool bAsync=true);
		SoundBuffer *GetBuffer(const std::string &path,bool bStereo=true);
		PDecoder CreateDecoder(const std::string &path,bool bConvertToMono=false);

		bool IsSupported(ChannelConfig channels,SampleType type) const;

		float GetDopplerFactor() const;
		void SetDopplerFactor(float factor);

		float GetSpeedOfSound() const;
		void SetSpeedOfSound(float speed);

		DistanceModel GetDistanceModel() const;
		void SetDistanceModel(DistanceModel mdl);
		
		std::string GetDeviceName() const;
		void PauseDeviceDSP();
		void ResumeDeviceDSP();

		// HRTF
		std::vector<std::string> GetHRTFNames() const;
		std::string GetCurrentHRTF() const;
		bool IsHRTFEnabled() const;
		void SetHRTF(uint32_t id);
		void DisableHRTF();

		uint32_t AddGlobalEffect(Effect &effect,GlobalEffectFlag flags=GlobalEffectFlag::All,const Effect::Params &params=Effect::Params());
		void RemoveGlobalEffect(Effect &effect);
		void RemoveGlobalEffect(uint32_t slotId);
		void SetGlobalEffectParameters(Effect &effect,const Effect::Params &params);
		void SetGlobalEffectParameters(uint32_t slotId,const Effect::Params &params);
		uint32_t GetMaxAuxiliaryEffectsPerSource() const;

		void SetSoundSourceFactory(std::unique_ptr<SoundSourceFactory> factory);

		void Update();

		uint32_t GetAudioFrameSampleCount() const;
		void SetAudioFrameSampleCount(uint32_t size);

		// Steam Audio
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		ipl::Scene *InitializeSteamAudioScene();
		ipl::Scene *GetSteamAudioScene();
		void ClearSteamAudioScene();
		void SetSteamAudioEnabled(bool b);
		bool IsSteamAudioEnabled() const;
		const steam_audio::Properties &GetSteamAudioProperties() const;
		steam_audio::Properties &GetSteamAudioProperties();
		void SetSteamAudioSpatializerEnabled(bool b);
		void SetSteamAudioReverbEnabled(bool b);
		util::Overridable<bool> &GetSteamAudioSpatializerEnabled();
		util::Overridable<bool> &GetSteamAudioReverbEnabled();
	private:
		steam_audio::Properties m_steamAudioProperties = {};
		util::Overridable<bool> m_bSteamAudioSpatializerEnabled = false;
		util::Overridable<bool> m_bSteamAudioReverbEnabled = false;
#endif
	private:
		Listener m_listener;
		std::vector<PSoundSource> m_sources;

		struct BufferCache
		{
			PSoundBuffer mono = nullptr;
			PSoundBuffer stereo = nullptr;
			bool fileSourceMonoOnly = false;
		};
		std::unordered_map<std::string,BufferCache> m_buffers;
		std::vector<EffectHandle> m_effects;
		std::vector<PAuxiliaryEffectSlot> m_effectSlots;
		std::unique_ptr<SoundSourceFactory> m_soundSourceFactory = nullptr;

		uint32_t m_audioFrameSampleCount = 1'024;

		struct DLLALSYS GlobalEffect
		{
			struct DLLALSYS SoundInfo
			{
				~SoundInfo();
				SoundSourceHandle source = {};
				CallbackHandle relativeCallback = {};
				uint32_t slotId = std::numeric_limits<uint32_t>::max();
			};
			Effect *effect = nullptr;
			GlobalEffectFlag flags = GlobalEffectFlag::None;
			Effect::Params params = {};
			std::vector<std::shared_ptr<SoundInfo>> sourceInfo;
		};
		std::uint32_t m_nextGlobalEffectId = 0;
		std::queue<uint32_t> m_freeGlobalEffectIds;
		std::unordered_map<uint32_t,GlobalEffect> m_globalEffects;
		PSoundSource InitializeSource(SoundSource *source);
		void RemoveGlobalEffect(GlobalEffect &globalEffect);
		void SetGlobalEffectParameters(GlobalEffect &globalEffect,const Effect::Params &params);
		void ApplyGlobalEffect(SoundSource &source,GlobalEffect &globalEffect);
		void ApplyGlobalEffects(SoundSource &source);
		PEffect CreateEffect();

		float m_speedOfSound = 343.3f;
		float m_dopplerFactor = 1.f;
		DistanceModel m_distanceModel = DistanceModel::LinearClamped;

		// Steam Audio
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		bool m_bSteamAudioEnabled = false;
		std::shared_ptr<ipl::Context> m_iplContext = nullptr;
		std::shared_ptr<ipl::Scene> m_iplScene = nullptr;
		std::unordered_map<std::string,std::shared_ptr<ipl::AudioDataBuffer>> m_audioBuffers;
#endif
	};
	REGISTER_BASIC_BITWISE_OPERATORS(SoundSystem::GlobalEffectFlag);
	DLLALSYS std::vector<std::string> get_devices();
	DLLALSYS std::string get_default_device_name();
	DLLALSYS bool get_sound_duration(const std::string path,float &duration);
};

template<class TEfxProperties>
	al::PEffect al::SoundSystem::CreateEffect(const TEfxProperties &props)
{
	auto effect = CreateEffect();
	if(effect == nullptr)
		return nullptr;
	effect->SetProperties(props);
	m_effects.push_back(effect->GetHandle());
	return effect;
}

#pragma warning(pop)

#endif
