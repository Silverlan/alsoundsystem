// SPDX-FileCopyrightText: © 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "steam_audio/alsound_steam_audio.hpp"

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1

#include <sharedutils/scope_guard.h>
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include "steam_audio/fmod/audio_engine_settings.h"
#include "steam_audio/fmod/environment_proxy.h"
#endif
//#include <networkstate.h>
//#include <material.h>
//#include <c_game.h>
//#include <raytraces.h>
//extern DLLCLIENT CGame *c_game;
ipl::Scene::FinalizeData::FinalizeData() {}
ipl::Scene::FinalizeData::~FinalizeData() { Stop(); }
void ipl::Scene::FinalizeData::Stop()
{
	if(working == false)
		return;
	working = false;
	threadMutex.lock();
	auto threadsCpy = threads;
	threadMutex.unlock();
	iplCancelBake();
	for(auto &t : threadsCpy) {
		if(t->joinable())
			t->join();
	}
}
std::shared_ptr<std::thread> ipl::Scene::FinalizeData::AddThread(const std::function<void(void)> &f)
{
	threadMutex.lock();
	if(working == false) {
		threadMutex.unlock();
		return nullptr;
	}
	auto t = std::make_shared<std::thread>(f);
	threads.push_back(t);
	threadMutex.unlock();
	return t;
}

/////////////////////

std::shared_ptr<ipl::Scene> ipl::Scene::Create(Context &context, const std::vector<uint8_t> &serializedData, void (*progress)(IPLfloat32))
{
	/*auto &iplContext = context->GetIplContext();
	auto simSettings = get_simulation_settings();
	IPLhandle sceneHandle {};
	auto err = iplLoadFinalizedScene(iplContext,simSettings,reinterpret_cast<IPLbyte*>(const_cast<uint8_t*>(serializedData.data())),serializedData.size() *sizeof(serializedData.front()),nullptr,progress,&sceneHandle);
	if(err != IPLerror::IPL_STATUS_SUCCESS)
		return nullptr;
	std::shared_ptr<Scene>(new Scene(context,sceneHandle,simSettings));*/

	return nullptr; // TODO
}

std::shared_ptr<ipl::Scene> ipl::Scene::Create(Context &context) { return std::shared_ptr<Scene>(new Scene(context)); }

ipl::Scene::Scene(Context &context) : std::enable_shared_from_this<Scene>(), m_context(context.shared_from_this())
{
	auto &simSettings = m_iplSimSettings;
	//simSettings.sceneType = IPLSceneType::IPL_SCENETYPE_EMBREE; // Much faster, but very extremely in the current version
	simSettings.sceneType = IPLSceneType::IPL_SCENETYPE_PHONON;
	simSettings.numRays = 16'384;
	simSettings.numBounces = 32;
	simSettings.irDuration = 4.f;
	simSettings.ambisonicsOrder = 3;
	simSettings.numDiffuseSamples = 4'096;
	simSettings.numThreads = 1;
	simSettings.bakingBatchSize = 1;
	simSettings.numOcclusionSamples = 1;

	auto &inputFormat = m_inputFormat;
	inputFormat = {};
	inputFormat.channelLayoutType = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
	inputFormat.channelLayout = IPL_CHANNELLAYOUT_MONO;
	inputFormat.channelOrder = IPL_CHANNELORDER_INTERLEAVED;
}

uint32_t ipl::Scene::AddMaterial(const IPLMaterial &mat)
{
	if(IsFinalized())
		throw std::logic_error("Cannot add material to scene after scene has been finalized!");
	if(m_materials.size() == m_materials.capacity())
		m_materials.reserve(m_materials.size() + 50u);
	m_materials.push_back(mat);
	return m_materials.size() - 1u;
}
const std::vector<IPLMaterial> &ipl::Scene::GetMaterials() const { return m_materials; }
std::vector<IPLMaterial> &ipl::Scene::GetMaterials()
{
	if(IsFinalized())
		throw std::logic_error("Cannot change materials of scene after scene has been finalized!");
	return m_materials;
}
bool ipl::Scene::DumpScene(const std::string &objFile) const
{
	if(m_iplScene == nullptr || IsFinalized() == false || IsComplete() == false)
		return false;
	auto path = util::FilePath(filemanager::get_program_write_path(), objFile).GetString();
	iplSaveSceneAsObj(m_iplScene.get(), const_cast<char *>(path.c_str()));
	return true;
}
bool ipl::Scene::IsFinalized() const { return m_bFinalized; }
bool ipl::Scene::IsComplete() const { return (m_finalizeData != nullptr) ? static_cast<bool>(m_finalizeData->complete) : false; }
void ipl::Scene::GetProbeSpheres(std::vector<ProbeSphere> &spheres)
{
	std::vector<IPLSphere> iplSpheres {};
	for(auto &probeBox : m_iplProbeBoxes) {
		auto numProbes = iplGetProbeSpheres(probeBox.get(), nullptr);
		iplSpheres.resize(numProbes);
		iplGetProbeSpheres(probeBox.get(), iplSpheres.data());

		spheres.reserve(iplSpheres.size());
		for(auto &iplSphere : iplSpheres) {
			spheres.push_back({});
			auto &sphere = spheres.back();
			sphere.origin = al::to_game_position(al::to_game_vector(iplSphere.center));
			sphere.radius = al::to_game_distance(iplSphere.radius);
		}
	}
}

bool ipl::Scene::FindBakedSoundSourceIdentifier(const std::string &name, DSPEffect dspEffectType, IPLint32 &identifier)
{
	auto it = std::find_if(m_propagationSoundSources.begin(), m_propagationSoundSources.end(), [&name](const PropagationSoundSource &src) { return ustring::compare(name, src.name); });
	if(it == m_propagationSoundSources.end())
		return false;
	auto strIdentifier = name;
	switch(dspEffectType) {
	case DSPEffect::Spatializer:
		strIdentifier += "_staticsource";
		break;
	case DSPEffect::Reverb:
		strIdentifier += "_reverb";
		break;
	}
	std::hash<std::string> hash {};
	identifier = hash(strIdentifier);
	return true;
}

void ipl::Scene::Finalize(const std::shared_ptr<VFilePtrInternal> &f, const FinalizeInfo &info, const std::function<void(LoadStage, float)> &fStageProgress, const std::function<void(void)> &fOnComplete, const std::function<void(IPLerror)> &errorHandler)
{
	if(m_bFinalized == true)
		return;
	if(m_context == nullptr)
		return;
	m_bFinalized = true;

	m_finalizeData = std::make_unique<FinalizeData>();
	m_finalizeData->working = true;
	m_iplSimSettings.maxConvolutionSources = umath::max(m_propagationSoundSources.size(), static_cast<size_t>(1));

	// Hack: It's not possible to pass user-data to any of the steam audio callback functions (since they're c-functions).
	// Steam audio should only be used once at a time, so we just set the callback as a static variable instead (making it accessable in the callbacks).
	static std::function<void(LoadStage, float)> loadStageProgressCallback = nullptr;
	loadStageProgressCallback = fStageProgress;

	m_finalizeData->AddThread([this, f, fOnComplete, errorHandler, fStageProgress, info]() {
		auto iplContext = m_context->GetIplContext();
		util::ScopeGuard sgScene {};
		Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
		Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
		auto bBakeData = false;
		if(f != nullptr) {
			auto err = Load(f, [](IPLfloat32 progress) { loadStageProgressCallback(LoadStage::LoadFromDisk, progress); });

			if(err != IPLerror::IPL_STATUS_SUCCESS) {
				if(errorHandler != nullptr)
					errorHandler(err);
				return;
			}
		}
		else {
			bBakeData = true;

			IPLhandle scene;
			auto err = iplCreateScene(iplContext, nullptr, m_iplSimSettings, m_materials.size(), m_materials.data(), nullptr, nullptr, nullptr, nullptr, nullptr, &scene);
			if(err != IPLerror::IPL_STATUS_SUCCESS) {
				if(errorHandler != nullptr)
					errorHandler(err);
				return;
			}
			m_iplScene = std::shared_ptr<void>(scene, [](IPLhandle scene) { iplDestroyScene(&scene); });
			sgScene = [this]() { m_iplScene = nullptr; };

			if(m_finalizeData->working == false)
				return; // Cancel pending operations

			if(m_staticMeshes.empty()) {
				min = {};
				max = {};
			}
			else {
				auto numVerts = 0u;
				auto numTris = 0u;
				auto numMatIndices = 0u;
				for(auto &mesh : m_staticMeshes) {
					numVerts += mesh->GetVertices().size();
					numTris += mesh->GetTriangles().size();
					numMatIndices += mesh->GetMaterialIndices().size();
				}
				std::vector<IPLVector3> verts;
				std::vector<IPLTriangle> tris;
				std::vector<IPLint32> matIndices;
				verts.reserve(numVerts);
				tris.reserve(numTris);
				matIndices.reserve(numMatIndices);
				// Merge all meshes into one
				for(auto &mesh : m_staticMeshes) {
					auto vertIdxOffset = verts.size();
					auto &meshVerts = mesh->GetVertices();
					auto &meshTris = mesh->GetTriangles();
					auto &meshMatIndices = mesh->GetMaterialIndices();

					for(auto &v : meshVerts) {
						verts.push_back(v);

						auto vGame = Vector3(v.x, v.y, v.z);
						uvec::min(&min, vGame);
						uvec::max(&max, vGame);
					}
					for(auto &tri : meshTris) {
						tris.push_back(tri);
						for(auto i = 0u; i < 3u; ++i)
							tris.back().indices[i] += vertIdxOffset;
					}
					for(auto idx : meshMatIndices)
						matIndices.push_back(idx);
				}
				auto meshMerged = StaticMesh::Create(*this, verts, tris, matIndices);
				meshMerged->Spawn();
			}
		}
		if(m_finalizeData->working == false)
			return; // Cancel pending operations

		IPLhandle envHandle = nullptr;
#if IPL_ENABLE_PROBES != 0
		if(bBakeData == true) {
			util::ScopeGuard sgProbeBox {};
			util::ScopeGuard sgProbeBatch {};
			util::ScopeGuard sgProbeManager {};

			// Generate probes
			if(m_probeBoxes.empty() == false) {
				m_iplProbeBoxes.reserve(m_probeBoxes.size());
				for(auto &probeBox : m_probeBoxes) {
					IPLProbePlacementParams probePlacementParams {};
					probePlacementParams.placement = probeBox.centroid ? IPLProbePlacement::IPL_PLACEMENT_CENTROID : IPLProbePlacement::IPL_PLACEMENT_UNIFORMFLOOR;
					probePlacementParams.spacing = al::to_audio_distance(probeBox.spacing);
					probePlacementParams.heightAboveFloor = al::to_audio_distance(probeBox.heightAboveFloor);
					auto min = probeBox.min;
					auto max = probeBox.max;
					uvec::to_min_max(min, max);
					auto center = (min + max) / 2.f;
					auto scale = (max - center) - (min - center);

					auto iplCenter = al::to_audio_position(center);
					auto iplScale = probeBox.centroid ? Vector3 {probePlacementParams.spacing, probePlacementParams.spacing, probePlacementParams.spacing} : al::to_audio_position(scale);

					std::array<float, 16> m = {iplScale.x, 0.f, 0.f, 0.f, 0.f, iplScale.y, 0.f, 0.f, 0.f, 0.f, iplScale.z, 0.f, iplCenter.x, iplCenter.y, iplCenter.z, 1.f};
					IPLhandle probeHandle = nullptr;
					auto err = iplCreateProbeBox(
					  iplContext, m_iplScene.get(), m.data(), probePlacementParams, [](float progress) { loadStageProgressCallback(LoadStage::GeneratingProbes, progress); }, &probeHandle);
					if(err == IPLerror::IPL_STATUS_SUCCESS)
						m_iplProbeBoxes.push_back(std::shared_ptr<void>(probeHandle, [](IPLhandle probeHandle) { iplDestroyProbeBox(&probeHandle); }));
					else if(errorHandler != nullptr)
						errorHandler(err);
				}
			}
			else {
				IPLProbePlacementParams probePlacementParams {};
				probePlacementParams.placement = IPLProbePlacement::IPL_PLACEMENT_UNIFORMFLOOR;

				uvec::to_min_max(min, max);
				auto center = (min + max) / 2.f;
				auto scale = (max - center) - (min - center);

				std::array<float, 16> m = {scale.x, 0.f, 0.f, 0.f, 0.f, scale.y, 0.f, 0.f, 0.f, 0.f, scale.z, 0.f, center.x, center.y, center.z, 1.f};
				probePlacementParams.spacing = al::to_audio_distance(info.defaultSpacing);
				probePlacementParams.heightAboveFloor = al::to_audio_distance(info.defaultHeightAboveFloor);

				IPLhandle probeBox = nullptr;
				auto err = iplCreateProbeBox(
				  iplContext, m_iplScene.get(), m.data(), probePlacementParams, [](float progress) { loadStageProgressCallback(LoadStage::GeneratingProbes, progress); }, &probeBox);
				if(err == IPLerror::IPL_STATUS_SUCCESS)
					m_iplProbeBoxes.push_back(std::shared_ptr<void>(probeBox, [](IPLhandle probeBox) { iplDestroyProbeBox(&probeBox); }));
				else if(errorHandler != nullptr)
					errorHandler(err);
			}
			std::vector<IPLint32> iplSphereCount;
			iplSphereCount.reserve(m_iplProbeBoxes.size());
			auto bHasSphere = false;
			for(auto &iplBox : m_iplProbeBoxes) {
				iplSphereCount.push_back(iplGetProbeSpheres(iplBox.get(), nullptr));
				if(iplSphereCount.size() > 0)
					bHasSphere = true;
			}
			if(bHasSphere == true) {
				sgProbeBox = [this]() mutable { m_iplProbeBoxes.clear(); };

				if(m_finalizeData->working == false)
					return; // Cancel pending operations

				IPLhandle probeBatch;
				auto err = iplCreateProbeBatch(iplContext, &probeBatch);
				if(err == IPLerror::IPL_STATUS_SUCCESS) {
					m_iplProbeBatches = {std::shared_ptr<void>(probeBatch, [](IPLhandle probeBatch) { iplDestroyProbeBatch(&probeBatch); })};
					sgProbeBatch = [this]() mutable { m_iplProbeBatches.clear(); };

					if(m_finalizeData->working == false)
						return; // Cancel pending operations

					for(auto i = decltype(m_iplProbeBoxes.size()) {0}; i < m_iplProbeBoxes.size(); ++i) {
						auto &probeBox = m_iplProbeBoxes.at(i);
						auto numSpheres = iplSphereCount.at(i);
						for(auto j = decltype(numSpheres) {0}; j < numSpheres; ++j)
							iplAddProbeToBatch(probeBatch, probeBox.get(), j);
					}
					iplFinalizeProbeBatch(probeBatch);

					if(m_finalizeData->working == false)
						return; // Cancel pending operations

					IPLhandle probeManager;
					err = iplCreateProbeManager(iplContext, &probeManager);
					if(err == IPLerror::IPL_STATUS_SUCCESS) {
						m_iplProbeManager = std::shared_ptr<void>(probeManager, [](IPLhandle probeManager) { iplDestroyProbeManager(&probeManager); });
						sgProbeManager = [this]() mutable { m_iplProbeManager = nullptr; };

						if(m_finalizeData->working == false)
							return; // Cancel pending operations

						iplAddProbeBatch(m_iplProbeManager.get(), probeBatch);
					}
					else if(errorHandler != nullptr)
						errorHandler(err);
				}
				else if(errorHandler != nullptr)
					errorHandler(err);
			}
			//

			if(m_finalizeData->working == false)
				return; // Cancel pending operations

			// Initialize environment handle
			auto err = iplCreateEnvironment(iplContext, nullptr, m_iplSimSettings, m_iplScene.get(), m_iplProbeManager.get(), &envHandle);
			if(err != IPLerror::IPL_STATUS_SUCCESS) {
				if(errorHandler != nullptr)
					errorHandler(err);
				return;
			}
			m_iplEnv = std::shared_ptr<void>(envHandle, [](IPLhandle envHandle) { iplDestroyEnvironment(&envHandle); });
			util::ScopeGuard sgEnvironment([this]() mutable { m_iplEnv = nullptr; });

			if(m_finalizeData->working == false)
				return; // Cancel pending operations

			IPLBakingSettings bakingSettings {};
			if((info.flags & InitializeFlags::BakeConvolution) != InitializeFlags::None)
				bakingSettings.bakeConvolution = IPLbool::IPL_TRUE;
			if((info.flags & InitializeFlags::BakeParametric) != InitializeFlags::None)
				bakingSettings.bakeParametric = IPLbool::IPL_TRUE;
			auto tBakeReverb = m_finalizeData->AddThread([this, errorHandler, bakingSettings]() {
				// Reverb
				iplBakeReverb(m_iplEnv.get(), m_iplProbeBoxes.front().get(), bakingSettings, [](float progress) { loadStageProgressCallback(LoadStage::BakingReverb, progress); });
			});
			if(tBakeReverb == nullptr)
				return;
			tBakeReverb->join(); // Reverb and propagation cannot be baked in parallel? (Results in crash)
			auto tBakePropagation = m_finalizeData->AddThread([this, bakingSettings]() {
				// Propagation
				for(auto &src : m_propagationSoundSources) {
					IPLSphere sourceInfluence {};
					sourceInfluence.center = al::to_custom_vector<IPLVector3>(al::to_audio_position(src.origin));
					sourceInfluence.radius = al::to_audio_distance(src.radius);

					std::hash<std::string> hash {};
					IPLBakedDataIdentifier identifier {static_cast<IPLint32>(hash(src.name + "_staticsource")), IPLBakedDataType::IPL_BAKEDDATATYPE_STATICSOURCE};
					iplBakePropagation(m_iplEnv.get(), m_iplProbeBoxes.front().get(), sourceInfluence, identifier, bakingSettings, [](float progress) { loadStageProgressCallback(LoadStage::BakingPropagation, progress); });

					identifier = {static_cast<IPLint32>(hash(src.name + "_reverb")), IPLBakedDataType::IPL_BAKEDDATATYPE_REVERB};
					iplBakePropagation(m_iplEnv.get(), m_iplProbeBoxes.front().get(), sourceInfluence, identifier, bakingSettings, [](float progress) { loadStageProgressCallback(LoadStage::BakingPropagation, progress); });
					if(m_finalizeData->working == false)
						return; // Cancel pending operations
				}
			});
			if(tBakePropagation == nullptr)
				return;
			tBakePropagation->join();
			if(m_finalizeData->working == false)
				return; // Cancel pending operations

			sgProbeBox.dismiss();
			sgProbeBatch.dismiss();
			sgProbeManager.dismiss();
			sgEnvironment.dismiss();
		}
#endif
		if(m_iplProbeManager == nullptr) {
			IPLhandle probeManager;
			auto err = iplCreateProbeManager(iplContext, &probeManager);
			if(err != IPLerror::IPL_STATUS_SUCCESS) {
				if(errorHandler != nullptr)
					errorHandler(err);
				return;
			}
			m_iplProbeManager = std::shared_ptr<void>(probeManager, [](IPLhandle probeManager) { iplDestroyProbeManager(&probeManager); });
		}
		if(m_iplEnv == nullptr) {
			// Initialize environment handle
			auto err = iplCreateEnvironment(iplContext, nullptr, m_iplSimSettings, m_iplScene.get(), m_iplProbeManager.get(), &envHandle);
			if(err != IPLerror::IPL_STATUS_SUCCESS) {
				if(errorHandler != nullptr)
					errorHandler(err);
				return;
			}
			m_iplEnv = std::shared_ptr<void>(envHandle, [](IPLhandle envHandle) { iplDestroyEnvironment(&envHandle); });
		}

		SceneState::resetEnvironment();
		SceneState::setEnvironment(m_iplSimSettings, envHandle, GlobalState::get()->renderingSettings().convolutionType);

		if(m_finalizeData->working == false)
			return; // Cancel pending operations

		/*IPLhandle directSoundEffect;
		err = iplCreateDirectSoundEffect(renderHandle,m_inputFormat,m_outputFormat,&directSoundEffect);
		if(err != IPLerror::IPL_STATUS_SUCCESS)
		{
			if(errorHandler != nullptr)
				errorHandler(err);
			return;
		}
		m_iplDirectSoundEffect = std::shared_ptr<void>(directSoundEffect,[](IPLhandle directSoundEffect) {
			iplDestroyDirectSoundEffect(&directSoundEffect);
		});*/

		// Don't need to dismiss probe box scope guard, as we won't need the probe box anymore
		sgScene.dismiss();
#if IPL_ENABLE_PROBES == 0
		sgEnvironment.dismiss();
#endif

		m_finalizeData->complete = true;

		// Test binaural renderer
		{
			/*IPLAudioFormat mono;
			mono.channelLayoutType  = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
			mono.channelLayout      = IPL_CHANNELLAYOUT_FIVEPOINTONE;//IPL_CHANNELLAYOUT_STEREO;//IPL_CHANNELLAYOUT_FIVEPOINTONE;//IPL_CHANNELLAYOUT_MONO;
			mono.channelOrder       = IPL_CHANNELORDER_INTERLEAVED;

			IPLAudioFormat stereo;
			stereo.channelLayoutType  = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
			stereo.channelLayout      = IPL_CHANNELLAYOUT_STEREO;//IPL_CHANNELLAYOUT_FIVEPOINTONE;//IPL_CHANNELLAYOUT_STEREO;
			stereo.channelOrder       = IPL_CHANNELORDER_INTERLEAVED;

			IPLhandle renderer{ nullptr };
			IPLHrtfParams hrtfParams{ IPL_HRTFDATABASETYPE_DEFAULT, nullptr, 0, nullptr, nullptr, nullptr };
			iplCreateBinauralRenderer(m_context->GetIplContext(), m_renderSettings, hrtfParams, &renderer);

			IPLhandle effect{ nullptr };
			iplCreateBinauralEffect(renderer, mono, stereo, &effect);

			IPLAudioBuffer inbuffer{ mono, m_renderSettings.frameSize, nullptr };
			std::vector<float> outputaudioframe(2 * m_renderSettings.frameSize);
			IPLAudioBuffer outbuffer{ stereo, m_renderSettings.frameSize, outputaudioframe.data() };


			std::hash<std::string> hash {};
			IPLBakedDataIdentifier identifier {};
			identifier.identifier = hash("world_sound4");
			identifier.type = IPLBakedDataType::IPL_BAKEDDATATYPE_REVERB;
		IPLhandle convolutionEffect;
		//err = iplCreateConvolutionEffect(renderHandle,identifier,IPLSimulationType::IPL_SIMTYPE_REALTIME,stereo,stereo,&convolutionEffect);
		err = iplCreateConvolutionEffect(renderHandle,identifier,IPLSimulationType::IPL_SIMTYPE_BAKED,stereo,stereo,&convolutionEffect);
		*/
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
			// steam audio fmod

			/*
static FMOD_PLUGINLIST gPluginList[] =
{
    { FMOD_PLUGINTYPE_DSP, &gSpatializerEffect },
    { FMOD_PLUGINTYPE_DSP, &gMixEffect },
    { FMOD_PLUGINTYPE_DSP, &gReverbEffect },
    { FMOD_PLUGINTYPE_MAX, nullptr }
};
*/

			//
#endif

			/*if(err != IPLerror::IPL_STATUS_SUCCESS)
		{
			if(errorHandler != nullptr)
				errorHandler(err);
			return;
		}*/

			//test_binauralEffect = effect;
			//test_inbuffer = inbuffer;
			//test_outbuffer = outbuffer;
			//test_frameSize = m_renderSettings.frameSize;
			//test_envHandle = envHandle;
			//test_directSoundEffect = convolutionEffect;
		}
		//

		if(fOnComplete != nullptr)
			fOnComplete();
	});
}
void ipl::Scene::Finalize(const FinalizeInfo &info, const std::function<void(LoadStage, float)> &fStageProgress, const std::function<void(void)> &fOnComplete, const std::function<void(IPLerror)> &errorHandler)
{
	std::shared_ptr<VFilePtrInternal> f = nullptr;
	Finalize(f, info, fStageProgress, fOnComplete, errorHandler);
}

const std::vector<std::shared_ptr<ipl::StaticMesh>> &ipl::Scene::GetStaticMeshes() const { return m_staticMeshes; }

std::shared_ptr<ipl::StaticMesh> ipl::Scene::CreateStaticMesh(const std::vector<IPLVector3> &verts, const std::vector<IPLTriangle> &triangles, const std::vector<IPLint32> &materialIndices)
{
	if(IsFinalized())
		throw std::logic_error("Cannot add static mesh to scene after scene has been finalized!");
	auto r = StaticMesh::Create(*this, verts, triangles, materialIndices);
	m_staticMeshes.push_back(r);
	return r;
}
void ipl::Scene::AddProbeSphere(const Vector3 &origin, float radius)
{
	if(m_probeBoxes.size() == m_probeBoxes.capacity())
		m_probeBoxes.reserve(m_probeBoxes.size() + 50);
	m_probeBoxes.push_back({origin, radius});
}
void ipl::Scene::AddProbeBox(const Vector3 &min, const Vector3 &max, float spacing, float heightAboveFloor)
{
	if(m_probeBoxes.size() == m_probeBoxes.capacity())
		m_probeBoxes.reserve(m_probeBoxes.size() + 50);
	m_probeBoxes.push_back({min, max, spacing, heightAboveFloor});
}
void ipl::Scene::RegisterPropagationSoundSource(const std::string &name, const Vector3 &origin, float radius)
{
	if(IsFinalized())
		throw std::logic_error("Cannot register propagation sound source to scene after scene has been finalized!");
	auto it = std::find_if(m_propagationSoundSources.begin(), m_propagationSoundSources.end(), [&name](const PropagationSoundSource &src) { return ustring::compare(src.name, name, false); });
	if(it == m_propagationSoundSources.end()) {
		m_propagationSoundSources.push_back({});
		it = m_propagationSoundSources.end() - 1;
	}
	auto &src = *it;
	src.name = name;
	src.origin = origin;
	src.radius = radius;
}

ipl::Scene::~Scene()
{
	SceneState::resetEnvironment();
	if(m_finalizeData != nullptr) {
		m_finalizeData->Stop();
		m_finalizeData = nullptr;
	}
}

IPLAudioFormat ipl::Scene::GetIplInputFormat() const { return m_inputFormat; }
IPLAudioFormat ipl::Scene::GetIplOutputFormat() const { return GlobalState::get()->outputFormat(); }
void *ipl::Scene::GetIplDirectSoundEffect() const { return m_iplDirectSoundEffect.get(); }

IPLhandle ipl::Scene::GetIplScene() const { return m_iplScene.get(); }
IPLhandle ipl::Scene::GetIplEnvironment() const { return m_iplEnv.get(); }
IPLhandle ipl::Scene::GetIplRenderer() const { return SceneState::get()->environmentalRenderer(); }
IPLRenderingSettings ipl::Scene::GetRenderSettings() const { return GlobalState::get()->renderingSettings(); }
IPLSimulationSettings &ipl::Scene::GetSimulationSettings() { return m_iplSimSettings; }
bool ipl::Scene::IsPropagationDelayEnabled() const { return m_bPropagationDelayEnabled; }
void ipl::Scene::SetPropagationDelayEnabled(bool b) { m_bPropagationDelayEnabled = b; }

IPLerror ipl::Scene::Load(const std::shared_ptr<VFilePtrInternal> &f, IPLLoadSceneProgressCallback callback)
{
	std::array<uint8_t, 3> header;
	f->Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'S' || header.at(1) != 'T' || header.at(2) != 'A')
		return IPLerror::IPL_STATUS_FAILURE;
	auto version = f->Read<uint16_t>();
	if(version < 1u || version > 2u)
		return IPLerror::IPL_STATUS_FAILURE;
	auto flags = f->Read<uint16_t>();
	f->Seek(f->Tell() + sizeof(uint64_t) * 4u);

	auto szSceneData = f->Read<uint64_t>();
	std::vector<uint8_t> sceneData(szSceneData);
	f->Read(sceneData.data(), sceneData.size() * sizeof(sceneData.front()));
	IPLhandle iplScene;
	auto iplContext = m_context->GetIplContext();
	auto err = iplLoadScene(iplContext, m_iplSimSettings, sceneData.data(), sceneData.size() * sizeof(sceneData.front()), nullptr, callback, &iplScene);
	if(err != IPLerror::IPL_STATUS_SUCCESS)
		return err;
	m_iplScene = std::shared_ptr<void>(iplScene, [](IPLhandle iplScene) { iplDestroyScene(&iplScene); });
	auto fReadData = [&f, iplContext](std::vector<std::shared_ptr<void>> &items) -> IPLerror {
		auto numItems = f->Read<uint32_t>();
		std::vector<uint64_t> sizes(numItems);
		for(auto i = decltype(numItems) {0}; i < numItems; ++i)
			sizes.at(i) = f->Read<uint64_t>();
		items.reserve(numItems);
		for(auto i = decltype(numItems) {0}; i < numItems; ++i) {
			auto size = sizes.at(i);
			std::vector<uint8_t> data(size);
			f->Read(data.data(), data.size() * sizeof(data.front()));
			IPLhandle probeBatch = nullptr;
			auto err = iplLoadProbeBatch(iplContext, data.data(), data.size() * sizeof(data.front()), &probeBatch);
			if(err != IPLerror::IPL_STATUS_SUCCESS)
				return err;
			items.push_back(std::shared_ptr<void>(probeBatch, [](IPLhandle probeBatch) { iplDestroyProbeBatch(&probeBatch); }));
		}
		return IPLerror::IPL_STATUS_SUCCESS;
	};

	// Probe boxes
	err = fReadData(m_iplProbeBoxes);
	if(err != IPLerror::IPL_STATUS_SUCCESS)
		return err;

	// Probe batches
	err = fReadData(m_iplProbeBatches);
	if(err != IPLerror::IPL_STATUS_SUCCESS)
		return err;

	// Sound sources
	m_propagationSoundSources.clear();
	if(version >= 2u) {
		auto numBakedSoundSources = f->Read<uint32_t>();
		m_propagationSoundSources.reserve(numBakedSoundSources);
		for(auto i = decltype(numBakedSoundSources) {0u}; i < numBakedSoundSources; ++i) {
			m_propagationSoundSources.push_back({});
			auto &sndSrc = m_propagationSoundSources.back();
			sndSrc.name = f->ReadString();
			sndSrc.origin = f->Read<Vector3>();
			sndSrc.radius = f->Read<float>();
		}
	}
	return IPLerror::IPL_STATUS_SUCCESS;
}

void ipl::Scene::Save(VFilePtrReal &f, bool bSaveProbeBoxes) const
{
	std::vector<uint8_t> serializedData;
	auto sz = iplSaveScene(m_iplScene.get(), nullptr);
	serializedData.resize(sz);
	iplSaveScene(m_iplScene.get(), serializedData.data());

	const std::array<uint8_t, 3> header = {'S', 'T', 'A'};
	f->Write(header.data(), header.size() * sizeof(header.front()));
	f->Write<uint16_t>(2u); // Version
	f->Write<uint16_t>(0u); // Flags;
	auto indexOffsetSceneData = f->Tell();
	f->Write<uint64_t>(0ull);
	auto indexOffsetProbeBoxes = f->Tell();
	f->Write<uint64_t>(0ull);
	auto indexOffsetProbeBatches = f->Tell();
	f->Write<uint64_t>(0ull);
	auto indexOffsetBakedSoundSources = f->Tell();
	f->Write<uint64_t>(0ull);

	const auto fWriteOffset = [&f](uint64_t indexOffset) {
		auto offset = f->Tell();
		f->Seek(indexOffset);
		f->Write<uint64_t>(offset);
		f->Seek(offset);
	};
	fWriteOffset(indexOffsetSceneData);

	f->Write<uint64_t>(serializedData.size() * sizeof(serializedData.front()));
	f->Write(serializedData.data(), serializedData.size() * sizeof(serializedData.front()));

	auto fWriteData = [&f, &fWriteOffset](uint64_t indexOffset, const std::vector<std::shared_ptr<void>> &items, const std::function<IPLint32(IPLhandle, IPLbyte *)> &fSave) {
		fWriteOffset(indexOffset);
		auto numElements = items.size();
		f->Write<uint32_t>(numElements);
		std::vector<uint64_t> sizes;
		sizes.reserve(numElements);
		for(auto &hItem : items) {
			auto size = fSave(hItem.get(), nullptr);
			sizes.push_back(size);
			f->Write<uint64_t>(size);
		}
		auto batchIdx = 0u;
		for(auto &hItem : items) {
			auto size = sizes.at(batchIdx++);
			std::vector<uint8_t> data(size);
			fSave(hItem.get(), data.data());
			f->Write(data.data(), data.size() * sizeof(data.front()));
		}
	};

	// Probe boxes
	if(bSaveProbeBoxes == true)
		fWriteData(indexOffsetProbeBoxes, m_iplProbeBoxes, iplSaveProbeBox);
	else {
		std::vector<std::shared_ptr<void>> probeBoxes;
		fWriteData(indexOffsetProbeBoxes, probeBoxes, iplSaveProbeBox);
	}

	// Probe batches
	fWriteData(indexOffsetProbeBatches, m_iplProbeBatches, iplSaveProbeBatch);

	// Sound sources
	fWriteOffset(indexOffsetBakedSoundSources);
	f->Write<uint32_t>(m_propagationSoundSources.size());
	for(auto &sndSrc : m_propagationSoundSources) {
		f->WriteString(sndSrc.name);
		f->Write<Vector3>(sndSrc.origin);
		f->Write<float>(sndSrc.radius);
	}
}

const std::shared_ptr<ipl::Context> &ipl::Scene::GetContext() const { return m_context; }

IPLDirectSoundPath ipl::Scene::GetDirectSoundPath(const Vector3 &listenerPos, const Vector3 &listenerDir, const Vector3 &listenerUp, const Vector3 &srcPos, float srcRadius) const
{
	IPLSource src {};
	src.position = al::to_custom_vector<IPLVector3>(al::to_audio_position(srcPos));
	src.ahead = IPLVector3 {1.f, 0.f, 0.f};
	src.directivity.dipoleWeight = 0.f; // TODO
	src.right = IPLVector3 {0.f, 0.f, -1.f};
	src.up = IPLVector3 {0.f, 1.f, 0.f};
	return iplGetDirectSoundPath(SceneState::get()->environmentalRenderer(), al::to_custom_vector<IPLVector3>(al::to_audio_position(listenerPos)), al::to_custom_vector<IPLVector3>(al::to_audio_direction(listenerDir)), al::to_custom_vector<IPLVector3>(al::to_audio_direction(listenerUp)),
	  src, al::to_audio_distance(srcRadius), IPLDirectOcclusionMode::IPL_DIRECTOCCLUSION_NOTRANSMISSION, IPLDirectOcclusionMethod::IPL_DIRECTOCCLUSION_VOLUMETRIC);
}

#endif
