#include <Math_Z.h>

class SetAndRestoreFloatControlDownward
{
public:
	SetAndRestoreFloatControlDownward()
	{
#if defined(_WIN32)
		// Save current rounding mode.
#if defined(_PC_SSE)
		m_previousMmCsr = _MM_GET_ROUNDING_MODE();
#endif
		m_previousControl87 = _control87(0, 0);

		// Set rounding mode to down.
#if defined(_PC_SSE)
		_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
#endif
		_control87(_RC_DOWN, _MCW_RC);
#if !defined(_M_X64)
		_control87(_PC_53, _MCW_PC);
#endif
#endif
	}

	~SetAndRestoreFloatControlDownward()
	{
		// Restore previous rounding mode.
#if defined(_WIN32)
#if defined(_PC_SSE)
		_MM_SET_ROUNDING_MODE(m_previousMmCsr);
#endif
		_control87(m_previousControl87, _MCW_RC);
#if !defined(_M_X64)
		_control87(m_previousControl87, _MCW_PC);
#endif
#endif
	}

private:
#if defined(_PC_SSE)
	unsigned int m_previousMmCsr;
#endif
	unsigned int m_previousControl87;
};

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
