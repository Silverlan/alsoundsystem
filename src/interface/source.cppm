// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:source;

export import :decoder;
export import :types;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace al {
	class ISoundBuffer;
	class ISoundSystem;
	class DLLALSYS ISoundChannel : virtual public util::CallbackHandler, virtual public util::inheritable_enable_shared_from_this<ISoundChannel> {
	  public:
		virtual ~ISoundChannel();

		enum class DSPEffectSlot : uint32_t {
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
			Spatializer,
			Reverb,
#endif
#endif
			Count
		};

		ISoundChannel(ISoundSystem &system, ISoundBuffer &buffer);
		ISoundChannel(ISoundSystem &system, Decoder &decoder);

		const ISoundBuffer *GetBuffer() const;
		ISoundBuffer *GetBuffer();
		const Decoder *GetDecoder() const;
		Decoder *GetDecoder();
		ISoundSystem &GetSoundSystem() const;

		float GetDuration() const;
		bool IsMono() const;
		bool IsStereo() const;

		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual bool IsPlaying() const = 0;
		virtual bool IsPaused() const = 0;
		bool IsStopped() const;

		virtual void SetPriority(uint32_t priority) = 0;
		virtual uint32_t GetPriority() const = 0;

		virtual void SetOffset(double offset) = 0;
		virtual double GetOffset() const = 0;

		virtual void SetLooping(bool bLoop) = 0;
		virtual bool IsLooping() const = 0;

		virtual void SetPitch(float pitch) = 0;
		virtual float GetPitch() const = 0;

		virtual void SetGain(float gain) = 0;
		virtual float GetGain() const = 0;

		virtual void SetGainRange(float minGain, float maxGain) = 0;
		virtual std::pair<float, float> GetGainRange() const = 0;
		void SetMinGain(float gain);
		virtual float GetMinGain() const = 0;
		void SetMaxGain(float gain);
		virtual float GetMaxGain() const = 0;

		virtual void SetDistanceRange(float refDist, float maxDist) = 0;
		virtual std::pair<float, float> GetDistanceRange() const = 0;
		void SetReferenceDistance(float dist);
		float GetReferenceDistance() const;
		void SetMaxDistance(float dist);
		float GetMaxDistance() const;

		virtual void SetPosition(const Vector3 &pos) = 0;
		virtual Vector3 GetPosition() const = 0;

		Vector3 GetWorldPosition() const;

		virtual void SetVelocity(const Vector3 &vel) = 0;
		virtual Vector3 GetVelocity() const = 0;

		virtual void SetDirection(const Vector3 &dir) = 0;
		virtual Vector3 GetDirection() const = 0;

		virtual void SetOrientation(const Vector3 &at, const Vector3 &up) = 0;
		virtual std::pair<Vector3, Vector3> GetOrientation() const = 0;

		virtual void SetConeAngles(float inner, float outer) = 0;
		virtual std::pair<float, float> GetConeAngles() const = 0;
		void SetInnerConeAngle(float inner);
		float GetInnerConeAngle() const;
		void SetOuterConeAngle(float outer);
		float GetOuterConeAngle() const;

		virtual void SetOuterConeGains(float gain, float gainHF = 1.f) = 0;
		void SetOuterConeGain(float gain);
		void SetOuterConeGainHF(float gain);
		virtual std::pair<float, float> GetOuterConeGains() const = 0;
		virtual float GetOuterConeGain() const = 0;
		virtual float GetOuterConeGainHF() const = 0;

		virtual void SetRolloffFactors(float factor, float roomFactor = 0.f) = 0;
		void SetRoomRolloffFactor(float roomFactor);
		virtual std::pair<float, float> GetRolloffFactors() const = 0;
		virtual float GetRolloffFactor() const = 0;
		virtual float GetRoomRolloffFactor() const = 0;

		virtual void SetDopplerFactor(float factor) = 0;
		virtual float GetDopplerFactor() const = 0;

		virtual void SetRelative(bool bRelative) = 0;
		virtual bool IsRelative() const = 0;

		virtual void SetRadius(float radius) = 0;
		virtual float GetRadius() const = 0;

		virtual void SetStereoAngles(float leftAngle, float rightAngle) = 0;
		virtual std::pair<float, float> GetStereoAngles() const = 0;
		void SetLeftStereoAngle(float ang);
		float GetLeftStereoAngle() const;
		void SetRightStereoAngle(float ang);
		float GetRightStereoAngle() const;

		virtual void SetAirAbsorptionFactor(float factor) = 0;
		virtual float GetAirAbsorptionFactor() const = 0;

		virtual void SetGainAuto(bool directHF, bool send, bool sendHF) = 0;
		virtual std::tuple<bool, bool, bool> GetGainAuto() const = 0;
		virtual bool GetDirectGainHFAuto() const = 0;
		virtual bool GetSendGainAuto() const = 0;
		virtual bool GetSendGainHFAuto() const = 0;

		float GetMaxAudibleDistance() const;

		virtual void SetDirectFilter(const EffectParams &params) = 0;
		const EffectParams &GetDirectFilter() const;
		bool AddEffect(IEffect &effect, const EffectParams &params = EffectParams());
		bool AddEffect(IEffect &effect, uint32_t &slotId, const EffectParams &params = EffectParams());
		bool AddEffect(IEffect &effect, float gain);
		bool AddEffect(IEffect &effect, uint32_t &slotId, float gain);
		void RemoveEffect(IEffect &effect);
		void RemoveEffect(uint32_t slotId);
		void RemoveEffects();
		void SetEffectParameters(IEffect &effect, const EffectParams &params);
		virtual void SetEffectParameters(uint32_t slotId, const EffectParams &params) = 0;
		void SetEffectGain(IEffect &effect, float gain);
		void SetEffectGain(uint32_t slotId, float gain);
		std::vector<IEffect *> GetEffects();

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
		void SetSteamAudioSpatializerDSPEnabled(bool b, bool bForceReload = false);
		void SetSteamAudioReverbDSPEnabled(bool b, bool bForceReload = false);
		void ClearSteamAudioSpatializerDSP();
		void ClearSteamAudioReverbDSP();
		FMOD::ChannelGroup &GetChannelGroup() const;
		void SetChannelGroup(FMOD::ChannelGroup &group);
		FMOD::ChannelGroup &InitializeChannelGroup();

		void UpdateSteamAudioAttributes();
		void UpdateSteamAudioIdentifier();
		void ApplySteamProperties();

		struct SteamAudioData {
			SteamAudioData(steam_audio::Properties &parent) : properties(parent) {}
			SteamAudioData(const SteamAudioData &) = delete;
			SteamAudioData &operator=(const SteamAudioData &) = delete;
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
	  protected:
		virtual void DoAddEffect(IAuxiliaryEffectSlot &slot, uint32_t slotId, const EffectParams &params) = 0;
		virtual void DoRemoveInternalEffect(uint32_t slotId) = 0;
		virtual void DoRemoveEffect(uint32_t slotId) = 0;
		ISoundSystem &m_system;
		bool m_b3DAttributesEffective = true;
		std::weak_ptr<ISoundBuffer> m_buffer = {};
		EffectParams m_directFilter = {};
		std::string m_identifier = {};
		bool m_bReady = true;
		bool m_bSchedulePlay = false;
		std::vector<std::pair<std::shared_ptr<IEffect>, uint32_t>> m_effects;
		PDecoder m_decoder = nullptr;

		uint32_t m_nextAuxSlot = 0;
		std::queue<uint32_t> m_freeAuxEffectIds;

		virtual void OnReady();
		impl::BufferBase *GetBaseBuffer() const;
		void RemoveInternalEffect(decltype(m_effects)::iterator it);
	};

	class DLLALSYS SoundSource {
	  public:
		static std::shared_ptr<SoundSource> Create(const std::shared_ptr<ISoundChannel> &channel);
		virtual ~SoundSource();
		void InitializeHandle(const std::shared_ptr<SoundSource> &ptr);
		SoundSourceHandle GetHandle() const;

		virtual void Update() { return (*this)->Update(); }
		virtual bool IsIdle() const { return (*this)->IsIdle(); }
		virtual void OnRelease();

		ISoundChannel &GetChannel() { return *m_channel; }
		const ISoundChannel &GetChannel() const { return const_cast<SoundSource *>(this)->GetChannel(); }
		ISoundChannel *operator->() { return &GetChannel(); }
		const ISoundChannel *operator->() const { return &GetChannel(); }
		ISoundChannel &operator*() { return GetChannel(); }
		const ISoundChannel &operator*() const { return GetChannel(); }
	protected:
		SoundSource(const std::shared_ptr<ISoundChannel> &channel);
	  private:
		std::shared_ptr<ISoundChannel> m_channel = nullptr;
		mutable SoundSourceHandle m_handle = {};
	};
	using PSoundSource = std::shared_ptr<SoundSource>;
	using WPSoundSource = std::weak_ptr<SoundSource>;
};
#pragma warning(pop)
