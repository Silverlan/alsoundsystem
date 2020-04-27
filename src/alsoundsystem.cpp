/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"
#include "alsoundsystem_filefactory.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#include "alsound_binary_decoder.hpp"
#include "alsound_coordinate_system.hpp"
#include <sstream>
#include <fsys/filesystem.h>
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#include <alext.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/fmod/steam_audio_effects.hpp"
#endif
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
void al::fmod::check_result(uint32_t r)
{
	if(r != FMOD_OK)
	{
		std::cout<<"[FMOD] Error: "<<std::string(FMOD_ErrorString(static_cast<FMOD_RESULT>(r)))<<std::endl;
		// throw std::runtime_error("FMOD error: " +std::string(FMOD_ErrorString(static_cast<FMOD_RESULT>(r))));
	}
}
#endif

namespace al
{
	class DLLALSYS DefaultSoundSourceFactory
		: public SoundSourceFactory
	{
	public:
		virtual SoundSource *CreateSoundSource(SoundSystem &system,SoundBuffer &buffer,InternalSource *source) override {return new SoundSource(system,buffer,source);};
		virtual SoundSource *CreateSoundSource(SoundSystem &system,Decoder &decoder,InternalSource *source) override {return new SoundSource(system,decoder,source);};
	};
#ifdef _DEBUG
	class MessageHandler
		: public alure::MessageHandler
	{
	public:
		virtual void deviceDisconnected(alure::Device *device) override
		{
			std::cout<<"[SoundSystem] Device Disconnected: "<<device->getName()<<std::endl;
		}
		virtual void sourceStopped(alure::Source *source, bool forced) override
		{
			std::cout<<"[SoundSystem] Source Stopped: "<<source<<" ("<<forced<<")"<<std::endl;
		}
		virtual void bufferLoading(const alure::String &name, alure::ChannelConfig channels, alure::SampleType type, ALuint samplerate, const alure::Vector<ALbyte> &data) override
		{
			std::cout<<"[SoundSystem] Buffer Loading: "<<name<<std::endl;
		}
		virtual alure::String resourceNotFound(const alure::String &name)  override
		{
			std::cout<<"[SoundSystem] Resource not found: "<<name<<std::endl;
			return alure::MessageHandler::resourceNotFound(name);
		}
	};
#endif
};

template<typename T,typename T2>
	void stereo_to_mono(T *stereoData,std::vector<uint8_t> &monoData,uint32_t datalen)
{
	auto numFrames = datalen /sizeof(T);
	monoData.resize(datalen /2);
	auto *dstData = monoData.data();
	for(auto i=decltype(numFrames){0};i<numFrames;i+=2)
	{
		auto left = stereoData[i];
		auto right = stereoData[i +1];
#pragma message("TODO: This leads to information loss / Worse quality. Dividing by two leads to lower volume however. Find a better way to convert from stereo to mono!")
		auto monoSample = static_cast<T2>(left) +static_cast<T2>(right); // /2;
		if(monoSample > std::numeric_limits<T>::max())
			monoSample = std::numeric_limits<T>::max();
		else if(monoSample < std::numeric_limits<T>::lowest())
			monoSample = std::numeric_limits<T>::lowest();
		auto rsMonoSample = static_cast<T>(monoSample);
		memcpy(&dstData[(i /2) *sizeof(T)],&rsMonoSample,sizeof(T));
	}
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
static bool apply_steam_audio_processing(ipl::Scene &scene,al::impl::BufferLoadData &loadData,void *sampleData,std::vector<uint8_t> &outputData,int32_t &format,alure::ChannelConfig channel,uint32_t &frequency,uint32_t dataLen)
{
	if(
		loadData.soundSystem.IsSteamAudioEnabled() == false || loadData.userData == nullptr || 
		(loadData.flags &al::impl::BufferLoadData::Flags::SingleSourceDecoder) == al::impl::BufferLoadData::Flags::None || 
		channel != alure::ChannelConfig::Mono || scene.IsComplete() == false
	)
		return false;
	auto hSound = *static_cast<al::SoundSourceHandle*>(loadData.userData.get());
	if(hSound.IsValid() == false)
		return false;
	auto &snd = *hSound.get();
	if(snd.IsRelative() == true)
	{
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
	if(snd.GetSteamAudioConvolutionEffect() == nullptr)
	{
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
	auto directSndPath = iplGetDirectSoundPath(scene.GetIplEnvironment(),listenerPos,listenerForward,listenerUp,sourcePos,sourceRadius,IPLDirectOcclusionMode::IPL_DIRECTOCCLUSION_TRANSMISSIONBYFREQUENCY,IPLDirectOcclusionMethod::IPL_DIRECTOCCLUSION_RAYCAST);
	if(scene.IsPropagationDelayEnabled() == false)
		directSndPath.propagationDelay = 0.f;

	auto &renderSettings = scene.GetRenderSettings();

	auto sampleSize = 0u;
	auto sampleCast = std::function<float(uint8_t*)>(nullptr);
	auto sampleBackCast = std::function<void(float,uint8_t*)>(nullptr);
	switch(format)
	{
		case AL_MONO8_SOFT:
			sampleSize = sizeof(uint8_t);
			sampleCast = [](uint8_t *sampleVal) -> float {
				// uint8_t has to be mapped to the range of an int8_t before conversion
				return static_cast<int8_t>(static_cast<int32_t>(*sampleVal) -umath::abs(static_cast<int32_t>(std::numeric_limits<int8_t>::lowest()))) /umath::abs(static_cast<float>(std::numeric_limits<int8_t>::lowest()));
			};
			sampleBackCast = [](float inVal,uint8_t *outVal) {
				*outVal = static_cast<uint8_t>(
					umath::clamp(
						inVal *umath::abs(static_cast<float>(std::numeric_limits<int8_t>::lowest())),
						static_cast<float>(std::numeric_limits<int8_t>::lowest()),
						static_cast<float>(std::numeric_limits<int8_t>::max())
					) +umath::abs(static_cast<int32_t>(std::numeric_limits<int8_t>::lowest())) // Map int8_t back to uint8_t
				);
			};
			break;
		case AL_MONO16_SOFT:
			sampleSize = sizeof(int16_t);
			sampleCast = [](uint8_t *sampleVal) -> float {
				return *reinterpret_cast<int16_t*>(sampleVal) /umath::abs(static_cast<float>(std::numeric_limits<int16_t>::lowest())); // Negative integer has higher range, so use 'lowest' instead of 'max'
			};
			sampleBackCast = [](float inVal,uint8_t *outVal) {
				*reinterpret_cast<int16_t*>(outVal) = static_cast<int16_t>(
					umath::clamp(
						inVal *umath::abs(static_cast<float>(std::numeric_limits<int16_t>::lowest())),
						static_cast<float>(std::numeric_limits<int16_t>::lowest()),
						static_cast<float>(std::numeric_limits<int16_t>::max())
					)
				);
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

	auto sampleCount = dataLen /sampleSize;
	auto frameSize = sampleCount;

	static std::vector<float> tmpData;

	auto *srcBytes = static_cast<uint8_t*>(sampleData);

	static std::vector<float> processedData;
	if(processedData.size() < frameSize)
		processedData.resize(frameSize);

	auto *steamInputData = reinterpret_cast<float*>(srcBytes);
	auto *steamOutputData = reinterpret_cast<float*>(srcBytes);
	if(bFloatFormat == false)
	{
		tmpData.reserve(sampleCount);
		tmpData.clear();
		for(auto i=decltype(sampleCount){0};i<sampleCount;++i)
			tmpData.push_back(sampleCast(srcBytes +i *sampleSize));
		steamInputData = tmpData.data();
		steamOutputData = processedData.data();
	}

	auto bOverwriteOutputData = false;
	if(dynamic_cast<al::BinaryDecoder*>(alDecoder.get()) == nullptr)
	{
		// If this isn't a binary decoder, we're still streaming into a buffer
		// so that a binary decoder can be used in the future
		auto &wpBuffer = loadData.buffer;
		if(wpBuffer.expired() == false)
		{
			auto buf = wpBuffer.lock();
			auto pos = alDecoder->getPosition() -frameSize;
			auto numWrite = frameSize;
			
			if(buf->audioResampler == nullptr || buf->audioResampler->IsComplete() == false)
			{
				if(buf->audioResampler == nullptr)
					buf->audioResampler = std::make_shared<ipl::AudioResampler>(alDecoder->getFrequency(),alDecoder->getLength(),ipl::OUTPUT_SAMPLE_RATE);
				auto bEndOfInput = (pos +numWrite >= alDecoder->getLength()) ? true : false;

				float *resampledData = nullptr;
				auto processedSamples = 0u;
				if(buf->audioResampler->Process(steamInputData,pos,numWrite,bEndOfInput,&resampledData,processedSamples) == true)
				{
					outputData.resize(processedSamples *sizeof(resampledData[0]));
					bOverwriteOutputData = true;

					frequency = ipl::OUTPUT_SAMPLE_RATE;
					steamInputData = resampledData;
					sampleCount = processedSamples;
					steamOutputData = reinterpret_cast<float*>(outputData.data());
					format = AL_MONO32F_SOFT;
				}
			}
		}
	}
	IPLAudioBuffer inbuffer {scene.GetIplInputFormat(),frameSize,steamInputData};
	IPLDirectSoundEffectOptions opt {};
	opt.applyAirAbsorption = IPLbool::IPL_TRUE;
	opt.applyDistanceAttenuation = IPLbool::IPL_FALSE;
	opt.directOcclusionMode = IPLDirectOcclusionMode::IPL_DIRECTOCCLUSION_TRANSMISSIONBYFREQUENCY;

	IPLAudioBuffer outbuffer {scene.GetIplOutputFormat(),frameSize,steamOutputData};
	auto numframes = umath::ceil(sampleCount /static_cast<float>(frameSize));
	for(auto i=decltype(numframes){0};i<numframes;++i)
	{
		auto effectSamples = umath::min(sampleCount,frameSize);
		inbuffer.numSamples = effectSamples;
		outbuffer.numSamples = effectSamples;
		iplApplyDirectSoundEffect(directSoundEffect,inbuffer,directSndPath,opt,outbuffer);

#if IPL_ENABLE_PROBES != 0
		auto *convolutionEffect = snd.GetSteamAudioConvolutionEffect();
		if(convolutionEffect != nullptr)
		{
			iplSetDryAudioForConvolutionEffect(convolutionEffect,listenerPos,outbuffer);
		
			iplGetWetAudioForConvolutionEffect(convolutionEffect,listenerPos,listenerForward,listenerUp,outbuffer);
		}
#endif

		inbuffer.interleavedBuffer += effectSamples;
		outbuffer.interleavedBuffer += effectSamples;
		sampleCount -= effectSamples;
	}
	if(bOverwriteOutputData == true)
		return bOverwriteOutputData;
	if(bFloatFormat == false)
	{
		auto offset = 0ull;
		for(auto v : processedData)
			sampleBackCast(v,srcBytes +offset++ *sampleSize);
	}
	return false;
}
#endif
#endif

std::shared_ptr<al::SoundSystem> al::SoundSystem::Create(const std::string &deviceName,float metersPerUnit)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto &devMgr = alure::DeviceManager::get();
	auto *dev = devMgr.openPlayback();
	if(dev == nullptr)
		return nullptr;
	auto *context = dev->createContext();
	if(context == nullptr)
	{
		dev->close();
		return nullptr;
	}
	alure::FileIOFactory::set(std::make_unique<al::FileFactory>());

	alure::Context::MakeCurrent(context);
#ifdef _DEBUG
	context->setMessageHandler(std::make_shared<MessageHandler>());
#endif

	context->setBufferDataCallback([](uint8_t *inputData,std::vector<uint8_t> &outputData,int32_t &format,alure::ChannelConfig &channel,uint32_t &frequency,uint32_t dataLen,uint32_t fragmentSize,const std::shared_ptr<void> &userData) -> bool {
		if(userData == nullptr)
			return false;
		auto &loadData = *static_cast<impl::BufferLoadData*>(userData.get());
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		if(channel == alure::ChannelConfig::Mono && loadData.soundSystem.m_iplScene != nullptr)
			return apply_steam_audio_processing(*loadData.soundSystem.m_iplScene,loadData,inputData,outputData,format,channel,frequency,dataLen);
#endif
		if(channel != alure::ChannelConfig::Stereo || (loadData.flags &impl::BufferLoadData::Flags::ConvertToMono) == impl::BufferLoadData::Flags::None)
			return false;
		switch(format)
		{
			case AL_STEREO8_SOFT:
				stereo_to_mono<int8_t,int16_t>(reinterpret_cast<int8_t*>(inputData),outputData,dataLen);
				format = AL_MONO8_SOFT;
				break;
			case AL_STEREO16_SOFT:
				stereo_to_mono<int16_t,int32_t>(reinterpret_cast<int16_t*>(inputData),outputData,dataLen);
				format = AL_MONO16_SOFT;
				break;
			case AL_STEREO32F_SOFT:
				stereo_to_mono<float,double>(reinterpret_cast<float*>(inputData),outputData,dataLen);
				format = AL_MONO32F_SOFT;
				break;
			default:
				return false;
		}
		channel = static_cast<alure::ChannelConfig>(ChannelConfig::Mono);
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		apply_steam_audio_processing(*loadData.soundSystem.m_iplScene,loadData,outputData.data(),outputData,format,channel,frequency,outputData.size());
#endif
		return true;
	});

	auto r = std::shared_ptr<SoundSystem>(new SoundSystem(dev,context,metersPerUnit));
	return r;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::Studio::System *system = nullptr;
	al::fmod::check_result(FMOD::Studio::System::create(&system));
	auto ptrSystem = std::shared_ptr<FMOD::Studio::System>(system,[](FMOD::Studio::System *system) {
		al::fmod::check_result(system->release());
	});

	FMOD::System *lowLevelSystem = nullptr;
	al::fmod::check_result(system->getLowLevelSystem(&lowLevelSystem));
	al::fmod::check_result(lowLevelSystem->setSoftwareFormat(0,FMOD_SPEAKERMODE_5POINT1,0));

	void *extraDriverData = nullptr;
	al::fmod::check_result(system->initialize(1'024,FMOD_STUDIO_INIT_NORMAL,FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL,extraDriverData));
	al::fmod::check_result(lowLevelSystem->setFileSystem(
		[](const char *name,uint32_t *fileSize,void **handle,void *userData) -> FMOD_RESULT {
			auto f = FileManager::OpenFile(name,"rb");
			if(f == nullptr)
				return FMOD_RESULT::FMOD_ERR_FILE_NOTFOUND;
			*fileSize = f->GetSize();
			*handle = new VFilePtr(f);
			return FMOD_RESULT::FMOD_OK;
		},[](void *handle,void *userData) -> FMOD_RESULT {
			delete static_cast<VFilePtr*>(handle);
			return FMOD_RESULT::FMOD_OK;
		},[](void *handle,void *buffer,uint32_t sizeBytes,uint32_t *bytesRead,void *userData) -> FMOD_RESULT {
			*bytesRead = (*static_cast<VFilePtr*>(handle))->Read(buffer,sizeBytes);
			if((*static_cast<VFilePtr*>(handle))->Eof())
				return FMOD_RESULT::FMOD_ERR_FILE_EOF;
			return FMOD_RESULT::FMOD_OK;
		},[](void *handle,uint32_t pos,void *userData) -> FMOD_RESULT {
			(*static_cast<VFilePtr*>(handle))->Seek(pos);
			return FMOD_RESULT::FMOD_OK;
		},nullptr,nullptr,-1
	));
	return std::shared_ptr<SoundSystem>(new SoundSystem(ptrSystem,*lowLevelSystem,metersPerUnit));
#endif
}

std::shared_ptr<al::SoundSystem> al::SoundSystem::Create(float metersPerUnit) {return Create("",metersPerUnit);}

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
al::SoundSystem::SoundSystem(alure::Device *device,alure::Context *context,float metersPerUnit)
	: m_device(device),m_context(context),m_listener(*context->getListener())
{
	m_listener.SetMetersPerUnit(metersPerUnit);
	SetSpeedOfSound(340.29f /metersPerUnit);
	SetSoundSourceFactory(std::make_unique<al::DefaultSoundSourceFactory>());
}
const alure::Device *al::SoundSystem::GetALDevice() const {return const_cast<SoundSystem*>(this)->GetALDevice();}
alure::Device *al::SoundSystem::GetALDevice() {return m_device;}
const alure::Context *al::SoundSystem::GetALContext() const {return const_cast<SoundSystem*>(this)->GetALContext();}
alure::Context *al::SoundSystem::GetALContext() {return m_context;}
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
const FMOD::Studio::System &al::SoundSystem::GetFMODSystem() const {return const_cast<SoundSystem*>(this)->GetFMODSystem();}
FMOD::Studio::System &al::SoundSystem::GetFMODSystem() {return *m_fmSystem;}
const FMOD::System &al::SoundSystem::GetFMODLowLevelSystem() const {return const_cast<SoundSystem*>(this)->GetFMODLowLevelSystem();}
FMOD::System &al::SoundSystem::GetFMODLowLevelSystem() {return m_fmLowLevelSystem;}
al::SoundSystem::SoundSystem(const std::shared_ptr<FMOD::Studio::System> &fmSystem,FMOD::System &lowLevelSystem,float metersPerUnit)
	: m_fmSystem(fmSystem),m_fmLowLevelSystem(lowLevelSystem),m_listener(*this)
{
	lowLevelSystem.set3DSettings(1.f,1.f,1.f);
	// FMOD TODO
	//SetSpeedOfSound(340.29f /metersPerUnit);
	SetSoundSourceFactory(std::make_unique<al::DefaultSoundSourceFactory>());
}
#endif

al::SoundSystem::~SoundSystem()
{
	m_sources.clear();
	m_effectSlots.clear();
	for(auto &hEffect : m_effects)
	{
		if(hEffect.IsValid() == true)
			hEffect->Release();
	}
	m_effects.clear();
	m_buffers.clear();
	
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	alure::Context::MakeCurrent(nullptr);
	try
	{
		m_context->destroy();
	}
	catch(const std::exception &e)
	{
		std::cout<<"WARNING: Unable to destroy sound context: "<<e.what()<<std::endl;
	}
	try
	{
		m_device->close();
	}
	catch(const std::exception &e)
	{
		std::cout<<"WARNING: Unable to close sound device: "<<e.what()<<std::endl;
	}
#endif
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	ClearSteamAudioScene();
#endif
}

void al::SoundSystem::SetSoundSourceFactory(std::unique_ptr<SoundSourceFactory> factory)
{
	if(factory == nullptr)
		factory = std::make_unique<DefaultSoundSourceFactory>();
	std::swap(m_soundSourceFactory,factory);
}

uint32_t al::SoundSystem::GetMaxAuxiliaryEffectsPerSource() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_device->getMaxAuxiliarySends();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return 0u; // FMOD TODO
#endif
}

bool al::SoundSystem::IsSupported(ChannelConfig channels,SampleType type) const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_context->isSupported(static_cast<alure::ChannelConfig>(channels),static_cast<alure::SampleType>(type));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return true; // FMOD TODO
#endif
}

float al::SoundSystem::GetDopplerFactor() const {return m_dopplerFactor;}
void al::SoundSystem::SetDopplerFactor(float factor)
{
	m_dopplerFactor = factor;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_context->setDopplerFactor(m_dopplerFactor);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

float al::SoundSystem::GetSpeedOfSound() const {return m_speedOfSound;}
void al::SoundSystem::SetSpeedOfSound(float speed)
{
	m_speedOfSound = speed;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_context->setSpeedOfSound(m_speedOfSound);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

al::DistanceModel al::SoundSystem::GetDistanceModel() const {return m_distanceModel;}
void al::SoundSystem::SetDistanceModel(DistanceModel mdl)
{
	m_distanceModel = mdl;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_context->setDistanceModel(static_cast<alure::DistanceModel>(m_distanceModel));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

std::string al::SoundSystem::GetDeviceName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_device->getName();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return "";// FMOD TODO
#endif
}
void al::SoundSystem::PauseDeviceDSP()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_device->pauseDSP();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSystem::ResumeDeviceDSP()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_device->resumeDSP();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

al::AuxiliaryEffectSlot *al::SoundSystem::CreateAuxiliaryEffectSlot()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	alure::AuxiliaryEffectSlot *effectSlot = nullptr;
	try
	{
		effectSlot = m_context->createAuxiliaryEffectSlot();
	}
	catch(const std::runtime_error &err)
	{}
	if(effectSlot == nullptr)
		return nullptr;
	auto r = std::shared_ptr<AuxiliaryEffectSlot>(new AuxiliaryEffectSlot(effectSlot));
	m_effectSlots.push_back(r);
	return r.get();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}
void al::SoundSystem::FreeAuxiliaryEffectSlot(AuxiliaryEffectSlot *slot)
{
	auto it = std::find_if(m_effectSlots.begin(),m_effectSlots.end(),[slot](const PAuxiliaryEffectSlot &slotOther) {
		return (slotOther.get() == slot) ? true : false;
	});
	if(it == m_effectSlots.end())
		return;
	m_effectSlots.erase(it);
}

void al::SoundSystem::Update()
{
	for(auto it=m_sources.begin();it!=m_sources.end();)
	{
		auto &src = *it;
		if(src.use_count() <= 1 && src->IsIdle() == true)
			it = m_sources.erase(it); // Sound isn't in use anymore (This is the last shared pointer instance)
		else
		{
			src->Update();
			++it;
		}
	}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_context->update();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	al::fmod::check_result(m_fmSystem->update());
#endif
}

al::PEffect al::SoundSystem::CreateEffect()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	alure::Effect *effect = nullptr;
	try
	{
		effect = m_context->createEffect();
	}
	catch(const std::runtime_error &err)
	{}
	if(effect == nullptr)
		return nullptr;
	return std::shared_ptr<Effect>(new Effect(*this,effect));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}

al::PSoundSource al::SoundSystem::InitializeSource(SoundSource *source)
{
	if(source == nullptr)
		return nullptr;
	auto psource = PSoundSource(source);
	m_sources.push_back(psource);
	ApplyGlobalEffects(*source);

	//auto *decoder = source->GetDecoder();
	//auto *alDecoder = (decoder != nullptr) ? dynamic_cast<impl::DynamicDecoder*>(decoder->GetALDecoder().get()) : nullptr;
	//if(alDecoder != nullptr)
	//	alDecoder->SetSoundSource(*source);

	return psource;
}
al::PSoundSource al::SoundSystem::CreateSource(const std::string &name,bool bStereo,Type type)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(IsSteamAudioEnabled() == false || bStereo == true)
#endif
	{
		switch(type)
		{
			case Type::Buffer:
			{
				auto *buf = GetBuffer(name,bStereo);
				if(buf == nullptr)
					return nullptr;
				return CreateSource(*buf);
			}
			case Type::Decoder:
			{
				auto decoder = CreateDecoder(name,!bStereo);
				if(decoder == nullptr)
					return nullptr;
				return CreateSource(*decoder);
			}
		}
		return nullptr;
	}
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto nname = FileManager::GetCanonicalizedPath(name);
	ustring::to_lower(nname);
	auto it = m_audioBuffers.find(nname);
	auto bNewDataBuffer = false;
	if(it == m_audioBuffers.end())
	{
		it = m_audioBuffers.insert(std::make_pair(nname,std::make_shared<ipl::AudioDataBuffer>())).first;
		bNewDataBuffer = true;
	}
	auto &audioDataBuffer = *it->second;
	std::shared_ptr<al::Decoder> decoder = nullptr;
	if(audioDataBuffer.audioResampler != nullptr && audioDataBuffer.audioResampler->IsComplete())
	{
		auto alDecoder = std::shared_ptr<alure::Decoder>(new al::BinaryDecoder(it->second));
		decoder = std::shared_ptr<al::Decoder>(new al::Decoder(alDecoder,name));
		auto userData = std::make_shared<impl::BufferLoadData>(*this);
		userData->flags |= impl::BufferLoadData::Flags::SingleSourceDecoder;
		alDecoder->userData = userData;
	}
	else
	{
		decoder = CreateDecoder(name,!bStereo);
		if(decoder == nullptr)
		{
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
	auto &loadData = *static_cast<impl::BufferLoadData*>(userData.get());
	if((loadData.flags &impl::BufferLoadData::Flags::SingleSourceDecoder) != impl::BufferLoadData::Flags::None)
	{
		loadData.userData = std::make_shared<al::SoundSourceHandle>(source->GetHandle());
		loadData.buffer = it->second;
	}
	return source;
#endif
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
al::PSoundSource al::SoundSystem::CreateSource(Decoder &decoder)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	alure::Source *src = nullptr;
	try {src = m_context->createSource();}
	catch(const std::runtime_error &err) {}
	if(src == nullptr)
		return nullptr;
	auto *snd = m_soundSourceFactory->CreateSoundSource(*this,decoder,src);
	auto r = InitializeSource(snd);
	if(r == nullptr)
		return r;
	auto userData = decoder.GetALDecoder()->userData;
	if(userData != nullptr)
	{
		auto &loadData = *static_cast<impl::BufferLoadData*>(userData.get());
		if((loadData.flags &impl::BufferLoadData::Flags::SingleSourceDecoder) != impl::BufferLoadData::Flags::None)
			loadData.userData = std::make_shared<al::SoundSourceHandle>(r->GetHandle());
	}
	return r;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return nullptr; // FMOD TODO
#endif
}
#endif

al::PSoundSource al::SoundSystem::CreateSource(SoundBuffer &buffer)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(IsSteamAudioEnabled() && buffer.GetTargetChannelConfig() == al::ChannelConfig::Mono)
	{
		// Redirect to steam audio decoder (Can't use buffers with steam audio)
		auto r = CreateSource(buffer.GetFilePath(),false,Type::Buffer);
		if(r != nullptr)
			return r;
	}
#endif
	alure::Source *src = nullptr;
	try {src = m_context->createSource();}
	catch(const std::runtime_error &err) {}
	if(src == nullptr)
		return nullptr;
	auto *snd = m_soundSourceFactory->CreateSoundSource(*this,buffer,src);
	return InitializeSource(snd);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD::Channel *channel;
	al::fmod::check_result(m_fmLowLevelSystem.playSound(buffer.GetFMODSound(),nullptr,true,&channel));
	auto *snd = m_soundSourceFactory->CreateSoundSource(*this,buffer,channel);
	if(snd == nullptr)
		return nullptr;
	FMOD_MODE mode;
	al::fmod::check_result(channel->getMode(&mode));
	mode &= ~(FMOD_3D_HEADRELATIVE | FMOD_3D_WORLDRELATIVE | FMOD_3D | FMOD_2D);
	mode |= FMOD_3D_WORLDRELATIVE | FMOD_3D;

	channel->setMode(mode);
	return InitializeSource(snd);
#endif
}
const std::vector<al::PSoundSource> &al::SoundSystem::GetSources() const {return const_cast<SoundSystem*>(this)->GetSources();}
std::vector<al::PSoundSource> &al::SoundSystem::GetSources() {return m_sources;}

uint32_t al::SoundSystem::GetAudioFrameSampleCount() const {return m_audioFrameSampleCount;}
void al::SoundSystem::SetAudioFrameSampleCount(uint32_t size) {m_audioFrameSampleCount = size;}

std::vector<al::SoundBuffer*> al::SoundSystem::GetBuffers() const
{
	std::vector<al::SoundBuffer*> buffers;
	buffers.reserve(m_buffers.size() *2);
	for(auto &pair : m_buffers)
	{
		if(pair.second.mono != nullptr)
			buffers.push_back(pair.second.mono.get());
		if(pair.second.stereo != nullptr)
			buffers.push_back(pair.second.stereo.get());
	}
	return buffers;
}

void al::SoundSystem::StopSounds()
{
	for(auto &hSrc : m_sources)
		hSrc->Stop();
}

const al::Listener &al::SoundSystem::GetListener() const {return const_cast<SoundSystem*>(this)->GetListener();}
al::Listener &al::SoundSystem::GetListener() {return m_listener;}

std::vector<std::string> al::SoundSystem::GetHRTFNames() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_device->enumerateHRTFNames();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return {}; // FMOD TODO
#endif
}

std::string al::SoundSystem::GetCurrentHRTF() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_device->getCurrentHRTF();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return ""; // FMOD TODO
#endif
}
bool al::SoundSystem::IsHRTFEnabled() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_device->isHRTFEnabled();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	return false; // FMOD TODO
#endif
}

void al::SoundSystem::SetHRTF(uint32_t id)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(m_device->queryExtension("ALC_SOFT_HRTF") == false)
		return;
	m_device->reset({
		{ALC_HRTF_SOFT,ALC_TRUE},
		{ALC_HRTF_ID_SOFT,id},
		{0,0}
	});
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}
void al::SoundSystem::DisableHRTF()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(m_device->queryExtension("ALC_SOFT_HRTF") == false)
		return;
	m_device->reset({
		{ALC_HRTF_SOFT,ALC_FALSE},
		{0,0}
	});
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
#endif
}

/////////////////////////////////

std::vector<std::string> al::get_devices()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto &devMgr = alure::DeviceManager::get();
	return devMgr.enumerate(alure::DeviceEnumeration::Basic);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return {};
#endif
}

std::string al::get_default_device_name()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto &devMgr = alure::DeviceManager::get();
	return devMgr.defaultDeviceName(alure::DefaultDeviceType::Basic);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}

