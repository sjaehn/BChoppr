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

#include "BChoppr_GUI.hpp"
#include "BWidgets/BDevices/MouseButton.hpp"
#include "BWidgets/BEvents/Event.hpp"
#include "BWidgets/BEvents/ExposeEvent.hpp"
#include "BWidgets/BStyles/Status.hpp"
#include "BWidgets/BStyles/Types/Color.hpp"
#include "BWidgets/BStyles/Types/Fill.hpp"
#include "BWidgets/BUtilities/Dictionary.hpp"
#include "BWidgets/BUtilities/cairoplus.h"
#include "BWidgets/BUtilities/to_string.hpp"
#include "BWidgets/BWidgets/ImageButton.hpp"
#include "BWidgets/BWidgets/Supports/ValueTransferable.hpp"
#include "SwingHSlider.hpp"
#include "BWidgets/BUtilities/stof.hpp"
#include "BWidgets/BUtilities/vsystem.hpp"
#include <cairo/cairo.h>
#include <cstdio>
#include <string>

BChoppr_GUI::BChoppr_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (820, 560, parentWindow, URID (), "B.Choppr", true, PUGL_MODULE, 0),
	controller (NULL), write_function (NULL),

	bgImage (0, 0, 820, 560, URID ("/bgimage")),
	rContainer (260, 80, 540, 350, URID ("/rcontainer")),
	sContainer (3, 165, 534, 182, URID ("/scontainer")),
	monitorSwitch (540, 15, 40, 16, true, false, URID ("/dial"), BDICT ("Monitor")),
	monitorLabel (510, 35, 100, 20, BDICT ("Monitor"), URID ("/smlabel")),
	bypassButton (652, 15, 16, 16, 2, true, false, URID ("/redbutton"), BDICT ("Bypass")),
	bypassLabel (610, 35, 100, 20, BDICT ("Bypass"), URID ("/smlabel")),
	drywetDial (743, 5, 33, 40, 1.0, 0.0, 1.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, URID ("/dial"), BDICT ("Dry/wet")),
	drywetLabel (710, 35, 100, 20, BDICT ("Dry/wet"), URID ("/smlabel")),
	helpButton (20, 80, 24, 24, false, false, URID ("/invbutton"), BDICT ("Help")),
	ytButton (50, 80, 24, 24, false, false, URID ("/invbutton"), BDICT ("Tutorial")),
	rectButton (40, 240, 60, 40, NULL, true, true, URID ("/blendbutton")),
	sinButton (140, 240, 60, 40, NULL, true, false, URID ("/blendbutton")),
	stepshapeDisplay (30, 290, 180, 140, URID ("/smonitor")),
	attackControl (40, 440, 50, 60, 0.2, 0.01, 1.0, 0.01, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Attack")),
	attackLabel (20, 500, 90, 20, BDICT ("Attack"), URID ("/label")),
	releaseControl (150, 440, 50, 60, 0.2, 0.01, 1.0, -0.01, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Decay")),
	releaseLabel (130, 500, 90, 20, BDICT ("Decay"), URID ("/label")),
	monitorDisplay (3, 3, 534, 162, URID ("/mmonitor")),
	sequencesperbarControl (260, 442, 120, 28, 1.0, 1.0, 8.0, 1.0, BNOTRANSFERD, BNOTRANSFERD,
							[] (const double& x) {return BUtilities::to_string (x, "%1.0f");}, 
							BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Sequences per bar")),
	sequencesperbarLabel (260, 470, 120, 20, BDICT ("Sequences per bar"), URID ("/label")),
	ampSwingControl (420, 442, 110, 28, 1.0, 0.001, 1000.0, 0.0, 
					 [] (const double& x) {return log(x);}, 
					 [] (const double& x) {return exp(x);}, 
					 SwingHSlider::ratioToString, SwingHSlider::stringToRatio, URID ("/dial"), BDICT ("Amp swing")),
	ampSwingLabel (420, 470, 110, 20, BDICT ("Amp swing"), URID ("/label")),
	swingControl (565, 442, 110, 28, 1.0, 1.0 / 3.0, 3.0, 0.0, 
				  [] (const double& x) {return (x >= 1.0 ? 0.5 * (x - 1.0) : 0.5 * (1.0 - 1.0 / x));}, 
				  [] (const double& x) {return (x >= 0 ? 1.0 + 2.0 * x : -1.0 / (2.0 * x - 1.0));},
				  SwingHSlider::ratioToString, SwingHSlider::stringToRatio, URID ("/dial"), BDICT ("Step swing")),
	swingLabel (565, 470, 110, 20, BDICT ("Step swing"), URID ("/label")),
	markersAutoButton (715, 450, 80, 20, BDICT ("Auto"), false, false, URID ("/button"), BDICT ("Marker")),
	markersAutoLabel (715, 470, 80, 20, BDICT ("Marker"), URID ("/label")),
	nrStepsControl (260, 502, 540, 28, 1.0, 1.0, MAXSTEPS, 1.0, BNOTRANSFERD, BNOTRANSFERD,
					[] (const double& x) {return BUtilities::to_string (x, "%1.0f");}, BSTRING_TO_DOUBLE, 
					URID ("/dial"), BDICT ("Number of steps")),
	nrStepsLabel (260, 530, 540, 20, BDICT ("Number of steps"), URID ("/label")),
	stepshapeLabel (33, 293, 120, 20, BDICT ("Step shape"), URID ("/llabel")),
	sequencemonitorLabel (263, 83, 120, 20, BDICT ("Sequence monitor"), URID ("/llabel")),
	messageLabel (420, 83, 280, 20, "", URID ("/label")),
	markerListBox (12, -68, 66, 86, {BDICT ("Auto"), BDICT ("Manual"), BDICT ("Enter")}, 0, URID ("/menu")),
	enterFrame (66, 52, 320, 70, URID ("/menu")),
	enterPositionComboBox (10, 10, 120, 20, 0, 20, 120, 44, {BDICT ("New position:"), BDICT ("New length:")}, 1, URID ("/menu")),
	enterEdit (140, 15, 60, 20, "0.000", URID ("/lflabel")),
	enterSequencesComboBox (210, 10, 100, 20, 0, 20, 100, 44, {BDICT ("sequence(s)"), BDICT ("step(s)")}, 1, URID ("/menu")),
	enterOkButton (120, 40, 80, 20, BDICT ("Apply"), false, false, URID ("/button")),
	sharedDataDummy (28, 528, 194, 24, 0.0, 0.0, 4.0, 1.0),

	surface (NULL), pat5 (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),
	scale (1.0f),
	map (NULL)

{
	if (!init_Stepshape ())
	{
		std::cerr << "BChoppr.lv2#GUI: Failed to init monitor." <<  std::endl;
		destroy_Stepshape ();
		throw std::bad_alloc ();
	}

	// Initialize and configure images
	sContainer.createImage (BStyles::Status::normal);
	rectButton.image.createImage (BStyles::Status::normal);
	rectButton.image.createImage (BStyles::Status::active);
	sinButton.image.createImage (BStyles::Status::normal);
	sinButton.image.createImage (BStyles::Status::active);
	stepshapeDisplay.createImage (BStyles::Status::normal);

	//Initialialize and configure step controllers
	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepLevelControl[i] = new BWidgets::VSlider ((i + 0.5) * sw / MAXSTEPS + sx - 7, 60, 14, 75, 1.0, 0.0, 1.0, 0.01, 
													 BNOTRANSFERD, BNOTRANSFERD, URID ("/dial"), BDICT("Level") + " " + std::to_string(i + 1));
		stepLevelControl[i]->setClickable (false);
		stepLevelControl[i]->setScrollable (true);
		sContainer.add (stepLevelControl[i]);

		stepLevelControlLabel[i] = new BWidgets::EditLabel ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 28, 20, "1.00", URID ("/smlabel"));
		stepLevelControlLabel[i]->setCallbackFunction(BEvents::Event::EventType::valueChangedEvent, stepControlLabelChangedCallback);
		sContainer.add (stepLevelControlLabel[i]);

		stepPanControl[i] = new BWidgets::Dial ((i + 0.5) * sw / MAXSTEPS + sx - 15, 135, 30, 30, 0.0, -1.0, 1.0, 0.01, 
												BNOTRANSFERD, BNOTRANSFERD, URID ("/dial"), BDICT("Panning") + " " + std::to_string(i + 1));
		stepPanControl[i]->setClickable (false);
		stepPanControl[i]->setScrollable (true);
		sContainer.add (stepPanControl[i]);

		stepPanControlLabel[i] = new BWidgets::EditLabel ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 165, 20, "1.00", URID ("/smlabel"));
		stepPanControlLabel[i]->setCallbackFunction(BEvents::Event::EventType::valueChangedEvent, stepControlLabelChangedCallback);
		sContainer.add (stepPanControlLabel[i]);

		
	}

	//Initialialize and configure markers
	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		markerWidgets[i] = new Marker ((i + 1) * sw / MAXSTEPS + sx - 5, 10, 10, 16, (double(i) + 1.0) / MAXSTEPS, 0.0, 1.0, 0.0,
									   BNOTRANSFERD, BNOTRANSFERD, URID ("/marker"), BDICT ("Marker") + " " + std::to_string(i + 1));
		markerWidgets[i]->setHasValue (false);
		markerWidgets[i]->setCallbackFunction (BEvents::Event::EventType::buttonPressEvent, BChoppr_GUI::markerClickedCallback);
		markerWidgets[i]->setCallbackFunction (BEvents::Event::EventType::pointerDragEvent, BChoppr_GUI::markerDraggedCallback);
		sContainer.add (markerWidgets[i]);
	}

	// Inititaize shared data buttons
	for (int i = 0; i < 4; ++i) 
	{
		sharedDataButtons[i] = new BWidgets::Button (28 + 50 * i, 528, 44, 24, true, false, 
													 URID ("/halobutton"), BDICT ("Shared data") + " " + std::to_string (i + 1));
	}

	// Link controllers
	controllers[Bypass - Controllers] = &bypassButton;
	controllers[DryWet - Controllers] = &drywetDial;
	controllers[Blend - Controllers] = &rectButton;
	controllers[Attack - Controllers] = &attackControl;
	controllers[Release - Controllers] = &releaseControl;
	controllers[SequencesPerBar - Controllers] = &sequencesperbarControl;
	controllers[AmpSwing - Controllers] = &ampSwingControl;
	controllers[Swing - Controllers] = &swingControl;
	controllers[NrSteps - Controllers] = &nrStepsControl;
	for (int i = 0; i < MAXSTEPS - 1; ++i) controllers[StepPositions + i - Controllers] = markerWidgets[i];
	for (int i = 0; i < MAXSTEPS; ++i) controllers[StepLevels + i - Controllers] = stepLevelControl[i];
	for (int i = 0; i < MAXSTEPS; ++i) controllers[StepPans + i - Controllers] = stepPanControl[i];

	// Set callbacks
	for (int i = 0; i < NrControllers; ++i) controllers[i]->setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::valueChangedCallback);
	monitorSwitch.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::valueChangedCallback);
	markerListBox.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::listBoxChangedCallback);
	enterPositionComboBox.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::enterListBoxChangedCallback);
	enterSequencesComboBox.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::enterListBoxChangedCallback);
	enterOkButton.setCallbackFunction(BEvents::Event::EventType::buttonClickEvent, enterOkClickedCallback);
	markersAutoButton.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::markersAutoClickedCallback);
	rectButton.setCallbackFunction (BEvents::Event::EventType::buttonClickEvent, BChoppr_GUI::buttonClickedCallback);
	sinButton.setCallbackFunction (BEvents::Event::EventType::buttonClickEvent, BChoppr_GUI::buttonClickedCallback);
	helpButton.setCallbackFunction(BEvents::Event::EventType::buttonPressEvent, helpButtonClickedCallback);
	ytButton.setCallbackFunction(BEvents::Event::EventType::buttonPressEvent, ytButtonClickedCallback);
	for (BWidgets::Button* s: sharedDataButtons) s->setCallbackFunction (BEvents::Event::EventType::buttonClickEvent, BChoppr_GUI::sharedDataClickedCallback);
	sharedDataDummy.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BChoppr_GUI::valueChangedCallback);

	// Configure widgets
	bgImage.loadImage(BStyles::Status::normal,pluginPath + BG_FILE);
	drywetDial.setScrollable (true);
	drywetDial.setClickable (false);
	attackControl.setScrollable (true);
	attackControl.setClickable (false);
	releaseControl.setScrollable (true);
	releaseControl.setClickable (false);
	sequencesperbarControl.setScrollable (true);
	ampSwingControl.setClickable (false);
	swingControl.setClickable (false);
	nrStepsControl.setScrollable (true);
	markerListBox.setStacking (StackingType::escape);
	enterFrame.setStacking (StackingType::escape);
	enterFrame.hide();
	sharedDataDummy.hide();
	setTheme (theme);

	setAutoMarkers ();
	rearrange_controllers ();

	// Pack widgets
	enterFrame.add (&enterPositionComboBox);
	enterFrame.add (&enterEdit);
	enterFrame.add (&enterSequencesComboBox);
	enterFrame.add (&enterOkButton);
	markerListBox.add (&enterFrame);
	rContainer.add (&monitorDisplay);
	rContainer.add (&sContainer);
	add (&bgImage);
	add (&monitorSwitch);
	add (&monitorLabel);
	add (&bypassButton);
	add (&bypassLabel);
	add (&drywetDial);
	add (&drywetLabel);
	add (&helpButton);
	add (&ytButton);
	add (&rectButton);
	add (&sinButton);
	add (&stepshapeDisplay);
	add (&attackControl);
	add (&attackLabel);
	add (&releaseControl);
	add (&releaseLabel);
	add (&sequencesperbarControl);
	add (&sequencesperbarLabel);
	add (&ampSwingControl);
	add (&ampSwingLabel);
	add (&swingControl);
	add (&swingLabel);
	add (&markersAutoButton);
	add (&markersAutoLabel);
	add (&nrStepsControl);
	add (&nrStepsLabel);
	add (&stepshapeLabel);
	add (&sharedDataDummy);
	for (BWidgets::Button* b : sharedDataButtons) add (b);
	add (&rContainer);
	add (&sequencemonitorLabel);
	add (&messageLabel);

	redrawSContainer ();
	redrawButtons ();

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0) m = (LV2_URID_Map*) features[i]->data;
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);
}

BChoppr_GUI::~BChoppr_GUI()
{
	send_record_off ();
	destroy_Stepshape ();

	for (BWidgets::VSlider* v : stepLevelControl) if (v) delete v;
	for (BWidgets::EditLabel* l : stepLevelControlLabel) if (l) delete l;
	for (BWidgets::Dial* d : stepPanControl) if (d) delete d;
	for (BWidgets::EditLabel* l : stepPanControlLabel) if (l) delete l;
	for (Marker* m : markerWidgets) if (m) delete m;
	for (BWidgets::Button* b : sharedDataButtons) if (b) delete b;
}

void BChoppr_GUI::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port_index == Notify))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Linked / unlinked to shared data
			if (obj->body.otype == uris.notify_sharedDataLinkEvent)
			{
				LV2_Atom *oNr = NULL;

				lv2_atom_object_get
				(
					obj,
					uris.notify_sharedDataNr, &oNr,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int))
				{
					const int nr = ((LV2_Atom_Int*)oNr)->body;
					if ((nr >= 0) && (nr <= 4) && (nr != sharedDataDummy.getValue()))
					{
						sharedDataDummy.setValueable (false);
						sharedDataDummy.setValue (nr);
						sharedDataDummy.setValueable (true);

						for (int i = 0; i < 4; ++i)
						{
							sharedDataButtons[i]->setValueable (false);
							sharedDataButtons[i]->setValue (i == nr - 1);
							sharedDataButtons[i]->setValueable (true);
						}

					}
				}
			}

			// Controller changed
			else if (obj->body.otype == uris.notify_controllerEvent)
			{
				LV2_Atom *oNr = NULL, *oVal = NULL;

				lv2_atom_object_get
				(
					obj,
					uris.notify_controllerNr, &oNr,
					uris.notify_controllerValue, &oVal,
					NULL
				);

				if (oNr && (oNr->type == uris.atom_Int) && oVal && (oVal->type == uris.atom_Float))
				{
					const int nr =  ((LV2_Atom_Int*)oNr)->body;
					const float val = ((LV2_Atom_Float*)oVal)->body;

					if ((nr >= StepPositions - Controllers) && (nr < StepPositions - Controllers + MAXSTEPS - 1))
					{
						for (Marker* m : markerWidgets) m->setValueable(false);
						setMarker (nr - (StepPositions - Controllers), val);
						setAutoMarkers ();
						for (Marker* m : markerWidgets) m->setValueable(true);
						rearrange_controllers ();
						redrawSContainer ();
					}

					else setController (nr, val);
				}
			}

			// Monitor notification
			else if ((obj->body.otype == uris.notify_event) && monitorSwitch.getValue ())
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_key, &data, 0);
				if (data && (data->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) data;
					if (vec->body.child_type == uris.atom_Float)
					{
						uint32_t notificationsCount = (uint32_t) ((data->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (BChopprNotifications));
						BChopprNotifications* notifications = (BChopprNotifications*) (&vec->body + 1);
						if (notificationsCount > 0)
						{
							std::vector<BChopprNotifications> ndata;
							for (uint32_t i = 0; i < notificationsCount; ++i) ndata.push_back(notifications[i]);
							monitorDisplay.pushData (ndata);
						}
					}
				}
				else std::cerr << "BChoppr.lv2#GUI: Corrupt audio message." << std::endl;
			}

			// Message notification
			else if (obj->body.otype == uris.notify_messageEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_message, &data, 0);
				if (data && (data->type == uris.atom_Int))
				{
					const int messageNr = ((LV2_Atom_Int*)data)->body;
					std::string msg = ((messageNr >= NO_MSG) && (messageNr <= MAX_MSG) ? 
									   (messageStrings[messageNr] != "" ? "*** " + BDICT(messageStrings[messageNr]) + " ***" : "") : 
									   "");
					messageLabel.setText (msg);
				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port_index >= Controllers) && (port_index < Controllers + NrControllers) && (sharedDataDummy.getValue() == 0))
	{
		int nr = port_index - Controllers;
		float val = *(float*) buffer;
		if ((nr >= StepPositions - Controllers) && (nr < StepPositions - Controllers + MAXSTEPS - 1))
		{
			setMarker (nr - (StepPositions - Controllers), val);
			setAutoMarkers ();
			rearrange_controllers ();
			redrawSContainer ();
		}

		else setController (nr, val);
	}
}

void BChoppr_GUI::onConfigureRequest (BEvents::Event* event)
{
	Window::onConfigureRequest (event);

	BEvents::ExposeEvent* ee = dynamic_cast<BEvents::ExposeEvent*>(event);
	if (!ee) return;
	const double sz = (ee->getArea().getWidth() / 820.0 > ee->getArea().getHeight() / 560.0 ? ee->getArea().getHeight() / 560.0 : ee->getArea().getWidth() / 820.0);
	setZoom (sz);
}

void BChoppr_GUI::send_record_on ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
	monitorSwitch.setValue (1.0);
}

void BChoppr_GUI::send_record_off ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
	monitorSwitch.setValue (0.0);
}

void BChoppr_GUI::sendSharedDataNr ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.notify_sharedDataLinkEvent);
	lv2_atom_forge_key (&forge, uris.notify_sharedDataNr);
	lv2_atom_forge_int (&forge, sharedDataDummy.getValue());
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BChoppr_GUI::sendController (const int nr, const float value)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.notify_controllerEvent);
	lv2_atom_forge_key (&forge, uris.notify_controllerNr);
	lv2_atom_forge_int (&forge, nr);
	lv2_atom_forge_key (&forge, uris.notify_controllerValue);
	lv2_atom_forge_float (&forge, value);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

float BChoppr_GUI::setController (const int nr, const double value)
{
	BWidgets::Valueable* v = dynamic_cast<BWidgets::Valueable*>(controllers[nr]);
	if (!v) return value;

	// Prevent emission of ValueChangedEvent
	v->setValueable (false);

	// Set new value to widgets
	const int ctrl = nr + Controllers;
	if (ctrl == Bypass) bypassButton.setValue (value != 0.0);
	else if (ctrl == Blend)
	{
		sinButton.setValue (value == 2.0);
		rectButton.setValue (value != 2.0);
	}

	else 
	{
		BWidgets::ValueableTyped<double>* vd = dynamic_cast<BWidgets::ValueableTyped<double>*>(v);
		if (vd) vd->setValue (value);
	}

	// Switch on Valueable again
	v->setValueable (true);

	if (ctrl == Blend) redrawStepshape ();

	else if ((ctrl == Attack) || (ctrl == Release)) redrawStepshape ();

	else if (ctrl == AmpSwing) rearrange_controllers();

	else if (ctrl == Swing)
	{
		setAutoMarkers();
		rearrange_controllers();
		redrawSContainer();
	}

	else if (ctrl == NrSteps)
	{
		setAutoMarkers();
		rearrange_controllers();
		redrawSContainer();
	}

	else if ((ctrl >= StepPositions) and (ctrl < StepPositions + MAXSTEPS - 1))
	{
		return (((Marker*)controllers[nr])->hasValue() ? value : 0.0f);
	}

	else if ((ctrl >= StepLevels ) and (ctrl < StepLevels + MAXSTEPS))
	{
		stepLevelControlLabel[nr - (StepLevels - Controllers)]->setText (BUtilities::to_string (value, "%1.2f"));
	}

	else if ((nr >= StepPans - Controllers) and (nr < StepPans - Controllers + MAXSTEPS))
	{
		stepPanControlLabel[nr - (StepPans - Controllers)]->setText (BUtilities::to_string (value, "%1.2f"));
	}

	return value;
}

void BChoppr_GUI::setMarker (const int markerNr, double value)
{
	if ((markerNr < 0) || (markerNr >= MAXSTEPS - 1)) return;

	// Value 0.0: Automatic
	if (value == 0.0)
	{
		markerWidgets[markerNr]->setHasValue (false);
	}

	else
	{
		// Set value and switch off automatic
		value = std::min (std::max (value, MINMARKERVALUE), 1.0);
		markerWidgets[markerNr]->setHasValue (true);
		markerWidgets[markerNr]->setValue (value);

		// Validate ancessors
		for (int i = markerNr - 1; i >= 0; --i)
		{
			if (markerWidgets[i]->hasValue())
			{
				if (markerWidgets[i]->getValue() > value) markerWidgets[i]->setValue (value);
				else break;
			}
		}

		// Validate successors
		for (int i = markerNr + 1; i < MAXSTEPS - 1; ++i)
		{
			if (markerWidgets[i]->hasValue())
			{
				if (markerWidgets[i]->getValue() < value) markerWidgets[i]->setValue (value);
				else break;
			}
		}
	}

	std::vector<double> steps;
	const int nrMarkers = nrStepsControl.getValue() - 1;
	for (int i = 0; i < nrMarkers; ++i) steps.push_back(markerWidgets[i]->getValue());
	monitorDisplay.setSteps(steps);
}

void BChoppr_GUI::setAutoMarkers ()
{
	int nrMarkers = nrStepsControl.getValue() - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		if (!markerWidgets[i]->hasValue())
		{
			if ((i == nrMarkers - 1) || (markerWidgets[i + 1]->hasValue()))
			{
				double swing = 2.0 * swingControl.getValue() / (swingControl.getValue() + 1.0);
				double anc = (start == 0 ? 0 : markerWidgets[start - 1]->getValue());
				double suc = (i == nrMarkers - 1 ? 1 : markerWidgets[i + 1]->getValue());
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - swing : swing) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - swing : swing);
					anc += f * step;
					markerWidgets[j]->setValue (anc);
				}
			}
		}
		else start = i + 1;
	}

	std::vector<double> steps;
	for (int i = 0; i < nrMarkers; ++i) steps.push_back(markerWidgets[i]->getValue());
	monitorDisplay.setSteps(steps);
}

void BChoppr_GUI::rearrange_controllers ()
{
	int nrStepsi = nrStepsControl.getValue();

	if ((nrStepsi < 1) || (nrStepsi > MAXSTEPS)) return;

	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	const double oddf = (ampSwingControl.getValue() >= 1.0 ? 1.0 : ampSwingControl.getValue());
	const double evenf = (ampSwingControl.getValue() >= 1.0 ? 1.0 / ampSwingControl.getValue() : 1.0);

	for (int i = 0; i < MAXSTEPS; ++i)
	{
		if (i < nrStepsi)
		{
			stepLevelControl[i]->resize (14, (14 + std::min (std::max (66 * ((i % 2) == 0 ? oddf : evenf), 0.0), 66.0 )));
			stepLevelControl[i]->moveTo ((i + 0.5) * sw / nrStepsi + sx - 7, 135 - stepLevelControl[i]->getHeight());
			stepLevelControl[i]->show();

			stepLevelControlLabel[i]->moveTo ((i + 0.5) * sw / nrStepsi + sx - 14, 40);
			stepLevelControlLabel[i]->resize (28, 20);
			stepLevelControlLabel[i]->show();

			stepPanControl[i]->resize (30, 30);
			stepPanControl[i]->moveTo ((i + 0.5) * sw / nrStepsi + sx - 15, 135);
			stepPanControl[i]->show();

			stepPanControlLabel[i]->moveTo ((i + 0.5) * sw / nrStepsi + sx - 14, 165);
			stepPanControlLabel[i]->resize (28, 20);
			stepPanControlLabel[i]->show();

			if (i < nrStepsi - 1) markerWidgets[i]->resize (10, 16);
		}
		else
		{
			stepLevelControl[i]->hide ();
			stepPanControl[i]->hide();
			stepLevelControlLabel[i]->hide();
			stepPanControlLabel[i]->hide();
		}
	}

	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		if (i < nrStepsi - 1)
		{
			markerWidgets[i]->moveTo (markerWidgets[i]->getValue() * sw + sx - 5, 10);
			markerWidgets[i]->show ();
		}
		else markerWidgets[i]->hide ();
	}
}

void BChoppr_GUI::recalculateEnterEdit ()
{
	Marker* marker = (Marker*)markerListBox.getParent();
	if (!marker) return;

	int nrSteps = nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == markerWidgets[i])
		{
			const double rpos = marker->getValue();
			double val = rpos;

			if (enterPositionComboBox.getValue() == 1.0)
			{
				if (enterSequencesComboBox.getValue() == 2.0)
				{
					val = rpos * nrSteps;
					// TODO rounding
				}
			}

			else 
			{
				const double prpos = (i > 0 ? markerWidgets[i - 1]->getValue() : 0.0);
				if (enterSequencesComboBox.getValue() == 1.0) val = rpos - prpos;
				else
				{
					val = (rpos - prpos) * nrSteps;
					// TODO rounding
				}

			}

			enterEdit.setText (BUtilities::to_string (val, "%1.6f"));
			break;
		}
	}
}

void BChoppr_GUI::valueChangedCallback (BEvents::Event* event)
{
	if (event)
	{
		BWidgets::Widget* widget = event->getWidget ();
		if (widget)
		{
			BChoppr_GUI* ui = (BChoppr_GUI*) widget->getMainWindow ();
			if (ui)
			{
				
				// Get controller nr
				int controllerNr = -1;
				for (int i = 0; i < NrControllers; ++i)
				{
					if (widget == ui->controllers[i])
					{
						controllerNr = i;
						break;
					}
				}

				if (controllerNr >= 0)
				{
					BWidgets::ValueableTyped<bool>* vb = dynamic_cast<BWidgets::ValueableTyped<bool>*>(widget);
					BWidgets::ValueableTyped<double>* vd = dynamic_cast<BWidgets::ValueableTyped<double>*>(widget);
					float value = 0.0f;
					if ((controllerNr == Bypass - Controllers) && vb) value = vb->getValue();
					else if ((controllerNr == Blend - Controllers) && vb) value = (vb->getValue() ? 1.0 : 2.0);
					else if (vd) value = vd->getValue();
					else return;
					value = ui->setController (controllerNr, value);

					if (ui->sharedDataDummy.getValue()) ui->sendController (controllerNr, value);
					else ui->write_function (ui->controller, Controllers + controllerNr, sizeof (float), 0, &value);
				}

				// Shared data
				else if (widget == &ui->sharedDataDummy)
				{
					const int val = ui->sharedDataDummy.getValue() - 1;
					for (int i = 0; i < 4; ++i)
					{
						ui->sharedDataButtons[i]->setValueable (false);
						ui->sharedDataButtons[i]->setValue (i == val);
						ui->sharedDataButtons[i]->setValueable (true);
					}

					ui->sendSharedDataNr();
				}

				// monitor on/off changed
				else if (widget == &ui->monitorSwitch)
				{
					if (ui->monitorSwitch.getValue ()) ui->send_record_on ();
					else ui->send_record_off ();
					return;
				}
			}
		}
	}
}

void BChoppr_GUI::markerClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::MouseButton::ButtonType::right) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToFront();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	const int nrSteps = ui->nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == ui->markerWidgets[i])
		{
			Marker* oldMarker = (Marker*) ui->markerListBox.getParent();
			ui->markerListBox.setValue (0);

			if (oldMarker && (oldMarker == marker))
			{
				if (ui->markerListBox.isVisible()) ui->markerListBox.hide();
				else 
				{
					ui->markerListBox.show ();
					ui->enterFrame.hide();
				}
			}

			else if (oldMarker && (oldMarker != marker))
			{
				oldMarker->release (&ui->markerListBox);
				marker->add (&ui->markerListBox);
				ui->markerListBox.show();
				ui->enterFrame.hide();
			}

			else
			{
				marker->add (&ui->markerListBox);
				ui->markerListBox.show();
				ui->enterFrame.hide();
			}

		}
	}
}

void BChoppr_GUI::markerDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::MouseButton::ButtonType::left) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToFront();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	const int nrSteps = ui->nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == ui->markerWidgets[i])
		{
			double x0 = ui->sContainer.getXOffset();
			double w = ui->sContainer. getEffectiveWidth();
			double frac = (w > 0 ? (5 + pev->getDelta().x + marker->getPosition().x - x0) / w : MINMARKERVALUE);
			frac = std::min (std::max (frac, MINMARKERVALUE), 1.0);

			// Limit to antecessors value
			for (int j = i - 1; j >= 0; --j)
			{
				if (ui->markerWidgets[j]->hasValue())
				{
					if (frac < ui->markerWidgets[j]->getValue()) frac = ui->markerWidgets[j]->getValue();
					break;
				}
			}

			// Limit to successors value
			for (int j = i + 1; j < nrSteps - 1; ++j)
			{
				if (ui->markerWidgets[j]->hasValue())
				{
					if (frac > ui->markerWidgets[j]->getValue()) frac = ui->markerWidgets[j]->getValue();
					break;
				}
			}

			ui->setMarker (i, frac);
			ui->setAutoMarkers();
			ui->rearrange_controllers();
			ui->redrawSContainer();
			break;
		}
	}
}

void BChoppr_GUI::listBoxChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangeTypedEvent<size_t>* vev = dynamic_cast<BEvents::ValueChangeTypedEvent<size_t>*>(event);
	if (!vev) return;
	BWidgets::ListBox* lb = (BWidgets::ListBox*) vev->getWidget();
	if (!lb) return;
	Marker* m = (Marker*) lb->getParent();
	if (!m) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)m->getMainWindow();
	if (!ui) return;

	int value = vev->getValue();
	switch (value)
	{
		case 1:		// Auto
					m->setHasValue (false);
					ui->enterFrame.hide();
					lb->hide();
					ui->setAutoMarkers();
					ui->rearrange_controllers();
					ui->redrawSContainer();
					break;

		case 2:		// Manual
					m->setHasValue (true);
					ui->enterFrame.hide();
					lb->hide();
					ui->setAutoMarkers();
					ui->rearrange_controllers();
					ui->redrawSContainer();
					break;

		case 3: 	// Enter
					m->setHasValue (true);
					ui->recalculateEnterEdit();
					if (ui->markerListBox.getAbsolutePosition().x > 420) ui->enterFrame.moveTo (-320, ui->enterFrame.getPosition().y);
					else ui->enterFrame.moveTo (66, ui->enterFrame.getPosition().y);
					ui->enterFrame.show();
					break;

		default:	return;
	}
}

void BChoppr_GUI::markersAutoClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangeTypedEvent<bool>* vev = dynamic_cast<BEvents::ValueChangeTypedEvent<bool>*>(event);
	if (!vev) return;
	if (vev->getValue() == 0.0) return;
	BWidgets::TextButton* tb = (BWidgets::TextButton*) vev->getWidget();
	if (!tb) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)tb->getMainWindow();
	if (!ui) return;

	for (Marker* m : ui->markerWidgets) m->setHasValue (false);

	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
}

// TODO
void BChoppr_GUI::buttonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Widget* w = event->getWidget();
	if (!w) return;
	BChoppr_GUI* ui = (BChoppr_GUI*) w->getMainWindow();
	if (!ui) return;

	if (w == &ui->rectButton) 
	{
		ui->rectButton.setValue (true);
		ui->sinButton.setValue (false);
	}

	else if (w == &ui->sinButton) 
	{
		ui->rectButton.setValue (false);
		ui->sinButton.setValue (true);
	}
}

// TODO
void BChoppr_GUI::sharedDataClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Button* widget = dynamic_cast<BWidgets::Button*>(event->getWidget ());
	if (!widget) return;
	BChoppr_GUI* ui = (BChoppr_GUI*) widget->getMainWindow();
	if (!ui) return;

	double value = 0.0;
	if (widget->getValue())
	{
		for (size_t i = 0; i < ui->sharedDataButtons.size(); ++i)
		{
			if (widget != ui->sharedDataButtons[i]) ui->sharedDataButtons[i]->setValue (false);
			else (value = i + 1);
		}
	}

	ui->sharedDataDummy.setValue(value);
}

void BChoppr_GUI::helpButtonClickedCallback (BEvents::Event* event)
{
	char cmd[] = WWW_BROWSER_CMD;
	char param[] = HELP_URL;
	char* argv[] = {cmd, param, NULL};
	std::cerr << "BChoppr.lv2#GUI: Call " << HELP_URL << " for help.\n";
	if (BUtilities::vsystem (argv) == -1) std::cerr << "BChoppr.lv2#GUI: Couldn't fork.\n";
}

void BChoppr_GUI::ytButtonClickedCallback (BEvents::Event* event)
{
	char cmd[] = WWW_BROWSER_CMD;
	char param[] = YT_URL;
	char* argv[] = {cmd, param, NULL};
	std::cerr << "BChoppr.lv2#GUI: Call " << YT_URL << " for tutorial video.\n";
	if (BUtilities::vsystem (argv) == -1) std::cerr << "BChoppr.lv2#GUI: Couldn't fork.\n";
}

void BChoppr_GUI::stepControlLabelChangedCallback (BEvents::Event* event)
{
	BEvents::ValueChangeTypedEvent<std::string>* vew = dynamic_cast<BEvents::ValueChangeTypedEvent<std::string>*>(event);
	if (!vew) return;
	if (vew->getWidget())
	{
		BWidgets::EditLabel* l = dynamic_cast<BWidgets::EditLabel*>(vew->getWidget());
		BChoppr_GUI* ui = dynamic_cast<BChoppr_GUI*>(l->getMainWindow());
		if (ui)
		{
			for (int i = 0; i < MAXSTEPS; ++i)
			{
				if (l == ui->stepLevelControlLabel[i])
				{
					double val = ui->stepLevelControl[i]->getValue();
					try {val = BUtilities::stof (l->getText());}
					catch (std::invalid_argument &ia)
					{
						fprintf (stderr, "%s\n", ia.what());
						l->setText (BUtilities::to_string (val, "%1.2f"));
						return;
					}

					ui->stepLevelControl[i]->setValue (val);
					l->setText (BUtilities::to_string (ui->stepLevelControl[i]->getValue(), "%1.2f"));
					break;
				}

				else if (l == ui->stepPanControlLabel[i])
				{
					double val = ui->stepLevelControl[i]->getValue();
					try {val = BUtilities::stof (l->getText());}
					catch (std::invalid_argument &ia)
					{
						fprintf (stderr, "%s\n", ia.what());
						l->setText (BUtilities::to_string (val, "%1.2f"));
						return;
					}

					ui->stepPanControl[i]->setValue (val);
					l->setText (BUtilities::to_string (ui->stepPanControl[i]->getValue(), "%1.2f"));
					break;
				}
			}
		}
	}
}

void BChoppr_GUI::enterListBoxChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	BWidgets::ListBox* lb = (BWidgets::ListBox*) vev->getWidget();
	if (!lb) return;
	Marker* m = (Marker*) lb->getParent();
	if (!m) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)m->getMainWindow();
	if (!ui) return;

	ui->recalculateEnterEdit();
}

void BChoppr_GUI::enterOkClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::MouseButton::ButtonType::left) return;
	BWidgets::TextButton* button = (BWidgets::TextButton*)event->getWidget();
	if (!button) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)button->getMainWindow();
	if (!ui) return;
	Marker* marker = (Marker*)ui->markerListBox.getParent();
	if (!marker) return;

	int nrSteps = ui->nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == ui->markerWidgets[i])
		{
			double frac = marker->getValue();
			double val = 0.0;

			try 
			{
				val = BUtilities::stof (ui->enterEdit.getText());
				if (ui->enterPositionComboBox.getValue() == 1.0)
				{
					if (ui->enterSequencesComboBox.getValue() == 1.0) frac = val;
					else frac = val / nrSteps;
				}

				else
				{
					const double prec = (i > 0 ? ui->markerWidgets[i - 1]->getValue() : 0.0);
					if (i > 0) ui->markerWidgets[i - 1]->setHasValue (true);
					if (ui->enterSequencesComboBox.getValue() == 1.0) frac = val + prec;
					else frac = val / nrSteps + prec;
				}
			}
			catch (std::exception &exc) {std::cerr << "BSchaffl.lv2#GUI: " << exc.what() << "\n";}

			frac = std::min (std::max (frac, MINMARKERVALUE), 1.0);

			// Limit to antecessors value
			for (int j = i - 1; j >= 0; --j)
			{
				if (ui->markerWidgets[j]->hasValue())
				{
					if (frac < ui->markerWidgets[j]->getValue()) frac = ui->markerWidgets[j]->getValue();
					break;
				}
			}

			// Limit to successors value
			for (int j = i + 1; j < nrSteps - 1; ++j)
			{
				if (ui->markerWidgets[j]->hasValue())
				{
					if (frac > ui->markerWidgets[j]->getValue()) frac = ui->markerWidgets[j]->getValue();
					break;
				}
			}

			ui->enterFrame.hide();
			ui->markerListBox.hide();
			ui->setMarker (i, frac);
			ui->setAutoMarkers();
			ui->rearrange_controllers();
			ui->redrawSContainer();
			break;
		}
	}
}

bool BChoppr_GUI::init_Stepshape ()
{
	double height = stepshapeDisplay.getEffectiveHeight ();
	pat5 = cairo_pattern_create_linear (0, 0, 0, height);

	return (pat5 && (cairo_pattern_status (pat5) == CAIRO_STATUS_SUCCESS));
}

void BChoppr_GUI::destroy_Stepshape ()
{
	//Destroy also mainMonitors cairo data
	if (pat5 && (cairo_pattern_status (pat5) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat5);
}

void BChoppr_GUI::redrawStepshape ()
{
	cairo_surface_t* surface = stepshapeDisplay.getImageSurface (BStyles::Status::normal);
	cairo_t* cr = cairo_create (stepshapeDisplay.getImageSurface (BStyles::Status::normal));
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;
	const double width = cairo_image_surface_get_width (surface);
	const double height = cairo_image_surface_get_height(surface);
	const BStyles::Color fgColor = stepshapeDisplay.getFgColors()[BStyles::Status::normal];

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_RGBA(BStyles::black));
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_fill (cr);
	cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::grey));
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.2 * height);
	cairo_line_to (cr, width, 0.2 * height);
	cairo_move_to (cr, 0, 0.55 * height);
	cairo_line_to (cr, width, 0.55 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);
	cairo_move_to (cr, 0.25 * width, 0);
	cairo_line_to (cr, 0.25 * width, height);
	cairo_move_to (cr, 0.5 * width, 0);
	cairo_line_to (cr, 0.5 * width, height);
	cairo_move_to (cr, 0.75 * width, 0);
	cairo_line_to (cr, 0.75 * width, height);
	cairo_stroke (cr);

	// Draw step shape
	cairo_set_source_rgba (cr, CAIRO_RGBA(fgColor));
	cairo_set_line_width (cr, 3);

	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width * 0.25, 0.9 * height);

	const float attack = attackControl.getValue();
	const float release = releaseControl.getValue();

	if (rectButton.getValue())
	{
		if ((attack + release) > 1)
		{
			float crosspointX = attack / (attack + release);
			float crosspointY = crosspointX / attack - (crosspointX - (1 - release)) / release;
			cairo_line_to (cr, width * 0.25 + crosspointX * width * 0.5, 0.9 * height - 0.7 * height * crosspointY);
		}
		else
		{
			cairo_line_to (cr, width * 0.25 + attack * width * 0.5, 0.2 * height);
			cairo_line_to (cr, width * 0.75  - release * width * 0.5, 0.2 * height);

		}
	}

	else if (sinButton.getValue())
	{
		for (double i = 0.0; i <= 1.0; i += 0.025)
		{
			double vol = 1.0;
			if (i < attack) vol = sin (M_PI * (i / attack - 0.5));
			if (i > (1 - release)) vol = vol * sin (M_PI * ((1 - i) / release - 0.5));
			cairo_line_to (cr, width * (0.25 + 0.5 * i), height * (0.55 - 0.35 * vol));
		}
	}

	cairo_line_to (cr, width * 0.75, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	cairo_stroke_preserve (cr);

	cairo_pattern_add_color_stop_rgba (pat5, 0.1, CAIRO_RGB(fgColor), 1);
	cairo_pattern_add_color_stop_rgba (pat5, 0.9, CAIRO_RGB(fgColor), 0);
	cairo_set_source (cr, pat5);
	cairo_line_to(cr, 0, 0.9 * height);
	cairo_set_line_width (cr, 0);
	cairo_fill (cr);

	cairo_destroy (cr);

	stepshapeDisplay.update ();
}

void BChoppr_GUI::redrawSContainer ()
{
	cairo_surface_t* surface = sContainer.getImageSurface (BStyles::Status::normal);
	cairoplus_surface_clear (surface);
	cairo_t* cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	double width = cairo_image_surface_get_width(surface);
	double height = cairo_image_surface_get_height(surface);

	cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (BStyles::black));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.0, 0.0, 0.0, 0.5);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	for (int i = 0; i < nrStepsControl.getValue() - 1; ++i)
	{
		cairo_set_line_width (cr, 1.0);
		cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::grey));
		cairo_move_to (cr, markerWidgets[i]->getValue() * width, 0);
		cairo_rel_line_to (cr, 0, 30);
		cairo_line_to (cr, (i + 1) / nrStepsControl.getValue() * width, 40);
		cairo_rel_line_to (cr, 0, 145);
		cairo_stroke (cr);
	}

	cairo_destroy (cr);
	sContainer.update();
}

void BChoppr_GUI::redrawButtons ()
{

	// rectButton, inactive
	cairo_surface_t* surface = rectButton.image.getImageSurface (BStyles::Status::normal);
	cairo_t* cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	double width = cairo_image_surface_get_width(surface);
	double height = cairo_image_surface_get_height(surface);
	cairoplus_surface_clear(surface);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::black));
	cairo_rectangle(cr, 0.05 * width, 0.05 * height, 0.9 * width, 0.9 * height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (rectButton.getFgColors()[BStyles::Status::inactive]));
	cairo_set_line_width (cr, 2.0);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_move_to (cr, 0.1 * width, 0.8 * height);
	cairo_line_to (cr, 0.25 * width, 0.8 * height);
	cairo_line_to (cr, 0.3 * width, 0.2 * height);
	cairo_line_to (cr, 0.7 * width, 0.2 * height);
	cairo_line_to (cr, 0.75 * width, 0.8 * height);
	cairo_line_to (cr, 0.9 * width, 0.8 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

	// rectButton, active
	surface = rectButton.image.getImageSurface (BStyles::Status::active);
	cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	cairoplus_surface_clear(surface);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::black));
	cairo_rectangle(cr, 0.05 * width, 0.05 * height, 0.9 * width, 0.9 * height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (rectButton.getFgColors()[BStyles::Status::normal]));
	cairo_set_line_width (cr, 2.0);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_move_to (cr, 0.1 * width, 0.8 * height);
	cairo_line_to (cr, 0.25 * width, 0.8 * height);
	cairo_line_to (cr, 0.3 * width, 0.2 * height);
	cairo_line_to (cr, 0.7 * width, 0.2 * height);
	cairo_line_to (cr, 0.75 * width, 0.8 * height);
	cairo_line_to (cr, 0.9 * width, 0.8 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

	// sinButton, inactive
	surface = sinButton.image.getImageSurface (BStyles::Status::normal);
	cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	cairoplus_surface_clear(surface);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::black));
	cairo_rectangle(cr, 0.05 * width, 0.05 * height, 0.9 * width, 0.9 * height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (sinButton.getFgColors()[BStyles::Status::inactive]));
	cairo_set_line_width (cr, 2.0);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_move_to (cr, 0.1 * width, 0.8 * height);
	cairo_line_to (cr, 0.15 * width, 0.8 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.15 + i * 0.03) * width, (0.5 - 0.3 * sin (double (i - 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.55 * width, 0.2 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.55 + i * 0.03) * width, (0.5 - 0.3 * sin (double (i + 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.9 * width, 0.8 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

	// sinButton, active
	surface = sinButton.image.getImageSurface (BStyles::Status::active);
	cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	cairoplus_surface_clear(surface);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BStyles::black));
	cairo_rectangle(cr, 0.05 * width, 0.05 * height, 0.9 * width, 0.9 * height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (sinButton.getFgColors()[BStyles::Status::normal]));
	cairo_set_line_width (cr, 2.0);

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_move_to (cr, 0.1 * width, 0.8 * height);
	cairo_line_to (cr, 0.15 * width, 0.8 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.15 + i * 0.03) * width, (0.5 - 0.3 * sin (double (i - 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.55 * width, 0.2 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.55 + i * 0.03) * width, (0.5 - 0.3 * sin (double (i + 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.9 * width, 0.8 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);
}

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;

	if (strcmp(plugin_uri, BCHOPPR_URI) != 0)
	{
		std::cerr << "BChoppr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BChoppr.lv2#GUI: No parent window.\n";

	// New instance
	BChoppr_GUI* ui;
	try {ui = new BChoppr_GUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BChoppr.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;
	*widget = (LV2UI_Widget) ui->getNativeView ();
	ui->send_record_on();
	return (LV2UI_Handle) ui;
}

static void cleanup(LV2UI_Handle ui)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	if (pluginGui) delete pluginGui;
}

static void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	if (pluginGui) pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BChoppr_GUI* pluginGui = (BChoppr_GUI*) ui;
	if (pluginGui) pluginGui->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = {callIdle};

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

static const LV2UI_Descriptor guiDescriptor = {
		BCHOPPR_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
