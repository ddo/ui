/* 17 july 2014 */

#include "winapi_windows.h"

static ULONG_PTR comctlManifestCookie;
static HMODULE comctl32;

/* TODO do any of these take WINAPI? */
BOOL (*WINAPI fv_SetWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR);
BOOL (*WINAPI fv_RemoveWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR);
LRESULT (*WINAPI fv_DefSubclassProc)(HWND, UINT, WPARAM, LPARAM);

DWORD initCommonControls(LPCWSTR manifest, char **errmsg)
{
	ACTCTX actctx;
	HANDLE ac;
	INITCOMMONCONTROLSEX icc;
	FARPROC f;
	/* TODO does this take WINAPI? */
	BOOL (*WINAPI ficc)(const LPINITCOMMONCONTROLSEX);

	ZeroMemory(&actctx, sizeof (ACTCTX));
	actctx.cbSize = sizeof (ACTCTX);
	actctx.dwFlags = ACTCTX_FLAG_SET_PROCESS_DEFAULT;
	actctx.lpSource = manifest;
	ac = CreateActCtx(&actctx);
	if (ac == INVALID_HANDLE_VALUE) {
		*errmsg = "error creating activation context for synthesized manifest file";
		return GetLastError();
	}
	if (ActivateActCtx(ac, &comctlManifestCookie) == FALSE) {
		*errmsg = "error activating activation context for synthesized manifest file";
		return GetLastError();
	}

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS;

	comctl32 = LoadLibraryW(L"comctl32.dll");
	if (comctl32 == NULL) {
		*errmsg = "error loading comctl32.dll";
		return GetLastError();
	}

	/* GetProcAddress() only takes a multibyte string */
#define LOAD(fn) f = GetProcAddress(comctl32, fn); \
	if (f == NULL) { \
		*errmsg = "error loading " fn "()"; \
		return GetLastError(); \
	}

	LOAD("InitCommonControlsEx");
	ficc = (BOOL (*WINAPI)(const LPINITCOMMONCONTROLSEX)) f;
	LOAD("SetWindowSubclass");
	fv_SetWindowSubclass = (BOOL (*WINAPI)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR)) f;
	LOAD("RemoveWindowSubclass");
	fv_RemoveWindowSubclass = (BOOL (*WINAPI)(HWND, SUBCLASSPROC, UINT_PTR)) f;
	LOAD("DefSubclassProc");
	fv_DefSubclassProc = (LRESULT (*WINAPI)(HWND, UINT, WPARAM, LPARAM)) f;

	if ((*ficc)(&icc) == FALSE) {
		*errmsg = "error initializing Common Controls (comctl32.dll)";
		return GetLastError();
	}

	return 0;
}
