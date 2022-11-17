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
#include "BWidgets/Dial.hpp"
#include "Ports.hpp"
#include "screen.h"
#include "BUtilities/stof.hpp"
#include "BUtilities/vsystem.hpp"


BChoppr_GUI::BChoppr_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (820, 560, "B.Choppr", parentWindow, true, PUGL_MODULE, 0),
	controller (NULL), write_function (NULL),

	mContainer (0, 0, 820, 560, "main"),
	rContainer (260, 80, 540, 350, "rcontainer"),
	sContainer (3, 165, 534, 182, "scontainer"),
	monitorSwitch (660, 15, 40, 16, "switch", 0.0),
	monitorLabel (660, 35, 40, 20, "smlabel", BCHOPPR_LABEL_MONITOR),
	bypassButton (722, 15, 16, 16, "redbutton"),
	bypassLabel (710, 35, 40, 20, "smlabel", BCHOPPR_LABEL_BYPASS),
	drywetDial (763, 5, 33, 40, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f"),
	drywetLabel (750, 35, 60, 20, "smlabel", BCHOPPR_LABEL_DRY_WET),
	helpButton (20, 80, 24, 24, "halobutton", BCHOPPR_LABEL_HELP),
	ytButton (50, 80, 24, 24, "halobutton", BCHOPPR_LABEL_TUTORIAL),
	monitorDisplay (3, 3, 534, 162, "mmonitor"),
	blendControl (0, 0, 0, 0, "widget", 1, 1, 2, 1),
	rectButton (40, 240, 60, 40, "abutton"),
	sinButton (140, 240, 60, 40, "nbutton"),
	stepshapeDisplay (30, 290, 180, 140, "smonitor"),
	attackControl (40, 445, 50, 60, "dial", 0.2, 0.01, 1.0, 0.01, "%1.2f"),
	attackLabel (20, 500, 90, 20, "label", BCHOPPR_LABEL_ATTACK),
	releaseControl (150, 445, 50, 60, "dial", 0.2, 0.01, 1.0, -0.01, "%1.2f"),
	releaseLabel (130, 500, 90, 20, "label", BCHOPPR_LABEL_DECAY),
	sequencesperbarControl (260, 442, 120, 28, "slider", 1.0, 1.0, 8.0, 1.0, "%1.0f"),
	sequencesperbarLabel (260, 470, 120, 20, "label", BCHOPPR_LABEL_SEQUENCES_PER_BAR),
	ampSwingControl
	(
		420, 442, 110, 28, "slider", 1.0, 0.001, 1000.0, 0.0, "%4.1f",
		[] (const double val, const double min, const double max)
		{return (val >= 1.0 ? 1.0 - 0.5 / LIMIT (sqrt(val), sqrt(min), sqrt(max)) : 0.5 * LIMIT (sqrt(val), sqrt(min), sqrt(max)));},
		[] (const double frac, const double min, const double max)
		{return (frac >= 0.5 ? pow (0.5 / (1.0 - LIMIT (frac, 0, 1)), 2) : pow (2 * LIMIT (frac, 0, 1), 2));}
	),
	ampSwingLabel (420, 470, 110, 20, "label", BCHOPPR_LABEL_AMP_SWING),
	swingControl (565, 442, 110, 28, "slider", 1.0, 1.0 / 3.0, 3.0, 0.0),
	swingLabel (565, 470, 110, 20, "label", BCHOPPR_LABEL_STEPS_SWING),
	markersAutoButton (715, 450, 80, 20, "button", BCHOPPR_LABEL_AUTO),
	markersAutoLabel (715, 470, 80, 20, "label", BCHOPPR_LABEL_MARKER),
	nrStepsControl (260, 502, 540, 28, "slider", 1.0, 1.0, MAXSTEPS, 1.0, "%2.0f"),
	nrStepsLabel (260, 530, 540, 20, "label", BCHOPPR_LABEL_NUMBER_OF_STEPS),
	stepshapeLabel (33, 293, 120, 20, "llabel", BCHOPPR_LABEL_STEP_SHAPE),
	sequencemonitorLabel (263, 83, 120, 20, "llabel", BCHOPPR_LABEL_SEQUENCE_MONITOR),
	messageLabel (420, 83, 280, 20, "hilabel", ""),
	markerListBox (12, -68, 86, 66, "listbox", BItems::ItemList ({BCHOPPR_LABEL_MARKER, BCHOPPR_LABEL_MANUAL})),
	sharedDataSelection (28, 528, 194, 24, "widget", 0, 0, 4, 1),

	surface (NULL), cr1 (NULL), cr2 (NULL), cr3 (NULL), cr4 (NULL), pat1 (NULL), pat2 (NULL), pat3 (NULL), pat4 (NULL), pat5 (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),  sz (1.0), bgImageSurface (nullptr),
	scale (DB_CO(0.0)),
	map (NULL)

{
	if (!init_mainMonitor () || !init_Stepshape ())
	{
		std::cerr << "BChoppr.lv2#GUI: Failed to init monitor." <<  std::endl;
		destroy_mainMonitor ();
		destroy_Stepshape ();
		throw std::bad_alloc ();
	}

	//Initialialize and configure step controllers
	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepLevelControl[i] = BWidgets::VSlider ((i + 0.5) * sw / MAXSTEPS + sx - 7, 60, 14, 75, "slider", 1.0, 0.0, 1.0, 0.01);
		stepLevelControl[i].setHardChangeable (false);
		stepLevelControl[i].setScrollable (true);
		stepLevelControl[i].applyTheme (theme, "slider");
		sContainer.add (stepLevelControl[i]);

		stepLevelControlLabel[i] = BWidgets::Label ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 28, 20, "mlabel", "1.00");
		stepLevelControlLabel[i].applyTheme (theme, "mlabel");
		stepLevelControlLabel[i].setState (BColors::ACTIVE);
		stepLevelControlLabel[i].setEditable (true);
		stepLevelControlLabel[i].setCallbackFunction(BEvents::EventType::MESSAGE_EVENT, stepControlLabelMessageCallback);
		sContainer.add (stepLevelControlLabel[i]);

		stepPanControl[i] = BWidgets::Dial ((i + 0.5) * sw / MAXSTEPS + sx - 15, 135, 30, 30, "slider", 0.0, -1.0, 1.0, 0.01);
		stepPanControl[i].setHardChangeable (false);
		stepPanControl[i].setScrollable (true);
		stepPanControl[i].applyTheme (theme, "slider");
		sContainer.add (stepPanControl[i]);

		stepPanControlLabel[i] = BWidgets::Label ((i + 0.5) * sw / MAXSTEPS + sx - 14, 40, 165, 20, "mlabel", "1.00");
		stepPanControlLabel[i].applyTheme (theme, "mlabel");
		stepPanControlLabel[i].setState (BColors::ACTIVE);
		stepPanControlLabel[i].setEditable (true);
		stepPanControlLabel[i].setCallbackFunction(BEvents::EventType::MESSAGE_EVENT, stepControlLabelMessageCallback);
		sContainer.add (stepPanControlLabel[i]);

		
	}

	//Initialialize and configure markers
	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		markerWidgets[i] = Marker ((i + 1) * sw / MAXSTEPS + sx - 5, 10, 10, 16, "marker", (double(i) + 1.0) / MAXSTEPS, 0.0, 1.0, 0.0);
		markerWidgets[i].setHasValue (false);
		markerWidgets[i].setDraggable (true);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::markerClickedCallback);
		markerWidgets[i].setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BChoppr_GUI::markerDraggedCallback);
		markerWidgets[i].applyTheme (theme, "slider");
		sContainer.add (markerWidgets[i]);
	}

	for (int i = 0; i < 4; ++i) sharedDataButtons[i] = HaloToggleButton
	(50 * i, 0, 44, 24, "halobutton", BCHOPPR_LABEL_SHARED_DATA " " + std::to_string (i + 1));

	// Link controllers
	controllers[Bypass - Controllers] = &bypassButton;
	controllers[DryWet - Controllers] = &drywetDial;
	controllers[Blend - Controllers] = &blendControl;
	controllers[Attack - Controllers] = &attackControl;
	controllers[Release - Controllers] = &releaseControl;
	controllers[SequencesPerBar - Controllers] = &sequencesperbarControl;
	controllers[AmpSwing - Controllers] = &ampSwingControl;
	controllers[Swing - Controllers] = &swingControl;
	controllers[NrSteps - Controllers] = &nrStepsControl;
	for (int i = 0; i < MAXSTEPS - 1; ++i) controllers[StepPositions + i - Controllers] = &markerWidgets[i];
	for (int i = 0; i < MAXSTEPS; ++i) controllers[StepLevels + i - Controllers] = &stepLevelControl[i];
	for (int i = 0; i < MAXSTEPS; ++i) controllers[StepPans + i - Controllers] = &stepPanControl[i];

	// Set callbacks
	for (int i = 0; i < NrControllers; ++i) controllers[i]->setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	monitorSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);
	monitorDisplay.setCallbackFunction (BEvents::EventType::WHEEL_SCROLL_EVENT, BChoppr_GUI::monitorScrolledCallback);
	monitorDisplay.setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BChoppr_GUI::monitorDraggedCallback);
	markerListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::listBoxChangedCallback);
	markersAutoButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::markersAutoClickedCallback);
	rectButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::buttonClickedCallback);
	sinButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::buttonClickedCallback);
	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpButtonClickedCallback);
	ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytButtonClickedCallback);
	for (HaloToggleButton& s: sharedDataButtons) s.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BChoppr_GUI::sharedDataClickedCallback);
	sharedDataSelection.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BChoppr_GUI::valueChangedCallback);

	// Configure widgets
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	drywetDial.setScrollable (true);
	drywetDial.setHardChangeable (false);
	attackControl.setScrollable (true);
	attackControl.setHardChangeable (false);
	releaseControl.setScrollable (true);
	releaseControl.setHardChangeable (false);
	sequencesperbarControl.setScrollable (true);
	ampSwingControl.setHardChangeable (false);
	swingControl.setHardChangeable (false);
	nrStepsControl.setScrollable (true);
	monitorDisplay.setScrollable (true);
	monitorDisplay.setDraggable (true);
	markerListBox.setStacking (BWidgets::STACKING_OVERSIZE);
	applyTheme (theme);


	setAutoMarkers ();
	rearrange_controllers ();
	redrawMainMonitor ();
	redrawSContainer ();
	redrawButtons ();

	// Pack widgets
	mContainer.add (rContainer);
	rContainer.add (monitorDisplay);
	rContainer.add (sContainer);
	mContainer.add (blendControl);
	mContainer.add (monitorSwitch);
	mContainer.add (monitorLabel);
	mContainer.add (bypassButton);
	mContainer.add (bypassLabel);
	mContainer.add (drywetDial);
	mContainer.add (drywetLabel);
	mContainer.add (helpButton);
	mContainer.add (ytButton);
	mContainer.add (rectButton);
	mContainer.add (sinButton);
	mContainer.add (stepshapeDisplay);
	mContainer.add (attackControl);
	mContainer.add (attackLabel);
	mContainer.add (releaseControl);
	mContainer.add (releaseLabel);
	mContainer.add (sequencesperbarControl);
	mContainer.add (sequencesperbarLabel);
	mContainer.add (ampSwingControl);
	mContainer.add (ampSwingLabel);
	mContainer.add (swingControl);
	mContainer.add (swingLabel);
	mContainer.add (markersAutoButton);
	mContainer.add (markersAutoLabel);
	mContainer.add (nrStepsControl);
	mContainer.add (nrStepsLabel);
	mContainer.add (stepshapeLabel);
	mContainer.add (sequencemonitorLabel);
	mContainer.add (messageLabel);
	for (HaloToggleButton& s : sharedDataButtons) sharedDataSelection.add (s);
	mContainer.add (sharedDataSelection);
	add (mContainer);

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
	destroy_mainMonitor ();
	destroy_Stepshape ();
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
					if ((nr >= 0) && (nr <= 4) && (nr != sharedDataSelection.getValue()))
					{
						sharedDataSelection.setValueable (false);
						sharedDataSelection.setValue (nr);
						sharedDataSelection.setValueable (true);

						for (int i = 0; i < 4; ++i)
						{
							sharedDataButtons[i].setValueable (false);
							sharedDataButtons[i].setValue (i == nr - 1 ? 1 : 0);
							sharedDataButtons[i].setValueable (true);
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
						setMarker (nr - (StepPositions - Controllers), val);
						setAutoMarkers ();
						rearrange_controllers ();
						redrawSContainer ();
						redrawMainMonitor ();
					}

					else setController (nr, val);
				}
			}

			// Monitor notification
			else if (obj->body.otype == uris.notify_event)
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
							add_monitor_data (notifications, notificationsCount, mainMonitor.horizonPos);
							redrawMainMonitor ();
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
					std::string msg = ((messageNr >= NO_MSG) && (messageNr <= MAX_MSG) ? messageStrings[messageNr] : "");
					messageLabel.setText (msg);
				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port_index >= Controllers) && (port_index < Controllers + NrControllers) && (sharedDataSelection.getValue() == 0))
	{
		int nr = port_index - Controllers;
		float val = *(float*) buffer;
		if ((nr >= StepPositions - Controllers) && (nr < StepPositions - Controllers + MAXSTEPS - 1))
		{
			setMarker (nr - (StepPositions - Controllers), val);
			setAutoMarkers ();
			rearrange_controllers ();
			redrawSContainer ();
			redrawMainMonitor();
		}

		else setController (nr, val);
	}
}

void BChoppr_GUI::resizeGUI()
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12 * sz);
	leftFont.setFontSize (12 * sz);
	mdFont.setFontSize (10 * sz);
	smFont.setFontSize (8 * sz);

	// Resize Background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 820 * sz, 560 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 820, 560, sz);
	RESIZE (rContainer, 260, 80, 540, 350, sz);
	RESIZE (monitorSwitch, 660, 15, 40, 16, sz);
	RESIZE (monitorLabel, 660, 35, 40, 20, sz);
	RESIZE (bypassButton, 722, 15, 16, 16, sz);
	RESIZE (bypassLabel, 710, 35, 40, 20, sz);
	RESIZE (drywetDial, 763, 5, 33, 40, sz);
	RESIZE (drywetLabel, 750, 35, 60, 20, sz);
	RESIZE (helpButton, 20, 80, 24, 24, sz);
	RESIZE (ytButton, 50, 80, 24, 24, sz);
	RESIZE (monitorDisplay, 3, 3, 534, 162, sz);
	RESIZE (blendControl, 0, 0, 0, 0, sz);
	RESIZE (rectButton, 40, 240, 60, 40, sz);
	RESIZE (sinButton, 140, 240, 60, 40, sz);
	RESIZE (stepshapeDisplay, 30, 290, 180, 140, sz);
	RESIZE (attackControl, 40, 445, 50, 60, sz);
	RESIZE (attackLabel, 20, 500, 90, 20, sz);
	RESIZE (releaseControl, 150, 445, 50, 60, sz);
	RESIZE (releaseLabel, 130, 500, 90, 20, sz);
	RESIZE (sequencesperbarControl, 260, 442, 120, 28, sz);
	RESIZE (sequencesperbarLabel, 260, 470, 120, 20, sz);
	RESIZE (ampSwingControl, 420, 442, 110, 28, sz);
	RESIZE (ampSwingLabel, 420, 470, 110, 20, sz);
	RESIZE (swingControl, 565, 442, 110, 28, sz);
	RESIZE (swingLabel, 565, 470, 110, 20, sz);
	RESIZE (markersAutoButton, 715, 450, 80, 20, sz);
	RESIZE (markersAutoLabel, 715, 470, 80, 20, sz);
	RESIZE (nrStepsControl, 260, 502, 540, 28, sz);
	RESIZE (nrStepsLabel, 260, 530, 540, 20, sz);
	RESIZE (stepshapeLabel, 33, 293, 120, 20, sz);
	RESIZE (sequencemonitorLabel, 263, 83, 120, 20, sz);
	RESIZE (messageLabel, 420, 83, 280, 20,sz);
	RESIZE (sContainer, 3, 165, 534, 182, sz);
	RESIZE (markerListBox, 12, -68, 86, 66, sz);
	markerListBox.resizeItems (BUtilities::Point (80 * sz, 20 * sz));
	RESIZE (sharedDataSelection, 28, 528, 194, 24, sz);
	for (int i = 0; i < 4; ++i) {RESIZE (sharedDataButtons[i], 50 * i, 0, 44, 24, sz);}

	// Update monitors
	destroy_Stepshape ();
	init_Stepshape ();
	redrawStepshape ();
	destroy_mainMonitor ();
	init_mainMonitor ();
	redrawMainMonitor ();
	redrawSContainer ();
	rearrange_controllers ();
	redrawButtons ();

	// Apply changes
	applyTheme (theme);
	show ();
}

void BChoppr_GUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);
	rContainer.applyTheme (theme);
	monitorSwitch.applyTheme (theme);
	monitorLabel.applyTheme (theme);
	bypassButton.applyTheme (theme);
	bypassLabel.applyTheme (theme);
	drywetDial.applyTheme (theme);
	drywetLabel.applyTheme (theme);
	helpButton.applyTheme (theme);
	ytButton.applyTheme (theme);
	monitorDisplay.applyTheme (theme);
	blendControl.applyTheme (theme);
	rectButton.applyTheme (theme);
	sinButton.applyTheme (theme);
	stepshapeDisplay.applyTheme (theme);
	attackControl.applyTheme (theme);
	attackLabel.applyTheme (theme);
	releaseControl.applyTheme (theme);
	releaseLabel.applyTheme (theme);
	sequencesperbarControl.applyTheme (theme);
	sequencesperbarLabel.applyTheme (theme);
	ampSwingControl.applyTheme (theme);
	ampSwingLabel.applyTheme (theme);
	swingControl.applyTheme (theme);
	swingLabel.applyTheme (theme);
	markersAutoButton.applyTheme (theme);
	markersAutoLabel.applyTheme (theme);
	nrStepsControl.applyTheme (theme);
	nrStepsLabel.applyTheme (theme);
	stepshapeLabel.applyTheme (theme);
	sequencemonitorLabel.applyTheme (theme);
	messageLabel.applyTheme (theme);
	sContainer.applyTheme (theme);
	markerListBox.applyTheme (theme);
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		stepLevelControl[i].applyTheme (theme);
		stepPanControl[i].applyTheme (theme);
		stepLevelControlLabel[i].applyTheme (theme);
		stepPanControlLabel[i].applyTheme (theme);
	}
	sharedDataSelection.applyTheme (theme);
	for (HaloToggleButton& s : sharedDataButtons) s.applyTheme (theme);
}

void BChoppr_GUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (getWidth() / 820 > getHeight() / 560 ? getHeight() / 560 : getWidth() / 820);
	resizeGUI ();
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
	lv2_atom_forge_int (&forge, sharedDataSelection.getValue());
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
	controllers[nr]->setValueable (false);
	controllers[nr]->setValue (value);
	controllers[nr]->setValueable (true);

	if (nr == Blend - Controllers)
	{
		if (value == 1) {rectButton.rename ("abutton"); sinButton.rename ("nbutton");}
		else if (value == 2) {sinButton.rename ("abutton"); rectButton.rename ("nbutton");}
		rectButton.applyTheme (theme);
		sinButton.applyTheme (theme);
		redrawButtons ();
		redrawStepshape ();
	}

	else if ((nr == Attack - Controllers) || (nr == Release - Controllers)) redrawStepshape ();

	else if (nr == AmpSwing - Controllers) rearrange_controllers();

	else if (nr == Swing - Controllers)
	{
		setAutoMarkers();
		rearrange_controllers();
		redrawSContainer();
		redrawMainMonitor();
	}

	else if (nr == NrSteps - Controllers)
	{
		setAutoMarkers();
		rearrange_controllers();
		redrawSContainer();
		redrawMainMonitor();
	}

	else if ((nr >= StepPositions - Controllers) and (nr < StepPositions - Controllers + MAXSTEPS - 1))
	{
		return (((Marker*)controllers[nr])->hasValue() ? value : 0.0f);
	}

	else if ((nr >= StepLevels - Controllers) and (nr < StepLevels - Controllers + MAXSTEPS))
	{
		stepLevelControlLabel[nr - (StepLevels - Controllers)].setText (BUtilities::to_string (value, "%1.2f"));
	}

	else if ((nr >= StepPans - Controllers) and (nr < StepPans - Controllers + MAXSTEPS))
	{
		stepPanControlLabel[nr - (StepPans - Controllers)].setText (BUtilities::to_string (value, "%1.2f"));
	}

	return value;
}

void BChoppr_GUI::setMarker (const int markerNr, double value)
{
	if ((markerNr < 0) || (markerNr >= MAXSTEPS - 1)) return;

	// Value 0.0: Automatic
	if (value == 0.0)
	{
		markerWidgets[markerNr].setHasValue (false);
	}

	else
	{
		// Set value and switch off automatic
		value = LIMIT (value, MINMARKERVALUE, 1.0);
		markerWidgets[markerNr].setHasValue (true);
		markerWidgets[markerNr].setValue (value);

		// Validate ancessors
		for (int i = markerNr - 1; i >= 0; --i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() > value) markerWidgets[i].setValue (value);
				else break;
			}
		}

		// Validate successors
		for (int i = markerNr + 1; i < MAXSTEPS - 1; ++i)
		{
			if (markerWidgets[i].hasValue())
			{
				if (markerWidgets[i].getValue() < value) markerWidgets[i].setValue (value);
				else break;
			}
		}
	}
}

void BChoppr_GUI::setAutoMarkers ()
{
	int nrMarkers = nrStepsControl.getValue() - 1;
	int start = 0;
	for (int i = 0; i < nrMarkers; ++i)
	{
		if (!markerWidgets[i].hasValue())
		{
			if ((i == nrMarkers - 1) || (markerWidgets[i + 1].hasValue()))
			{
				double swing = 2.0 * swingControl.getValue() / (swingControl.getValue() + 1.0);
				double anc = (start == 0 ? 0 : markerWidgets[start - 1].getValue());
				double suc = (i == nrMarkers - 1 ? 1 : markerWidgets[i + 1].getValue());
				double diff = suc - anc;
				double dist = i - start + 1.0 + (int (i - start) & 1 ? ((start & 1) ? 2.0 - swing : swing) : 1.0);
				double step = (diff < 0 ? 0 : diff / dist);
				for (int j = start; j <= i; ++j)
				{
					double f = ((j & 1) ? 2.0 - swing : swing);
					anc += f * step;
					markerWidgets[j].setValue (anc);
				}
			}
		}
		else start = i + 1;
	}
}

void BChoppr_GUI::rearrange_controllers ()
{
	int nrStepsi = INT (nrStepsControl.getValue());

	if ((nrStepsi < 1) || (nrStepsi > MAXSTEPS)) return;

	double sw = sContainer.getEffectiveWidth();
	double sx = sContainer.getXOffset();
	const double oddf = (ampSwingControl.getValue() >= 1.0 ? 1.0 : ampSwingControl.getValue());
	const double evenf = (ampSwingControl.getValue() >= 1.0 ? 1.0 / ampSwingControl.getValue() : 1.0);

	for (int i = 0; i < MAXSTEPS; ++i)
	{
		if (i < nrStepsi)
		{
			stepLevelControl[i].resize (14 * sz, (14 + LIMIT (66 * ((i % 2) == 0 ? oddf : evenf), 0, 61 )) * sz);
			stepLevelControl[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 7 * sz, 135 * sz - stepLevelControl[i].getHeight());
			stepLevelControl[i].show();

			stepLevelControlLabel[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 14 * sz, 40 * sz);
			stepLevelControlLabel[i].resize (28 * sz, 20 * sz);
			stepLevelControlLabel[i].show();

			stepPanControl[i].resize (30 * sz, 30 * sz);
			stepPanControl[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 15 * sz, 135 * sz);
			stepPanControl[i].show();

			stepPanControlLabel[i].moveTo ((i + 0.5) * sw / nrStepsi + sx - 14 * sz, 165 * sz);
			stepPanControlLabel[i].resize (28 * sz, 20 * sz);
			stepPanControlLabel[i].show();

			if (i < nrStepsi - 1) markerWidgets[i].resize (10 * sz, 16 * sz);
		}
		else
		{
			stepLevelControl[i].hide ();
			stepPanControl[i].hide();
			stepLevelControlLabel[i].hide();
			stepPanControlLabel[i].hide();
		}
	}

	for (int i = 0; i < MAXSTEPS - 1; ++i)
	{
		if (i < nrStepsi - 1)
		{
			markerWidgets[i].moveTo (markerWidgets[i].getValue() * sw + sx - 5 * sz, 10 * sz);
			markerWidgets[i].show ();
		}
		else markerWidgets[i].hide ();
	}
}

void BChoppr_GUI::valueChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
		const double value = widget->getValue();

		if (widget->getMainWindow ())
		{
			BChoppr_GUI* ui = (BChoppr_GUI*) widget->getMainWindow ();

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
				const float v = ui->setController (controllerNr, value);
				if (ui->sharedDataSelection.getValue()) ui->sendController (controllerNr, v);
				else ui->write_function (ui->controller, Controllers + controllerNr, sizeof (float), 0, &v);
			}

			else if (widget == &ui->sharedDataSelection)
			{
				const int val = ui->sharedDataSelection.getValue() - 1;
				for (int i = 0; i < 4; ++i)
				{
					ui->sharedDataButtons[i].setValueable (false);
					ui->sharedDataButtons[i].setValue (i == val ? 1 : 0);
					ui->sharedDataButtons[i].setValueable (true);
				}

				ui->sendSharedDataNr();
			}

			// monitor on/off changed
			else if (widget == &ui->monitorSwitch)
			{
				int value = INT (widget->getValue ());
				if (value == 1)
				{
					ui->mainMonitor.record_on = true;
					ui->send_record_on ();
				}
				else
				{
					ui->mainMonitor.record_on = false;
					ui->send_record_off ();
				}
				return;
			}
		}
	}
}

void BChoppr_GUI::markerClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::RIGHT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	const int nrSteps = ui->nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			Marker* oldMarker = (Marker*) ui->markerListBox.getParent();
			ui->markerListBox.setValue (UNSELECTED);

			if (oldMarker && (oldMarker == marker))
			{
				if (ui->markerListBox.isVisible()) ui->markerListBox.hide();
				else ui->markerListBox.show ();
			}

			else if (oldMarker && (oldMarker != marker))
			{
				oldMarker->release (&ui->markerListBox);
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

			else
			{
				marker->add (ui->markerListBox);
				ui->markerListBox.show();
			}

		}
	}
}

void BChoppr_GUI::markerDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	if (pev->getButton() != BDevices::LEFT_BUTTON) return;
	Marker* marker = (Marker*)event->getWidget();
	if (!marker) return;
	marker->raiseToTop();
	BChoppr_GUI* ui = (BChoppr_GUI*)marker->getMainWindow();
	if (!ui) return;

	const int nrSteps = ui->nrStepsControl.getValue();

	for (int i = 0; i < nrSteps - 1; ++i)
	{
		if (marker == &ui->markerWidgets[i])
		{
			double x0 = ui->sContainer.getXOffset();
			double w = ui->sContainer. getEffectiveWidth();
			double frac = (w > 0 ? (pev->getPosition().x + marker->getPosition().x - x0) / w : MINMARKERVALUE);
			frac = LIMIT (frac, MINMARKERVALUE, 1.0);

			// Limit to antecessors value
			for (int j = i - 1; j >= 0; --j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac < ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			// Limit to successors value
			for (int j = i + 1; j < nrSteps - 1; ++j)
			{
				if (ui->markerWidgets[j].hasValue())
				{
					if (frac > ui->markerWidgets[j].getValue()) frac = ui->markerWidgets[j].getValue();
					break;
				}
			}

			ui->setMarker (i, frac);
			ui->setAutoMarkers();
			ui->rearrange_controllers();
			ui->redrawSContainer();
			ui->redrawMainMonitor();
			break;
		}
	}
}

void BChoppr_GUI::monitorScrolledCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::WheelEvent* wev = (BEvents::WheelEvent*) event;
	BWidgets::Widget* widget = event->getWidget();
	if (!widget) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)widget->getMainWindow();
	if (!ui) return;

	ui->scale += 0.1 * wev->getDelta().y * ui->scale;
	if (ui->scale < 0.0001f) ui->scale = 0.0001f;
	ui->redrawMainMonitor ();
}

void BChoppr_GUI::monitorDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* wev = (BEvents::PointerEvent*) event;
	BWidgets::Widget* widget = event->getWidget();
	if (!widget) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)widget->getMainWindow();
	if (!ui) return;

	ui->scale += 0.01 * wev->getDelta().y * ui->scale;
	if (ui->scale < 0.0001f) ui->scale = 0.0001f;
	ui->redrawMainMonitor ();
}

void BChoppr_GUI::listBoxChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	BWidgets::ListBox* lb = (BWidgets::ListBox*) vev->getWidget();
	if (!lb) return;
	Marker* m = (Marker*) lb->getParent();
	if (!m) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)m->getMainWindow();
	if (!ui) return;

	double value = vev->getValue();
	if (value == 1.0) m->setHasValue (false);
	else if (value == 2.0) m->setHasValue (true);
	else return;

	lb->hide();
	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
	ui->redrawMainMonitor();
}

void BChoppr_GUI::markersAutoClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::ValueChangedEvent* vev = (BEvents::ValueChangedEvent*) event;
	if (vev->getValue() == 0.0) return;
	BWidgets::TextButton* tb = (BWidgets::TextButton*) vev->getWidget();
	if (!tb) return;
	BChoppr_GUI* ui = (BChoppr_GUI*)tb->getMainWindow();
	if (!ui) return;

	for (Marker& m : ui->markerWidgets) m.setHasValue (false);

	ui->setAutoMarkers();
	ui->rearrange_controllers();
	ui->redrawSContainer();
	ui->redrawMainMonitor();
}

void BChoppr_GUI::buttonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::DrawingSurface* w = (BWidgets::DrawingSurface*) event->getWidget();
	if (!w) return;
	BChoppr_GUI* ui = (BChoppr_GUI*) w->getMainWindow();
	if (!ui) return;

	if (w == &ui->rectButton) ui->blendControl.setValue (1);
	else if (w == &ui->sinButton) ui->blendControl.setValue (2);
}

void BChoppr_GUI::sharedDataClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	HaloToggleButton* widget = (HaloToggleButton*) event->getWidget ();
	if (!widget) return;
	double value = widget->getValue();
	BChoppr_GUI* ui = (BChoppr_GUI*) widget->getMainWindow();
	if (!ui) return;

	if (value)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (widget == &ui->sharedDataButtons[i])
			{
				ui->sharedDataSelection.setValue (i + 1);
				return;
			}
		}
	}
	ui->sharedDataSelection.setValue (0);
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

void BChoppr_GUI::stepControlLabelMessageCallback (BEvents::Event* event)
{
	if (event && event->getWidget())
	{
		BWidgets::Label* l = (BWidgets::Label*)event->getWidget();
		BChoppr_GUI* ui = (BChoppr_GUI*)l->getMainWindow();
		if (ui)
		{
			for (int i = 0; i < MAXSTEPS; ++i)
			{
				if (l == &ui->stepLevelControlLabel[i])
				{
					double val = ui->stepLevelControl[i].getValue();
					try {val = BUtilities::stof (l->getText());}
					catch (std::invalid_argument &ia)
					{
						fprintf (stderr, "%s\n", ia.what());
						l->setText (BUtilities::to_string (val, "%1.2f"));
						return;
					}

					ui->stepLevelControl[i].setValue (val);
					l->setText (BUtilities::to_string (ui->stepLevelControl[i].getValue(), "%1.2f"));
					break;
				}

				else if (l == &ui->stepPanControlLabel[i])
				{
					double val = ui->stepLevelControl[i].getValue();
					try {val = BUtilities::stof (l->getText());}
					catch (std::invalid_argument &ia)
					{
						fprintf (stderr, "%s\n", ia.what());
						l->setText (BUtilities::to_string (val, "%1.2f"));
						return;
					}

					ui->stepPanControl[i].setValue (val);
					l->setText (BUtilities::to_string (ui->stepPanControl[i].getValue(), "%1.2f"));
					break;
				}
			}
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
	double width = stepshapeDisplay.getEffectiveWidth ();
	double height = stepshapeDisplay.getEffectiveHeight ();

	cairo_t* cr = cairo_create (stepshapeDisplay.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_fill (cr);
	cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
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
	cairo_set_source_rgba (cr, CAIRO_INK1, 1.0);
	cairo_set_line_width (cr, 3);

	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width * 0.25, 0.9 * height);

	const float attack = attackControl.getValue();
	const float release = releaseControl.getValue();

	if (blendControl.getValue() == 1)
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

	else if (blendControl.getValue() == 2)
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

	cairo_pattern_add_color_stop_rgba (pat5, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat5, 0.9, CAIRO_INK1, 0);
	cairo_set_source (cr, pat5);
	cairo_line_to(cr, 0, 0.9 * height);
	cairo_set_line_width (cr, 0);
	cairo_fill (cr);

	cairo_destroy (cr);

	stepshapeDisplay.update ();
}

bool BChoppr_GUI::init_mainMonitor ()
{
	//Initialize mainMonitor
	mainMonitor.record_on = true;
	mainMonitor.width = 0;
	mainMonitor.height = 0;
	mainMonitor.data.fill (defaultNotification);
	mainMonitor.horizonPos = 0;

	//Initialize mainMonitors cairo data
	double width = monitorDisplay.getEffectiveWidth ();
	double height = monitorDisplay.getEffectiveHeight ();
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	cr1 = cairo_create (surface);
	cr2 = cairo_create (surface);
	cr3 = cairo_create (surface);
	cr4 = cairo_create (surface);
	pat1 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat1, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat1, 0.6, CAIRO_INK1, 0);
	pat2 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat2, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat2, 0.6, CAIRO_INK2, 0);
	pat3 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat3, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat3, 0.6, CAIRO_INK1, 0);
	pat4 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat4, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat4, 0.6, CAIRO_INK2, 0);

	return (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS) &&
			pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS) &&
			pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS) &&
			pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS) &&
			cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS) &&
			cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)&&
			cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS) &&
			cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS) &&
			surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS));
}

void BChoppr_GUI::destroy_mainMonitor ()
{
	//Destroy also mainMonitors cairo data
	if (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat4);
	if (pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat3);
	if (pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat2);
	if (pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat1);
	if (cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr4);
	if (cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr3);
	if (cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr2);
	if (cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr1);
	if (surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS)) cairo_surface_destroy (surface);
}

void BChoppr_GUI::add_monitor_data (BChopprNotifications* notifications, uint32_t notificationsCount, uint32_t& end)
{
	for (uint32_t i = 0; i < notificationsCount; ++i)
	{
		int monitorpos = notifications[i].position;
		if (monitorpos >= MONITORBUFFERSIZE) monitorpos = MONITORBUFFERSIZE;
		if (monitorpos < 0) monitorpos = 0;

		mainMonitor.data[monitorpos].input1 = notifications[i].input1;
		mainMonitor.data[monitorpos].input2 = notifications[i].input2;
		mainMonitor.data[monitorpos].output1 = notifications[i].output1;
		mainMonitor.data[monitorpos].output2 = notifications[i].output2;
		mainMonitor.horizonPos = monitorpos;
	}
}

void BChoppr_GUI::redrawMainMonitor ()
{
	double width = monitorDisplay.getEffectiveWidth ();
	double height = monitorDisplay.getEffectiveHeight ();

	cairo_t* cr = cairo_create (monitorDisplay.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.1 * height);
	cairo_line_to (cr, width, 0.1 * height);
	cairo_move_to (cr, 0, 0.5 * height);
	cairo_line_to (cr, width, 0.5 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	uint32_t steps = (uint32_t) nrStepsControl.getValue() - 1;
	for (uint32_t i = 0; i < steps; ++i)
	{
		cairo_move_to (cr, markerWidgets[i].getValue() * width, 0);
		cairo_rel_line_to (cr, 0, height);
	}
	cairo_stroke (cr);

	if (mainMonitor.record_on)
	{
		cairo_surface_clear (surface);

		// Draw input (cr, cr3) and output (cr2, cr4) curves
		cairo_move_to (cr1, 0, height * (0.5  + (0.4 * LIM ((mainMonitor.data[0].input2 / scale), 0.0f, 1.0f))));
		cairo_move_to (cr2, 0, height * (0.5  + (0.4 * LIM ((mainMonitor.data[0].output2 / scale), 0.0f, 1.0f))));
		cairo_move_to (cr3, 0, height * (0.5  - (0.4 * LIM ((mainMonitor.data[0].input1 / scale), 0.0f, 1.0f))));
		cairo_move_to (cr4, 0, height * (0.5  - (0.4 * LIM ((mainMonitor.data[0].output1 / scale), 0.0f, 1.0f))));

		for (int i = 0; i < MONITORBUFFERSIZE; ++i)
		{
			double pos = ((double) i) / (MONITORBUFFERSIZE - 1.0f);
			cairo_line_to (cr1, pos * width, height * (0.5  + (0.4 * LIM ((mainMonitor.data[i].input2 / scale), 0.0f, 1.0f))));
			cairo_line_to (cr2, pos * width, height * (0.5  + (0.4 * LIM ((mainMonitor.data[i].output2 / scale), 0.0f, 1.0f))));
			cairo_line_to (cr3, pos * width, height * (0.5  - (0.4 * LIM ((mainMonitor.data[i].input1 / scale), 0.0f, 1.0f))));
			cairo_line_to (cr4, pos * width, height * (0.5  - (0.4 * LIM ((mainMonitor.data[i].output1 / scale), 0.0f, 1.0f))));
		}

		// Visualize input (cr, cr3) and output (cr2, cr4) curves
		cairo_set_source_rgba (cr1, CAIRO_INK1, 1.0);
		cairo_set_line_width (cr1, 3);
		cairo_set_source_rgba (cr2, CAIRO_INK2, 1.0);
		cairo_set_line_width (cr2, 3);
		cairo_stroke_preserve (cr1);
		cairo_stroke_preserve (cr2);
		cairo_set_source_rgba (cr3, CAIRO_INK1, 1.0);
		cairo_set_line_width (cr3, 3);
		cairo_set_source_rgba (cr4, CAIRO_INK2, 1.0);
		cairo_set_line_width (cr4, 3);
		cairo_stroke_preserve (cr3);
		cairo_stroke_preserve (cr4);

		// Visualize input (cr, cr3) and output (cr2, cr4) areas under the curves
		cairo_line_to (cr1, width, height * 0.5);
		cairo_line_to (cr1, 0, height * 0.5);
		cairo_close_path (cr1);
		cairo_line_to (cr2, width, height * 0.5);
		cairo_line_to (cr2, 0, height * 0.5);
		cairo_close_path (cr2);
		cairo_set_source (cr1, pat1);
		cairo_set_line_width (cr1, 0);
		cairo_set_source (cr2, pat2);
		cairo_set_line_width (cr2, 0);
		cairo_fill (cr1);
		cairo_fill (cr2);
		cairo_line_to (cr3, width, height * 0.5);
		cairo_line_to (cr3, 0, height * 0.5);
		cairo_close_path (cr3);
		cairo_line_to (cr4, width, height * 0.5);
		cairo_line_to (cr4, 0, height * 0.5);
		cairo_close_path (cr4);
		cairo_set_source (cr3, pat3);
		cairo_set_line_width (cr3, 0);
		cairo_set_source (cr4, pat4);
		cairo_set_line_width (cr4, 0);
		cairo_fill (cr3);
		cairo_fill (cr4);

		// Draw fade out
		double horizon = ((double) mainMonitor.horizonPos) / (MONITORBUFFERSIZE - 1.0f);
		cairo_pattern_t* pat6 = cairo_pattern_create_linear (horizon * width, 0, horizon * width + 63, 0);
		if (cairo_pattern_status (pat6) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_add_color_stop_rgba (pat6, 0.0, CAIRO_BG_COLOR);
			cairo_pattern_add_color_stop_rgba (pat6, 1.0, CAIRO_TRANSPARENT);
			cairo_set_line_width (cr1, 0.0);
			cairo_set_source (cr1, pat6);
			cairo_rectangle (cr1, horizon * width, 0, 63, height);
			cairo_fill (cr1);
			cairo_pattern_destroy (pat6);
		}

		if (horizon * width > width - 63)
		{
			cairo_pattern_t* pat6 = cairo_pattern_create_linear ((horizon - 1) * width, 0, (horizon - 1) * width + 63, 0);
			if (cairo_pattern_status (pat6) == CAIRO_STATUS_SUCCESS)
			{
				cairo_pattern_add_color_stop_rgba (pat6, 0.0, CAIRO_BG_COLOR);
				cairo_pattern_add_color_stop_rgba (pat6, 1.0, CAIRO_TRANSPARENT);
				cairo_set_line_width (cr1, 0.0);
				cairo_set_source (cr1, pat6);
				cairo_rectangle (cr1, (horizon - 1) * width, 0, 63, height);
				cairo_fill (cr1);
				cairo_pattern_destroy (pat6);
			}
		}

		// Draw horizon line
		cairo_set_source_rgba (cr1, CAIRO_FG_COLOR);
		cairo_set_line_width (cr1, 1);
		cairo_move_to (cr1, horizon * width, 0);
		cairo_line_to (cr1, horizon * width, height);
		cairo_stroke (cr1);
	}

	cairo_set_source_surface (cr, surface, 0, 0);
	cairo_paint (cr);

	cairo_destroy (cr);
	monitorDisplay.update ();
}

void BChoppr_GUI::redrawSContainer ()
{
	double width = sContainer.getEffectiveWidth ();
	double height = sContainer.getEffectiveHeight ();

	cairo_surface_clear (sContainer.getDrawingSurface ());
	cairo_t* cr = cairo_create (sContainer.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (BColors::black));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.0, 0.0, 0.0, 0.5);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	for (int i = 0; i < nrStepsControl.getValue() - 1; ++i)
	{
		cairo_set_line_width (cr, 1.0);
		cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));
		cairo_move_to (cr, markerWidgets[i].getValue() * width, 0);
		cairo_rel_line_to (cr, 0, 30 * sz);
		cairo_line_to (cr, (i + 1) / nrStepsControl.getValue() * width, 40 * sz);
		cairo_rel_line_to (cr, 0, 145 * sz);
		cairo_stroke (cr);
	}

	cairo_destroy (cr);
	sContainer.update();
}

void BChoppr_GUI::redrawButtons ()
{
	// rectButton
	double width = rectButton.getEffectiveWidth ();
	double height = rectButton.getEffectiveHeight ();

	cairo_surface_clear (rectButton.getDrawingSurface ());
	cairo_t* cr = cairo_create (rectButton.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_set_source_rgba (cr, CAIRO_RGBA (*rectButton.getBorder()->getLine()->getColor()));
	cairo_set_line_width (cr, 2.0);

	cairo_move_to (cr, 0.05 * width, 0.9 * height);
	cairo_line_to (cr, 0.25 * width, 0.9 * height);
	cairo_line_to (cr, 0.3 * width, 0.1 * height);
	cairo_line_to (cr, 0.7 * width, 0.1 * height);
	cairo_line_to (cr, 0.75 * width, 0.9 * height);
	cairo_line_to (cr, 0.95 * width, 0.9 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

	// sinButton
	width = sinButton.getEffectiveWidth ();
	height = sinButton.getEffectiveHeight ();

	cairo_surface_clear (sinButton.getDrawingSurface ());
	cr = cairo_create (sinButton.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	cairo_set_source_rgba (cr, CAIRO_RGBA (*sinButton.getBorder()->getLine()->getColor()));
	cairo_set_line_width (cr, 2.0);

	cairo_move_to (cr, 0.05 * width, 0.9 * height);
	cairo_line_to (cr, 0.15 * width, 0.9 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.15 + i * 0.03) * width, (0.5 - 0.4 * sin (double (i - 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.55 * width, 0.1 * height);
	for (int i = 0; i <= 10; ++i) cairo_line_to (cr, (0.55 + i * 0.03) * width, (0.5 - 0.4 * sin (double (i + 5) * M_PI / 10)) * height);
	cairo_line_to (cr, 0.95 * width, 0.9 * height);
	cairo_stroke (cr);

	cairo_destroy (cr);

}

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BCHOPPR_URI) != 0)
	{
		std::cerr << "BChoppr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
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

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 880) || (screenHeight < 600)) sz = 0.66;
	if (resize) resize->ui_resize(resize->handle, 820 * sz, 560 * sz);

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());
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

static int callResize (LV2UI_Handle ui, int width, int height)
{
	BChoppr_GUI* self = (BChoppr_GUI*) ui;
	if (!self) return 0;

	BEvents::ExposeEvent* ev = new BEvents::ExposeEvent (self, self, BEvents::CONFIGURE_REQUEST_EVENT, self->getPosition().x, self->getPosition().y, width, height);
	self->addEventToQueue (ev);
	return 0;
}

static const LV2UI_Idle_Interface idle = {callIdle};
static const LV2UI_Resize resize = {nullptr, callResize} ;

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else if(!strcmp(uri, LV2_UI__resize)) return &resize;
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
