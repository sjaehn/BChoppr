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

#ifndef DEFINITIONS_HPP_
#define DEFINITIONS_HPP_

#define NOTIFYBUFFERSIZE 64
#define MONITORBUFFERSIZE 64
#define MAXSTEPS 16
#define MAXINSTANCES 64
#define MINMARKERVALUE 0.000001
#define BCHOPPR_URI "https://www.jahnichen.de/plugins/lv2/BChoppr"
#define BCHOPPR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BChoppr#gui"

struct BChopprNotifications
{
	float position;
	float input1;
	float input2;
	float output1;
	float output2;
};

const BChopprNotifications defaultNotification = {0.0, 0.0, 0.0, 0.0, 0.0};

#endif /* DEFINITIONS_HPP_ */
