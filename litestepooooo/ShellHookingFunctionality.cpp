#include "ShellHookingFunctionality.h"


unsigned WM_ShellHook = 0;

// registers the Shell32
void (WINAPI* pRegisterShellHook)(HWND, DWORD);

static const char* rtl_libs[] =
{
	"SHELL32",
	NULL
};

void init_runtime_libs(void)
{
	struct proc_info { const char* lib; char* procname; void* procadr; };

	static struct proc_info rtl_list[] =
	{
		{ "SHELL32", (char*)0xB5, &pRegisterShellHook },
		{ NULL }
	};
	struct proc_info* rtl_ptr = rtl_list;

	do
	{
		*(FARPROC*)rtl_ptr->procadr = GetProcAddress(LoadLibrary(rtl_ptr->lib), rtl_ptr->procname);
	} while ((++rtl_ptr)->lib);

}

void exit_runtime_libs(void)
{
	const char** p = rtl_libs;
	do FreeLibrary(GetModuleHandle(*p)); while (*++p);
}



void register_shellhook(HWND hwnd)
{
	if (pRegisterShellHook)
	{
		pRegisterShellHook(NULL, TRUE);
		pRegisterShellHook(hwnd, 3);
		WM_ShellHook = RegisterWindowMessage("SHELLHOOK");
	}
}

void unregister_shellhook(HWND hwnd)
{
	if (pRegisterShellHook)
	{
		pRegisterShellHook(hwnd, 0);
	}
}

void initShellHook(HWND hwnd)
{
	init_runtime_libs();
	register_shellhook(hwnd);
}

void exitShellHook(HWND hwnd)
{
	exit_runtime_libs();
	unregister_shellhook(hwnd);
}