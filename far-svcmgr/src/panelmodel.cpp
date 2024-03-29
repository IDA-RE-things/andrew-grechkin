﻿/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR3 plugin

	© 2013 Andrew Grechkin

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

#include <panelmodel.hpp>
#include <globalinfo.hpp>

#include <libbase/logger.hpp>


PanelModel::PanelModel()
{
	set_wait_state(get_global_info()->waitForState);
	set_wait_timeout(get_global_info()->waitTimeout);
}

bool PanelModel::is_drivers() const {
	return get_type() == Ext::Service::EnumerateType_t::DRIVERS;
}
