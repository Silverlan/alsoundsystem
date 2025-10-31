// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.soundsystem;

import :buffer_base;

al::impl::BufferLoadData::BufferLoadData(SoundSystem &sys) : soundSystem(sys) {}
al::impl::BufferBase::BufferBase::BufferBase(const std::string &path) : m_filePath(path) {}

#if 0
std::string al::impl::BufferBase::GetChannelConfigName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetChannelConfigName(static_cast<alure::ChannelConfig>(GetChannelConfig()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
std::string al::impl::BufferBase::GetSampleTypeName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetSampleTypeName(static_cast<alure::SampleType>(GetSampleType()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
#endif
std::string al::impl::BufferBase::GetFilePath() const { return m_filePath; }

void al::impl::BufferBase::SetTargetChannelConfig(ChannelConfig config) { m_targetChannelConfig = std::make_unique<ChannelConfig>(config); }
al::ChannelConfig al::impl::BufferBase::GetTargetChannelConfig() const { return (m_targetChannelConfig != nullptr) ? *m_targetChannelConfig : (IsStereo() ? ChannelConfig::Stereo : ChannelConfig::Mono); }

void al::impl::BufferBase::SetUserData(const std::shared_ptr<void> &userData) { m_userData = userData; }
std::shared_ptr<void> al::impl::BufferBase::GetUserData() const { return m_userData; }

bool al::impl::BufferBase::IsMono() const { return !IsStereo(); }
