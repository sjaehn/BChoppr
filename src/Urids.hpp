/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef URIDS_HPP_
#define URIDS_HPP_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include "definitions.hpp"

struct BChopprURIs
{
	LV2_URID atom_Float;
	LV2_URID atom_Int;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID atom_Vector;
	LV2_URID time_Position;
	LV2_URID time_barBeat;
	LV2_URID time_beatsPerMinute;
	LV2_URID time_beatsPerBar;
	LV2_URID time_beatUnit;
	LV2_URID time_speed;
	LV2_URID ui_on;
	LV2_URID ui_off;
	LV2_URID notify_event;
	LV2_URID notify_key;
	LV2_URID notify_messageEvent;
	LV2_URID notify_message;
	LV2_URID notify_controllerEvent;
	LV2_URID notify_controllerNr;
	LV2_URID notify_controllerValue;
	LV2_URID notify_controllers;
	LV2_URID notify_sharedDataLinkEvent;
	LV2_URID notify_sharedDataNr;

};

inline void getURIs (LV2_URID_Map* m, BChopprURIs* uris)
{
	uris->atom_Float = m->map(m->handle, LV2_ATOM__Float);
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->atom_Vector = m->map(m->handle, LV2_ATOM__Vector);
	uris->time_Position = m->map(m->handle, LV2_TIME__Position);
	uris->time_barBeat = m->map(m->handle, LV2_TIME__barBeat);
	uris->time_beatsPerMinute = m->map(m->handle, LV2_TIME__beatsPerMinute);
	uris->time_beatUnit = m->map(m->handle, LV2_TIME__beatUnit);
	uris->time_beatsPerBar = m->map(m->handle, LV2_TIME__beatsPerBar);
	uris->time_speed = m->map(m->handle, LV2_TIME__speed);
	uris->ui_on = m->map(m->handle, BCHOPPR_URI "#UIon");
	uris->ui_off = m->map(m->handle, BCHOPPR_URI "#UIoff");
	uris->notify_event = m->map(m->handle, BCHOPPR_URI "#NOTIFYev");
	uris->notify_key = m->map(m->handle, BCHOPPR_URI "#NOTIFYkey");
	uris->notify_messageEvent = m->map(m->handle, BCHOPPR_URI "#NOTIFYmessageEvent");
	uris->notify_message = m->map(m->handle, BCHOPPR_URI "#NOTIFYmessage");
	uris->notify_controllerEvent = m->map(m->handle, BCHOPPR_URI "#BSchafflControllerEvent");
	uris->notify_controllerNr = m->map(m->handle, BCHOPPR_URI "#BSchafflControllerNr");
	uris->notify_controllerValue = m->map(m->handle, BCHOPPR_URI "#BSchafflControllerValue");
	uris->notify_controllers = m->map(m->handle, BCHOPPR_URI "#BSchafflControllers");
	uris->notify_sharedDataLinkEvent = m->map(m->handle, BCHOPPR_URI "#BSchafflSharedDataLinkEvent");
	uris->notify_sharedDataNr = m->map(m->handle, BCHOPPR_URI "#BSchafflSharedDataNr");
}

#endif /* URIDS_HPP_ */
