#include <windows.h>

static void Patch(void* address, void* data, int size)
{
	unsigned long protect[2];
	VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &protect[0]);
	memcpy(address, data, size);
	VirtualProtect(address, size, protect[0], &protect[1]);
}

static void SetInt(int address, int value)
{
	Patch((void *)address, &value, 4);
}

static void SetFloat(int address, float value)
{
	Patch((void *)address, &value, 4);
}

void WideScreenFix()
{
	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int res_x = info.rcMonitor.right - info.rcMonitor.left;
	int res_y = info.rcMonitor.bottom - info.rcMonitor.top;

	SetInt(0x403DA2, res_x);
	SetInt(0x403DAC, res_y);

	SetInt(0x40949A, res_x);
	SetInt(0x409495, res_y);

	SetInt(0x432B8C, res_x);
	SetInt(0x432B91, res_y);

	SetInt(0x4553FC, res_x);
	SetInt(0x455406, res_y);

	SetInt(0x90B9DC, res_x);
	SetInt(0x90B9E0, res_y);

	SetInt(0xA0A930, res_x);
	SetInt(0xA0A934, res_y);

	SetFloat(0x552487 + 0x1, (float)res_x / (float)res_y);
	SetFloat(0x55248E + 0x1, (float)res_x / (float)res_y);
}
