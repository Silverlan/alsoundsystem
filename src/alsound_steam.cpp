/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_source.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#include "alsoundsystem.hpp"
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <alure2.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/fmod/steam_audio_effects.hpp"
#include <sharedutils/scope_guard.h>
#endif
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
void al::SoundSource::InitializeSteamAudio()
{
	m_bSteamAudioSpatializerEnabled.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(newVal.get() == false)
			ClearSteamAudioSpatializerDSP();
		else
			UpdateSteamAudioDSPEffects();
	});
	m_bSteamAudioReverbEnabled.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(newVal.get() == false)
			ClearSteamAudioReverbDSP();
		else
			UpdateSteamAudioDSPEffects();
	});
}
bool al::SoundSource::InitializeConvolutionEffect(const std::string &name)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(m_iplConvolutionEffect != nullptr || m_decoder == nullptr)
		return false;
	auto &sys = GetSoundSystem();
	auto *iplScene = sys.GetSteamAudioScene();
	if(iplScene == nullptr)
		return false;
	auto &userData = m_decoder->GetALDecoder()->userData;
	if(userData == nullptr)
		return false;
	auto &bufferLoadData = *static_cast<impl::BufferLoadData*>(userData.get());
	if((bufferLoadData.flags &impl::BufferLoadData::Flags::SingleSourceDecoder) == impl::BufferLoadData::Flags::None)
		return false;
	auto err = iplCreateConvolutionEffect(iplScene->GetIplRenderer(),const_cast<char*>(name.c_str()),IPLSimulationType::IPL_SIMTYPE_REALTIME,iplScene->GetIplInputFormat(),iplScene->GetIplOutputFormat(),&m_iplConvolutionEffect);
	return (err == IPLerror::IPL_STATUS_SUCCESS) ? true : false;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}
void al::SoundSource::ClearSteamSoundEffects()
{
	ClearSteamAudioSpatializerDSP();
	ClearSteamAudioReverbDSP();
	m_steamAudioData = nullptr;
}
void al::SoundSource::SetSteamAudioEffectsEnabled(bool b)
{
	SetFMOD3DAttributesEffective(!b);
	if(b == false)
	{
		ClearSteamSoundEffects();
		m_steamAudioData = nullptr;
		return;
	}
	m_steamAudioData = std::make_unique<SteamAudioData>(m_system.GetSteamAudioProperties());
	auto &steamAudioData = *m_steamAudioData;
	steamAudioData.properties.spatializer.directBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_DIRECTBINAURAL,newVal.get()));
	});
	steamAudioData.properties.spatializer.distanceAttenuation.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_DISTANCEATTENUATION,newVal.get()));
	});
	steamAudioData.properties.spatializer.airAbsorption.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_AIRABSORPTION,newVal.get()));
	});
	steamAudioData.properties.spatializer.indirect.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_INDIRECT,newVal.get()));
	});
	steamAudioData.properties.spatializer.indirectBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_INDIRECTBINAURAL,newVal.get()));
	});
	steamAudioData.properties.spatializer.staticListener.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_STATICLISTENER,newVal.get()));
	});
	steamAudioData.properties.spatializer.HRTFInterpolation.AddChangeCallback([this](std::reference_wrapper<const al::steam_audio::SpatializerInterpolation> oldVal,std::reference_wrapper<const al::steam_audio::SpatializerInterpolation> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_HRTFINTERPOLATION,umath::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.occlusionMode.AddChangeCallback([this](std::reference_wrapper<const al::steam_audio::SpatializerOcclusionMode> oldVal,std::reference_wrapper<const al::steam_audio::SpatializerOcclusionMode> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_OCCLUSIONMODE,umath::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.occlusionMethod.AddChangeCallback([this](std::reference_wrapper<const al::steam_audio::OcclusionMethod> oldVal,std::reference_wrapper<const al::steam_audio::OcclusionMethod> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_OCCLUSIONMETHOD,umath::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.simulationType.AddChangeCallback([this](std::reference_wrapper<const al::steam_audio::SimulationType> oldVal,std::reference_wrapper<const al::steam_audio::SimulationType> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_SIMTYPE,umath::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.directLevel.AddChangeCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_DIRECTLEVEL,newVal.get()));
	});
	steamAudioData.properties.spatializer.indirectLevel.AddChangeCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_INDIRECTLEVEL,newVal.get()));
	});

	steamAudioData.properties.reverb.indirectBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspReverb == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspReverb->setParameterBool(SA_REVERB_PARAM_BINAURAL,newVal.get()));
	});
	steamAudioData.properties.reverb.simulationType.AddChangeCallback([this](std::reference_wrapper<const al::steam_audio::SimulationType> oldVal,std::reference_wrapper<const al::steam_audio::SimulationType> newVal) {
		if(m_steamAudioData->dspReverb == nullptr)
			return;
		al::fmod::check_result(m_steamAudioData->dspReverb->setParameterInt(SA_REVERB_PARAM_SIMTYPE,umath::to_integral(newVal.get())));
	});
}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
void al::SoundSource::UpdateSteamAudioDSPEffects()
{
	if(m_system.IsSteamAudioEnabled() == false)
	{
		ClearSteamAudioSpatializerDSP();
		ClearSteamAudioReverbDSP();
		return;
	}
	if(m_bSteamAudioSpatializerEnabled)
		SetSteamAudioSpatializerDSPEnabled(true);
	if(m_bSteamAudioReverbEnabled)
		SetSteamAudioReverbDSPEnabled(true);
	UpdateSteamAudioIdentifier();
}
void al::SoundSource::ClearSteamAudioSpatializerDSP()
{
	if(m_steamAudioData == nullptr || m_steamAudioData->dspSpatializer == nullptr)
		return;
	FMOD::ChannelGroup *group;
	al::fmod::check_result(m_source->getChannelGroup(&group));
	if(group != nullptr)
		al::fmod::check_result(group->removeDSP(m_steamAudioData->dspSpatializer.get()));
	m_steamAudioData->dspSpatializer = nullptr;
}
void al::SoundSource::ClearSteamAudioReverbDSP()
{
	if(m_steamAudioData == nullptr || m_steamAudioData->dspReverb == nullptr)
		return;
	FMOD::ChannelGroup *group;
	al::fmod::check_result(m_source->getChannelGroup(&group));
	if(group != nullptr)
		al::fmod::check_result(group->removeDSP(m_steamAudioData->dspReverb.get()));
	m_steamAudioData->dspReverb = nullptr;
}
FMOD::ChannelGroup &al::SoundSource::GetChannelGroup() const
{
	if(m_channelGroup != nullptr)
		return *m_channelGroup;
	FMOD::ChannelGroup *masterGroup;
	al::fmod::check_result(m_system.GetFMODLowLevelSystem().getMasterChannelGroup(&masterGroup));
	assert(masterGroup != nullptr);
	if(masterGroup == nullptr)
		throw std::runtime_error("FMOD master channel group is invalid!");
	return *masterGroup;
}
void al::SoundSource::SetChannelGroup(FMOD::ChannelGroup &group)
{
	al::fmod::check_result(m_source->setChannelGroup(&group));
	m_channelGroup = &group;
}
FMOD::ChannelGroup &al::SoundSource::InitializeChannelGroup()
{
	if(m_bCustomChannelGroup)
		return *m_channelGroup;
	m_bCustomChannelGroup = true;
	al::fmod::check_result(m_system.GetFMODLowLevelSystem().createChannelGroup(nullptr,&m_channelGroup));
	SetChannelGroup(*m_channelGroup);
	return *m_channelGroup;
}
void al::SoundSource::UpdateSteamAudioAttributes()
{
	auto orientation = GetOrientation();
	FMOD_3D_ATTRIBUTES attr {};
	attr.position = al::to_custom_vector<FMOD_VECTOR>(al::to_audio_position(GetPosition()));
	attr.up = al::to_custom_vector<FMOD_VECTOR>(al::to_audio_direction(orientation.second));
	attr.forward = al::to_custom_vector<FMOD_VECTOR>(al::to_audio_direction(orientation.first));
	attr.velocity = al::to_custom_vector<FMOD_VECTOR>(al::to_audio_position(GetVelocity()));
	FMOD_DSP_PARAMETER_3DATTRIBUTES attrParam {};
	attrParam.absolute = attr;
	if(m_steamAudioData != nullptr && m_steamAudioData->dspSpatializer != nullptr)
	{
		m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_SOURCERADIUS,al::to_audio_distance(GetRadius()));
		m_steamAudioData->dspSpatializer->setParameterData(SA_SPATIALIZE_PARAM_SOURCEPOSITION,&attrParam,sizeof(attrParam));
	}
	if(m_steamAudioData != nullptr && m_steamAudioData->dspReverb != nullptr)
		m_steamAudioData->dspReverb->setParameterData(SA_REVERB_PARAM_SOURCEPOSITION,&attrParam,sizeof(attrParam));
}
void al::SoundSource::UpdateSteamAudioIdentifier()
{
	auto *iplScene = m_system.GetSteamAudioScene();
	IPLint32 identifier;
	if(iplScene == nullptr)
		return;
	if(m_steamAudioData != nullptr && m_steamAudioData->dspSpatializer != nullptr && iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(),ipl::Scene::DSPEffect::Spatializer,identifier) == true)
		m_steamAudioData->dspSpatializer->setParameterData(SA_SPATIALIZE_PARAM_NAME,&identifier,sizeof(identifier));
	if(m_steamAudioData != nullptr && m_steamAudioData->dspReverb != nullptr && iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(),ipl::Scene::DSPEffect::Reverb,identifier) == true)
		m_steamAudioData->dspReverb->setParameterData(SA_REVERB_PARAM_NAME,&identifier,sizeof(identifier));
}
void al::SoundSource::SetSteamAudioSpatializerDSPEnabled(bool b,bool bForceReload)
{
	m_bSteamAudioSpatializerEnabled = b;
	if(m_steamAudioData == nullptr)
		return;
	ScopeGuard sg([this]() {
		ClearSteamAudioSpatializerDSP();
	});
	if(b == false)
		return;
	if(bForceReload == true || m_steamAudioData->dspSpatializer == nullptr)
	{
		auto *iplScene = m_system.GetSteamAudioScene();
		IPLint32 identifier;
		if(m_steamAudioData->dspSpatializer != nullptr || iplScene == nullptr || iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(),ipl::Scene::DSPEffect::Spatializer,identifier) == false)
			return;
		extern FMOD_DSP_DESCRIPTION gSpatializerEffect;

		FMOD::DSP *dsp;
		al::fmod::check_result(m_system.GetFMODLowLevelSystem().createDSP(&gSpatializerEffect,&dsp));

		auto &channelGroup = InitializeChannelGroup();
		al::fmod::check_result(channelGroup.addDSP(umath::to_integral(DSPEffectSlot::Spatializer),dsp));

		m_steamAudioData->dspSpatializer = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
			dsp->release();
		});
	}
	sg.dismiss();
	UpdateSteamAudioIdentifier();
	UpdateSteamAudioAttributes();
	ApplySteamProperties();
}
void al::SoundSource::SetSteamAudioReverbDSPEnabled(bool b,bool bForceReload)
{
	m_bSteamAudioReverbEnabled = b;
	if(m_steamAudioData == nullptr)
		return;
	ScopeGuard sg([this]() {
		ClearSteamAudioReverbDSP();
	});
	if(b == false)
		return;
	if(bForceReload == true || m_steamAudioData->dspReverb == nullptr)
	{
		auto *iplScene = m_system.GetSteamAudioScene();
		IPLint32 identifier;
		if(m_steamAudioData->dspReverb != nullptr || iplScene == nullptr || iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(),ipl::Scene::DSPEffect::Reverb,identifier) == false)
			return;
		extern FMOD_DSP_DESCRIPTION gReverbEffect;

		FMOD::DSP *dsp;
		al::fmod::check_result(m_system.GetFMODLowLevelSystem().createDSP(&gReverbEffect,&dsp));

		auto &channelGroup = InitializeChannelGroup();
		al::fmod::check_result(channelGroup.addDSP(umath::to_integral(DSPEffectSlot::Reverb),dsp));

		m_steamAudioData->dspReverb = std::shared_ptr<FMOD::DSP>(dsp,[](FMOD::DSP *dsp) {
			dsp->release();
		});
	}
	sg.dismiss();
	UpdateSteamAudioIdentifier();
	UpdateSteamAudioAttributes();
	ApplySteamProperties();
}
void al::SoundSource::ApplySteamProperties()
{
	if(m_steamAudioData == nullptr)
		return;
	auto *dspSpatializer = m_steamAudioData->dspSpatializer.get();
	auto *dspReverb = m_steamAudioData->dspReverb.get();
	m_steamAudioData->properties.spatializer.directBinaural.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.distanceAttenuation.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.airAbsorption.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.indirect.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.indirectBinaural.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.staticListener.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.HRTFInterpolation.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.occlusionMode.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.occlusionMethod.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.simulationType.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.directLevel.InvokeChangeCallbacks();
	m_steamAudioData->properties.spatializer.indirectLevel.InvokeChangeCallbacks();

	m_steamAudioData->properties.reverb.indirectBinaural.InvokeChangeCallbacks();
	m_steamAudioData->properties.reverb.simulationType.InvokeChangeCallbacks();
}
#endif
#endif
