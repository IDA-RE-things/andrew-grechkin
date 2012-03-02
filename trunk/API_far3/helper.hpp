﻿/**
 © 2012 Andrew Grechkin
 Source code: <http://code.google.com/p/andrew-grechkin>

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

#ifndef __FAR_HELPER_HPP__
#define __FAR_HELPER_HPP__

#include <libwin_def/std.h>
#include <libwin_def/str.h>
#include "plugin.hpp"

namespace Far {
	enum {
		MenuTitle,
		DiskTitle,
		DlgTitle,

		txtBtnOk,
		txtBtnCancel,
	};

	///==================================================================================== helper_t
	struct helper_t {
		static helper_t & inst() {
			static helper_t ret;
			return ret;
		}

		helper_t & init(const GUID & guid, const PluginStartupInfo * psi) {
			m_guid = guid;
			m_psi = *psi;
			m_fsf = *psi->FSF;
			m_psi.FSF = &m_fsf;
			return *this;
		}

		const GUID * guid() const {
			return &m_guid;
		}

		const PluginStartupInfo & psi() const {
			return m_psi;
		}

		const FarStandardFunctions & fsf() const {
			return m_fsf;
		}

	private:
		helper_t() {
		}

		GUID m_guid;
		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

	inline const GUID * get_plugin_guid() {
		return helper_t::inst().guid();
	}

	inline const PluginStartupInfo & psi() {
		return helper_t::inst().psi();
	}

	inline const FarStandardFunctions & fsf() {
		return helper_t::inst().fsf();
	}

	///=============================================================================================
	struct InitDialogItemF {
		FARDIALOGITEMTYPES Type;
		int X1, Y1, X2, Y2;
		DWORD Flags;
		PCWSTR Data;
	};

	///=============================================================================================
	inline PCWSTR get_msg(int MsgId) {
		return psi().GetMsg(get_plugin_guid(), MsgId);
	}

	inline void InitDialogItemsF(const InitDialogItemF *Init, FarDialogItem *Item, int ItemsNumber) {
		for (int i = 0; i < ItemsNumber; ++i) {
			WinMem::Zero(Item[i]);
			Item[i].Type = Init[i].Type;
			Item[i].X1 = Init[i].X1;
			Item[i].Y1 = Init[i].Y1;
			Item[i].X2 = Init[i].X2;
			Item[i].Y2 = Init[i].Y2;
			Item[i].Flags = Init[i].Flags;
			if ((DWORD_PTR)Init[i].Data < 2048) {
				Item[i].Data = get_msg((size_t)Init[i].Data);
			} else {
				Item[i].Data = Init[i].Data;
			}
		}
	}

	inline void ibox(PCWSTR text, PCWSTR tit = L"Info") {
		PCWSTR Msg[] = {tit, text, };
		psi().Message(get_plugin_guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 0);
	}

	inline void mbox(PCWSTR text, PCWSTR tit = L"Message") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(get_plugin_guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void ebox(PCWSTR text, PCWSTR tit = L"Error") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void ebox(PCWSTR msgs[], size_t size, PCWSTR help = nullptr) {
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, help, msgs, size, 1);
	}

	inline void ebox_code(DWORD err) {
		ustring title(L"Error: ");
		title += as_str(err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), L"OK", };
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void ebox_code(DWORD err, PCWSTR line) {
		ustring title(L"Error: ");
		title += as_str(err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), line, L"OK", };
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline bool question(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", L"Cancel", };
		return psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 2) == 0;
	}

	///====================================================================================== Dialog
	struct Dialog {
		~Dialog() {
			Free();
		}

		Dialog() :
			m_hndl(INVALID_HANDLE_VALUE) {
		}

		bool Init(const GUID & dguid, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item, int ItemsNumber, DWORD Reserved = 0, DWORD Flags = 0, FARWINDOWPROC DlgProc = nullptr,
		          PVOID Param = nullptr) {
			Free();
			m_hndl = psi().DialogInit(get_plugin_guid(), &dguid, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
			return (m_hndl && m_hndl != INVALID_HANDLE_VALUE);
		}

		int Run() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE)
				return psi().DialogRun(m_hndl);
			return -1;
		}

		HANDLE Handle() const {
			return m_hndl;
		}

		operator HANDLE() const {
			return m_hndl;
		}

		int Check(int index) const {
			return (int)psi().SendDlgMessage(m_hndl, DM_GETCHECK, index, 0);
		}

		bool IsChanged(int index) const {
			return !(bool)psi().SendDlgMessage(m_hndl, DM_EDITUNCHANGEDFLAG, index, nullptr);
		}

		PCWSTR Str(int index) const {
			return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, index, nullptr);
		}

		DWORD Flags(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Flags : 0;
		}

		DWORD Type(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Type : 0;
		}

		ssize_t get_list_position(int index) const {
			return psi().SendDlgMessage(m_hndl, DM_LISTGETCURPOS, index, 0);
		}

	private:
		void Free() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
				psi().DialogFree(m_hndl);
				m_hndl = INVALID_HANDLE_VALUE;
			}
		}

		HANDLE m_hndl;
	};

	inline PCWSTR get_data_ptr(HANDLE m_hndl, size_t in) {
		return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, in, 0);
	}

	inline bool get_checkbox(HANDLE m_hndl, size_t in) {
		return (bool)psi().SendDlgMessage(m_hndl, DM_GETCHECK, in, 0);
	}

	///======================================================================================= Panel
	struct IPanel {
		virtual ~IPanel() {}

		virtual void destroy() = 0;
		virtual void GetOpenPanelInfo(OpenPanelInfo * Info) = 0;

		virtual int GetFindData(GetFindDataInfo * Info) = 0;
		virtual void FreeFindData(const FreeFindDataInfo * Info) = 0;

		virtual int Compare(const CompareInfo * Info) = 0;
		virtual int SetDirectory(const SetDirectoryInfo * Info) = 0;

		virtual int ProcessEvent(const ProcessPanelEventInfo * Info) = 0;
		virtual int ProcessKey(INPUT_RECORD rec) = 0;

		INT_PTR update(bool keep_selection = true) const {
			return psi().PanelControl((HANDLE)this, FCTL_UPDATEPANEL, keep_selection, nullptr);
		}

		INT_PTR redraw() const {
			return psi().PanelControl((HANDLE)this, FCTL_REDRAWPANEL, 0, nullptr);
		}
	};

	struct Panel {
		~Panel() {
			WinMem::Free(m_dir);
			WinMem::Free(m_ppi);
		}

		Panel(const HANDLE aPlugin, FILE_CONTROL_COMMANDS cmd = FCTL_GETPANELINFO):
			m_hndl(aPlugin),
			m_ppi(nullptr),
			m_dir(nullptr) {
			m_pi.StructSize = sizeof(m_pi);
			m_Result = psi().PanelControl(aPlugin, cmd, sizeof(m_pi), &m_pi);
		}

		bool is_ok() const {
			return m_Result;
		}

		int PanelType() const {
			return m_pi.PanelType;
		}

		size_t size()const {
			return m_pi.ItemsNumber;
		}

		size_t selected() const {
			return m_pi.SelectedItemsNumber;
		}

		size_t current() const {
			return m_pi.CurrentItem;
		}

		int view_mode() const {
			return m_pi.ViewMode;
		}

		PANELINFOFLAGS flags() const {
			return m_pi.Flags;
		}

		PCWSTR get_current_directory() const {
			size_t size = psi().PanelControl(m_hndl, FCTL_GETPANELDIRECTORY, 0, nullptr);
			if (WinMem::Realloc(m_dir, size)) {
				m_dir->StructSize = size;
				if (psi().PanelControl(m_hndl, FCTL_GETPANELDIRECTORY, size, m_dir))
					return m_dir->Name;
			}
			return L"";
		}

		const PluginPanelItem * operator [](size_t index) const {
			size_t m_ppiSize = psi().PanelControl(m_hndl, FCTL_GETPANELITEM, index, nullptr);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				FarGetPluginPanelItem gpi = {m_ppiSize, m_ppi};
				psi().PanelControl(m_hndl, FCTL_GETPANELITEM, index, &gpi);
			}
			return m_ppi;
		}

		const PluginPanelItem * get_selected(size_t index) const {
			size_t m_ppiSize = psi().PanelControl(m_hndl, FCTL_GETSELECTEDPANELITEM, index, nullptr);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				FarGetPluginPanelItem gpi = {m_ppiSize, m_ppi};
				psi().PanelControl(m_hndl, FCTL_GETSELECTEDPANELITEM, index, &gpi);
			}
			return m_ppi;
		}

		const PluginPanelItem * get_current() const {
			return operator [](m_pi.CurrentItem);
		}

		void StartSelection() {
			psi().PanelControl(m_hndl, FCTL_BEGINSELECTION, 0, nullptr);
		}

		void Select(size_t index, bool in) {
			psi().PanelControl(m_hndl, FCTL_SETSELECTION, index, (PVOID)in);
		}

		void clear_selection(size_t index) {
			psi().PanelControl(m_hndl, FCTL_CLEARSELECTION, index, nullptr);
		}

		void CommitSelection() {
			psi().PanelControl(m_hndl, FCTL_ENDSELECTION, 0, nullptr);
		}

	private:
		const HANDLE m_hndl;
		PanelInfo m_pi;
		mutable PluginPanelItem * m_ppi;
		mutable FarPanelDirectory * m_dir;

		bool m_Result;
	};

	inline uint64_t get_panel_settings() {
		return psi().AdvControl(get_plugin_guid(), ACTL_GETPANELSETTINGS, 0, nullptr);
	}

	inline uint64_t get_interface_settings() {
		return psi().AdvControl(get_plugin_guid(), ACTL_GETINTERFACESETTINGS, 0, nullptr);
	}

	///========================================================================================== Editor
	namespace Editor {
		inline ustring get_filename() {
			WCHAR Result[psi().EditorControl(-1, ECTL_GETFILENAME, 0, nullptr) + 1];
			psi().EditorControl(-1, ECTL_GETFILENAME, 0, (void*)Result);
			return ustring(Result);
		}

		inline ustring get_string(ssize_t y) {
			EditorGetString egs = {(int)y};
			psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
			return ustring(egs.StringText, egs.StringLength);
		}

		inline INT_PTR set_position(ssize_t y, ssize_t x = -1) {
			EditorSetPosition esp = {(int)y, (int)x, -1, -1, -1, -1};
			return psi().EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
		}

		inline INT_PTR set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol) {
			EditorSetString ess = {(int)y, (int)size, str, eol};
			return psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		inline INT_PTR set_string(ssize_t y, const ustring & str, PCWSTR eol) {
			EditorSetString ess = {(int)y, (int)str.size(), str.c_str(), eol};
			return psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		inline INT_PTR insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol) {
			if (set_position(y) && psi().EditorControl(-1, ECTL_INSERTSTRING, 0, 0)) {
				return set_string(y, str, size, eol);
			}
			return nullptr;
		}

		inline INT_PTR del_string(ssize_t y) {
			set_position(y);
			return psi().EditorControl(-1, ECTL_DELETESTRING, 0, nullptr);
		}

		inline INT_PTR unselect_block() {
			EditorSelect tmp = {BTYPE_NONE};
			return psi().EditorControl(-1, ECTL_SELECT, 0, &tmp);
		}

		inline INT_PTR start_undo() {
			EditorUndoRedo eur = {EUR_BEGIN, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		inline INT_PTR stop_undo() {
			EditorUndoRedo eur = {EUR_END, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		inline INT_PTR redraw() {
			return psi().EditorControl(-1, ECTL_REDRAW, 0, nullptr);
		}

		inline int get_info(EditorInfo & info) {
			return psi().EditorControl(-1, ECTL_GETINFO, 0, &info);
		}

		inline int get_string(size_t index, EditorGetString & egs) {
			egs.StringNumber = index;
			return psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
		}
	}

	///======================================================================================= Settings_t
	struct Settings_t {
		~Settings_t() {
			psi().SettingsControl(m_hndl, SCTL_FREE, 0, 0);
		}

		Settings_t(const GUID & guid) :
			m_hndl(INVALID_HANDLE_VALUE) {
			FarSettingsCreate settings = {sizeof(FarSettingsCreate), guid, m_hndl};
			if (psi().SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings))
				m_hndl = settings.Handle;
		}

		int create_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsValue value = {root, name};
			return (int)psi().SettingsControl(m_hndl, SCTL_CREATESUBKEY, 0, &value);
		}

		int open_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			FarSettingsValue value = {root, name};
			return (int)psi().SettingsControl(m_hndl, SCTL_OPENSUBKEY, 0, &value);
		}

		bool del(FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsValue value = {root, nullptr};
			return psi().SettingsControl(m_hndl, SCTL_DELETE, 0, &value);
		}

		bool del(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsValue value = {root, name};
			return psi().SettingsControl(m_hndl, SCTL_DELETE, 0, &value);
		}

		size_t get(PCWSTR name, PVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			FarSettingsItem item = {root, name, FST_DATA};
			if (psi().SettingsControl(m_hndl, SCTL_GET, 0, &item)) {
				if (value) {
					size = (item.Data.Size > size) ? size : item.Data.Size;
					memcpy(value, item.Data.Data, size);
					return size;
				} else {
					return item.Data.Size;
				}
			}
			return 0;
		}

		PCWSTR get(PCWSTR name, PCWSTR def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			FarSettingsItem item = {root, name, FST_STRING};
			return psi().SettingsControl(m_hndl, SCTL_GET, 0, &item) ? item.String : def;
		}

		uint64_t get(PCWSTR name, uint64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			FarSettingsItem item = {root, name, FST_QWORD};
			return psi().SettingsControl(m_hndl, SCTL_GET, 0, &item) ? item.Number : def;
		}

		int64_t get(PCWSTR name, int64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			return (int64_t)get(name, (uint64_t)def, root);
		}

		uint32_t get(PCWSTR name, uint32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			return (uint32_t)get(name, (uint64_t)def, root);
		}

		int32_t get(PCWSTR name, int32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			return (int32_t)get(name, (uint64_t)def, root);
		}

		bool get(PCWSTR name, bool def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const {
			return get(name, def ? 1ull : 0ull, root);
		}

		bool set(PCWSTR name, PCVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsItem item = {root, name, FST_DATA};
			item.Data.Size = size;
			item.Data.Data = value;
			return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
		}

		bool set(PCWSTR name, PCWSTR value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsItem item = {root, name, FST_STRING};
			item.String = value;
			return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
		}

		bool set(PCWSTR name, uint64_t value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) {
			FarSettingsItem item = {root, name, FST_QWORD};
			item.Number = value;
			return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
		}

	private:
		HANDLE m_hndl;
	};

}

#endif