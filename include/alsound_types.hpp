/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_ENUMS_HPP__
#define __ALSOUND_ENUMS_HPP__

#include "alsound_definitions.hpp"
#include <sharedutils/def_handle.h>
#include <functional>
#include <array>
#include <mathutil/umath.h>

namespace al
{
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
	
	using SoundSourceFactory = std::function<PSoundSource(const PSoundChannel&)>;

	DECLARE_BASE_HANDLE(DLLALSYS,IEffect,Effect);
	DECLARE_BASE_HANDLE(DLLALSYS,SoundSource,SoundSource);

	struct DLLALSYS EffectParams
	{
		EffectParams(float gain=1.f,float gainHF=1.f,float gainLF=1.f);
		float gain = 1.f;
		float gainHF = 1.f; // For low-pass and band-pass filters
		float gainLF = 1.f; // For high-pass and band-pass filters
	};

	enum class ChannelType : uint32_t
	{
		Mono = 0,
		Stereo
	};

	// See alure2.h
	enum class ChannelConfig : uint32_t
	{
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

	enum class SampleType : uint32_t
	{
		UInt8,
		Int16,
		Float32,
		Mulaw
	};

	enum class DistanceModel : uint32_t
	{
		InverseClamped  = 0xD002,
		LinearClamped   = 0xD004,
		ExponentClamped = 0xD006,
		Inverse  = 0xD001,
		Linear   = 0xD003,
		Exponent = 0xD005,
		None  = 0
	};

	struct DLLALSYS FilterParams {
		float mGain;
		float mGainHF; // For low-pass and band-pass filters
		float mGainLF; // For high-pass and band-pass filters
	};
};

#endif
