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

#ifndef PORTS_HPP_
#define PORTS_HPP_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include "definitions.hpp"

enum BChopprPortIndex
{
	Control_1	= 0,
	Control_2	= 1,
	Notify		= 2,
	AudioPorts	= 3,
	AudioIn_1	= 3,
	AudioIn_2	= 4,
	AudioOut_1	= 5,
	AudioOut_2	= 6,
	NrAudioPorts	= 4,
	Controllers	= 7,
	Bypass		= 7,
	DryWet		= 8,
	Blend		= 9,
	Attack		= 10,
	Release		= 11,
	SequencesPerBar	= 12,
	AmpSwing	= 13,
	Swing		= 14,
	NrSteps		= 15,
	StepPositions	= 16,
	StepLevels	= 31,
	StepPans	= 47,
	NrControllers	= 56
};

#endif /* PORTS_HPP_ */
