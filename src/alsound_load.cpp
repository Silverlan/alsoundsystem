/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"
#include <fsys/filesystem.h>
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#endif
#include <cstring>

al::SoundBuffer *al::SoundSystem::LoadSound(const std::string &path,bool bConvertToMono,bool bAsync)
{
	auto normPath = FileManager::GetNormalizedPath(path);

	auto it = m_buffers.find(normPath);
	if(it != m_buffers.end())
	{
		auto &cache = it->second;
		if(bConvertToMono == false && cache.stereo != nullptr)
			return cache.stereo.get(); // Return stereo buffer
		if(cache.mono != nullptr && (bConvertToMono == true || cache.fileSourceMonoOnly == true))
			return cache.mono.get(); // Return mono buffer
	}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	alure::Buffer alBuf = nullptr;
	auto userData = std::make_shared<impl::BufferLoadData>(*this);
	if(bConvertToMono == true)
		userData->flags |= impl::BufferLoadData::Flags::ConvertToMono;
	auto originalChannel = ChannelConfig::Mono;
	if(bAsync == false)
		alBuf = m_context->getBuffer(normPath,userData,false,nullptr,reinterpret_cast<alure::ChannelConfig*>(&originalChannel));
	else
		alBuf = m_context->getBufferAsync(normPath,userData,false,nullptr,reinterpret_cast<alure::ChannelConfig*>(&originalChannel));
	if(alBuf == nullptr)
		return nullptr;
	auto buf = PSoundBuffer(new SoundBuffer(*m_context,alBuf,normPath));

	if(it == m_buffers.end())
		it = m_buffers.insert(decltype(m_buffers)::value_type({normPath,{}})).first;
	if(buf->GetChannelConfig() == al::ChannelConfig::Mono || bConvertToMono == true)
		it->second.mono = buf;
	else
		it->second.stereo = buf;
	if(originalChannel == ChannelConfig::Mono)
		it->second.fileSourceMonoOnly = true;
	if(bConvertToMono == true)
		buf->SetTargetChannelConfig(al::ChannelConfig::Mono);
	return buf.get();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::Sound *sound;
	FMOD_CREATESOUNDEXINFO exInfo {};
	memset(&exInfo,0,sizeof(exInfo));
	exInfo.cbsize = sizeof(exInfo);
	al::fmod::check_result(m_fmLowLevelSystem.createSound(normPath.c_str(),FMOD_DEFAULT,&exInfo,&sound));
	auto ptrSound = std::shared_ptr<FMOD::Sound>(sound,[](FMOD::Sound *sound) {
		al::fmod::check_result(sound->release());
	});
	auto buf = PSoundBuffer(new SoundBuffer(m_fmLowLevelSystem,ptrSound));
	if(it == m_buffers.end())
		it = m_buffers.insert(decltype(m_buffers)::value_type({normPath,{}})).first;
	if(buf->GetChannelConfig() == al::ChannelConfig::Mono || bConvertToMono == true)
		it->second.mono = buf;
	else
		it->second.stereo = buf;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(originalChannel == ChannelConfig::Mono)
		it->second.fileSourceMonoOnly = true; // FMOD TODO
#endif
	if(bConvertToMono == true)
		buf->SetTargetChannelConfig(al::ChannelConfig::Mono);
	return buf.get();
#endif
}

al::SoundBuffer *al::SoundSystem::GetBuffer(const std::string &path,bool bStereo)
{
	auto normPath = FileManager::GetNormalizedPath(path);
	auto it = m_buffers.find(normPath);
	if(it == m_buffers.end())
		return nullptr;
	if(bStereo == true && it->second.stereo != nullptr)
		return it->second.stereo.get();
	return it->second.mono.get();
}

al::PDecoder al::SoundSystem::CreateDecoder(const std::string &path,bool bConvertToMono)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto normPath = FileManager::GetNormalizedPath(path);

	auto userData = std::make_shared<impl::BufferLoadData>(*this);
	if(bConvertToMono == true)
		userData->flags |= impl::BufferLoadData::Flags::ConvertToMono;
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(IsSteamAudioEnabled() == true)
		userData->flags |= impl::BufferLoadData::Flags::SingleSourceDecoder; // TODO
#endif
	auto originalChannel = ChannelConfig::Mono;
	auto alDecoder = m_context->createDecoder(normPath,userData,nullptr,reinterpret_cast<alure::ChannelConfig*>(&originalChannel));
	if(alDecoder == nullptr)
		return nullptr;
	auto r = PDecoder(new Decoder(alDecoder,normPath));
	if(bConvertToMono == true)
		r->SetTargetChannelConfig(al::ChannelConfig::Mono);
	return r;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr;
#endif
}
