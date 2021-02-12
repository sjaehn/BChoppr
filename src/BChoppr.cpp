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

#include "BChoppr.hpp"

#include <cstdio>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "SharedData.hpp"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))

static SharedData sharedData[4] = {SharedData(), SharedData(), SharedData(), SharedData()};

BChoppr::BChoppr (double samplerate, const LV2_Feature* const* features) :
	map(NULL),
	rate(samplerate), bpm(120.0f), speed(1), position(0),
	beatsPerBar (4), beatUnit (4), refFrame(0),
	prevStep(0), actStep(0), nextStep(1),
	audioInput1(NULL), audioInput2(NULL), audioOutput1(NULL), audioOutput2(NULL),
	sharedDataNr (0),
	controlPtrs {nullptr}, controllers {0.0f},
	stepPositions {0.0},
	controlPort1(NULL), controlPort2(NULL),  notifyPort(NULL),
	record_on(false),
	notify_sharedData (false),
	notify_controllers {false},
	monitorpos(-1), message ()

{
	// Init array members
	notifications.fill (defaultNotification);
	monitor.fill (defaultMonitorData);
	std::fill (stepAutoPositions, stepAutoPositions + MAXSTEPS - 1, true);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (m, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);

	recalculateAutoPositions ();
}

BChoppr::~BChoppr ()
{
	if ((sharedDataNr > 0) && (sharedDataNr <= 4)) sharedData[sharedDataNr - 1].unlink (this);
}

void BChoppr::connect_port(uint32_t port, void *data)
{
	switch (port) {
	case Control_1:
		controlPort1 = (LV2_Atom_Sequence*) data;
		break;
	case Control_2:
		controlPort2 = (LV2_Atom_Sequence*) data;
		break;
	case Notify:
		notifyPort = (LV2_Atom_Sequence*) data;
		break;
	case AudioIn_1:
		audioInput1 = (float*) data;
		break;
	case AudioIn_2:
		audioInput2 = (float*) data;
		break;
	case AudioOut_1:
		audioOutput1 = (float*) data;
		break;
	case AudioOut_2:
		audioOutput2 = (float*) data;
		break;
	default:
		if ((port >= Controllers) && (port < Controllers + NrControllers)) controlPtrs[port - Controllers] = (float*) data;
	}
}

void BChoppr::run (uint32_t n_samples)
{

	// Check ports
	if ((!controlPort1) || (!controlPort2) || (!notifyPort) || (!audioInput1) || (!audioInput2) || (!audioOutput1) || (!audioOutput2)) return;
	for (int i = 0; i < NrControllers; ++i)
	{
		if (!controlPtrs[i]) return;
	}

	// Update controller values
	for (int i = 0; i < NrControllers; ++i)
	{

		float newValue = readController (i);
		if (newValue != controllers[i])
		{
			setController (i, newValue);
			if ((sharedDataNr >= 1) && (sharedDataNr <= 4)) notify_controllers[i] = true;
		}
	}

	// Process GUI data
	const LV2_Atom_Event* ev2 = lv2_atom_sequence_begin(&(controlPort2)->body);
	while (!lv2_atom_sequence_is_end (&controlPort2->body, controlPort2->atom.size, ev2))
	{
		if (lv2_atom_forge_is_object_type(&forge, ev2->body.type))
		{
			const LV2_Atom_Object* obj2 = (const LV2_Atom_Object*)&ev2->body;

			// UI on
			if (obj2->body.otype == uris.ui_on)
			{
				record_on = true;
				notify_sharedData = true;
				std::fill (notify_controllers, notify_controllers + NrControllers, true);
			}

			// UI off
			else if (obj2->body.otype == uris.ui_off) record_on = false;

			// Linked / unlinked to shared data
			else if (obj2->body.otype == uris.notify_sharedDataLinkEvent)
			{
				LV2_Atom *oNr = NULL;

				lv2_atom_object_get
				(
					obj2,
					uris.notify_sharedDataNr, &oNr,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int))
				{
					const int nr = ((LV2_Atom_Int*)oNr)->body;

					if ((nr >= 0) && (nr <= 4) && (nr != sharedDataNr))
					{
						if (sharedDataNr != 0) sharedData[sharedDataNr - 1].unlink (this);

						if ((nr != 0) && sharedData[nr - 1].empty())
						{
							for (int i = 0; i < NrControllers; ++i) sharedData[nr - 1].set (i, controllers[i]);
						}

						if (nr != 0) sharedData[nr - 1].link (this);
						sharedDataNr = nr;
						notify_sharedData = true;

						for (int i = 0; i < NrControllers; ++i)
						{
							float newValue = readController (i);
							if (newValue != controllers[i])
							{
								setController (i, newValue);
								notify_controllers[i] = true;
							}
						}
					}
				}
			}

			// Controller changed
			else if ((obj2->body.otype == uris.notify_controllerEvent) && (sharedDataNr != 0))
			{
				LV2_Atom *oNr = NULL, *oVal = NULL;

				lv2_atom_object_get
				(
					obj2,
					uris.notify_controllerNr, &oNr,
					uris.notify_controllerValue, &oVal,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int) && oVal && (oVal->type == uris.atom_Float))
				{
					const int nr =  ((LV2_Atom_Int*)oNr)->body;

					if ((nr >= 0) && (nr < NrControllers))
					{
						const float val = controllerLimits[nr].validate(((LV2_Atom_Float*)oVal)->body);
						if ((sharedDataNr > 0) && (sharedDataNr <= 4)) sharedData[sharedDataNr - 1].set (nr, val);
						setController (nr, val);
					}
				}
			}
		}
		ev2 = lv2_atom_sequence_next(ev2);
	}

	// Prepare forge buffer and initialize atom sequence
	const uint32_t space = notifyPort->atom.size;
	lv2_atom_forge_set_buffer(&forge, (uint8_t*) notifyPort, space);
	lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);

	const LV2_Atom_Sequence* in = controlPort1;
	uint32_t last_t =0;

	// Process audio data
	for (const LV2_Atom_Event* ev1 = lv2_atom_sequence_begin(&in->body);
		 !lv2_atom_sequence_is_end(&in->body, in->atom.size, ev1);
		 ev1 = lv2_atom_sequence_next(ev1))
	{
		if ((ev1->body.type == uris.atom_Object) || (ev1->body.type == uris.atom_Blank))
		{
			//update bpm, speed, position
			LV2_Atom *oBbeat = NULL, *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL;
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev1->body;
			lv2_atom_object_get (obj, uris.time_barBeat, &oBbeat,
									  uris.time_beatsPerMinute,  &oBpm,
									  uris.time_beatsPerBar,  &oBpb,
									  uris.time_beatUnit,  &oBu,
									  uris.time_speed, &oSpeed,
									  NULL);

			// BPM changed?
			if (oBpm && (oBpm->type == uris.atom_Float))
			{
				float nbpm = ((LV2_Atom_Float*)oBpm)->body;
				if (bpm != nbpm)
				{
					bpm = nbpm;
					if (nbpm < 1.0) message.setMessage (JACK_STOP_MSG);
					else message.deleteMessage (JACK_STOP_MSG);
				}
			}

			// Beats per bar changed?
			if (oBpb && (oBpb->type == uris.atom_Float) && (((LV2_Atom_Float*)oBpb)->body > 0)) beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;

			// BeatUnit changed?
			if (oBu && (oBu->type == uris.atom_Int) && (((LV2_Atom_Int*)oBu)->body > 0)) beatUnit = ((LV2_Atom_Int*)oBu)->body;

			// Speed changed?
			if (oSpeed && (oSpeed->type == uris.atom_Float))
			{
				float nspeed = ((LV2_Atom_Float*)oSpeed)->body;
				if (speed != nspeed)
				{
					speed = nspeed;
					if (nspeed == 0.0) message.setMessage (JACK_STOP_MSG);
					else message.deleteMessage (JACK_STOP_MSG);
				}
			}

			// Beat position changed (during playing) ?
			if (oBbeat && (oBbeat->type == uris.atom_Float))
			{
				// Get position within a sequence (0..1)
				float barsequencepos = ((LV2_Atom_Float*)oBbeat)->body * controllers[SequencesPerBar - Controllers] / beatsPerBar; // Position within a bar (0..sequencesperbar)
				position = MODFL (barsequencepos);			// Position within a sequence
				refFrame = ev1->time.frames;				// Reference frame
			}
		}

		play(last_t, ev1->time.frames);
		last_t = ev1->time.frames;
	}
	if (last_t < n_samples) play(last_t, n_samples);		// Play remaining samples

	// Update position in case of no new barBeat submitted on next call
	double relpos = (n_samples - refFrame) * speed / (rate / (bpm / 60)) * controllers[SequencesPerBar - Controllers] / beatsPerBar;	// Position relative to reference frame
	position = MODFL (position + relpos);
	refFrame = 0;

	// Send collected data to GUI
	if (record_on)
	{
		notifyGUI();
		if (notify_sharedData) notifySharedDataNrToGui();
		for (int i = 0; i < NrControllers; ++i) if (notify_controllers[i]) notifyControllerToGui (i);
		if (message.isScheduled ()) notifyMessageToGui ();
	}

	// Close off sequence
	lv2_atom_forge_pop(&forge, &notify_frame);
}

float BChoppr::readController (const int ctrlNr)
{
	// Sync with control ports
	if ((sharedDataNr == 0) && controlPtrs[ctrlNr]) return controllerLimits[ctrlNr].validate (*controlPtrs[ctrlNr]);

	// Otherwise sync with globally shared data
	if (sharedDataNr <= 4) return controllerLimits[ctrlNr].validate (sharedData[sharedDataNr - 1].get (ctrlNr));

	return controllerLimits[ctrlNr].min;
}

void BChoppr::setController (const int ctrlNr, const float value)
{
	controllers[ctrlNr] = value;

	if (ctrlNr == Swing - Controllers) recalculateAutoPositions();

	else if (ctrlNr == NrSteps - Controllers) recalculateAutoPositions();

	else if ((ctrlNr >= StepPositions - Controllers) && (ctrlNr < StepPositions - Controllers + MAXSTEPS - 1))
	{
		const int step = ctrlNr - (StepPositions - Controllers);
		if (value == 0.0f)
		{
			if (!stepAutoPositions[step])
			{
				stepAutoPositions[step] = true;
				recalculateAutoPositions();
			}
		}

		else if (stepPositions[step] != value)
		{
			stepAutoPositions[step] = false;
			stepPositions[step] = value;
			recalculateAutoPositions();
		}
	}
}

void BChoppr::recalculateAutoPositions ()
{
	int nrMarkers = controllers[NrSteps - Controllers] - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		if (stepAutoPositions[i])
		{
			if ((i == nrMarkers - 1) || (!stepAutoPositions[i + 1]))
			{
				double swing = controllers[Swing - Controllers];
				double s = 2.0 * swing / (swing + 1.0);
				double anc = (start == 0 ? 0 : stepPositions[start - 1]);
				double suc = (i == nrMarkers - 1 ? 1 : stepPositions[i + 1]);
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - s : s) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - s : s);
					anc += f * step;
					stepPositions[j] = anc;
				}
			}
		}
		else start = i + 1;
	}
}

void BChoppr::notifyControllerToGui (const int nr)
{
	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time (&forge, 0);
	lv2_atom_forge_object (&forge, &frame, 0, uris.notify_controllerEvent);
	lv2_atom_forge_key (&forge, uris.notify_controllerNr);
	lv2_atom_forge_int (&forge, nr);
	lv2_atom_forge_key (&forge, uris.notify_controllerValue);
	lv2_atom_forge_float (&forge, controllers[nr]);
	lv2_atom_forge_pop (&forge, &frame);
	notify_controllers[nr] = false;
}

void BChoppr::notifySharedDataNrToGui ()
{
	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time (&forge, 0);
	lv2_atom_forge_object (&forge, &frame, 0, uris.notify_sharedDataLinkEvent);
	lv2_atom_forge_key (&forge, uris.notify_sharedDataNr);
	lv2_atom_forge_int (&forge, sharedDataNr);
	lv2_atom_forge_pop (&forge, &frame);
	notify_sharedData = false;
}

void BChoppr::notifyGUI()
{
	if (record_on)
	{
		int notificationsCount = 0;
		// Scan monitor and build notifications
		for (int i = 0; i < MONITORBUFFERSIZE; ++i)
		{
			if (monitor[i].ready)
			{
				// Copy data monitor -> notifications
				if (notificationsCount < NOTIFYBUFFERSIZE - 1)
				{
					notifications[notificationsCount].position = i;
					notifications[notificationsCount].inputMin = monitor[i].inputMin;
					notifications[notificationsCount].inputMax = monitor[i].inputMax;
					notifications[notificationsCount].outputMin = monitor[i].outputMin;
					notifications[notificationsCount].outputMax = monitor[i].outputMax;
					notificationsCount++;
				}

				// Reset monitor data
				monitor[i].ready = false;
				monitor[i].inputMin = 0.0;
				monitor[i].inputMax = 0.0;
				monitor[i].outputMin = 0.0;
				monitor[i].outputMax = 0.0;
			}
		}

		// And build one closing notification block for submission of current position (horizon)
		notifications[notificationsCount].position = monitorpos;
		notifications[notificationsCount].inputMin = monitor[monitorpos].inputMin;
		notifications[notificationsCount].inputMax = monitor[monitorpos].inputMax;
		notifications[notificationsCount].outputMin = monitor[monitorpos].outputMin;
		notifications[notificationsCount].outputMax = monitor[monitorpos].outputMax;

		// Send notifications
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&forge, 0);
		lv2_atom_forge_object(&forge, &frame, 0, uris.notify_event);
		lv2_atom_forge_key(&forge, uris.notify_key);
		lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, (uint32_t) (5 * notificationsCount), &notifications);
		lv2_atom_forge_pop(&forge, &frame);

		notificationsCount = 0;
	}
}

void BChoppr::notifyMessageToGui()
{
	uint32_t messageNr = message.loadMessage ();

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, uris.notify_messageEvent);
	lv2_atom_forge_key(&forge, uris.notify_message);
	lv2_atom_forge_int(&forge, messageNr);
	lv2_atom_forge_pop(&forge, &frame);
}

void BChoppr::play(uint32_t start, uint32_t end)
{
	int steps = controllers[NrSteps - Controllers];

	//Silence if halted or bpm == 0
	if ((speed == 0.0f) || (bpm < 1.0f))
	{
		memset(&audioOutput1[start], 0, (end-start)*sizeof(float));
		memset(&audioOutput2[start], 0, (end-start)*sizeof(float));
		return;
	}

	for (uint32_t i = start; i < end; ++i)
	{
		float effect1 = audioInput1[i];
		float effect2 = audioInput2[i];

		// Interpolate position within the loop
		float relpos = (i - refFrame) * speed / (rate / (bpm / 60)) * controllers[SequencesPerBar - Controllers] / beatsPerBar;	// Position relative to reference frame
		float pos = MODFL (position + relpos);

		if (!controllers[Bypass - Controllers])
		{
			// Calculate step number
			int iStep;
			for (iStep = 0; (iStep < steps - 1) && (stepPositions[iStep] < pos); ++iStep) {}

			// Calculate fraction of active step
			float steppos = (iStep == 0 ? 0 : stepPositions[iStep - 1]);
			float nextpos = (int (iStep) == steps - 1 ? 1 : stepPositions[iStep]);
			float stepsize = nextpos - steppos;
			float iStepFrac = (stepsize <= 0 ? 0 : (pos - steppos) / stepsize);

			// Move to the next step?
			if (actStep != uint32_t (iStep))
			{
				prevStep = actStep;
				actStep = iStep;
				nextStep = (iStep < steps - 1 ? iStep + 1 : 0);
			}

			// Calculate effect (vol) for the position
			float ampSwing = controllers[AmpSwing - Controllers];
			float* stepLevels = &controllers[StepLevels - Controllers];
			float act = stepLevels[actStep] * LIM ((actStep % 2 == 0 ? ampSwing : 1.0f / ampSwing), 0, 1);
			float prev = stepLevels[prevStep] * LIM ((prevStep % 2 == 0 ? ampSwing : 1.0f / ampSwing), 0, 1);
			float next = stepLevels[nextStep] * LIM ((nextStep % 2 == 0 ? ampSwing : 1.0f / ampSwing), 0, 1);
			float vol = stepLevels[actStep] * LIM ((actStep % 2 == 0 ? ampSwing : 1.0f / ampSwing), 0, 1);

			// On attack
			if (iStepFrac < controllers[Attack - Controllers])
			{
				if (prev < act)
				{
					if (controllers[Blend - Controllers] == 1) vol = prev + (iStepFrac / controllers[Attack - Controllers]) * (vol - prev);
					else if (controllers[Blend - Controllers] == 2) vol = prev + 0.5 * (sin (M_PI * (iStepFrac / controllers[Attack - Controllers] - 0.5)) + 1) * (vol - prev);
				}

			}

			// On release
			if (iStepFrac > (1 - controllers[Release - Controllers]))
			{
				if (next < act)
				{
					if (controllers[Blend - Controllers] == 1) vol = next + (((1 - iStepFrac)) / controllers[Release - Controllers]) * (vol - next);
					else if (controllers[Blend - Controllers] == 2) vol = next + 0.5 * (sin (M_PI * ((1 - iStepFrac) / controllers[Release - Controllers] - 0.5)) + 1) * (vol - next);
				}
			}

			// Apply effect on input
			effect1 = audioInput1[i] * vol;
			effect2 = audioInput2[i] * vol;
		}


		// Analyze input and output data for GUI notification
		if (record_on)
		{
			// Calculate position in monitor
			int newmonitorpos = (int) (pos * MONITORBUFFERSIZE);
			if (newmonitorpos >= MONITORBUFFERSIZE) newmonitorpos = MONITORBUFFERSIZE;
			if (newmonitorpos < 0) newmonitorpos = 0;

			// Position changed? => Ready to send
			if (newmonitorpos != monitorpos)
			{
				if (monitorpos >= 0) monitor[monitorpos].ready = true;
				monitorpos = newmonitorpos;
			}

			// Get max input and output values for a block
			if (effect1 < monitor[monitorpos].outputMin) monitor[monitorpos].outputMin = effect1;
			if (effect1 > monitor[monitorpos].outputMax) monitor[monitorpos].outputMax = effect1;
			if (effect2 < monitor[monitorpos].outputMin) monitor[monitorpos].outputMin = effect2;
			if (effect2 > monitor[monitorpos].outputMax) monitor[monitorpos].outputMax = effect2;
			if (audioInput1[i] < monitor[monitorpos].inputMin) monitor[monitorpos].inputMin = audioInput1[i];
			if (audioInput1[i] > monitor[monitorpos].inputMax) monitor[monitorpos].inputMax = audioInput1[i];
			if (audioInput2[i] < monitor[monitorpos].inputMin) monitor[monitorpos].inputMin = audioInput2[i];
			if (audioInput2[i] > monitor[monitorpos].inputMax) monitor[monitorpos].inputMax = audioInput2[i];

			monitor[monitorpos].ready = false;
		}

		// Send effect to audio output
		const float drywet = controllers[DryWet - Controllers];
		audioOutput1[i] = audioInput1[i] * (1 - drywet) + effect1 * drywet;
		audioOutput2[i] = audioInput1[i] * (1 - drywet) + effect2 * drywet;
	}
}

LV2_State_Status BChoppr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	// Save shared data
	store (handle, uris.notify_sharedDataNr, &sharedDataNr, sizeof(sharedDataNr), uris.atom_Int, LV2_STATE_IS_POD);
	if (sharedDataNr != 0)
	{
		Atom_Controllers atom;
		for (int i = 0; i < NrControllers; ++i) atom.data[i] = sharedData[sharedDataNr - 1].get (i);
		atom.body.child_type = uris.atom_Float;
		atom.body.child_size = sizeof(float);
		store (handle, uris.notify_controllers, &atom, NrControllers * sizeof (float) + sizeof(LV2_Atom_Vector_Body), uris.atom_Vector, LV2_STATE_IS_POD);
	}

	return LV2_STATE_SUCCESS;
}

LV2_State_Status BChoppr::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	size_t   size;
	uint32_t type;
	uint32_t valflags;

	// Restore sharedDataNr
	if (sharedDataNr != 0) sharedData[sharedDataNr - 1].unlink (this);
	sharedDataNr = 0;
	const void* sharedDataNrData = retrieve (handle, uris.notify_sharedDataNr, &size, &type, &valflags);
	if (sharedDataNrData && (type == uris.atom_Int))
	{
		const int nr = *(int*)sharedDataNrData;
		if (nr != 0) sharedData[nr - 1].link (this);
		sharedDataNr = nr;
	}

	// Restore sharedData
	if ((sharedDataNr >= 0) && (sharedDataNr <= 4))
	{
		const void* controllersData = retrieve (handle, uris.notify_controllers, &size, &type, &valflags);
		if (controllersData && (type == uris.atom_Vector) && (sharedDataNr > 0))
		{
			const Atom_Controllers* atom = (const Atom_Controllers*) controllersData;
			if (atom->body.child_type == uris.atom_Float)
			{
				for (int i = 0; i < NrControllers; ++i) sharedData[sharedDataNr - 1].set (i, atom->data[i]);
			}
		}
	}

	notify_sharedData = true;

	// Load controllers
	for (int i = 0; i < NrControllers; ++i)
	{
		float newValue = readController (i);
		if (newValue != controllers[i])
		{
			setController (i, newValue);
			notify_controllers[i] = true;
		}
	}

	return LV2_STATE_SUCCESS;
}

static LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BChoppr* instance;
	try {instance = new BChoppr(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BChoppr.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	if (!instance)
	{
		fprintf(stderr, "BChoppr.lv2: Plugin instantiation failed.\n");
		return NULL;
	}

	if (!instance->map)
	{
		fprintf(stderr, "BChoppr.lv2: Host does not support urid:map.\n");
		delete (instance);
		return NULL;
	}

	return (LV2_Handle)instance;
}

static void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BChoppr* inst = (BChoppr*) instance;
	inst->connect_port (port, data);
}

static void run (LV2_Handle instance, uint32_t n_samples)
{
	BChoppr* inst = (BChoppr*) instance;
	inst->run (n_samples);
}

static LV2_State_Status state_save(LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BChoppr* inst = (BChoppr*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_save (store, handle, flags, features);
}

static LV2_State_Status state_restore(LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BChoppr* inst = (BChoppr*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_restore (retrieve, handle, flags, features);
}

static void cleanup (LV2_Handle instance)
{
	BChoppr* inst = (BChoppr*) instance;
	delete inst;
}

static const void* extension_data(const char* uri)
{
	static const LV2_State_Interface state  = {state_save, state_restore};
	if (!strcmp(uri, LV2_STATE__interface)) return &state;
	return NULL;
}

const LV2_Descriptor descriptor =
{
		BCHOPPR_URI,
		instantiate,
		connect_port,
		NULL, //activate,
		run,
		NULL, //deactivate,
		cleanup,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index)
	{
	case 0: return &descriptor;
	default: return NULL;
	}
}
