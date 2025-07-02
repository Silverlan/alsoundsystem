// SPDX-FileCopyrightText: © 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "alsound_definitions.hpp"

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#pragma comment(lib, "fmodstudio64_vc.lib")
#pragma comment(lib, "fmod64_vc.lib")
#endif

#ifdef ALSOUNDSYSTEM_EXECUTABLE
#include <iostream>
#include "alsoundsystem.hpp"
#include <efx-presets.h>
#include <AL/alure2.h>
#include <chrono>
#include <fstream>

int main(int argc, char *argv[])
{
	//test_steam_audio();
	auto sndSys = al::SoundSystem::Create();
	try {
		auto *buf = sndSys->LoadSound("br_tofreeman08.wav");
		auto sndSrc = sndSys->CreateSource(*buf);
		sndSrc->Play();
		while(sndSrc->IsPlaying())
			sndSys->Update();
	}
	catch(const std::exception &e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	return EXIT_SUCCESS;
}
#endif
