﻿/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
	FAR3 plugin

	© 2012 Andrew Grechkin

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

#include "farplugin.hpp"
#include "lang.hpp"
#include "guid.hpp"
#include "fileversion.hpp"

#include <libbase/memory.hpp>

Base::shared_ptr<FarPlugin> plugin;

Base::NamedValues<WORD> Machines[] = {
	{ L"UNKNOWN", IMAGE_FILE_MACHINE_UNKNOWN },
	{ L"I386", IMAGE_FILE_MACHINE_I386 },
	{ L"R4000", IMAGE_FILE_MACHINE_R4000 },
	{ L"WCEMIPSV2", IMAGE_FILE_MACHINE_WCEMIPSV2 },
	{ L"SH3", IMAGE_FILE_MACHINE_SH3 },
	{ L"SH3DSP", IMAGE_FILE_MACHINE_SH3DSP },
	{ L"SH4", IMAGE_FILE_MACHINE_SH4 },
	{ L"SH5", IMAGE_FILE_MACHINE_SH5 },
	{ L"ARM", IMAGE_FILE_MACHINE_ARM },
	{ L"THUMB", IMAGE_FILE_MACHINE_THUMB },
	{ L"AM33", IMAGE_FILE_MACHINE_AM33 },
	{ L"POWERPC", IMAGE_FILE_MACHINE_POWERPC },
	{ L"POWERPCFP", IMAGE_FILE_MACHINE_POWERPCFP },
	{ L"IA64", IMAGE_FILE_MACHINE_IA64 },
	{ L"MIPS16", IMAGE_FILE_MACHINE_MIPS16 },
	{ L"MIPSFPU", IMAGE_FILE_MACHINE_MIPSFPU },
	{ L"MIPSFPU16", IMAGE_FILE_MACHINE_MIPSFPU16 },
	{ L"EBC", IMAGE_FILE_MACHINE_EBC },
	{ L"AMD64", IMAGE_FILE_MACHINE_AMD64 },
	{ L"M32R", IMAGE_FILE_MACHINE_M32R },
};

GUID FarPlugin::get_guid() {
	return PluginGuid;
}

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"fver";
	return ret;
}

PCWSTR FarPlugin::get_name() {
	return L"filever";
}

PCWSTR FarPlugin::get_description() {
	return L"Displays version information from file resource";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	Far::helper_t::inst().init(FarPlugin::get_guid(), psi);
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(*pi);
	pi->Flags = 0;
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle)};
	pi->PluginMenu.Guids = &MenuGuid;
	pi->PluginMenu.Strings = PluginMenuStrings;
	pi->PluginMenu.Count = Base::lengthof(PluginMenuStrings);
	pi->CommandPrefix = get_prefix();
}

HANDLE FarPlugin::open(const OpenInfo * Info) {
	OPENFROM OpenFrom = Info->OpenFrom;
	if (!version_dll::inst().is_valid()) {
		Far::ebox(L"Can't load version.dll");
		return INVALID_HANDLE_VALUE;
	}

	WCHAR buf[Base::MAX_PATH_LEN] = {0};
	if (OpenFrom == OPEN_PLUGINSMENU) {
		Far::Panel pi(PANEL_ACTIVE);
		if (pi.is_ok()) {
			const PluginPanelItem * ppi = pi.get_current();
			PCWSTR fileName = ppi->FileName;
			if (Base::find_str(fileName, Base::PATH_SEPARATOR)) {
				Base::copy_str(buf, fileName, Base::lengthof(buf));
			} else {
				Base::copy_str(buf, pi.get_current_directory(), Base::lengthof(buf));
				if (!Base::is_str_empty(buf)) {
					Far::fsf().AddEndSlash(buf);
				}
				Base::cat_str(buf, fileName, Base::lengthof(buf));
			}
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		Base::copy_str(buf, (PCWSTR)Info->Data, Base::lengthof(buf));
	}
	Far::fsf().Trim(buf);
	Far::fsf().Unquote(buf);

	FileVersion fv(buf);
	if (fv.is_ok()) {
		FVI fvi(fv);
			int i = 0, x = 70, y = 2;
			Far::InitDialogItemF Items[] = {
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileFullVer},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.ver()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileLang},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.lng()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtMachine},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, Base::NamedValues<WORD>::GetName(Machines, lengthof(Machines), fv.machine())},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_BUTTON, 0, y++, 0, 0,       DIF_CENTERGROUP, (PCWSTR)Far::txtBtnOk},
				{DI_DOUBLEBOX, 3, 1, x, y,      0,               (PCWSTR)Far::DlgTitle},
			};

			size_t size = Base::lengthof(Items);
			FarDialogItem FarItems[size];
			InitDialogItemsF(Items, FarItems, size);
			HANDLE hndl = Far::psi().DialogInit(Far::get_plugin_guid(), &DialogGuid, -1, -1, x + 4, y + 2, L"Contents", FarItems, size, 0, 0, nullptr, 0);
			Far::psi().DialogRun(hndl);
			Far::psi().DialogFree(hndl);
	}
	return nullptr;
}