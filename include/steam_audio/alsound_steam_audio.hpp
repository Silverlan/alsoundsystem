/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_STEAM_AUDIO_HPP__
#define __ALSOUND_STEAM_AUDIO_HPP__

#include "alsound_definitions.hpp"

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include <phonon.h>
#include <memory>
#include <functional>
#include <vector>
#include <uvec.h>
#include <thread>
#include <atomic>
#include <mutex>

#define IPL_ENABLE_PROBES 1

class VFilePtrInternal;
class VFilePtrInternalReal;
namespace ipl
{
	struct AudioResampler;
	const auto OUTPUT_SAMPLE_RATE = 44'100u;

	DLLALSYS void check_result(IPLerror err);
	DLLALSYS std::string result_to_string(IPLerror err);

	class Scene;
	class DLLALSYS StaticMesh
	{
	public:
		~StaticMesh()=default;

		bool Spawn();

		const std::vector<IPLVector3> &GetVertices() const;
		std::vector<IPLVector3> &GetVertices();

		const std::vector<IPLTriangle> &GetTriangles() const;
		std::vector<IPLTriangle> &GetTriangles();

		const std::vector<IPLint32> &GetMaterialIndices() const;
		std::vector<IPLint32> &GetMaterialIndices();
	protected:
		static std::shared_ptr<StaticMesh> Create(Scene &scene,const std::vector<IPLVector3> &verts={},const std::vector<IPLTriangle> &tris={},const std::vector<IPLint32> &materialIndices={});
		friend Scene;
	private:
		StaticMesh(Scene &scene,const std::vector<IPLVector3> &verts={},const std::vector<IPLTriangle> &tris={},const std::vector<IPLint32> &materialIndices={});
		std::shared_ptr<void> m_iplMesh = nullptr;
		std::vector<IPLVector3> m_vertices = {};
		std::vector<IPLTriangle> m_triangles = {};
		std::vector<IPLint32> m_materialIndices = {};
		std::weak_ptr<Scene> m_scene = {};
	};

	class Context;
	class Environment;
	class DLLALSYS Scene
		: public std::enable_shared_from_this<Scene>
	{
	public:
		enum class LoadStage : uint32_t
		{
			LoadFromDisk = 0,
			FinalizingScene,
			GeneratingProbes,
			BakingReverb,
			BakingPropagation
		};

		enum class InitializeFlags : uint32_t
		{
			None = 0,
			BakeConvolution = 1,
			BakeParametric = BakeConvolution<<1,

			BakeAll = BakeConvolution | BakeParametric
		};
		struct DLLALSYS FinalizeInfo
		{
			FinalizeInfo() {}
			float defaultSpacing = 1'024.f;
			float defaultHeightAboveFloor = 50.f;
			InitializeFlags flags = InitializeFlags::BakeAll;
		};
		struct DLLALSYS ProbeSphere
		{
			ProbeSphere() {};
			Vector3 origin;
			float radius = 0.f;
		};
		enum class DSPEffect
		{
			Spatializer,
			Reverb
		};

		~Scene();
		IPLDirectSoundPath GetDirectSoundPath(const Vector3 &listenerPos,const Vector3 &listenerDir,const Vector3 &listenerUp,const Vector3 &srcPos,float srcRadius) const;
		const std::shared_ptr<Context> &GetContext() const;
		// Function callbacks are called from processing thread
		void Finalize(const FinalizeInfo &info={},const std::function<void(LoadStage,float)> &fStageProgress=nullptr,const std::function<void(void)> &fOnComplete=nullptr,const std::function<void(IPLerror)> &errorHandler=nullptr);
		void Finalize(const std::shared_ptr<VFilePtrInternal> &f,const FinalizeInfo &info={},const std::function<void(LoadStage,float)> &fStageProgress=nullptr,const std::function<void(void)> &fOnComplete=nullptr,const std::function<void(IPLerror)> &errorHandler=nullptr);
		const std::vector<std::shared_ptr<StaticMesh>> &GetStaticMeshes() const;
		std::shared_ptr<StaticMesh> CreateStaticMesh(const std::vector<IPLVector3> &verts={},const std::vector<IPLTriangle> &triangles={},const std::vector<IPLint32> &materialIndices={});
		void AddProbeBox(const Vector3 &min,const Vector3 &max,float spacing=512.f,float heightAboveFloor=50.f);
		void AddProbeSphere(const Vector3 &origin,float radius);
		void RegisterPropagationSoundSource(const std::string &name,const Vector3 &origin,float radius);
		uint32_t AddMaterial(const IPLMaterial &mat);
		const std::vector<IPLMaterial> &GetMaterials() const;
		std::vector<IPLMaterial> &GetMaterials();
		bool FindBakedSoundSourceIdentifier(const std::string &name,DSPEffect dspEffectType,IPLint32 &identifier);

		bool DumpScene(const std::string &objFile) const;

		IPLhandle GetIplScene() const;
		IPLhandle GetIplEnvironment() const;
		IPLhandle GetIplRenderer() const;
		IPLAudioFormat GetIplInputFormat() const;
		IPLAudioFormat GetIplOutputFormat() const;
		void *GetIplDirectSoundEffect() const;

		IPLRenderingSettings GetRenderSettings() const;
		IPLSimulationSettings &GetSimulationSettings();

		bool IsPropagationDelayEnabled() const;
		void SetPropagationDelayEnabled(bool b);

		void Save(std::shared_ptr<VFilePtrInternalReal> &f,bool bSaveProbeBoxes=true) const;
		IPLerror Load(const std::shared_ptr<VFilePtrInternal> &f,IPLLoadSceneProgressCallback callback=nullptr);
		bool IsComplete() const;

		void GetProbeSpheres(std::vector<ProbeSphere> &spheres);
	protected:
		static std::shared_ptr<Scene> Create(Context &context);
		static std::shared_ptr<Scene> Create(Context &context,const std::vector<uint8_t> &serializedData,void(*progress)(IPLfloat32)=nullptr);
		friend Context;
		friend StaticMesh;
	private:
		struct DLLALSYS ProbeBox
		{
			ProbeBox()=default;
			ProbeBox(const Vector3 &min,const Vector3 &max,float spacing,float heightAboveFloor,bool centroid=false)
				: min(min),max(max),spacing(spacing),heightAboveFloor(heightAboveFloor),centroid(centroid)
			{}
			ProbeBox(const Vector3 &origin,float radius)
				: ProbeBox(origin,origin,radius,0.f,true)
			{}
			Vector3 min = {};
			Vector3 max = {};
			float spacing = 512.f;
			float heightAboveFloor = 50.f;
			bool centroid = false;
		};

		Scene(Context &context);
		bool IsFinalized() const;
		std::shared_ptr<void> m_iplScene = nullptr;
		std::shared_ptr<void> m_iplEnv = nullptr;
		std::vector<ProbeBox> m_probeBoxes;
		std::vector<std::shared_ptr<void>> m_iplProbeBoxes;
		std::vector<std::shared_ptr<void>> m_iplProbeBatches;
		std::shared_ptr<void> m_iplProbeManager = nullptr;
		IPLSimulationSettings m_iplSimSettings = {};
		bool m_bPropagationDelayEnabled = true;
		bool m_bFinalized = false;
		std::vector<IPLMaterial> m_materials = {};
		std::shared_ptr<Context> m_context = nullptr;
		std::vector<std::shared_ptr<StaticMesh>> m_staticMeshes;

		IPLAudioFormat m_inputFormat = {};
		std::shared_ptr<void> m_iplDirectSoundEffect = nullptr;

		struct PropagationSoundSource
		{
			std::string name = {};
			Vector3 origin = {};
			float radius = 0.f;
		};
		std::vector<PropagationSoundSource> m_propagationSoundSources;

		struct FinalizeData
		{
			FinalizeData();
			~FinalizeData();
			void Stop();
			std::shared_ptr<std::thread> AddThread(const std::function<void(void)> &f);
			std::mutex threadMutex;
			std::vector<std::shared_ptr<std::thread>> threads = {};
			std::atomic<bool> working = {false};
			std::atomic<bool> complete = {false};
		};
		std::unique_ptr<FinalizeData> m_finalizeData = nullptr;
	};
	REGISTER_BASIC_BITWISE_OPERATORS(Scene::InitializeFlags);

	class DLLALSYS Context
		: public std::enable_shared_from_this<Context>
	{
	public:
		friend void ipl::check_result(IPLerror err);
		static std::shared_ptr<Context> Create(uint32_t frameSize);
		~Context();
		std::shared_ptr<Scene> CreateScene();
		void SetErrorHandler(const std::function<void(IPLerror)> &handler);
		void SetLogHandler(const std::function<void(std::string)> &handler);
	protected:
		IPLhandle GetIplContext() const;
		void InvokeErrorHandler(IPLerror err);
		friend Scene;
		friend StaticMesh;
	private:
		Context(const std::shared_ptr<void> &context,uint32_t frameSize);
		std::shared_ptr<void> m_iplContext = {};
		std::function<void(IPLerror)> m_errorHandler = nullptr;
		std::function<void(std::string)> m_logHandler = nullptr;
	};

	struct AudioResampler
	{
		AudioResampler(uint32_t inputFrequency,uint64_t inputLength,uint32_t outputFrequency);
		~AudioResampler();
		bool Process(float *inputData,uint64_t inputOffset,uint32_t sampleCount,bool bEndOfInput,float **processedData,uint32_t &processedDataSize);
		bool Process(float *inputData,uint64_t inputOffset,uint32_t sampleCount,bool bEndOfInput);
		bool IsComplete() const;
		std::vector<float> &GetOutputData();
	private:
		void *m_srcState = nullptr;
		std::shared_ptr<void> m_srcData = nullptr;
		uint64_t m_lastOffset = 0ull;
		std::vector<float> m_outputData = {};
		bool m_bComplete = false;
	};

	struct DLLALSYS AudioDataBuffer
	{
		uint32_t frequency = ALSYS_INTERNAL_AUDIO_SAMPLE_RATE;
		std::shared_ptr<AudioResampler> audioResampler = nullptr;
	};
};
#endif

#endif
