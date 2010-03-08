﻿/**
	filever plugin for FAR Manager
	Copyright (C) 2009 GrAnD

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
**/

#include "win_def.h"

#include "far/far_helper.hpp"
#include "far/farkeys.hpp"

#include "svcmgr.hpp"
#include "svcmgr-opt.hpp"

///========================================================================================== define
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

///========================================================================================== struct
enum		eMessage {
	MenuTitle,
	DiskTitle,
	DlgTitle,

	MbuttonOk,
	MbuttonCancel,

	MSelectComputer,
	MComputer,
	MEmptyForLocal,
	MUsername,
	MPaswd,
	MEmptyForCurrent
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;
PluginOptions			Options;
PCWSTR 					prefix = L"svcmgr";


class		cShowWaitState {
public:
	void		operator()(DWORD state, uintmax_t elapsed, PVOID param) {
		static	DWORD	time;
//		static	FarDlg	*hDlg;
		if (elapsed / 1000 != time) {
			time = elapsed / 1000;
//			psi.SendDlgMessage(hDlg->Handle(), DM_SETTEXT, 0, (LONG_PTR)L"ddddddddddddddddddddd");
		}
	}
} ShowWaitState;
struct		InfoDlgParams {
};

struct		WinSvcAction {
	SvcMgr	*svcmgr;
	UINT	ControlState;
	int		Key;

	WinSvcAction(SvcMgr *s, UINT cs, int k): svcmgr(s), ControlState(cs), Key(k) {}
};
///========================================================================================= dialogs
bool				DlgConnection(SvcMgr* svcmgr) {
	TCHAR HostName[64] = {0};

	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 11, 0, 0, 0, 0, GetMsg(MSelectComputer)},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(MComputer)},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"svcmgr.Host", DIF_HISTORY, 1, HostName},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(MEmptyForLocal)},
		{DI_TEXT, 0, 5, 0, 0, 0, 0, DIF_SEPARATOR, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, GetMsg(MUsername)},
		{DI_TEXT, 25, 6, 0, 0, 0, 0, 0, 0, GetMsg(MPaswd)},
		{DI_EDIT, 5, 7, 22, 0, 0, (DWORD_PTR)L"svcmgr.Login", DIF_HISTORY, 0, L""},
		{DI_PSWEDIT, 26, 7, 42, 0, 0, 0, 0, 0, L""},
		{DI_TEXT, 5, 8, 0, 0, 0, 0, 0, 0, GetMsg(MEmptyForCurrent)},
		{DI_TEXT, 5, 9, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON, 0, 10, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(MbuttonOk)},
		{DI_BUTTON, 0, 10, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MbuttonCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 48, 13, L"Contents", FarItems, size, 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == 11) {
			try {
				svcmgr->Connect(GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 7), GetDataPtr(hDlg, 8));
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}
bool				DlgCreateService(SvcMgr* svcmgr) {
	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 10, 0, 0, 0, 0, L"Create service"},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, L"Name:"},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"svcmgr.Name", DIF_HISTORY, 1, L""},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, L"Display name:"},
		{DI_EDIT, 5, 5, 42, 0, 0, (DWORD_PTR)L"svcmgr.DName", DIF_HISTORY, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, L"Path:"},
		{DI_EDIT, 5, 7, 42, 0, 0, (DWORD_PTR)L"svcmgr.Path", DIF_HISTORY, 0, L""},
		{DI_TEXT, 5, 8, 0,  0,  0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON, 0, 9, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(MbuttonOk)},
		{DI_BUTTON, 0, 9, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MbuttonCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 48, 12, L"DldCreateService",
								 FarItems, size, 0, 0, NULL, 0);
	if (hDlg != INVALID_HANDLE_VALUE) {
		int		ret = psi.DialogRun(hDlg);
		if (ret == (int)(size - 2)) {
			CStrW	name(GetDataPtr(hDlg, 2));
			CStrW	dname(GetDataPtr(hDlg, 4));
			CStrW	path(GetDataPtr(hDlg, 6));
			try {
				WinScm	scm(SC_MANAGER_CREATE_SERVICE, svcmgr->conn());
				scm.Create(name, path, dname);
			} catch (WinError e) {
				farebox(e.code());
			}
		}

		/*
				HRESULT	err = NO_ERROR;
				while (true) {
					if (ret == -1 || ret == 11)
						break;
					CStrW	cmd(GetDataPtr(hDlg, 8));
					if (GetCheck(hDlg, 5)) {
						err = ExecRestricted(cmd);
					} else {
						err = ExecAsUser(cmd, user, pass);
					}
					if (err == NO_ERROR) {
						break;
					} else {
						PCWSTR Msg[] = {GetMsg(MError), cmd.c_str(), L"", GetMsg(buttonOk), };
						::SetLastError(err);
						psi.Message(psi.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE,
									L"Contents", Msg, sizeofa(Msg), 1);
					}
				}
		*/
		psi.DialogFree(hDlg);
	}
	return	true;
}
LONG_PTR WINAPI		DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2) {
	static WinSvcAction	*action;
	static	PCWSTR		txt = L"Action in process";
	switch (Msg) {
		case DN_INITDIALOG: {
			action = (WinSvcAction*)Param2;
			if ((action->ControlState == 0 && action->Key == VK_F5)) {
				txt = L"Starting service...";
			}
			if ((action->ControlState == 0 && action->Key == VK_F7)) {
				txt = L"Pausing service...";
			}
			if ((action->ControlState == 0 && action->Key == VK_F8)) {
				txt = L"Stopping service...";
			}
			if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
				txt = L"Restarting service...";
			}
			psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, 1, (LONG_PTR)txt);
//			int SelectFolders = FPS_SELECTFOLDERS & psi.AdvControl(psi.ModuleNumber, ACTL_GETPANELSETTINGS, 0);
//			psi.SendDlgMessage(hDlg, DM_SETCHECK, 2, SelectFolders ? BSTATE_CHECKED : BSTATE_UNCHECKED);
//			CStrW	aa;
//			aa.AddNum(action->key);
//			mbox(aa, L"dlg init");
//			psi.SendDlgMessage(hDlg, DM_USER, -1, 0);
			return	true;
		}
		case DN_KEY:
			if (Param2 == KEY_ESC) {
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
				return TRUE;
			}
			return FALSE;
		case DN_ENTERIDLE: {
			try {
				WinSvc	svc(action->svcmgr->name().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS, action->svcmgr->conn());
				if ((action->ControlState == 0 && action->Key == VK_F5)) {
					if (action->svcmgr->state() == SERVICE_PAUSED) {
						svc.Continue();
						svc.WaitForState(SERVICE_RUNNING, 30000);
					} else {
						svc.Start();
						svc.WaitForState(SERVICE_RUNNING, 30000);
//					svc.WaitForState(SERVICE_RUNNING, 30000, ShowWaitState, &dlg);
					}
//		CStrW	aa;
//		aa.AddNum(::GetLastError());
//		mbox(aa);
				}
				if ((action->ControlState == 0 && action->Key == VK_F7)) {
					svc.Pause();
					svc.WaitForState(SERVICE_PAUSED, 30000);
				}
				if ((action->ControlState == 0 && action->Key == VK_F8)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, 30000);
					} catch (WinError e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
				}
				if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, 30000);
					} catch (WinError e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
					svc.Start();
					svc.WaitForState(SERVICE_RUNNING, 30000);
				}
				if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
					svc.Continue();
					svc.WaitForState(SERVICE_RUNNING, 30000);
//					svc.WaitForState(SERVICE_RUNNING, 30000, ShowWaitState, &dlg);
				}
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
			} catch (WinError e) {
				farebox(e.code());
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
				return	false;
			}
		}
	}
	return	psi.DefDlgProc(hDlg, Msg, Param1, Param2);
}

///========================================================================================== export
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
	delete(SvcMgr*)hPlugin;
}
int		WINAPI	EXP_NAME(Compare)(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
	WinServices* sm = (WinServices*)hPlugin;
	if (((Mode == SM_NAME) || (Mode == SM_EXT))) {
		return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
	}
	if (Mode == SM_UNSORTED) {
		return	WinStr::Cmpi(Item1->FindData.lpwszAlternateFileName, Item2->FindData.lpwszAlternateFileName);
	}
	s_ServiceSmallInfo	&info1 = sm->operator[](Item1->FindData.lpwszAlternateFileName);
	s_ServiceSmallInfo	&info2 = sm->operator[](Item2->FindData.lpwszAlternateFileName);
	if (Mode == SM_MTIME) {
		if (info1.ssp.dwCurrentState == info2.ssp.dwCurrentState)
			return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
		if (info1.ssp.dwCurrentState < info2.ssp.dwCurrentState)
			return	1;
		return	-1;
	}
	if (Mode == SM_SIZE) {
		if (info1.StartType == info2.StartType)
			return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
		if (info1.StartType < info2.StartType)
			return	-1;
		return	1;
	}
	return	-2;
}
int		WINAPI	EXP_NAME(Configure)(int) {
	Options.Write();
	return	true;
}
int		WINAPI	EXP_NAME(DeleteFiles)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) {
	return	0;
}
void	WINAPI	EXP_NAME(ExitFAR)() {
}
void	WINAPI	EXP_NAME(FreeFindData)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		//	for (int j = 0; j < PanelItem[i].CustomColumnNumber; ++j)
		//		free (PanelItem[i].CustomColumnData[j]);
		WinMem::Free(PanelItem[i].CustomColumnData);
	}

	WinMem::Free(PanelItem);
}
int		WINAPI	EXP_NAME(GetFiles)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, const TCHAR **DestPath, int OpMode) {
	return	true;
}
int		WINAPI	EXP_NAME(GetFindData)(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	SvcMgr	*svcmgr = (SvcMgr*)hPlugin;

	*pPanelItem = NULL;
	*pItemsNumber = 0;

	if (1/*sm->GetType()*/ == 0) {
		*pPanelItem = (PluginPanelItem *)malloc(sizeof(PluginPanelItem) * 2);
		memset(*pPanelItem, 0, sizeof(PluginPanelItem)*2);
		*pItemsNumber = 2;

//		(*pPanelItem)[0].FindData.lpwszFileName = DriversDirName;
//		(*pPanelItem)[0].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
//		(*pPanelItem)[1].FindData.lpwszFileName = ServicesDirName;
//		(*pPanelItem)[1].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		svcmgr->sm()->Cache();

		WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * svcmgr->sm()->Size());
		*pItemsNumber = svcmgr->sm()->Size();

		int		i = 0;
		ForEachIn(svcmgr->sm()) {
			s_ServiceSmallInfo	&info = svcmgr->sm()->Value();
			(*pPanelItem)[i].FindData.lpwszAlternateFileName = svcmgr->name().c_str();
			(*pPanelItem)[i].FindData.lpwszFileName = info.dname.c_str();
			(*pPanelItem)[i].Description = info.descr.c_str();

			if (info.StartType == SERVICE_DISABLED)
				(*pPanelItem)[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_HIDDEN;
			PCWSTR	*CustomColumnData;// = (const wchar_t**)calloc(2, sizeof(const wchar_t*));
			WinMem::Alloc(CustomColumnData, 2 * sizeof(PCWSTR));
			if (CustomColumnData) {
				switch (info.ssp.dwCurrentState) {
					case SERVICE_CONTINUE_PENDING:
						CustomColumnData[0] = L"Continue";
						break;
					case SERVICE_PAUSE_PENDING:
						CustomColumnData[0] = L"Pausing";
						break;
					case SERVICE_PAUSED:
						CustomColumnData[0] = L"Paused";
						break;
					case SERVICE_RUNNING:
						CustomColumnData[0] = L"Started";
						break;
					case SERVICE_START_PENDING:
						CustomColumnData[0] = L"Starting";
						break;
					case SERVICE_STOP_PENDING:
						CustomColumnData[0] = L"Stopping";
						break;
					case SERVICE_STOPPED:
						CustomColumnData[0] = L"";
						break;
				}
				switch (info.StartType) {
					case SERVICE_AUTO_START:
						CustomColumnData[1] = L"Auto";
						break;
					case SERVICE_BOOT_START:
						CustomColumnData[1] = L"Boot";
						break;
					case SERVICE_DEMAND_START:
						CustomColumnData[1] = L"Manual";
						break;
					case SERVICE_DISABLED:
						CustomColumnData[1] = L"Disabled";
						break;
					case SERVICE_SYSTEM_START:
						CustomColumnData[1] = L"System";
						break;
				}
				(*pPanelItem)[i].CustomColumnData = CustomColumnData;
				(*pPanelItem)[i].CustomColumnNumber = 2;
			}
			(*pPanelItem)[i].UserData = i;
			++i;
		}
	}

	return	true;
}
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, OpenPluginInfo *Info) {
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEHIGHLIGHTING | OPIF_USEFILTER;
	Info->HostFile	= NULL;
	Info->CurDir = NULL;
	Info->Format	= L"svcmgr";
	static WCHAR PanelTitle[64];
	SvcMgr	*svcmgr = (SvcMgr*)hPlugin;
	CStrW	tmp(L"svcmgr");
	if (!svcmgr->host().empty()) {
		tmp += L": ";
		tmp += svcmgr->host();
	}
	WinStr::Copy(PanelTitle, tmp.c_str(), sizeofa(PanelTitle));
	Info->PanelTitle = PanelTitle;

	/*
		static PanelMode PanelModesArray[10];
		WinMem::Zero(PanelModesArray, sizeof(PanelModesArray));
		Info->PanelModesArray = PanelModesArray;
		Info->PanelModesNumber = sizeofa(PanelModesArray);
		if (1 == 0) {
			PanelModesArray[0].ColumnTypes = L"N";
			PanelModesArray[0].ColumnWidths = L"0";
			PanelModesArray[0].StatusColumnTypes = L"N";
			PanelModesArray[0].StatusColumnWidths = L"0";
		} else {
			static const wchar_t* ColumnTitles[] = {NULL, L"Status", L"Startup"};
			PanelModesArray[0].ColumnTypes = L"N,C0,C1";
			PanelModesArray[0].ColumnWidths = L"0,8,8";
			PanelModesArray[0].StatusColumnTypes = L"N";
			PanelModesArray[0].StatusColumnWidths = L"0";
			PanelModesArray[0].ColumnTitles = ColumnTitles;
		}
	*/
	Info->StartPanelMode = L'0';

	static const wchar_t* ColumnTitles[] = {NULL, L"Status", L"Startup"};
	static PanelMode CustomPanelModes[1] = {
		{L"N,C0,C1", L"0,8,8", ColumnTitles, FALSE, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}}
	};

	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = 1;

	static KeyBarTitles keybartitles = {
		{ 0, 0, 0, 0, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ 0, },
		{ 0, 0, 0, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", },
		{(PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", }
	};

	keybartitles.Titles[4] = (PWSTR)L"Start";
	keybartitles.Titles[5] = (PWSTR)L"Comp";
	keybartitles.Titles[6] = (PWSTR)L"Pause";
	keybartitles.Titles[7] = (PWSTR)L"Stop";
	keybartitles.AltTitles[3] = (PWSTR)L""; //HWProf
	keybartitles.AltTitles[4] = (PWSTR)L"Restrt";
	keybartitles.ShiftTitles[3] = (PWSTR)L"Create";
	keybartitles.ShiftTitles[4] = (PWSTR)L"StartP";
	keybartitles.ShiftTitles[5] = (PWSTR)L"Local";
	keybartitles.ShiftTitles[6] = (PWSTR)L"Contin";
	keybartitles.ShiftTitles[7] = (PWSTR)L"Delete";
	Info->KeyBar = &keybartitles;
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

	static const TCHAR	*DiskStrings[1];
	static int			DiskNumbers[1] = {6};
	DiskStrings[0] = GetMsg(DiskTitle);
//	DiskNumbers[0] = 6;
	pi->DiskMenuStrings = DiskStrings;
	pi->DiskMenuNumbers = DiskNumbers;
	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);

	static const TCHAR	*MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

	pi->PluginConfigStrings = MenuStrings;
	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->CommandPrefix = prefix;
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const WCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	Options.Read();
	CStrW	cline;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo	pi;
		if (psi.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
			CStrW	buf(MAX_PATH_LENGTH + MAX_PATH + 1);
			fsf.GetCurrentDirectory(buf.capacity(), buf.buffer());
			if (!buf.empty())
				fsf.AddEndSlash(buf.buffer());

			PluginPanelItem PPI;
			psi.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)&PPI);
			buf += PPI.FindData.lpwszFileName;
			cline = buf;
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		cline = (PCWSTR)Item;
		fsf.Trim(cline.buffer());
		fsf.Unquote(cline.buffer());
	}

	SvcMgr*	hPlugin = new SvcMgr;
	return	(HANDLE)hPlugin;
}
int		WINAPI	EXP_NAME(ProcessKey)(HANDLE hPlugin, int Key, unsigned int ControlState) {
	SvcMgr	*svcmgr = (SvcMgr*)hPlugin;

	/*
		if ((ControlState == PKF_CONTROL) && (Key == L'R')) {
			mbox(L"Reread");
			svcmgr->sm()->Cache();
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
			return	true;
		}
	*/
	if (ControlState == 0 && Key == VK_F3) {
		FarPnl pInfo(hPlugin, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && svcmgr->sm()->Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
			CStrW	out(svcmgr->sm()->Info());
			CStrW	tempfile(TempFile(L"svcmgr"));
			HANDLE	hdata = ::CreateFile(tempfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hdata != INVALID_HANDLE_VALUE) {
				DWORD	dWritten;
				WriteFile(hdata, (PWSTR)out.c_str(), out.data_length(), &dWritten, NULL);
				::CloseHandle(hdata);
				psi.Viewer(tempfile, NULL, 0, 0, -1, -1,
						   VF_DELETEONLYFILEONCLOSE | VF_ENABLE_F6 | VF_DISABLEHISTORY |
						   VF_NONMODAL | VF_IMMEDIATERETURN, CP_AUTODETECT);
			}
		}
		return	true;
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection(svcmgr)) {
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F4) {
		if (DlgCreateService(svcmgr)) {
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F6) {
		try {
			svcmgr->Connect(NULL);
		} catch (WinError e) {
			farebox(e.code());
		}
		psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
		psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		return	true;
	}

	UINT	tcs = 0;

	if (ControlState == PKF_SHIFT && Key == VK_F8) {
		if (farquestion(L"Are you sure?", L"Delete service")) {
			tcs = ControlState;
			ControlState = 0;
		} else {
			return	false;
		}
	}

	if ((ControlState == 0 && (Key == VK_F5 || Key == VK_F7 || Key == VK_F8)) ||
			(ControlState == PKF_ALT && Key == VK_F5) ||
			(ControlState == PKF_SHIFT && Key == VK_F7) ||
			(ControlState == PKF_SHIFT && Key == VK_F8)
	   ) {
		FarPnl pInfo(hPlugin, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber()) {
			if (svcmgr->sm()->Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
				try {
					//PCWSTR	name = sm->Value().dname.c_str();
					WinSvcAction	action(svcmgr, ControlState, Key);

					InitDialogItem	InitItems[] = {
						{DI_DOUBLEBOX, 3, 1, 41, 3, 0, 0, 0, 0, L"Wait a moment..."},
						{DI_TEXT,      5, 2, 0,  0, 0, 0, 0, 0, L"Action in process"},
					};
					FarDialogItem	Items[sizeofa(InitItems)];
					InitDialogItems(InitItems, Items, sizeofa(InitItems));
					FarDlg	dlg;
					dlg.Execute(psi.ModuleNumber, -1, -1, 45, 5, NULL, Items, sizeofa(Items), 0, 0, DlgProc, (LONG_PTR)&action);
				} catch (WinError e) {
					farebox(e.code());
					return	false;
				}
				try {
					if (tcs == PKF_SHIFT && Key == VK_F8) {
						WinSvc	svc(svcmgr->name(), GENERIC_EXECUTE | DELETE, svcmgr->conn());
						svc.Del();
					}
				} catch (WinError e) {
					farebox(e.code());
					return	false;
				}
			}
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	return	false;
}
int		WINAPI	EXP_NAME(SetDirectory)(HANDLE hPlugin, const WCHAR *Dir, int OpMode) {
	return	true;
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}

///========================================================================================= WinMain
/*
extern		"C" {
	BOOL		WINAPI	DllMainCRTStartup(HANDLE, DWORD, PVOID) {
		return	true;
	}
}
*/
