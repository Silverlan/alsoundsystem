// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:steam_audio.properties;

export import pragma.util;

export namespace al {
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
				spatializer.directBinaural = util::Overridable<bool> {parent.spatializer.directBinaural};
				spatializer.distanceAttenuation = util::Overridable<bool> {parent.spatializer.distanceAttenuation};
				spatializer.airAbsorption = util::Overridable<bool> {parent.spatializer.airAbsorption};
				spatializer.indirect = util::Overridable<bool> {parent.spatializer.indirect};
				spatializer.indirectBinaural = util::Overridable<bool> {parent.spatializer.indirectBinaural};
				spatializer.staticListener = util::Overridable<bool> {parent.spatializer.staticListener};

				spatializer.HRTFInterpolation = util::Overridable<al::steam_audio::SpatializerInterpolation> {parent.spatializer.HRTFInterpolation};
				spatializer.occlusionMode = util::Overridable<al::steam_audio::SpatializerOcclusionMode> {parent.spatializer.occlusionMode};
				spatializer.occlusionMethod = util::Overridable<al::steam_audio::OcclusionMethod> {parent.spatializer.occlusionMethod};
				spatializer.simulationType = util::Overridable<al::steam_audio::SimulationType> {parent.spatializer.simulationType};

				spatializer.directLevel = util::Overridable<float> {parent.spatializer.directLevel};
				spatializer.indirectLevel = util::Overridable<float> {parent.spatializer.indirectLevel};

				reverb.indirectBinaural = util::Overridable<bool> {parent.reverb.indirectBinaural};
				reverb.simulationType = util::Overridable<al::steam_audio::SimulationType> {parent.reverb.simulationType};
			}
			struct {
				util::Overridable<bool> directBinaural = {true};
				util::Overridable<bool> distanceAttenuation = {false};
				util::Overridable<bool> airAbsorption = {false};
				util::Overridable<bool> indirect = {false};
				util::Overridable<bool> indirectBinaural = {false};
				util::Overridable<bool> staticListener = {false};

				util::Overridable<SpatializerInterpolation> HRTFInterpolation = {SpatializerInterpolation::Nearest};
				util::Overridable<SpatializerOcclusionMode> occlusionMode = {SpatializerOcclusionMode::Off};
				util::Overridable<OcclusionMethod> occlusionMethod = {OcclusionMethod::Raycast};
				util::Overridable<SimulationType> simulationType = {SimulationType::Baked};

				util::Overridable<float> directLevel = {1.f};
				util::Overridable<float> indirectLevel = {1.f};
			} spatializer;
			struct {
				util::Overridable<bool> indirectBinaural = {false};
				util::Overridable<SimulationType> simulationType = {SimulationType::RealTime};
			} reverb;
		};
	};
};
