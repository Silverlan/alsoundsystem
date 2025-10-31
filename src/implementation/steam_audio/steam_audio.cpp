// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.alsoundsystem;

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
