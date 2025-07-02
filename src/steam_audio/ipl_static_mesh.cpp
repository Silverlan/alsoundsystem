// SPDX-FileCopyrightText: © 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "steam_audio/alsound_steam_audio.hpp"

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1

#include <iostream>
std::shared_ptr<ipl::StaticMesh> ipl::StaticMesh::Create(Scene &scene, const std::vector<IPLVector3> &verts, const std::vector<IPLTriangle> &tris, const std::vector<IPLint32> &materialIndices)
{
	return std::shared_ptr<ipl::StaticMesh>(new ipl::StaticMesh(scene, verts, tris, materialIndices));
}

ipl::StaticMesh::StaticMesh(Scene &scene, const std::vector<IPLVector3> &verts, const std::vector<IPLTriangle> &tris, const std::vector<IPLint32> &materialIndices) : m_scene(scene.shared_from_this()), m_vertices(verts), m_triangles(tris), m_materialIndices(materialIndices) {}

const std::vector<IPLVector3> &ipl::StaticMesh::GetVertices() const { return const_cast<StaticMesh *>(this)->GetVertices(); }
std::vector<IPLVector3> &ipl::StaticMesh::GetVertices() { return m_vertices; }

const std::vector<IPLTriangle> &ipl::StaticMesh::GetTriangles() const { return const_cast<StaticMesh *>(this)->GetTriangles(); }
std::vector<IPLTriangle> &ipl::StaticMesh::GetTriangles() { return m_triangles; }

const std::vector<IPLint32> &ipl::StaticMesh::GetMaterialIndices() const { return const_cast<StaticMesh *>(this)->GetMaterialIndices(); }
std::vector<IPLint32> &ipl::StaticMesh::GetMaterialIndices() { return m_materialIndices; }

bool ipl::StaticMesh::Spawn()
{
	if(m_scene.expired() || m_iplMesh != nullptr)
		return false;
	auto scene = m_scene.lock();
	IPLhandle iplMesh;
	auto err = iplCreateStaticMesh(scene->GetIplScene(), m_vertices.size(), m_triangles.size(), m_vertices.data(), m_triangles.data(), m_materialIndices.data(), &iplMesh);
	if(err != IPLerror::IPL_STATUS_SUCCESS) {
		scene->GetContext()->InvokeErrorHandler(err);
		return false;
	}
	m_iplMesh = std::shared_ptr<void>(iplMesh, [](IPLhandle iplMesh) { iplDestroyStaticMesh(&iplMesh); });

	// Don't need to keep these around anymore
	m_vertices = {};
	m_triangles = {};
	m_materialIndices = {};
	return true;
}

#endif
