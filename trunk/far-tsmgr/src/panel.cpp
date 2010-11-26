﻿/**
	tsmgr: Terminal sessions manager FAR plugin
	Allow to manage sessions on Terminal server

	© 2010 Andrew Grechkin

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

#include "panel.hpp"
#include "options.hpp"

#include <far/farkeys.hpp>

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;

bool ExecCMD(const AutoUTF &cmd) {
	HRESULT Result = 2;
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	if (::CreateProcessW(nullptr, (PWSTR)cmd.c_str(), nullptr, nullptr, false,
						 CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi)) {
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	} else {
		Result = ::GetLastError();
	}
	return Result == NO_ERROR;
}

///=========================================================================================== Panel
bool	Panel::DlgConnection() {
	enum {
		HEIGHT = 13,
		WIDTH = 48,

		indHost = 2,
		indUser = 7,
		indPass = 8,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtSelectComputer},
		{DI_TEXT,      5,  2,  0,  0,  0, (PCWSTR)txtHost},
		{DI_EDIT,      5,  3,  42, 0,  DIF_HISTORY,     L""},
		{DI_TEXT,      5,  4,  0,  0,  0, (PCWSTR)txtEmptyForLocal},
		{DI_TEXT,      0,  5,  0,  0,  DIF_SEPARATOR,   L""},
		{DI_TEXT,      5,  6,  0,  0,  0, (PCWSTR)txtLogin},
		{DI_TEXT,      26, 6,  0,  0,  0, (PCWSTR)txtPass},
		{DI_EDIT,      5,  7,  22, 0,  DIF_HISTORY,     L""},
		{DI_PSWEDIT,   26, 7,  42, 0,  0,               L""},
		{DI_TEXT,      5,  8,  0,  0,  0, (PCWSTR)txtEmptyForCurrent},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indHost].History = L"Connect.Host";
	FarItems[indUser].History = L"Connect.Login";

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgRemoteConnection", FarItems, size)) {
		while (true) {
			int	ret = hDlg.Run();
			if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
				try {
					shared_ptr<RemoteConnection> tmp(new RemoteConnection(hDlg.Str(indHost),
													 hDlg.Str(indUser),
													 hDlg.Str(indPass)));
					m_ts.Cache(tmp);
					m_conn.swap(tmp);
				} catch (WinError &e) {
					if (e.code() == ERROR_ACCESS_DENIED) {
						PCWSTR msg[] = {
							L"Error",
							L"To operate with remote sessions you must set on remote system:",
							L"[HKLM\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server]",
							L"\"AllowRemoteRPC\"=dword:1",
							L"OK",
						};
						farebox(msg, sizeofa(msg));
					} else {
						farebox(e.code(), e.where().c_str());
					}
					continue;
				}
				return	true;
			} else {
				break;
			}
		}
	}
	return	false;
}

bool	Panel::DlgMessage() {
	enum {
		HEIGHT = 10,
		WIDTH = 56,

		indTitle = 2,
		indMessage = 4,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtSendMessage},
		{DI_TEXT,    5, 2, 0, 0,            0, (PCWSTR)txtTitle},
		{DI_EDIT,    5, 3, 49, 0,           DIF_HISTORY, L""},
		{DI_TEXT,    5, 4, 0, 0,            0, (PCWSTR)txtMessage},
		{DI_EDIT,    5, 5, 49, 0,           DIF_HISTORY, L""},
		{DI_TEXT,    0, HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indTitle].History = L"tsmgr.Title";
	FarItems[indMessage].History = L"tsmgr.Message";

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgRemoteMessage", FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			try {
				WinTSession::Message(id(), conn(), hDlg.Str(2), hDlg.Str(4), 60, false);
			} catch (WinError &e) {
				farebox(e.code(), e.where().c_str());
			}
			return	true;
		}
	}
	return	false;
}

bool	Panel::DlgShutdown() {
	enum {
		HEIGHT = 8,
		WIDTH = 38,

		indReboot = 1,
		indTurnoff = 2,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtShutdown},
		{DI_RADIOBUTTON, 5, 2, 0, 0,  0, (PCWSTR)txtDlgReboot},
		{DI_RADIOBUTTON, 5, 3, 0, 0,  0, (PCWSTR)txtDlgTurnOff},
		{DI_TEXT,    0, HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indReboot].Selected = 1;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgRemoteMessage", FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			try {
				if (hDlg.Check(indReboot)) {
					WinTSession::Reboot(conn());
				} else if (hDlg.Check(indTurnoff)) {
					WinTSession::Turnoff(conn());
				}
			} catch (WinError &e) {
				farebox(e.code(), e.where().c_str());
			}
			return	true;
		}
	}
	return	false;
}

void	Panel::GetOpenPluginInfo(OpenPluginInfo *Info) {
	static WCHAR PanelTitle[64];
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEHIGHLIGHTING | OPIF_USEFILTER;
	Info->HostFile	= nullptr;
	Info->CurDir	= nullptr;
	Info->Format	= Options.Prefix.c_str();
	Info->PanelTitle = PanelTitle;
	if (!host().empty()) {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s: %s", Options.Prefix.c_str(), host().c_str());
	} else {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s", Options.Prefix.c_str());
	}

	static PCWSTR ColumnTitles0[] = {nullptr, GetMsg(txtStatus), GetMsg(txtSession)};
	static PanelMode CustomPanelModes[] = {
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = sizeofa(CustomPanelModes);
	Info->StartPanelMode = L'0';

//	KeyBar
	static KeyBarTitles keybartitles = {
		{ nullptr, nullptr, nullptr, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ nullptr, },
		{ nullptr, },
		{(PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", }
	};
	keybartitles.Titles[3] = (PWSTR)GetMsg(txtOperation);
	keybartitles.Titles[4] = (PWSTR)GetMsg(txtMessg);
	keybartitles.Titles[5] = (PWSTR)GetMsg(txtComp);
	keybartitles.Titles[6] = (PWSTR)GetMsg(txtTerm);
	keybartitles.Titles[7] = (PWSTR)GetMsg(txtDisconn);
	keybartitles.ShiftTitles[3] = (PWSTR)L"";
	keybartitles.ShiftTitles[4] = (PWSTR)L"";
	keybartitles.ShiftTitles[5] = (PWSTR)GetMsg(txtLocal);
	keybartitles.ShiftTitles[6] = (PWSTR)L"";
	keybartitles.ShiftTitles[7] = (PWSTR)GetMsg(txtLogOff);
	Info->KeyBar = &keybartitles;
}

int		Panel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	*pPanelItem = nullptr;
	*pItemsNumber = 0;

	try {
		m_ts.Cache(m_conn);
		WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * m_ts.Size());
		*pItemsNumber = m_ts.Size();

		int		i = 0;
		ForEachInO(m_ts) {
			WinTSInfo	&info = m_ts.Value();
			(*pPanelItem)[i].FindData.lpwszFileName = info.user.c_str();
			(*pPanelItem)[i].FindData.lpwszAlternateFileName = info.user.c_str();
			(*pPanelItem)[i].FindData.nFileSize = info.id;
			(*pPanelItem)[i].Description = info.GetState();
			(*pPanelItem)[i].Owner = info.sess.c_str();

			if (info.state == WTSDisconnected)
				(*pPanelItem)[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_HIDDEN;
			PCWSTR	*CustomColumnData;// = (const wchar_t**)calloc(2, sizeof(const wchar_t*));
			WinMem::Alloc(CustomColumnData, 1 * sizeof(PCWSTR));
			if (CustomColumnData) {
				CustomColumnData[0] = info.client.c_str();
				(*pPanelItem)[i].CustomColumnData = CustomColumnData;
				(*pPanelItem)[i].CustomColumnNumber = 1;
			}
			(*pPanelItem)[i].UserData = i;
			++i;
		}
	} catch (WinError &e) {
		farebox(e.code(), e.where().c_str());
	}

	return	true;
}

void	Panel::FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		WinMem::Free(PanelItem[i].CustomColumnData);
	}
	WinMem::Free(PanelItem);
}

int		Panel::Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
	return	-2;
}

int		Panel::ProcessEvent(int Event, void *Param) {
	return	false;
}

int		Panel::ProcessKey(int Key, unsigned int ControlState) {
	if (ControlState == 0 && Key == VK_F3) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && m_ts.Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			AutoUTF	tmp(TempFile(TempDir()));
			HANDLE	hfile = ::CreateFile(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hfile != INVALID_HANDLE_VALUE) {
				FileWrite(hfile, m_ts.Info());
				::CloseHandle(hfile);
				psi.Viewer(tmp.c_str(), nullptr, 0, 0, -1, -1,
						   VF_DELETEONLYFILEONCLOSE | VF_ENABLE_F6 | VF_DISABLEHISTORY |
						   VF_NONMODAL | VF_IMMEDIATERETURN, CP_AUTODETECT);
			}
		}
		return	true;
	}
	if (ControlState == 0 && Key == VK_F4) {
		DlgShutdown();
	}
	if (ControlState == 0 && Key == VK_F7) {
		AutoUTF cmd = L"mstsc.exe";
		if (!m_conn->host().empty()) {
			cmd += L" /v:";
			cmd += m_conn->host();
		}
		ExecCMD(cmd);
	}
	if ((ControlState == 0 && Key == VK_F8) ||
			(ControlState == PKF_SHIFT && Key == VK_F7) ||
			(ControlState == PKF_SHIFT && Key == VK_F8) ||
			(ControlState == 0 && Key == VK_F5)) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && m_ts.Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			if (ControlState == PKF_SHIFT && Key == VK_F7) {
				farebox(id());
				WinTSession::ConnectLocal(id(), L"user");
			} else 	if (ControlState == 0 && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtDisconnectSession)))
					WinTSession::Disconnect(id(), conn());
			} else if (ControlState == PKF_SHIFT && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtLogoffSession)))
					WinTSession::LogOff(id(), conn());
			} else if (ControlState == 0 && Key == VK_F5) {
				DlgMessage();
			}
			psi.Control(this, FCTL_UPDATEPANEL, 0, nullptr);
			psi.Control(this, FCTL_REDRAWPANEL, 0, nullptr);
			return	true;
		}
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection()) {
			psi.Control(this, FCTL_UPDATEPANEL, 0, nullptr);
			psi.Control(this, FCTL_REDRAWPANEL, 0, nullptr);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F6) {
		try {
			m_conn->Open(nullptr);
		} catch (WinError &e) {
			farebox(e.code(), e.where().c_str());
		}
		psi.Control(this, FCTL_UPDATEPANEL, 0, nullptr);
		psi.Control(this, FCTL_REDRAWPANEL, 0, nullptr);
		return	true;
	}
	if ((ControlState == PKF_SHIFT && Key == VK_F3) ||
			(ControlState == PKF_SHIFT && Key == VK_F4)) {
		return	true;
	}
	return	false;
}

int		Panel::SetDirectory(const WCHAR *Dir, int OpMode) {
	return	true;
}
