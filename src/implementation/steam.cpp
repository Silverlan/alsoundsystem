// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
void pragma::audio::SoundSource::InitializeSteamAudio()
{
	m_bSteamAudioSpatializerEnabled.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(newVal.get() == false)
			ClearSteamAudioSpatializerDSP();
		else
			UpdateSteamAudioDSPEffects();
	});
	m_bSteamAudioReverbEnabled.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(newVal.get() == false)
			ClearSteamAudioReverbDSP();
		else
			UpdateSteamAudioDSPEffects();
	});
}
bool pragma::audio::SoundSource::InitializeConvolutionEffect(const std::string &name)
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
	auto &bufferLoadData = *static_cast<impl::BufferLoadData *>(userData.get());
	if((bufferLoadData.flags & impl::BufferLoadData::Flags::SingleSourceDecoder) == impl::BufferLoadData::Flags::None)
		return false;
	auto err = iplCreateConvolutionEffect(iplScene->GetIplRenderer(), const_cast<char *>(name.c_str()), IPLSimulationType::IPL_SIMTYPE_REALTIME, iplScene->GetIplInputFormat(), iplScene->GetIplOutputFormat(), &m_iplConvolutionEffect);
	return (err == IPLerror::IPL_STATUS_SUCCESS) ? true : false;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return false;
#endif
}
void pragma::audio::SoundSource::ClearSteamSoundEffects()
{
	ClearSteamAudioSpatializerDSP();
	ClearSteamAudioReverbDSP();
	m_steamAudioData = nullptr;
}
void pragma::audio::SoundSource::SetSteamAudioEffectsEnabled(bool b)
{
	SetFMOD3DAttributesEffective(!b);
	if(b == false) {
		ClearSteamSoundEffects();
		m_steamAudioData = nullptr;
		return;
	}
	m_steamAudioData = std::make_unique<SteamAudioData>(m_system.GetSteamAudioProperties());
	auto &steamAudioData = *m_steamAudioData;
	steamAudioData.properties.spatializer.directBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_DIRECTBINAURAL, newVal.get()));
	});
	steamAudioData.properties.spatializer.distanceAttenuation.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_DISTANCEATTENUATION, newVal.get()));
	});
	steamAudioData.properties.spatializer.airAbsorption.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_AIRABSORPTION, newVal.get()));
	});
	steamAudioData.properties.spatializer.indirect.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_INDIRECT, newVal.get()));
	});
	steamAudioData.properties.spatializer.indirectBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_INDIRECTBINAURAL, newVal.get()));
	});
	steamAudioData.properties.spatializer.staticListener.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterBool(SA_SPATIALIZE_PARAM_STATICLISTENER, newVal.get()));
	});
	steamAudioData.properties.spatializer.HRTFInterpolation.AddChangeCallback([this](std::reference_wrapper<const pragma::audio::steam_audio::SpatializerInterpolation> oldVal, std::reference_wrapper<const pragma::audio::steam_audio::SpatializerInterpolation> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_HRTFINTERPOLATION, pragma::math::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.occlusionMode.AddChangeCallback([this](std::reference_wrapper<const pragma::audio::steam_audio::SpatializerOcclusionMode> oldVal, std::reference_wrapper<const pragma::audio::steam_audio::SpatializerOcclusionMode> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_OCCLUSIONMODE, pragma::math::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.occlusionMethod.AddChangeCallback([this](std::reference_wrapper<const pragma::audio::steam_audio::OcclusionMethod> oldVal, std::reference_wrapper<const pragma::audio::steam_audio::OcclusionMethod> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_OCCLUSIONMETHOD, pragma::math::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.simulationType.AddChangeCallback([this](std::reference_wrapper<const pragma::audio::steam_audio::SimulationType> oldVal, std::reference_wrapper<const pragma::audio::steam_audio::SimulationType> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterInt(SA_SPATIALIZE_PARAM_SIMTYPE, pragma::math::to_integral(newVal.get())));
	});
	steamAudioData.properties.spatializer.directLevel.AddChangeCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_DIRECTLEVEL, newVal.get()));
	});
	steamAudioData.properties.spatializer.indirectLevel.AddChangeCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
		if(m_steamAudioData->dspSpatializer == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_INDIRECTLEVEL, newVal.get()));
	});

	steamAudioData.properties.reverb.indirectBinaural.AddChangeCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		if(m_steamAudioData->dspReverb == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspReverb->setParameterBool(SA_REVERB_PARAM_BINAURAL, newVal.get()));
	});
	steamAudioData.properties.reverb.simulationType.AddChangeCallback([this](std::reference_wrapper<const pragma::audio::steam_audio::SimulationType> oldVal, std::reference_wrapper<const pragma::audio::steam_audio::SimulationType> newVal) {
		if(m_steamAudioData->dspReverb == nullptr)
			return;
		pragma::audio::fmod::check_result(m_steamAudioData->dspReverb->setParameterInt(SA_REVERB_PARAM_SIMTYPE, pragma::math::to_integral(newVal.get())));
	});
}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
extern FMOD_DSP_DESCRIPTION gReverbEffect;
extern FMOD_DSP_DESCRIPTION gSpatializerEffect;
void pragma::audio::SoundSource::UpdateSteamAudioDSPEffects()
{
	if(m_system.IsSteamAudioEnabled() == false) {
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
void pragma::audio::SoundSource::ClearSteamAudioSpatializerDSP()
{
	if(m_steamAudioData == nullptr || m_steamAudioData->dspSpatializer == nullptr)
		return;
	FMOD::ChannelGroup *group;
	pragma::audio::fmod::check_result(m_source->getChannelGroup(&group));
	if(group != nullptr)
		pragma::audio::fmod::check_result(group->removeDSP(m_steamAudioData->dspSpatializer.get()));
	m_steamAudioData->dspSpatializer = nullptr;
}
void pragma::audio::SoundSource::ClearSteamAudioReverbDSP()
{
	if(m_steamAudioData == nullptr || m_steamAudioData->dspReverb == nullptr)
		return;
	FMOD::ChannelGroup *group;
	pragma::audio::fmod::check_result(m_source->getChannelGroup(&group));
	if(group != nullptr)
		pragma::audio::fmod::check_result(group->removeDSP(m_steamAudioData->dspReverb.get()));
	m_steamAudioData->dspReverb = nullptr;
}
FMOD::ChannelGroup &pragma::audio::SoundSource::GetChannelGroup() const
{
	if(m_channelGroup != nullptr)
		return *m_channelGroup;
	FMOD::ChannelGroup *masterGroup;
	pragma::audio::fmod::check_result(m_system.GetFMODLowLevelSystem().getMasterChannelGroup(&masterGroup));
	assert(masterGroup != nullptr);
	if(masterGroup == nullptr)
		throw std::runtime_error("FMOD master channel group is invalid!");
	return *masterGroup;
}
void pragma::audio::SoundSource::SetChannelGroup(FMOD::ChannelGroup &group)
{
	pragma::audio::fmod::check_result(m_source->setChannelGroup(&group));
	m_channelGroup = &group;
}
FMOD::ChannelGroup &pragma::audio::SoundSource::InitializeChannelGroup()
{
	if(m_bCustomChannelGroup)
		return *m_channelGroup;
	m_bCustomChannelGroup = true;
	pragma::audio::fmod::check_result(m_system.GetFMODLowLevelSystem().createChannelGroup(nullptr, &m_channelGroup));
	SetChannelGroup(*m_channelGroup);
	return *m_channelGroup;
}
void pragma::audio::SoundSource::UpdateSteamAudioAttributes()
{
	auto orientation = GetOrientation();
	FMOD_3D_ATTRIBUTES attr {};
	attr.position = pragma::audio::to_custom_vector<FMOD_VECTOR>(pragma::audio::to_audio_position(GetPosition()));
	attr.up = pragma::audio::to_custom_vector<FMOD_VECTOR>(pragma::audio::to_audio_direction(orientation.second));
	attr.forward = pragma::audio::to_custom_vector<FMOD_VECTOR>(pragma::audio::to_audio_direction(orientation.first));
	attr.velocity = pragma::audio::to_custom_vector<FMOD_VECTOR>(pragma::audio::to_audio_position(GetVelocity()));
	FMOD_DSP_PARAMETER_3DATTRIBUTES attrParam {};
	attrParam.absolute = attr;
	if(m_steamAudioData != nullptr && m_steamAudioData->dspSpatializer != nullptr) {
		m_steamAudioData->dspSpatializer->setParameterFloat(SA_SPATIALIZE_PARAM_SOURCERADIUS, pragma::audio::to_audio_distance(GetRadius()));
		m_steamAudioData->dspSpatializer->setParameterData(SA_SPATIALIZE_PARAM_SOURCEPOSITION, &attrParam, sizeof(attrParam));
	}
	if(m_steamAudioData != nullptr && m_steamAudioData->dspReverb != nullptr)
		m_steamAudioData->dspReverb->setParameterData(SA_REVERB_PARAM_SOURCEPOSITION, &attrParam, sizeof(attrParam));
}
void pragma::audio::SoundSource::UpdateSteamAudioIdentifier()
{
	auto *iplScene = m_system.GetSteamAudioScene();
	IPLint32 identifier;
	if(iplScene == nullptr)
		return;
	if(m_steamAudioData != nullptr && m_steamAudioData->dspSpatializer != nullptr && iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(), ipl::Scene::DSPEffect::Spatializer, identifier) == true)
		m_steamAudioData->dspSpatializer->setParameterData(SA_SPATIALIZE_PARAM_NAME, &identifier, sizeof(identifier));
	if(m_steamAudioData != nullptr && m_steamAudioData->dspReverb != nullptr && iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(), ipl::Scene::DSPEffect::Reverb, identifier) == true)
		m_steamAudioData->dspReverb->setParameterData(SA_REVERB_PARAM_NAME, &identifier, sizeof(identifier));
}
void pragma::audio::SoundSource::SetSteamAudioSpatializerDSPEnabled(bool b, bool bForceReload)
{
	m_bSteamAudioSpatializerEnabled = b;
	if(m_steamAudioData == nullptr)
		return;
	pragma::util::ScopeGuard sg([this]() { ClearSteamAudioSpatializerDSP(); });
	if(b == false)
		return;
	if(bForceReload == true || m_steamAudioData->dspSpatializer == nullptr) {
		auto *iplScene = m_system.GetSteamAudioScene();
		IPLint32 identifier;
		if(m_steamAudioData->dspSpatializer != nullptr || iplScene == nullptr || iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(), ipl::Scene::DSPEffect::Spatializer, identifier) == false)
			return;
		FMOD::DSP *dsp;
		pragma::audio::fmod::check_result(m_system.GetFMODLowLevelSystem().createDSP(&gSpatializerEffect, &dsp));

		auto &channelGroup = InitializeChannelGroup();
		pragma::audio::fmod::check_result(channelGroup.addDSP(pragma::math::to_integral(DSPEffectSlot::Spatializer), dsp));

		m_steamAudioData->dspSpatializer = std::shared_ptr<FMOD::DSP>(dsp, [](FMOD::DSP *dsp) { dsp->release(); });
	}
	sg.dismiss();
	UpdateSteamAudioIdentifier();
	UpdateSteamAudioAttributes();
	ApplySteamProperties();
}
void pragma::audio::SoundSource::SetSteamAudioReverbDSPEnabled(bool b, bool bForceReload)
{
	m_bSteamAudioReverbEnabled = b;
	if(m_steamAudioData == nullptr)
		return;
	pragma::util::ScopeGuard sg([this]() { ClearSteamAudioReverbDSP(); });
	if(b == false)
		return;
	if(bForceReload == true || m_steamAudioData->dspReverb == nullptr) {
		auto *iplScene = m_system.GetSteamAudioScene();
		IPLint32 identifier;
		if(m_steamAudioData->dspReverb != nullptr || iplScene == nullptr || iplScene->FindBakedSoundSourceIdentifier(GetIdentifier(), ipl::Scene::DSPEffect::Reverb, identifier) == false)
			return;
		FMOD::DSP *dsp;
		pragma::audio::fmod::check_result(m_system.GetFMODLowLevelSystem().createDSP(&gReverbEffect, &dsp));

		auto &channelGroup = InitializeChannelGroup();
		pragma::audio::fmod::check_result(channelGroup.addDSP(pragma::math::to_integral(DSPEffectSlot::Reverb), dsp));

		m_steamAudioData->dspReverb = std::shared_ptr<FMOD::DSP>(dsp, [](FMOD::DSP *dsp) { dsp->release(); });
	}
	sg.dismiss();
	UpdateSteamAudioIdentifier();
	UpdateSteamAudioAttributes();
	ApplySteamProperties();
}
void pragma::audio::SoundSource::ApplySteamProperties()
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
