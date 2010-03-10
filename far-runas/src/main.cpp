﻿/**
	runas: Run As FAR plugin
	Allow to run applications from other user

	© 2010  Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "win_def.h"

#include "far/far_helper.hpp"
#include "far/CRT/crt.hpp"

#include <shlwapi.h>
#include <lm.h>

///========================================================================================== define
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

extern "C" {
	WINADVAPI BOOL WINAPI	IsTokenRestricted(HANDLE TokenHandle);
	WINADVAPI BOOL APIENTRY	CreateRestrictedToken(HANDLE ExistingTokenHandle, DWORD Flags,
			DWORD DisableSidCount, PSID_AND_ATTRIBUTES SidsToDisable,
			DWORD DeletePrivilegeCount, PLUID_AND_ATTRIBUTES PrivilegesToDelete,
			DWORD RestrictedSidCount, PSID_AND_ATTRIBUTES SidsToRestrict,
			PHANDLE NewTokenHandle);
};

#ifndef DISABLE_MAX_PRIVILEGE
#define DISABLE_MAX_PRIVILEGE   0x1
#endif

PCWSTR prefix = L"runas";

///========================================================================================== struct
enum		{
	MUsername = 5,
	MPasword,
	MRestricted,
	MCommandLine,
	MError,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;
//PluginOptions					Options;


bool			InitUsers(FarList &users) {
	DWORD dwLevel = 3;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	USER_INFO_3 *psi = NULL;
	nStatus = ::NetUserEnum(NULL, dwLevel,
							FILTER_NORMAL_ACCOUNT,
							(PBYTE*) & psi,
							MAX_PREFERRED_LENGTH,
							&dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
	if (nStatus == NERR_Success) {
		users.ItemsNumber = 0;
		WinMem::Alloc(users.Items, sizeof(*users.Items) * dwEntriesRead);
		for (DWORD i = 0; i < dwEntriesRead; ++i) {
			if (!WinFlag<DWORD>::Check(psi[i].usri3_flags, UF_ACCOUNTDISABLE)) {
				users.Items[users.ItemsNumber].Text = WinStr::Assign(psi[i].usri3_name);
				if (psi[i].usri3_priv == USER_PRIV_ADMIN) {
					WinFlag<DWORD>::Set(users.Items[users.ItemsNumber].Flags, LIF_CHECKED);
				}
				++users.ItemsNumber;
			}
		}
		::NetApiBufferFree(psi);
	}
	return	nStatus == NERR_Success;
}
bool			FreeUsers(FarList &users) {
	for (int i = 0; i < users.ItemsNumber; ++i) {
		WinStr::Free(users.Items[i].Text);
	}
	WinMem::Free(users.Items);
	return	true;
}

HRESULT			ExecAsUser(PCWSTR app, PCWSTR user, PCWSTR pass) {
	CStrW	cmd = WinFS::Expand(app);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	if (::CreateProcessWithLogonW(user, NULL, pass, LOGON_WITH_PROFILE, NULL, (PWSTR)cmd.c_str(),
								  CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE,
								  NULL, NULL, &si, &pi)) {
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		return	NO_ERROR;
	}
	return	::GetLastError();
}
HRESULT			ExecRestricted(PCWSTR app) {
	CStrW	cmd = WinFS::Expand(app);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	WinToken	hToken(TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT);
	if (hToken.IsOK()) {
		HANDLE	hTokenRest = NULL;
		//TODO сделать restricted DACL и Deny only проверку
		if (::CreateRestrictedToken(hToken, DISABLE_MAX_PRIVILEGE, 0, NULL, 0, NULL, 0, NULL, &hTokenRest)) {
			if (::CreateProcessAsUserW(hTokenRest, NULL, (PWSTR)cmd.c_str(), NULL, NULL, false,
									   NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
				::CloseHandle(pi.hThread);
				::CloseHandle(pi.hProcess);
				::CloseHandle(hTokenRest);
				return	NO_ERROR;
			}
			::CloseHandle(hTokenRest);
		}
	}
	return	::GetLastError();
}

///========================================================================================== Export
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
//	delete(PList*)hPlugin;
}
int		WINAPI	EXP_NAME(Configure)(int) {
//	Options.Write();
	return	true;
}
void	WINAPI	EXP_NAME(ExitFAR)() {
}
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

//	static const TCHAR	*DiskStrings[1];
//	static int			DiskNumbers[1] = {6};
//	DiskStrings[0] = GetMsg(DTitle);
//	pi->DiskMenuStrings = DiskStrings;
//	pi->DiskMenuNumbers = DiskNumbers;
//	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	pi->DiskMenuStringsNumber = 0;

	static const TCHAR	*MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

//	pi->PluginConfigStrings = MenuStrings;
//	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = prefix;
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const TCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
//	Options.Read();

	CStrW	cline;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		FarPnl	pi(PANEL_ACTIVE);
		if (pi.IsOK()) {
			CStrW	buf(MAX_PATH_LENGTH + MAX_PATH + 1);
			fsf.GetCurrentDirectory(buf.capacity(), (PWSTR)buf.c_str());
			if (!buf.empty())
				::PathAddBackslash((PWSTR)buf.c_str());

			PluginPanelItem &PPI = pi[pi.CurrentItem()];
			buf += PPI.FindData.lpwszFileName;
			cline = buf;
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		cline = (PCWSTR)Item;
	}
	FarList	users;
	if (InitUsers(users)) {
		InitDialogItem	Items[] = {
			{DI_DOUBLEBOX, 3, 1, 44, 12, 0, 0, 0, 0, GetMsg(DlgTitle)},
			{DI_TEXT,      5, 2, 0,  0,  0, 0, 0, 0, GetMsg(MUsername)},
			{DI_COMBOBOX,  5, 3, 42, 0,  1, (DWORD_PTR)&users, DIF_SELECTONENTRY, 1, L""},
			{DI_TEXT,      5, 4, 0,  0,  0, 0, 0, 0, GetMsg(MPasword)},
			{DI_PSWEDIT,   5, 5, 42, 0,  0, 0, 0, 0, L""},
			{DI_CHECKBOX , 5, 6, 42, 0,  0, 0, 0, 0, GetMsg(MRestricted)},
			{DI_TEXT,      0, 7, 0,  0,  0, 0, DIF_SEPARATOR, 0, L""},
			{DI_TEXT,      5, 8, 0,  0,  0, 0, 0, 0, GetMsg(MCommandLine)},
			{DI_EDIT,      5, 9, 42, 0,  0, (DWORD_PTR)L"runas.comline", DIF_HISTORY, 0, cline},
			{DI_TEXT,      5, 10, 0,  0,  0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
			{DI_BUTTON,    0, 11, 0,  0,  0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
			{DI_BUTTON,    0, 11, 0,  0,  0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
		};
		size_t	size = sizeofa(Items);
		FarDialogItem FarItems[size];
		InitDialogItems(Items, FarItems, size);
		HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 48, 14, L"Contents",
									  FarItems, size, 0, 0, NULL, 0);
		if (hDlg != INVALID_HANDLE_VALUE) {
			HRESULT	err = NO_ERROR;
			while (true) {
				int		ret = psi.DialogRun(hDlg);
				if (ret == -1 || ret == 11)
					break;
				CStrW	cmd(GetDataPtr(hDlg, 8));
				if (GetCheck(hDlg, 5)) {
					err = ExecRestricted(cmd);
				} else {
					CStrW	user(GetDataPtr(hDlg, 2));
					CStrW	pass(GetDataPtr(hDlg, 4));
					err = ExecAsUser(cmd, user, pass);
				}
				if (err == NO_ERROR) {
					break;
				} else {
					PCWSTR Msg[] = {GetMsg(MError), cmd.c_str(), L"", GetMsg(txtBtnOk), };
					::SetLastError(err);
					psi.Message(psi.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE,
								 L"Contents", Msg, sizeofa(Msg), 1);
				}
			}
			psi.DialogFree(hDlg);
		}
		FreeUsers(users);
	}

	return	INVALID_HANDLE_VALUE;
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}

///========================================================================================= WinMain
extern		"C" {
	BOOL		WINAPI	DllMainCRTStartup(HANDLE, DWORD, PVOID) {
		return	true;
	}
}
