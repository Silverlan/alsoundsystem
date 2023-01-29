/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "steam_audio/alsound_steam_audio.hpp"

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1

#pragma comment(lib, "phonon.lib")

std::string ipl::result_to_string(IPLerror err)
{
	switch(err) {
	case IPL_STATUS_SUCCESS:
		return "The operation completed successfully.";
	case IPL_STATUS_FAILURE:
		return "An unspecified error occurred.";
	case IPL_STATUS_OUTOFMEMORY:
		return "The system ran out of memory.";
	case IPL_STATUS_INITIALIZATION:
		return "An error occurred while initializing an external dependency.";
	default:
		return "An unknown error occurred.";
	}
}

#endif
