// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:types;

export import pragma.util;
export import std.compat;

export namespace pragma::audio {
	struct EffectParams;
	class SoundSource;
	class Decoder;
	class IEffect;
	class SoundSystem;
	class IListener;
	class ISoundBuffer;
	class ISoundChannel;
	class IAuxiliaryEffectSlot;

	using PEffect = std::shared_ptr<IEffect>;
	using PSoundSource = std::shared_ptr<SoundSource>;
	using PSoundChannel = std::shared_ptr<ISoundChannel>;
	using PDecoder = std::shared_ptr<Decoder>;
	using PAuxiliaryEffectSlot = std::shared_ptr<IAuxiliaryEffectSlot>;
	using PSoundBuffer = std::shared_ptr<ISoundBuffer>;

	using SoundSourceFactory = std::function<PSoundSource(const PSoundChannel &)>;

	using EffectHandle = pragma::util::TSharedHandle<IEffect>;
	using SoundSourceHandle = pragma::util::TSharedHandle<SoundSource>;

	struct DLLALSYS EffectParams {
		EffectParams(float gain = 1.f, float gainHF = 1.f, float gainLF = 1.f);
		float gain = 1.f;
		float gainHF = 1.f; // For low-pass and band-pass filters
		float gainLF = 1.f; // For high-pass and band-pass filters
	};

	enum class ChannelType : uint32_t { Mono = 0, Stereo };

	// See alure2.h
	enum class ChannelConfig : uint32_t {
		/** 1-channel mono sound. */
		Mono,
		/** 2-channel stereo sound. */
		Stereo,
		/** 2-channel rear sound (back-left and back-right). */
		Rear,
		/** 4-channel surround sound. */
		Quad,
		/** 5.1 surround sound. */
		X51,
		/** 6.1 surround sound. */
		X61,
		/** 7.1 surround sound. */
		X71,
		/** 3-channel B-Format, using FuMa channel ordering and scaling. */
		BFormat2D,
		/** 4-channel B-Format, using FuMa channel ordering and scaling. */
		BFormat3D
	};

	enum class SampleType : uint32_t { UInt8, Int16, Float32, Mulaw };

	enum class DistanceModel : uint32_t { InverseClamped = 0xD002, LinearClamped = 0xD004, ExponentClamped = 0xD006, Inverse = 0xD001, Linear = 0xD003, Exponent = 0xD005, None = 0 };

	struct DLLALSYS FilterParams {
		float mGain;
		float mGainHF; // For low-pass and band-pass filters
		float mGainLF; // For high-pass and band-pass filters
	};

	constexpr uint32_t INTERNAL_AUDIO_SAMPLE_RATE = 44'100u;
	constexpr uint32_t DECODER_FREQUENCY = INTERNAL_AUDIO_SAMPLE_RATE;

	// FMOD
	// constexpr uint32_t INTERNAL_AUDIO_SAMPLE_RATE = 48'000u;
	// constexpr uint32_t DECODER_FREQUENCY = 44'100u;
};
