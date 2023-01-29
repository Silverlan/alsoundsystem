/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"
#include <fsys/filesystem.h>
#include <cstring>

al::ISoundBuffer *al::ISoundSystem::LoadSound(const std::string &path, bool bConvertToMono, bool bAsync)
{
	auto normPath = FileManager::GetNormalizedPath(path);

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

al::ISoundBuffer *al::ISoundSystem::GetBuffer(const std::string &path, bool bStereo)
{
	auto normPath = FileManager::GetNormalizedPath(path);
	auto it = m_buffers.find(normPath);
	if(it == m_buffers.end())
		return nullptr;
	if(bStereo == true && it->second.stereo != nullptr)
		return it->second.stereo.get();
	return it->second.mono.get();
}
