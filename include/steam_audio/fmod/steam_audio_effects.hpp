#ifndef __STEAM_AUDIO_EFFECTS_HPP__
#define __STEAM_AUDIO_EFFECTS_HPP__

#include "alsound_definitions.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
/** Parameters that can be set by the user on a Reverb effect.
 */
enum ReverbEffectParams {
	SA_REVERB_PARAM_BINAURAL,       /* bool */
	SA_REVERB_PARAM_SIMTYPE,        /* int (IPLSimulationType) */
	SA_REVERB_PARAM_SOURCEPOSITION, /* FMOD_DSP_PARAMETER_3DATTRIBUTES */
	SA_REVERB_PARAM_NAME,           /* IPLint32 */
	SA_REVERB_NUM_PARAMS
};

enum SpatializerEffectParams {
	SA_SPATIALIZE_PARAM_DIRECTBINAURAL,      /* bool */
	SA_SPATIALIZE_PARAM_HRTFINTERPOLATION,   /* int (IPLHrtfInterpolation) */
	SA_SPATIALIZE_PARAM_DISTANCEATTENUATION, /* bool */
	SA_SPATIALIZE_PARAM_AIRABSORPTION,       /* bool */
	SA_SPATIALIZE_PARAM_OCCLUSIONMODE,       /* int (IPLDirectOcclusionMode) */
	SA_SPATIALIZE_PARAM_OCCLUSIONMETHOD,     /* int (IPLDirectOcclusionMethod) */
	SA_SPATIALIZE_PARAM_SOURCERADIUS,        /* float */
	SA_SPATIALIZE_PARAM_DIRECTLEVEL,         /* float */
	SA_SPATIALIZE_PARAM_INDIRECT,            /* bool */
	SA_SPATIALIZE_PARAM_INDIRECTBINAURAL,    /* bool */
	SA_SPATIALIZE_PARAM_INDIRECTLEVEL,       /* float */
	SA_SPATIALIZE_PARAM_SIMTYPE,             /* int (IPLSimulationType) */
	SA_SPATIALIZE_PARAM_STATICLISTENER,      /* bool */
	SA_SPATIALIZE_PARAM_NAME,                /* IPLint32 */
	SA_SPATIALIZE_PARAM_SOURCEPOSITION,      /* FMOD_DSP_PARAMETER_3DATTRIBUTES */
	SA_SPATIALIZE_NUM_PARAMS
};

/** Parameters that can be set by the user on a Mixer Return effect.
 */
enum MixEffectParams {
	SA_MIX_PARAM_BINAURAL, /* bool */
	SA_MIX_NUM_PARAMS
};
#endif
#endif
