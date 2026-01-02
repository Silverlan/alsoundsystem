// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.soundsystem;

import :binary_decoder;

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE && ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
pragma::audio::BinaryDecoder::BinaryDecoder(std::shared_ptr<ipl::AudioDataBuffer> &audioDataBuffer) : m_audioDataBuffer(audioDataBuffer) {}
pragma::audio::BinaryDecoder::~BinaryDecoder() {}
ALuint pragma::audio::BinaryDecoder::getFrequency() const { return DECODER_FREQUENCY; }
alure::ChannelConfig pragma::audio::BinaryDecoder::getChannelConfig() const { return alure::ChannelConfig::Mono; }
alure::SampleType pragma::audio::BinaryDecoder::getSampleType() const { return alure::SampleType::Float32; }
uint64_t pragma::audio::BinaryDecoder::getLength() const { return m_audioDataBuffer->audioResampler->GetOutputData().size(); }
uint64_t pragma::audio::BinaryDecoder::getPosition() const { return m_offset; }
bool pragma::audio::BinaryDecoder::seek(uint64_t pos)
{
	if(pos >= getLength())
		return false;
	m_offset = pos;
	return true;
}
std::pair<uint64_t, uint64_t> pragma::audio::BinaryDecoder::getLoopPoints() const { return {0ull, 0ull}; }
ALuint pragma::audio::BinaryDecoder::read(ALvoid *ptr, ALuint count)
{
	if(m_offset >= getLength())
		return 0u;
	auto readCount = pragma::math::min<uint64_t>(getLength() - m_offset, static_cast<uint64_t>(count));
	memcpy(ptr, m_audioDataBuffer->audioResampler->GetOutputData().data() + m_offset, readCount * sizeof(float));
	m_offset += readCount;
	return readCount;
}
#endif
