#pragma once

#define PDparentKey _T("SOFTWARE\\ESI\\PhotoDiode")

#pragma pack(push)
#pragma pack(1)
union PDplacement
{
	ULONGLONG qword;
	struct
	{
		short left;
		short top;
		short right;
		short bottom;
	};
};
#pragma pack(pop)

class CPDplacement
{
public:
	static void ReadRegistry(D2D1_RECT_F* prect)
	{
		PDplacement placement;
		HKEY hkResult;
		LSTATUS res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PDparentKey, 0, KEY_READ, &hkResult);
		if (res != ERROR_FILE_NOT_FOUND)
		{
			ASSERT(res == ERROR_SUCCESS);
			TRACE("Registry key opened\n");
			DWORD nData;
			res = RegQueryValueEx(
				hkResult,
				_T("Placement"),
				NULL,
				NULL,
				(BYTE*)&placement.qword,
				&nData);
			ASSERT(res == ERROR_SUCCESS);
			ASSERT(nData == 8);
			res = RegCloseKey(hkResult);
			ASSERT(res == ERROR_SUCCESS);
			prect->left = (float)placement.left;
			prect->top = (float)placement.top;
			prect->right = (float)placement.right;
			prect->bottom = (float)placement.bottom;
		}
	}
};