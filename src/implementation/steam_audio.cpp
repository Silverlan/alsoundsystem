// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
void pragma::audio::ISoundSystem::ClearSteamAudioScene()
{
	for(auto &pSource : m_sources) {
		if(pSource == nullptr)
			continue;
		pSource->ClearSteamSoundEffects();
	}
	m_iplScene = nullptr;
	m_iplContext = nullptr;
}

ipl::Scene *pragma::audio::ISoundSystem::InitializeSteamAudioScene()
{
	m_iplContext = ipl::Context::Create(GetAudioFrameSampleCount());
	if(m_iplContext == nullptr)
		return nullptr;
	// TODO
	m_iplContext->SetLogHandler([](std::string msg) {
		std::cout << "[STEAM AUDIO] Log: " << msg << std::endl;
		//throw std::runtime_error("Steam audio log: " +msg);
	});
	m_iplContext->SetErrorHandler([](IPLerror err) {
		std::cout << "[STEAM AUDIO] Error: " << ipl::result_to_string(err) << std::endl;
		throw std::runtime_error("Steam audio error: " + ipl::result_to_string(err));
	});
	m_iplScene = m_iplContext->CreateScene();
	return m_iplScene.get();
}
ipl::Scene *pragma::audio::ISoundSystem::GetSteamAudioScene() { return m_iplScene.get(); }

void pragma::audio::ISoundSystem::SetSteamAudioEnabled(bool b)
{
	m_bSteamAudioEnabled = b;
	for(auto &pSource : m_sources) {
		if(pSource == nullptr)
			continue;
		pSource->SetSteamAudioEffectsEnabled(b);
		pSource->UpdateSteamAudioDSPEffects();
	}
	if(b == false)
		ClearSteamAudioScene();
}
bool pragma::audio::ISoundSystem::IsSteamAudioEnabled() const { return m_bSteamAudioEnabled; }
const pragma::audio::steam_audio::Properties &pragma::audio::ISoundSystem::GetSteamAudioProperties() const { return const_cast<ISoundSystem *>(this)->GetSteamAudioProperties(); }
pragma::audio::steam_audio::Properties &pragma::audio::ISoundSystem::GetSteamAudioProperties() { return m_steamAudioProperties; }
void pragma::audio::ISoundSystem::SetSteamAudioSpatializerEnabled(bool b) { m_bSteamAudioSpatializerEnabled = b; }
void pragma::audio::ISoundSystem::SetSteamAudioReverbEnabled(bool b) { m_bSteamAudioReverbEnabled = b; }
pragma::util::Overridable<bool> &pragma::audio::ISoundSystem::GetSteamAudioSpatializerEnabled() { return m_bSteamAudioSpatializerEnabled; }
pragma::util::Overridable<bool> &pragma::audio::ISoundSystem::GetSteamAudioReverbEnabled() { return m_bSteamAudioReverbEnabled; }
#endif
