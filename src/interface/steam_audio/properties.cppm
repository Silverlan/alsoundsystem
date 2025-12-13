// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:steam_audio.properties;

export import pragma.util;

export namespace pragma::audio {
	namespace steam_audio {
		enum class SpatializerInterpolation : uint32_t { Nearest = 0u, Bilinear };
		enum class SpatializerOcclusionMode : uint32_t { Off = 0u, NoTransmission, FrequencyIndependentTransmission, FrequencyDependentTransmission };
		enum class OcclusionMethod : uint32_t { Raycast = 0u, Partial };
		enum class IndirectType : uint32_t { RealTime = 0u, Baked };
		enum class SimulationType : uint32_t { RealTime = 0u, Baked };
		struct DLLALSYS Properties {
			Properties() = default;
			Properties(Properties &parent) : spatializer {}, reverb {}
			{
				spatializer.directBinaural = pragma::util::Overridable<bool> {parent.spatializer.directBinaural};
				spatializer.distanceAttenuation = pragma::util::Overridable<bool> {parent.spatializer.distanceAttenuation};
				spatializer.airAbsorption = pragma::util::Overridable<bool> {parent.spatializer.airAbsorption};
				spatializer.indirect = pragma::util::Overridable<bool> {parent.spatializer.indirect};
				spatializer.indirectBinaural = pragma::util::Overridable<bool> {parent.spatializer.indirectBinaural};
				spatializer.staticListener = pragma::util::Overridable<bool> {parent.spatializer.staticListener};

				spatializer.HRTFInterpolation = pragma::util::Overridable<SpatializerInterpolation> {parent.spatializer.HRTFInterpolation};
				spatializer.occlusionMode = pragma::util::Overridable<SpatializerOcclusionMode> {parent.spatializer.occlusionMode};
				spatializer.occlusionMethod = pragma::util::Overridable<OcclusionMethod> {parent.spatializer.occlusionMethod};
				spatializer.simulationType = pragma::util::Overridable<SimulationType> {parent.spatializer.simulationType};

				spatializer.directLevel = pragma::util::Overridable<float> {parent.spatializer.directLevel};
				spatializer.indirectLevel = pragma::util::Overridable<float> {parent.spatializer.indirectLevel};

				reverb.indirectBinaural = pragma::util::Overridable<bool> {parent.reverb.indirectBinaural};
				reverb.simulationType = pragma::util::Overridable<SimulationType> {parent.reverb.simulationType};
			}
			struct {
				pragma::util::Overridable<bool> directBinaural = {true};
				pragma::util::Overridable<bool> distanceAttenuation = {false};
				pragma::util::Overridable<bool> airAbsorption = {false};
				pragma::util::Overridable<bool> indirect = {false};
				pragma::util::Overridable<bool> indirectBinaural = {false};
				pragma::util::Overridable<bool> staticListener = {false};

				pragma::util::Overridable<SpatializerInterpolation> HRTFInterpolation = {SpatializerInterpolation::Nearest};
				pragma::util::Overridable<SpatializerOcclusionMode> occlusionMode = {SpatializerOcclusionMode::Off};
				pragma::util::Overridable<OcclusionMethod> occlusionMethod = {OcclusionMethod::Raycast};
				pragma::util::Overridable<SimulationType> simulationType = {SimulationType::Baked};

				pragma::util::Overridable<float> directLevel = {1.f};
				pragma::util::Overridable<float> indirectLevel = {1.f};
			} spatializer;
			struct {
				pragma::util::Overridable<bool> indirectBinaural = {false};
				pragma::util::Overridable<SimulationType> simulationType = {SimulationType::RealTime};
			} reverb;
		};
	};
};
