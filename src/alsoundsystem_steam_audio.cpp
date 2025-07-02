// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "alsoundsystem.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#include "steam_audio/fmod/steam_audio_effects.hpp"
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
void al::ISoundSystem::ClearSteamAudioScene()
{
	for(auto &pSource : m_sources) {
		if(pSource == nullptr)
			continue;
		pSource->ClearSteamSoundEffects();
	}
	m_iplScene = nullptr;
	m_iplContext = nullptr;
}

ipl::Scene *al::ISoundSystem::InitializeSteamAudioScene()
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
ipl::Scene *al::ISoundSystem::GetSteamAudioScene() { return m_iplScene.get(); }

void al::ISoundSystem::SetSteamAudioEnabled(bool b)
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
bool al::ISoundSystem::IsSteamAudioEnabled() const { return m_bSteamAudioEnabled; }
const al::steam_audio::Properties &al::ISoundSystem::GetSteamAudioProperties() const { return const_cast<ISoundSystem *>(this)->GetSteamAudioProperties(); }
al::steam_audio::Properties &al::ISoundSystem::GetSteamAudioProperties() { return m_steamAudioProperties; }
void al::ISoundSystem::SetSteamAudioSpatializerEnabled(bool b) { m_bSteamAudioSpatializerEnabled = b; }
void al::ISoundSystem::SetSteamAudioReverbEnabled(bool b) { m_bSteamAudioReverbEnabled = b; }
util::Overridable<bool> &al::ISoundSystem::GetSteamAudioSpatializerEnabled() { return m_bSteamAudioSpatializerEnabled; }
util::Overridable<bool> &al::ISoundSystem::GetSteamAudioReverbEnabled() { return m_bSteamAudioReverbEnabled; }
#endif
