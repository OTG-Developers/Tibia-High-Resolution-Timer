#include <windows.h>

#include <stdint.h>
#include <sys/timeb.h>

#pragma warning(disable:4996)

int64_t started_ticks = 0;

bool hires_timer_available = false;
LARGE_INTEGER hires_start_ticks;
LARGE_INTEGER hires_ticks_per_second;

static int Init()
{
	if(QueryPerformanceFrequency(&hires_ticks_per_second))
	{
		hires_timer_available = true;
		QueryPerformanceCounter(&hires_start_ticks);
	}
	else
	{
		hires_timer_available = false;
		_timeb t;
		_ftime(&t);
		started_ticks = static_cast<int64_t>(t.millitm) + static_cast<int64_t>(t.time) * 1000;
	}
	return 1;
}

extern "C"
{
	MMRESULT WINAPI new_timeBeginPeriod(UINT uPeriod)
	{
		return TIMERR_NOERROR;
	}

	MMRESULT WINAPI new_timeEndPeriod(UINT uPeriod)
	{
		return TIMERR_NOERROR;
	}

	MMRESULT WINAPI new_timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
	{
		return TIMERR_NOCANDO;
	}

	DWORD WINAPI new_timeGetTime(void)
	{
		if(hires_timer_available)
		{
			LARGE_INTEGER hires_now;
			QueryPerformanceCounter(&hires_now);
			hires_now.QuadPart -= hires_start_ticks.QuadPart;
			hires_now.QuadPart *= 1000;
			hires_now.QuadPart /= hires_ticks_per_second.QuadPart;
			return static_cast<DWORD>(hires_now.QuadPart);
		}
		_timeb t;
		_ftime(&t);
		return static_cast<DWORD>((static_cast<int64_t>(t.millitm) + static_cast<int64_t>(t.time) * 1000) - started_ticks);
	}

	MMRESULT WINAPI new_timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
	{
		ptc->wPeriodMin = 1;
		ptc->wPeriodMax = 1;
		return TIMERR_NOERROR;
	}
}

extern "C"
{
	BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
	{
		switch(dwReason)
		{
			case DLL_PROCESS_ATTACH:
				return Init();

			case DLL_THREAD_ATTACH:
			case DLL_THREAD_DETACH:
			case DLL_PROCESS_DETACH:
				break;
		}
		return 1;
	}
}

