// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

import :system;
import pragma.filesystem;

pragma::audio::ISoundBuffer *pragma::audio::ISoundSystem::LoadSound(const std::string &path, bool bConvertToMono, bool bAsync)
{
	auto normPath = pragma::fs::get_normalized_path(path);

	auto it = m_buffers.find(normPath);
	if(it != m_buffers.end()) {
		auto &cache = it->second;
		if(bConvertToMono == false && cache.stereo != nullptr)
			return cache.stereo.get(); // Return stereo buffer
		if(cache.mono != nullptr && (bConvertToMono == true || cache.fileSourceMonoOnly == true))
			return cache.mono.get(); // Return mono buffer
	}
	return DoLoadSound(normPath, bConvertToMono, bAsync);
}

pragma::audio::ISoundBuffer *pragma::audio::ISoundSystem::GetBuffer(const std::string &path, bool bStereo)
{
	auto normPath = pragma::fs::get_normalized_path(path);
	auto it = m_buffers.find(normPath);
	if(it == m_buffers.end())
		return nullptr;
	if(bStereo == true && it->second.stereo != nullptr)
		return it->second.stereo.get();
	return it->second.mono.get();
}
