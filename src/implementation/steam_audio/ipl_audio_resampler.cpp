// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.soundsystem;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include <samplerate.h>

ipl::AudioResampler::AudioResampler(uint32_t inputFrequency, uint64_t inputLength, uint32_t outputFrequency)
{
	auto srcRatio = outputFrequency / static_cast<float>(inputFrequency);
	auto outputLength = pragma::math::ceil(srcRatio * inputLength);
	m_outputData.resize(outputLength);

	int err;
	m_srcState = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &err);

	m_srcData = std::make_shared<SRC_DATA>();
	auto &srcData = *static_cast<SRC_DATA *>(m_srcData.get());
	srcData.data_in = nullptr;
	srcData.input_frames = 0;
	srcData.data_out = m_outputData.data();
	srcData.output_frames = m_outputData.size();
	srcData.src_ratio = srcRatio;
}
ipl::AudioResampler::~AudioResampler()
{
	if(m_srcState != nullptr)
		src_delete(static_cast<SRC_STATE *>(m_srcState));
}
bool ipl::AudioResampler::Process(float *inputData, uint64_t inputOffset, uint32_t sampleCount, bool bEndOfInput, float **processedData, uint32_t &processedDataSize)
{
	if(m_srcState == nullptr || inputOffset != m_lastOffset || IsComplete())
		return false;
	auto &srcData = *static_cast<SRC_DATA *>(m_srcData.get());
	srcData.data_in = inputData;
	srcData.input_frames = sampleCount;
	srcData.end_of_input = bEndOfInput;

	auto err = src_process(static_cast<SRC_STATE *>(m_srcState), &srcData);
	if(err)
		return false;
	if(processedData != nullptr)
		*processedData = srcData.data_out;
	processedDataSize = srcData.output_frames_gen;

	srcData.data_out += srcData.output_frames_gen;
	srcData.output_frames -= srcData.output_frames_gen;
	m_lastOffset += sampleCount;
	if(srcData.end_of_input)
		m_bComplete = true;
	return true;
}
bool ipl::AudioResampler::Process(float *inputData, uint64_t inputOffset, uint32_t sampleCount, bool bEndOfInput)
{
	auto sz = 0u;
	return Process(inputData, inputOffset, sampleCount, bEndOfInput, nullptr, sz);
}
bool ipl::AudioResampler::IsComplete() const { return m_bComplete; }
std::vector<float> &ipl::AudioResampler::GetOutputData() { return m_outputData; }
#endif
