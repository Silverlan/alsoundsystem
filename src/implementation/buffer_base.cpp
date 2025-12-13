// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.soundsystem;

import :buffer_base;

pragma::audio::impl::BufferLoadData::BufferLoadData(SoundSystem &sys) : soundSystem(sys) {}
pragma::audio::impl::BufferBase::BufferBase::BufferBase(const std::string &path) : m_filePath(path) {}

#if 0
std::string pragma::audio::impl::BufferBase::GetChannelConfigName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetChannelConfigName(static_cast<alure::ChannelConfig>(GetChannelConfig()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
std::string pragma::audio::impl::BufferBase::GetSampleTypeName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetSampleTypeName(static_cast<alure::SampleType>(GetSampleType()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
#endif
std::string pragma::audio::impl::BufferBase::GetFilePath() const { return m_filePath; }

void pragma::audio::impl::BufferBase::SetTargetChannelConfig(ChannelConfig config) { m_targetChannelConfig = std::make_unique<ChannelConfig>(config); }
pragma::audio::ChannelConfig pragma::audio::impl::BufferBase::GetTargetChannelConfig() const { return (m_targetChannelConfig != nullptr) ? *m_targetChannelConfig : (IsStereo() ? ChannelConfig::Stereo : ChannelConfig::Mono); }

void pragma::audio::impl::BufferBase::SetUserData(const std::shared_ptr<void> &userData) { m_userData = userData; }
std::shared_ptr<void> pragma::audio::impl::BufferBase::GetUserData() const { return m_userData; }

bool pragma::audio::impl::BufferBase::IsMono() const { return !IsStereo(); }
