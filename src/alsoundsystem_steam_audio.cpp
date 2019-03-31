/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#include "steam_audio/fmod/steam_audio_effects.hpp"
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
void al::SoundSystem::ClearSteamAudioScene()
{
	for(auto &pSource : m_sources)
	{
		if(pSource == nullptr)
			continue;
		pSource->ClearSteamSoundEffects();
	}
	m_iplScene = nullptr;
	m_iplContext = nullptr;
}

ipl::Scene *al::SoundSystem::InitializeSteamAudioScene()
{
	m_iplContext = ipl::Context::Create(GetAudioFrameSampleCount());
	if(m_iplContext == nullptr)
		return nullptr;
	// TODO
	m_iplContext->SetLogHandler([](std::string msg) {
		std::cout<<"[STEAM AUDIO] Log: "<<msg<<std::endl;
		//throw std::runtime_error("Steam audio log: " +msg);
	});
	m_iplContext->SetErrorHandler([](IPLerror err) {
		std::cout<<"[STEAM AUDIO] Error: "<<ipl::result_to_string(err)<<std::endl;
		throw std::runtime_error("Steam audio error: " +ipl::result_to_string(err));
	});
	m_iplScene = m_iplContext->CreateScene();
	return m_iplScene.get();
}
ipl::Scene *al::SoundSystem::GetSteamAudioScene() {return m_iplScene.get();}

void al::SoundSystem::SetSteamAudioEnabled(bool b)
{
	m_bSteamAudioEnabled = b;
	for(auto &pSource : m_sources)
	{
		if(pSource == nullptr)
			continue;
		pSource->SetSteamAudioEffectsEnabled(b);
		pSource->UpdateSteamAudioDSPEffects();
	}
	if(b == false)
		ClearSteamAudioScene();
}
bool al::SoundSystem::IsSteamAudioEnabled() const {return m_bSteamAudioEnabled;}
const al::steam_audio::Properties &al::SoundSystem::GetSteamAudioProperties() const {return const_cast<SoundSystem*>(this)->GetSteamAudioProperties();}
al::steam_audio::Properties &al::SoundSystem::GetSteamAudioProperties() {return m_steamAudioProperties;}
void al::SoundSystem::SetSteamAudioSpatializerEnabled(bool b) {m_bSteamAudioSpatializerEnabled = b;}
void al::SoundSystem::SetSteamAudioReverbEnabled(bool b) {m_bSteamAudioReverbEnabled = b;}
util::Overridable<bool> &al::SoundSystem::GetSteamAudioSpatializerEnabled() {return m_bSteamAudioSpatializerEnabled;}
util::Overridable<bool> &al::SoundSystem::GetSteamAudioReverbEnabled() {return m_bSteamAudioReverbEnabled;}
#endif
