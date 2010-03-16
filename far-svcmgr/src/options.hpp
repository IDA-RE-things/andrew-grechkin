﻿/**
	svcmgr: Manage services
	Allow to manage windows services

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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "win_def.h"
#include "win_kernel.h"

struct		PluginOptions {
	WinReg	reg;
	int		AddToPluginsMenu;
	int		AddToDisksMenu;
	int		DiskMenuDigit;
	CStrW	Prefix;

	PluginOptions();
	void		Read();
	void		Write();
};

extern PluginOptions		Options;

#endif