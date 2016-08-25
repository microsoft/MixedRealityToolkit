#include <Math_Z.h>

inline void	SetFloatControlDownward()
{
#if defined(_WIN32)
#if defined(_PC_SSE)
	_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
#endif
	_control87(_RC_DOWN, _MCW_RC);
#if !defined(_M_X64)
	_control87(_PC_53, _MCW_PC);
#endif
#endif
	CHECKFTOLMODE();
}

inline  void	SetFloatControlExceptions()
{

#if defined(_WIN32) && !defined(_SUBMISSION) && defined(_M_IX86)
	unsigned int x86_cw;
	_clearfp();
	__control87_2(_EM_UNDERFLOW | _EM_INEXACT | _EM_DENORMAL | _EM_ZERODIVIDE, _MCW_EM, &x86_cw, NULL);
#endif
}

inline  void	UnSetFloatControlExceptions()
{

#if defined(_WIN32) && !defined(_SUBMISSION) && defined(_M_IX86)
	unsigned int x86_cw;
	_clearfp();
	__control87_2(_MCW_EM, _MCW_EM, &x86_cw, NULL);
#endif
}
