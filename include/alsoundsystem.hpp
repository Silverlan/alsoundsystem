/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUNDSYSTEM_HPP__
#define __ALSOUNDSYSTEM_HPP__

#include "alsound_definitions.hpp"
#include "alsound_types.hpp"
#include <sharedutils/functioncallback.h>
#include <unordered_map>
#include <queue>

#pragma warning(push)
#pragma warning(disable : 4251)
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/alsound_steam_audio_properties.hpp"
namespace ipl {
	class Context;
	class Scene;
	struct AudioDataBuffer;
};
#endif
namespace al {
	class DLLALSYS ISoundSystem {
	  public:
		enum class GlobalEffectFlag : uint32_t { None = 0, RelativeSounds = 1, WorldSounds = RelativeSounds << 1, All = RelativeSounds | WorldSounds };
		enum class Type : uint32_t { Buffer = 0, Decoder = 1 };

		virtual ~ISoundSystem();
		virtual void OnRelease();
	  public:
		template<class TEfxProperties>
		PEffect CreateEffect(const TEfxProperties &props);
		virtual IAuxiliaryEffectSlot *CreateAuxiliaryEffectSlot() = 0;
		void FreeAuxiliaryEffectSlot(IAuxiliaryEffectSlot *slot);

		// It's the caller's responsibility to destroy all shared pointer instances before the sound system is destroyed
		PSoundSource CreateSource(ISoundBuffer &buffer);
		PSoundSource CreateSource(Decoder &decoder);
		PSoundSource CreateSource(const std::string &name, bool bStereo, Type type = Type::Buffer);
		const std::vector<PSoundSource> &GetSources() const;
		std::vector<PSoundSource> &GetSources();
		void StopSounds();

		std::vector<ISoundBuffer *> GetBuffers() const;

		const IListener &GetListener() const;
		IListener &GetListener();

		ISoundBuffer *LoadSound(const std::string &path, bool bConvertToMono = false, bool bAsync = true);
		ISoundBuffer *GetBuffer(const std::string &path, bool bStereo = true);
		virtual PDecoder CreateDecoder(const std::string &path, bool bConvertToMono = false) = 0;

		virtual bool IsSupported(ChannelConfig channels, SampleType type) const = 0;

		virtual float GetDopplerFactor() const = 0;
		virtual void SetDopplerFactor(float factor) = 0;

		virtual float GetSpeedOfSound() const = 0;
		virtual void SetSpeedOfSound(float speed) = 0;

		virtual DistanceModel GetDistanceModel() const = 0;
		virtual void SetDistanceModel(DistanceModel mdl) = 0;

		virtual std::string GetDeviceName() const = 0;
		virtual void PauseDeviceDSP() = 0;
		virtual void ResumeDeviceDSP() = 0;

		virtual std::vector<std::string> GetDevices() = 0;
		virtual std::string GetDefaultDeviceName() = 0;

		// HRTF
		virtual std::vector<std::string> GetHRTFNames() const = 0;
		virtual std::string GetCurrentHRTF() const = 0;
		virtual bool IsHRTFEnabled() const = 0;
		virtual void SetHRTF(uint32_t id) = 0;
		virtual void DisableHRTF() = 0;

		uint32_t AddGlobalEffect(IEffect &effect, GlobalEffectFlag flags = GlobalEffectFlag::All, const EffectParams &params = EffectParams());
		void RemoveGlobalEffect(IEffect &effect);
		void RemoveGlobalEffect(uint32_t slotId);
		void SetGlobalEffectParameters(IEffect &effect, const EffectParams &params);
		void SetGlobalEffectParameters(uint32_t slotId, const EffectParams &params);
		virtual uint32_t GetMaxAuxiliaryEffectsPerSource() const = 0;

		void SetSoundSourceFactory(const SoundSourceFactory &factory);
		void SetOnReleaseSoundCallback(const std::function<void(const SoundSource &)> &onReleaseSoundCallback) { m_onReleaseSoundCallback = onReleaseSoundCallback; }

		virtual void Update();

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

		// Internal use only
		void OnSoundRelease(const SoundSource &snd);
	  protected:
		ISoundSystem(float metersPerUnit);
		void Initialize();
		virtual PSoundChannel CreateChannel(ISoundBuffer &buffer) = 0;
		virtual PSoundChannel CreateChannel(Decoder &decoder) = 0;
		virtual ISoundBuffer *DoLoadSound(const std::string &path, bool bConvertToMono = false, bool bAsync = true) = 0;
		PSoundSource InitializeSource(const std::shared_ptr<ISoundChannel> &channel);
		virtual std::unique_ptr<IListener> CreateListener() = 0;
		std::unique_ptr<IListener> m_listener;
		std::vector<PSoundSource> m_sources;

		struct BufferCache {
			PSoundBuffer mono = nullptr;
			PSoundBuffer stereo = nullptr;
			bool fileSourceMonoOnly = false;
		};
		std::unordered_map<std::string, BufferCache> m_buffers;
		std::vector<EffectHandle> m_effects;
		std::vector<PAuxiliaryEffectSlot> m_effectSlots;
		SoundSourceFactory m_soundSourceFactory = nullptr;

		uint32_t m_audioFrameSampleCount = 1'024;

		struct DLLALSYS GlobalEffect {
			struct DLLALSYS SoundInfo {
				~SoundInfo();
				SoundSourceHandle source = {};
				CallbackHandle relativeCallback = {};
				uint32_t slotId = std::numeric_limits<uint32_t>::max();
			};
			IEffect *effect = nullptr;
			GlobalEffectFlag flags = GlobalEffectFlag::None;
			EffectParams params = {};
			std::vector<std::shared_ptr<SoundInfo>> sourceInfo;
		};
		std::uint32_t m_nextGlobalEffectId = 0;
		std::queue<uint32_t> m_freeGlobalEffectIds;
		std::unordered_map<uint32_t, GlobalEffect> m_globalEffects;
		std::function<void(const SoundSource &)> m_onReleaseSoundCallback = nullptr;
		void RemoveGlobalEffect(GlobalEffect &globalEffect);
		void SetGlobalEffectParameters(GlobalEffect &globalEffect, const EffectParams &params);
		void ApplyGlobalEffect(SoundSource &source, GlobalEffect &globalEffect);
		void ApplyGlobalEffects(SoundSource &source);
		virtual PEffect CreateEffect() = 0;

		float m_metersPerUnit = 1.f;
		float m_speedOfSound = 343.3f;
		float m_dopplerFactor = 1.f;
		DistanceModel m_distanceModel = DistanceModel::LinearClamped;

		// Steam Audio
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		bool m_bSteamAudioEnabled = false;
		std::shared_ptr<ipl::Context> m_iplContext = nullptr;
		std::shared_ptr<ipl::Scene> m_iplScene = nullptr;
		std::unordered_map<std::string, std::shared_ptr<ipl::AudioDataBuffer>> m_audioBuffers;
#endif
	};
	REGISTER_BASIC_BITWISE_OPERATORS(ISoundSystem::GlobalEffectFlag);

	DLLALSYS bool get_sound_duration(const std::string path, float &duration);
};

#pragma warning(pop)

#endif
