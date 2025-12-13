// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.soundsystem;

import :source;
import :system;
import pragma.audio.util;

namespace pragma::audio {
#ifdef _DEBUG
	class MessageHandler : public alure::MessageHandler {
	  public:
		virtual void deviceDisconnected(alure::Device *device) override { std::cout << "[SoundSystem] Device Disconnected: " << device->getName() << std::endl; }
		virtual void sourceStopped(alure::Source *source, bool forced) override { std::cout << "[SoundSystem] Source Stopped: " << source << " (" << forced << ")" << std::endl; }
		virtual void bufferLoading(const alure::String &name, alure::ChannelConfig channels, alure::SampleType type, ALuint samplerate, const alure::Vector<ALbyte> &data) override { std::cout << "[SoundSystem] Buffer Loading: " << name << std::endl; }
		virtual alure::String resourceNotFound(const alure::String &name) override
		{
			std::cout << "[SoundSystem] Resource not found: " << name << std::endl;
			return alure::MessageHandler::resourceNotFound(name);
		}
	};
#endif
};

template<typename T, typename T2>
void stereo_to_mono(T *stereoData, std::vector<uint8_t> &monoData, uint32_t datalen)
{
	auto numFrames = datalen / sizeof(T);
	monoData.resize(datalen / 2);
	auto *dstData = monoData.data();
	for(auto i = decltype(numFrames) {0}; i < numFrames; i += 2) {
		auto left = stereoData[i];
		auto right = stereoData[i + 1];
#pragma message("TODO: This leads to information loss / Worse quality. Dividing by two leads to lower volume however. Find a better way to convert from stereo to mono!")
		auto monoSample = static_cast<T2>(left) + static_cast<T2>(right); // /2;
		if(monoSample > std::numeric_limits<T>::max())
			monoSample = std::numeric_limits<T>::max();
		else if(monoSample < std::numeric_limits<T>::lowest())
			monoSample = std::numeric_limits<T>::lowest();
		auto rsMonoSample = static_cast<T>(monoSample);
		memcpy(&dstData[(i / 2) * sizeof(T)], &rsMonoSample, sizeof(T));
	}
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
static bool apply_steam_audio_processing(ipl::Scene &scene, pragma::audio::impl::BufferLoadData &loadData, void *sampleData, std::vector<uint8_t> &outputData, int32_t &format, alure::ChannelConfig channel, uint32_t &frequency, uint32_t dataLen)
{
	if(loadData.soundSystem.IsSteamAudioEnabled() == false || loadData.userData == nullptr || (loadData.flags & pragma::audio::impl::BufferLoadData::Flags::SingleSourceDecoder) == pragma::audio::impl::BufferLoadData::Flags::None || channel != alure::ChannelConfig::Mono || scene.IsComplete() == false)
		return false;
	auto hSound = *static_cast<pragma::audio::SoundSourceHandle *>(loadData.userData.get());
	if(hSound.IsValid() == false)
		return false;
	auto &snd = *hSound.get();
	if(snd.IsRelative() == true) {
		snd.ClearSteamSoundEffects();
		return false;
	}
	auto *decoder = snd.GetDecoder();
	if(decoder == nullptr)
		return false;

	if(snd.GetSteamAudioDirectSoundEffect() == nullptr)
		snd.InitializeDirectSoundEffect();
	auto *directSoundEffect = snd.GetSteamAudioDirectSoundEffect();
	if(directSoundEffect == nullptr)
		return false;

	auto &alDecoder = decoder->GetALDecoder();
#if IPL_ENABLE_PROBES != 0
	if(snd.GetSteamAudioConvolutionEffect() == nullptr) {
		auto &identifier = snd.GetIdentifier();
		if(identifier.empty() == false)
			snd.InitializeConvolutionEffect(identifier);
	}
#endif

	auto &listener = loadData.soundSystem.GetListener();
	auto &orientation = listener.GetOrientation();
	auto listenerPos = ipl::to_ipl_position(listener.GetPosition());
	auto listenerForward = ipl::to_ipl_direction(orientation.first);
	auto listenerUp = ipl::to_ipl_direction(orientation.second);

	auto sourcePos = ipl::to_ipl_position(snd.GetPosition());
	auto sourceRadius = ipl::to_ipl_distance(ipl::to_ipl_distance(snd.GetRadius()));
	auto directSndPath = iplGetDirectSoundPath(scene.GetIplEnvironment(), listenerPos, listenerForward, listenerUp, sourcePos, sourceRadius, IPLDirectOcclusionMode::IPL_DIRECTOCCLUSION_TRANSMISSIONBYFREQUENCY, IPLDirectOcclusionMethod::IPL_DIRECTOCCLUSION_RAYCAST);
	if(scene.IsPropagationDelayEnabled() == false)
		directSndPath.propagationDelay = 0.f;

	auto &renderSettings = scene.GetRenderSettings();

	auto sampleSize = 0u;
	auto sampleCast = std::function<float(uint8_t *)>(nullptr);
	auto sampleBackCast = std::function<void(float, uint8_t *)>(nullptr);
	switch(format) {
	case AL_MONO8_SOFT:
		sampleSize = sizeof(uint8_t);
		sampleCast = [](uint8_t *sampleVal) -> float {
			// uint8_t has to be mapped to the range of an int8_t before conversion
			return static_cast<int8_t>(static_cast<int32_t>(*sampleVal) - pragma::math::abs(static_cast<int32_t>(std::numeric_limits<int8_t>::lowest()))) / pragma::math::abs(static_cast<float>(std::numeric_limits<int8_t>::lowest()));
		};
		sampleBackCast = [](float inVal, uint8_t *outVal) {
			*outVal = static_cast<uint8_t>(pragma::math::clamp(inVal * pragma::math::abs(static_cast<float>(std::numeric_limits<int8_t>::lowest())), static_cast<float>(std::numeric_limits<int8_t>::lowest()),
			                                 static_cast<float>(std::numeric_limits<int8_t>::max()))
			  + pragma::math::abs(static_cast<int32_t>(std::numeric_limits<int8_t>::lowest())) // Map int8_t back to uint8_t
			);
		};
		break;
	case AL_MONO16_SOFT:
		sampleSize = sizeof(int16_t);
		sampleCast = [](uint8_t *sampleVal) -> float {
			return *reinterpret_cast<int16_t *>(sampleVal) / pragma::math::abs(static_cast<float>(std::numeric_limits<int16_t>::lowest())); // Negative integer has higher range, so use 'lowest' instead of 'max'
		};
		sampleBackCast = [](float inVal, uint8_t *outVal) {
			*reinterpret_cast<int16_t *>(outVal) = static_cast<int16_t>(pragma::math::clamp(inVal * pragma::math::abs(static_cast<float>(std::numeric_limits<int16_t>::lowest())), static_cast<float>(std::numeric_limits<int16_t>::lowest()), static_cast<float>(std::numeric_limits<int16_t>::max())));
		};
		break;
	case AL_MONO32F_SOFT:
		sampleSize = sizeof(float);
		// No data conversion required for floating point formats!
		break;
	default:
		return false;
	}
	auto bFloatFormat = format == AL_MONO32F_SOFT; // Float is a special case, since we can give float data to the steam API directly without converting anything

	auto sampleCount = dataLen / sampleSize;
	auto frameSize = sampleCount;

	static std::vector<float> tmpData;

	auto *srcBytes = static_cast<uint8_t *>(sampleData);

	static std::vector<float> processedData;
	if(processedData.size() < frameSize)
		processedData.resize(frameSize);

	auto *steamInputData = reinterpret_cast<float *>(srcBytes);
	auto *steamOutputData = reinterpret_cast<float *>(srcBytes);
	if(bFloatFormat == false) {
		tmpData.reserve(sampleCount);
		tmpData.clear();
		for(auto i = decltype(sampleCount) {0}; i < sampleCount; ++i)
			tmpData.push_back(sampleCast(srcBytes + i * sampleSize));
		steamInputData = tmpData.data();
		steamOutputData = processedData.data();
	}

	auto bOverwriteOutputData = false;
	if(dynamic_cast<pragma::audio::BinaryDecoder *>(alDecoder.get()) == nullptr) {
		// If this isn't a binary decoder, we're still streaming into a buffer
		// so that a binary decoder can be used in the future
		auto &wpBuffer = loadData.buffer;
		if(wpBuffer.expired() == false) {
			auto buf = wpBuffer.lock();
			auto pos = alDecoder->getPosition() - frameSize;
			auto numWrite = frameSize;

			if(buf->audioResampler == nullptr || buf->audioResampler->IsComplete() == false) {
				if(buf->audioResampler == nullptr)
					buf->audioResampler = std::make_shared<ipl::AudioResampler>(alDecoder->getFrequency(), alDecoder->getLength(), ipl::OUTPUT_SAMPLE_RATE);
				auto bEndOfInput = (pos + numWrite >= alDecoder->getLength()) ? true : false;

				float *resampledData = nullptr;
				auto processedSamples = 0u;
				if(buf->audioResampler->Process(steamInputData, pos, numWrite, bEndOfInput, &resampledData, processedSamples) == true) {
					outputData.resize(processedSamples * sizeof(resampledData[0]));
					bOverwriteOutputData = true;

					frequency = ipl::OUTPUT_SAMPLE_RATE;
					steamInputData = resampledData;
					sampleCount = processedSamples;
					steamOutputData = reinterpret_cast<float *>(outputData.data());
					format = AL_MONO32F_SOFT;
				}
			}
		}
	}
	IPLAudioBuffer inbuffer {scene.GetIplInputFormat(), frameSize, steamInputData};
	IPLDirectSoundEffectOptions opt {};
	opt.applyAirAbsorption = IPLbool::IPL_TRUE;
	opt.applyDistanceAttenuation = IPLbool::IPL_FALSE;
	opt.directOcclusionMode = IPLDirectOcclusionMode::IPL_DIRECTOCCLUSION_TRANSMISSIONBYFREQUENCY;

	IPLAudioBuffer outbuffer {scene.GetIplOutputFormat(), frameSize, steamOutputData};
	auto numframes = pragma::math::ceil(sampleCount / static_cast<float>(frameSize));
	for(auto i = decltype(numframes) {0}; i < numframes; ++i) {
		auto effectSamples = pragma::math::min(sampleCount, frameSize);
		inbuffer.numSamples = effectSamples;
		outbuffer.numSamples = effectSamples;
		iplApplyDirectSoundEffect(directSoundEffect, inbuffer, directSndPath, opt, outbuffer);

#if IPL_ENABLE_PROBES != 0
		auto *convolutionEffect = snd.GetSteamAudioConvolutionEffect();
		if(convolutionEffect != nullptr) {
			iplSetDryAudioForConvolutionEffect(convolutionEffect, listenerPos, outbuffer);

			iplGetWetAudioForConvolutionEffect(convolutionEffect, listenerPos, listenerForward, listenerUp, outbuffer);
		}
#endif

		inbuffer.interleavedBuffer += effectSamples;
		outbuffer.interleavedBuffer += effectSamples;
		sampleCount -= effectSamples;
	}
	if(bOverwriteOutputData == true)
		return bOverwriteOutputData;
	if(bFloatFormat == false) {
		auto offset = 0ull;
		for(auto v : processedData)
			sampleBackCast(v, srcBytes + offset++ * sampleSize);
	}
	return false;
}
#endif
#endif

pragma::audio::ISoundSystem::ISoundSystem(float metersPerUnit) : m_metersPerUnit {metersPerUnit}
{
	SetSoundSourceFactory([](const PSoundChannel &channel) -> PSoundSource {
		return SoundSource::Create(channel);
	});
}

pragma::audio::ISoundSystem::~ISoundSystem() { OnRelease(); }

void pragma::audio::ISoundSystem::OnRelease()
{
	m_sources.clear();
	m_effectSlots.clear();
	for(auto &hEffect : m_effects) {
		if(hEffect.IsValid() == true)
			hEffect->Release();
	}
	m_effects.clear();
	m_buffers.clear();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	ClearSteamAudioScene();
#endif
}

void pragma::audio::ISoundSystem::OnSoundRelease(const SoundSource &snd)
{
	if(m_onReleaseSoundCallback)
		m_onReleaseSoundCallback(snd);
}

void pragma::audio::ISoundSystem::Initialize()
{
	SetSpeedOfSound(340.29f / m_metersPerUnit);
	m_listener = CreateListener();
	m_listener->SetMetersPerUnit(m_metersPerUnit);
	assert(m_listener);
}

void pragma::audio::ISoundSystem::SetSoundSourceFactory(const SoundSourceFactory &factory) { m_soundSourceFactory = factory; }

void pragma::audio::ISoundSystem::FreeAuxiliaryEffectSlot(IAuxiliaryEffectSlot *slot)
{
	auto it = std::find_if(m_effectSlots.begin(), m_effectSlots.end(), [slot](const PAuxiliaryEffectSlot &slotOther) { return (slotOther.get() == slot) ? true : false; });
	if(it == m_effectSlots.end())
		return;
	m_effectSlots.erase(it);
}

void pragma::audio::ISoundSystem::Update()
{
	for(auto it = m_sources.begin(); it != m_sources.end();) {
		auto &src = *it;
		if(src.use_count() <= 1 && src->IsIdle() == true)
			it = m_sources.erase(it); // Sound isn't in use anymore (This is the last shared pointer instance)
		else {
			src->Update();
			++it;
		}
	}
}

pragma::audio::PSoundSource pragma::audio::ISoundSystem::InitializeSource(const std::shared_ptr<ISoundChannel> &channel)
{
	if(channel == nullptr)
		return nullptr;
	auto psource = m_soundSourceFactory ? m_soundSourceFactory(channel) : nullptr;
	if(!psource)
		return nullptr;
	m_sources.push_back(pragma::util::to_shared_handle(psource));
	psource->InitializeHandle(m_sources.back());
	ApplyGlobalEffects(*psource);

	//auto *decoder = source->GetDecoder();
	//auto *alDecoder = (decoder != nullptr) ? dynamic_cast<impl::DynamicDecoder*>(decoder->GetALDecoder().get()) : nullptr;
	//if(alDecoder != nullptr)
	//	alDecoder->SetSoundSource(*source);

	return psource;
}
pragma::audio::PSoundSource pragma::audio::ISoundSystem::CreateSource(const std::string &name, bool bStereo, Type type)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(IsSteamAudioEnabled() == false || bStereo == true)
#endif
	{
		switch(type) {
		case Type::Buffer:
			{
				auto *buf = GetBuffer(name, bStereo);
				if(buf == nullptr)
					return nullptr;
				return CreateSource(*buf);
			}
		case Type::Decoder:
			{
				auto decoder = CreateDecoder(name, !bStereo);
				if(decoder == nullptr)
					return nullptr;
				return CreateSource(*decoder);
			}
		}
		return nullptr;
	}
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto nname = FileManager::GetCanonicalizedPath(name);
	pragma::string::to_lower(nname);
	auto it = m_audioBuffers.find(nname);
	auto bNewDataBuffer = false;
	if(it == m_audioBuffers.end()) {
		it = m_audioBuffers.insert(std::make_pair(nname, std::make_shared<ipl::AudioDataBuffer>())).first;
		bNewDataBuffer = true;
	}
	auto &audioDataBuffer = *it->second;
	std::shared_ptr<pragma::audio::Decoder> decoder = nullptr;
	if(audioDataBuffer.audioResampler != nullptr && audioDataBuffer.audioResampler->IsComplete()) {
		auto alDecoder = std::shared_ptr<alure::Decoder>(new pragma::audio::BinaryDecoder(it->second));
		decoder = std::shared_ptr<pragma::audio::Decoder>(new pragma::audio::Decoder(alDecoder, name));
		auto userData = std::make_shared<impl::BufferLoadData>(*this);
		userData->flags |= impl::BufferLoadData::Flags::SingleSourceDecoder;
		alDecoder->userData = userData;
	}
	else {
		decoder = CreateDecoder(name, !bStereo);
		if(decoder == nullptr) {
			if(bNewDataBuffer == true)
				m_audioBuffers.erase(it);
		}
	}
	if(decoder == nullptr)
		return nullptr;
	auto source = CreateSource(*decoder);
	auto userData = decoder->GetALDecoder()->userData;
	if(userData == nullptr)
		return source;
	auto &loadData = *static_cast<impl::BufferLoadData *>(userData.get());
	if((loadData.flags & impl::BufferLoadData::Flags::SingleSourceDecoder) != impl::BufferLoadData::Flags::None) {
		loadData.userData = std::make_shared<pragma::audio::SoundSourceHandle>(source->GetHandle());
		loadData.buffer = it->second;
	}
	return source;
#endif
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}
const std::vector<pragma::audio::SoundSourceHandle> &pragma::audio::ISoundSystem::GetSources() const { return const_cast<ISoundSystem *>(this)->GetSources(); }
std::vector<pragma::audio::SoundSourceHandle> &pragma::audio::ISoundSystem::GetSources() { return m_sources; }

pragma::audio::PSoundSource pragma::audio::ISoundSystem::CreateSource(ISoundBuffer &buffer)
{
	auto channel = CreateChannel(buffer);
	if(!channel)
		return nullptr;
	return InitializeSource(channel);
}
pragma::audio::PSoundSource pragma::audio::ISoundSystem::CreateSource(Decoder &decoder)
{
	auto channel = CreateChannel(decoder);
	if(!channel)
		return nullptr;
	return InitializeSource(channel);
}

uint32_t pragma::audio::ISoundSystem::GetAudioFrameSampleCount() const { return m_audioFrameSampleCount; }
void pragma::audio::ISoundSystem::SetAudioFrameSampleCount(uint32_t size) { m_audioFrameSampleCount = size; }

std::vector<pragma::audio::ISoundBuffer *> pragma::audio::ISoundSystem::GetBuffers() const
{
	std::vector<ISoundBuffer *> buffers;
	buffers.reserve(m_buffers.size() * 2);
	for(auto &pair : m_buffers) {
		if(pair.second.mono != nullptr)
			buffers.push_back(pair.second.mono.get());
		if(pair.second.stereo != nullptr)
			buffers.push_back(pair.second.stereo.get());
	}
	return buffers;
}

void pragma::audio::ISoundSystem::StopSounds()
{
	for(auto &hSrc : m_sources)
		(*hSrc)->Stop();
}

const pragma::audio::IListener &pragma::audio::ISoundSystem::GetListener() const { return const_cast<ISoundSystem *>(this)->GetListener(); }
pragma::audio::IListener &pragma::audio::ISoundSystem::GetListener() { return *m_listener; }

bool pragma::audio::get_sound_duration(const std::string path, float &duration) { return pragma::audio::util::get_duration(path, duration); }
