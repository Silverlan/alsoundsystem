/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_ENUMS_HPP__
#define __ALSOUND_ENUMS_HPP__

#include "alsound_definitions.hpp"
#include <array>
#include <mathutil/umath.h>

namespace al
{
	enum class DLLALSYS ChannelType : uint32_t
	{
		Mono = 0,
		Stereo
	};

	// See alure2.h
	enum class DLLALSYS ChannelConfig : uint32_t
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

	enum class DLLALSYS SampleType : uint32_t
	{
		UInt8,
		Int16,
		Float32,
		Mulaw
	};

	enum class DLLALSYS DistanceModel : uint32_t
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
