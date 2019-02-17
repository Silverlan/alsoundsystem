/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_effect.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#include <effect.h>
#include <context.h>
#include <efx.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include "alsoundsystem.hpp"
#include <fmod_studio.hpp>
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
static alure::ALContext *apply_effect_type(alure::Effect *effect,int32_t newType,const std::string &typeName,ALuint &effectId)
{
	auto *alEffect = static_cast<alure::ALEffect*>(effect);
	auto *context = alEffect->getContext();
	if(context == nullptr)
		return nullptr;
	auto type = alEffect->getType();
	effectId = alEffect->getId();

	if(type != newType)
	{
		alGetError();
		context->alEffecti(effectId,AL_EFFECT_TYPE,newType);
		if(alGetError() == AL_NO_ERROR)
			alEffect->setType(newType);
		else
			throw std::runtime_error("Failed to set " +typeName +" type");
	}
	return context;
}
#endif
void al::Effect::SetProperties(al::EfxChorusProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_CHORUS,"chorus",effectId);
	if(context == nullptr)
		return;
	context->alEffecti(effectId,AL_CHORUS_WAVEFORM,props.iWaveform);
	context->alEffecti(effectId,AL_CHORUS_PHASE,props.iPhase);
	context->alEffectf(effectId,AL_CHORUS_RATE,props.flRate);
	context->alEffectf(effectId,AL_CHORUS_DEPTH,props.flDepth);
	context->alEffectf(effectId,AL_CHORUS_FEEDBACK,props.flFeedback);
	context->alEffectf(effectId,AL_CHORUS_DELAY,props.flDelay);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::DSP *dsp;
	al::fmod::check_result(m_soundSystem.GetFMODLowLevelSystem().createDSPByType(FMOD_DSP_TYPE_CHORUS,&dsp));
	m_fmDsp = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
		al::fmod::check_result(dsp->release());
	});
	dsp->setParameterFloat(FMOD_DSP_ECHO_DELAY,props.flDelay);
	dsp->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK,props.flFeedback);
	//dsp->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL,props.);
	//dsp->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL,props.);
#endif
}

void al::Effect::SetProperties(al::EfxDistortionProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_DISTORTION,"distortion",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_DISTORTION_EDGE,props.flEdge);
	context->alEffectf(effectId,AL_DISTORTION_GAIN,props.flGain);
	context->alEffectf(effectId,AL_DISTORTION_LOWPASS_CUTOFF,props.flLowpassCutoff);
	context->alEffectf(effectId,AL_DISTORTION_EQCENTER,props.flEQCenter);
	context->alEffectf(effectId,AL_DISTORTION_EQBANDWIDTH,props.flEQBandwidth);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::DSP *dsp;
	al::fmod::check_result(m_soundSystem.GetFMODLowLevelSystem().createDSPByType(FMOD_DSP_TYPE_DISTORTION,&dsp));
	m_fmDsp = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
		al::fmod::check_result(dsp->release());
	});
	dsp->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL,props.flGain);
#endif
}
void al::Effect::SetProperties(al::EfxEchoProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_ECHO,"echo",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_ECHO_DELAY,props.flDelay);
	context->alEffectf(effectId,AL_ECHO_LRDELAY,props.flLRDelay);
	context->alEffectf(effectId,AL_ECHO_DAMPING,props.flDamping);
	context->alEffectf(effectId,AL_ECHO_FEEDBACK,props.flFeedback);
	context->alEffectf(effectId,AL_ECHO_SPREAD,props.flSpread);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::DSP *dsp;
	al::fmod::check_result(m_soundSystem.GetFMODLowLevelSystem().createDSPByType(FMOD_DSP_TYPE_ECHO,&dsp));
	m_fmDsp = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
		al::fmod::check_result(dsp->release());
	});
	dsp->setParameterFloat(FMOD_DSP_ECHO_DELAY,props.flDelay);
	dsp->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK,props.flFeedback);
	//dsp->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL,props.);
	//dsp->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL,props.);
#endif
}
void al::Effect::SetProperties(al::EfxFlangerProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_FLANGER,"flanger",effectId);
	if(context == nullptr)
		return;
	context->alEffecti(effectId,AL_FLANGER_WAVEFORM,props.iWaveform);
	context->alEffecti(effectId,AL_FLANGER_PHASE,props.iPhase);
	context->alEffectf(effectId,AL_FLANGER_RATE,props.flRate);
	context->alEffectf(effectId,AL_FLANGER_DEPTH,props.flDepth);
	context->alEffectf(effectId,AL_FLANGER_FEEDBACK,props.flFeedback);
	context->alEffectf(effectId,AL_FLANGER_DELAY,props.flDelay);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::DSP *dsp;
	al::fmod::check_result(m_soundSystem.GetFMODLowLevelSystem().createDSPByType(FMOD_DSP_TYPE_FLANGE,&dsp));
	m_fmDsp = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
		al::fmod::check_result(dsp->release());
	});
	//dsp->setParameterFloat(FMOD_DSP_FLANGE_MIX,props.); // FMOD TODO
	dsp->setParameterFloat(FMOD_DSP_FLANGE_DEPTH,props.flDepth);
	dsp->setParameterFloat(FMOD_DSP_FLANGE_RATE,props.flRate);
#endif
}
void al::Effect::SetProperties(al::EfxFrequencyShifterProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_FREQUENCY_SHIFTER,"frequency shifter",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_FREQUENCY_SHIFTER_FREQUENCY,props.flFrequency);
	context->alEffecti(effectId,AL_FREQUENCY_SHIFTER_LEFT_DIRECTION,props.iLeftDirection);
	context->alEffecti(effectId,AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION,props.iRightDirection);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxVocalMorpherProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_VOCAL_MORPHER,"vocal morpher",effectId);
	if(context == nullptr)
		return;
	context->alEffecti(effectId,AL_VOCAL_MORPHER_PHONEMEA,props.iPhonemeA);
	context->alEffecti(effectId,AL_VOCAL_MORPHER_PHONEMEB,props.iPhonemeB);
	context->alEffecti(effectId,AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING,props.iPhonemeACoarseTuning);
	context->alEffecti(effectId,AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING,props.iPhonemeBCoarseTuning);
	context->alEffecti(effectId,AL_VOCAL_MORPHER_WAVEFORM,props.iWaveform);
	context->alEffectf(effectId,AL_VOCAL_MORPHER_RATE,props.flRate);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxPitchShifterProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_PITCH_SHIFTER,"pitch shifter",effectId);
	if(context == nullptr)
		return;
	context->alEffecti(effectId,AL_PITCH_SHIFTER_COARSE_TUNE,props.iCoarseTune);
	context->alEffecti(effectId,AL_PITCH_SHIFTER_FINE_TUNE,props.iFineTune);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxRingModulatorProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_RING_MODULATOR,"ring modulator",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_RING_MODULATOR_FREQUENCY,props.flFrequency);
	context->alEffectf(effectId,AL_RING_MODULATOR_HIGHPASS_CUTOFF,props.flHighpassCutoff);
	context->alEffecti(effectId,AL_RING_MODULATOR_WAVEFORM,props.iWaveform);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxAutoWahProperties props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_AUTOWAH,"auto wah",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_AUTOWAH_ATTACK_TIME,props.flAttackTime);
	context->alEffectf(effectId,AL_AUTOWAH_RELEASE_TIME,props.flReleaseTime);
	context->alEffectf(effectId,AL_AUTOWAH_RESONANCE,props.flResonance);
	context->alEffectf(effectId,AL_AUTOWAH_PEAK_GAIN,props.flPeakGain);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxCompressor props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_COMPRESSOR,"compressor",effectId);
	if(context == nullptr)
		return;
	context->alEffecti(effectId,AL_COMPRESSOR_ONOFF,props.iOnOff);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::Effect::SetProperties(al::EfxEqualizer props)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	ALuint effectId;
	auto *context = apply_effect_type(m_effect,AL_EFFECT_EQUALIZER,"equalizer",effectId);
	if(context == nullptr)
		return;
	context->alEffectf(effectId,AL_EQUALIZER_LOW_GAIN,props.flLowGain);
	context->alEffectf(effectId,AL_EQUALIZER_LOW_CUTOFF,props.flLowCutoff);
	context->alEffectf(effectId,AL_EQUALIZER_MID1_GAIN,props.flMid1Gain);
	context->alEffectf(effectId,AL_EQUALIZER_MID1_CENTER,props.flMid1Center);
	context->alEffectf(effectId,AL_EQUALIZER_MID1_WIDTH,props.flMid1Width);
	context->alEffectf(effectId,AL_EQUALIZER_MID2_GAIN,props.flMid2Gain);
	context->alEffectf(effectId,AL_EQUALIZER_MID2_CENTER,props.flMid2Center);
	context->alEffectf(effectId,AL_EQUALIZER_MID2_WIDTH,props.flMid2Width);
	context->alEffectf(effectId,AL_EQUALIZER_HIGH_GAIN,props.flHighGain);
	context->alEffectf(effectId,AL_EQUALIZER_HIGH_CUTOFF,props.flHighCutoff);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
