// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ALSOUND_EFFECT_HPP__
#define __ALSOUND_EFFECT_HPP__

#include "alsound_definitions.hpp"
#include "alsound_types.hpp"
#include "impl_alsound_source_handle.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {
	class Effect;
};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
namespace FMOD {
	class DSP;
};
#endif
namespace al {
	enum class VocalMorpherPhoneme : uint32_t { A = 0, E, I, O, U, AA, AE, AH, AO, EH, ER, IH, IY, UH, UW, B, D, F, G, J, K, L, M, N, P, R, S, T, V, Z };

	enum class Waveform : uint32_t {
		Sinusoid = 0,
		Triangle,
		Sawtooth // Vocal Morpher only
	};

	struct DLLALSYS EfxEaxReverbProperties {
		float flDensity = 1.0000f;
		float flDiffusion = 1.0000f;
		float flGain = 0.3162f;
		float flGainHF = 0.8913f;
		float flGainLF = 1.0000f;
		float flDecayTime = 1.4900f;
		float flDecayHFRatio = 0.8300f;
		float flDecayLFRatio = 1.0000f;
		float flReflectionsGain = 0.0500f;
		float flReflectionsDelay = 0.0070f;
		std::array<float, 3> flReflectionsPan = {0.0000f, 0.0000f, 0.0000f};
		float flLateReverbGain = 1.2589f;
		float flLateReverbDelay = 0.0110f;
		std::array<float, 3> flLateReverbPan = {0.0000f, 0.0000f, 0.0000f};
		float flEchoTime = 0.2500f;
		float flEchoDepth = 0.0000f;
		float flModulationTime = 0.2500f;
		float flModulationDepth = 0.0000f;
		float flAirAbsorptionGainHF = 0.9943f;
		float flHFReference = 5000.0000f;
		float flLFReference = 250.0000f;
		float flRoomRolloffFactor = 0.0000f;
		int iDecayHFLimit = 0x1;
	};

	struct DLLALSYS EfxChorusProperties {
		int32_t iWaveform = 1;
		int32_t iPhase = 90;
		float flRate = 1.1f;
		float flDepth = 0.1f;
		float flFeedback = 0.25f;
		float flDelay = 0.016f;
	};

	struct DLLALSYS EfxDistortionProperties {
		float flEdge = 0.2f;
		float flGain = 0.05f;
		float flLowpassCutoff = 8'000.f;
		float flEQCenter = 3'600.f;
		float flEQBandwidth = 3'600.f;
	};

	struct DLLALSYS EfxEchoProperties {
		float flDelay = 0.1f;
		float flLRDelay = 0.1f;
		float flDamping = 0.5f;
		float flFeedback = 0.5f;
		float flSpread = -1.f;
	};

	struct DLLALSYS EfxFlangerProperties {
		int32_t iWaveform = 1;
		int32_t iPhase = 0;
		float flRate = 0.27f;
		float flDepth = 1.f;
		float flFeedback = -0.5f;
		float flDelay = 0.002f;
	};

	struct DLLALSYS EfxFrequencyShifterProperties {
		float flFrequency = 0.f;
		int32_t iLeftDirection = 0;
		int32_t iRightDirection = 0;
	};

	struct DLLALSYS EfxVocalMorpherProperties {
		int32_t iPhonemeA = 0;
		int32_t iPhonemeB = 0;
		int32_t iPhonemeACoarseTuning = 0;
		int32_t iPhonemeBCoarseTuning = 0;
		int32_t iWaveform = 0;
		float flRate = 1.41f;
	};

	struct DLLALSYS EfxPitchShifterProperties {
		int32_t iCoarseTune = 12;
		int32_t iFineTune = 0;
	};

	struct DLLALSYS EfxRingModulatorProperties {
		float flFrequency = 440.f;
		float flHighpassCutoff = 800.f;
		int32_t iWaveform = 0;
	};

	struct DLLALSYS EfxAutoWahProperties {
		float flAttackTime = 0.06f;
		float flReleaseTime = 0.06f;
		float flResonance = 1'000.f;
		float flPeakGain = 11.22f;
	};

	struct DLLALSYS EfxCompressor {
		int32_t iOnOff = 1;
	};

	struct DLLALSYS EfxEqualizer {
		float flLowGain = 1.f;
		float flLowCutoff = 200.f;
		float flMid1Gain = 1.f;
		float flMid1Center = 500.f;
		float flMid1Width = 1.f;
		float flMid2Gain = 1.f;
		float flMid2Center = 3'000.f;
		float flMid2Width = 1.f;
		float flHighGain = 1.f;
		float flHighCutoff = 6'000.f;
	};

	class ISoundSystem;
	class ISoundChannel;
	class DLLALSYS IEffect : public std::enable_shared_from_this<IEffect> {
	  public:
		virtual ~IEffect();

		virtual void SetProperties(al::EfxEaxReverbProperties props) = 0;
		virtual void SetProperties(al::EfxChorusProperties props) = 0;
		virtual void SetProperties(al::EfxDistortionProperties props) = 0;
		virtual void SetProperties(al::EfxEchoProperties props) = 0;
		virtual void SetProperties(al::EfxFlangerProperties props) = 0;
		virtual void SetProperties(al::EfxFrequencyShifterProperties props) = 0;
		virtual void SetProperties(al::EfxVocalMorpherProperties props) = 0;
		virtual void SetProperties(al::EfxPitchShifterProperties props) = 0;
		virtual void SetProperties(al::EfxRingModulatorProperties props) = 0;
		virtual void SetProperties(al::EfxAutoWahProperties props) = 0;
		virtual void SetProperties(al::EfxCompressor props) = 0;
		virtual void SetProperties(al::EfxEqualizer props) = 0;

		void Release();
		EffectHandle GetHandle() const;
	  protected:
		IEffect(ISoundSystem &soundSys);
		ISoundSystem &m_soundSystem;
		EffectHandle m_handle = {};
		std::vector<SoundSourceHandle> m_attachedSources;

		// These should only be called by a SoundSource-instance!
		virtual IAuxiliaryEffectSlot *AttachSource(ISoundChannel &source) = 0;
		virtual void DetachSource(ISoundChannel &source) = 0;

		friend ISoundChannel;
		friend ISoundSystem;
	};
	using PEffect = std::shared_ptr<IEffect>;
	using WPEffect = std::weak_ptr<IEffect>;
};
#pragma warning(pop)

#endif
