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

#ifndef BCHOPPR_H_
#define BCHOPPR_H_

#define MODFL(x) (x - floorf (x))

#include <cmath>
#include <array>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include "definitions.hpp"
#include "Urids.hpp"
#include "Ports.hpp"
#include "Message.hpp"

struct BChopprMonitor_t
{
	int count;
	bool ready;
	double inputMin;
	double inputMax;
	double outputMin;
	double outputMax;
};

const BChopprMonitor_t defaultMonitorData = {0, false, 0.0, 0.0, 0.0, 0.0};

class BChoppr
{
public:
	BChoppr (double samplerate, const LV2_Feature* const* features);
	~BChoppr();

	void connect_port (uint32_t port, void *data);
	void run (uint32_t n_samples);

	LV2_URID_Map* map;

private:
	double rate;
	float bpm;
	float speed;
	float position;
	float beatsPerBar;
	uint32_t beatUnit;
	uint32_t refFrame;
	uint32_t prevStep;
	uint32_t actStep;
	uint32_t nextStep;

	// Audio buffers
	float* audioInput1;
	float* audioInput2;
	float* audioOutput1;
	float* audioOutput2;

	// Controllers
	float* controllers[NrControllers];
	float sequencesperbar;
	float ampSwing;
	float swing;
	float nrSteps;
	bool bypass;
	float drywet;
	int blend;
	float attack;
	float release;
	float stepLevels[MAXSTEPS];
	float stepPositions[MAXSTEPS - 1];
	bool stepAutoPositions[MAXSTEPS - 1];

	// Atom port
	BChopprURIs uris;

	LV2_Atom_Sequence* controlPort1;
	LV2_Atom_Sequence* controlPort2;
	LV2_Atom_Sequence* notifyPort;

	LV2_Atom_Forge forge;
	LV2_Atom_Forge_Frame notify_frame;

	bool record_on;
	int monitorpos;
	Message message;
	std::array<BChopprNotifications, NOTIFYBUFFERSIZE> notifications;
	std::array<BChopprMonitor_t, MONITORBUFFERSIZE> monitor;

	void recalculateAutoPositions ();
	void play(uint32_t start, uint32_t end);
	void notifyGUI();
	void notifyMessageToGui ();

};

#endif /* BCHOPPR_H_ */
