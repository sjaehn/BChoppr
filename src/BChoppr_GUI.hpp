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

#ifndef BCHOPPR_GUI_HPP_
#define BCHOPPR_GUI_HPP_


#include "BWidgets/BStyles/Status.hpp"
#include "BWidgets/BStyles/Style.hpp"
#include "BWidgets/BStyles/Types/Border.hpp"
#include "BWidgets/BStyles/Types/ColorMap.hpp"
#include "BWidgets/BStyles/Types/Fill.hpp"
#include "BWidgets/BStyles/Types/Font.hpp"
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include "BWidgets/BUtilities/Dictionary.hpp"
#include "BWidgets/BWidgets/Widget.hpp"
#include "BWidgets/BWidgets/Window.hpp"
#include "BWidgets/BWidgets/Label.hpp"
#include "BWidgets/BWidgets/HSwitch.hpp"
#include "BWidgets/BWidgets/VSlider.hpp"
#include "BWidgets/BWidgets/ValueVSlider.hpp"
#include "BWidgets/BWidgets/ValueHSlider.hpp"
#include "BWidgets/BWidgets/ValueDial.hpp"
#include "BWidgets/BWidgets/ListBox.hpp"
#include "BWidgets/BWidgets/ComboBox.hpp"
#include "BWidgets/BWidgets/Button.hpp"
#include "BWidgets/BWidgets/TextButton.hpp"
#include "BWidgets/BWidgets/Knob.hpp"
#include "BWidgets/BWidgets/Image.hpp"
#include "BWidgets/BWidgets/ImageButton.hpp"
#include "Ports.hpp"
#include "Marker.hpp"
#include "Monitor.hpp"
#include "SwingHSlider.hpp"
#include "definitions.hpp"
#include "Urids.hpp"

#ifndef MESSAGENR_
#define MESSAGENR_
enum MessageNr
{
	NO_MSG		= 0,
	JACK_STOP_MSG	= 1,
	MAX_MSG		= 1
};
#endif /* MESSAGENR_ */

#define URID(x) (BURID(BCHOPPR_GUI_URI x))

#define BG_FILE "surface.png"
#define HELP_URL "https://github.com/sjaehn/BChoppr/blob/master/README.md"
#define YT_URL " https://youtu.be/PuzoxiAs-h8"
#define WWW_BROWSER_CMD "x-www-browser"

const std::string messageStrings[MAX_MSG + 1] =
{
	"",
	"Jack transport off or halted."
};

class BChoppr_GUI : public BWidgets::Window
{
public:
	BChoppr_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow);
	~BChoppr_GUI ();
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	void send_record_on ();
	void send_record_off ();
	void sendSharedDataNr ();
	void sendController (const int nr, const float value);
	virtual void onConfigureRequest (BEvents::Event* event) override;

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
	float setController (const int nr, const double value);
	void setMarker (const int markerNr, double value);
	void setAutoMarkers ();
	void rearrange_controllers ();
	void recalculateEnterEdit ();
	static void valueChangedCallback (BEvents::Event* event);
    static void markerClickedCallback (BEvents::Event* event);
	static void markerDraggedCallback (BEvents::Event* event);
	static void listBoxChangedCallback (BEvents::Event* event);
	static void markersAutoClickedCallback (BEvents::Event* event);
	static void buttonClickedCallback (BEvents::Event* event);
	static void sharedDataClickedCallback (BEvents::Event* event);
	static void helpButtonClickedCallback (BEvents::Event* event);
	static void ytButtonClickedCallback (BEvents::Event* event);
	static void stepControlLabelMessageCallback (BEvents::Event* event);
	static void enterListBoxChangedCallback (BEvents::Event* event);
    static void enterOkClickedCallback (BEvents::Event* event);
	bool init_Stepshape ();
	void destroy_Stepshape ();
	void redrawStepshape ();
	void redrawSContainer ();
    void redrawButtons ();

	BWidgets::Image bgImage;
	BWidgets::Widget rContainer;
	BWidgets::Image sContainer;
	BWidgets::HSwitch monitorSwitch;
	BWidgets::Label monitorLabel;
	BWidgets::Knob bypassButton;
	BWidgets::Label bypassLabel;
	BWidgets::Dial drywetDial;
	BWidgets::Label drywetLabel;
	BWidgets::Button helpButton;
	BWidgets::Button ytButton;
	BWidgets::ImageButton rectButton;
	BWidgets::ImageButton sinButton;
	BWidgets::Image stepshapeDisplay;
	BWidgets::ValueDial attackControl;
	BWidgets::Label attackLabel;
	BWidgets::ValueDial releaseControl;
	BWidgets::Label releaseLabel;
	Monitor monitorDisplay;
	BWidgets::ValueHSlider sequencesperbarControl;
	BWidgets::Label sequencesperbarLabel;
	SwingHSlider ampSwingControl;
	BWidgets::Label ampSwingLabel;
	SwingHSlider swingControl;
	BWidgets::Label swingLabel;
	BWidgets::TextButton markersAutoButton;
	BWidgets::Label markersAutoLabel;
	BWidgets::ValueHSlider nrStepsControl;
	BWidgets::Label nrStepsLabel;
	BWidgets::Label stepshapeLabel;
	BWidgets::Label sequencemonitorLabel;
	BWidgets::Label messageLabel;
	std::array<BWidgets::VSlider*, MAXSTEPS> stepLevelControl;
	std::array<BWidgets::Dial*, MAXSTEPS> stepPanControl;
	std::array<BWidgets::EditLabel*, MAXSTEPS> stepLevelControlLabel;
	std::array<BWidgets::EditLabel*, MAXSTEPS> stepPanControlLabel;
	std::array<Marker*, MAXSTEPS - 1> markerWidgets;
	BWidgets::ListBox markerListBox;
	BWidgets::Widget enterFrame;
	BWidgets::ComboBox enterPositionComboBox;
	BWidgets::EditLabel enterEdit;
	BWidgets::ComboBox enterSequencesComboBox;
	BWidgets::TextButton enterOkButton;
	std::array<BWidgets::Button*, 4> sharedDataButtons;
	BWidgets::HScale sharedDataDummy;


	cairo_surface_t* surface;
	cairo_pattern_t* pat5;

	std::string pluginPath;
	std::array<BWidgets::Widget*, NrControllers> controllers;
    float scale;

	LV2_Atom_Forge forge;
	BChopprURIs uris;
	LV2_URID_Map* map;


	// Definition of styles
	BStyles::ColorMap fgColors = {{0.0, 0.75, 0.2, 1.0}, {0.2, 1.0, 0.6, 1.0}, {0.0, 0.2, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}};
	BStyles::ColorMap bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap txColors = {{0.0, 1.0, 0.4, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.2, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}};
	BStyles::ColorMap monColors = {{0.0, 1.0, 0.4, 1.0}, {0.8, 0.6, 0.2, 1.0}, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}};

	BStyles::Border border = {{fgColors[BStyles::STATUS_NORMAL], 1.0}, 0.0, 2.0, 0.0};

	BStyles::Fill screenBg = BStyles::Fill (BStyles::Color (0.0, 0.0, 0.0, 0.75));

	BStyles::Font defaultFont = BStyles::Font ("sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0, BStyles::Font::TEXT_ALIGN_CENTER, BStyles::Font::TEXT_VALIGN_MIDDLE);
	BStyles::Font smFont = BStyles::Font ("sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 10.0, BStyles::Font::TEXT_ALIGN_CENTER, BStyles::Font::TEXT_VALIGN_MIDDLE);
	BStyles::Theme theme =
	{
		// rcontainer
		{
			URID ("/rcontainer"), 
			BStyles::Style ({{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(border)}})
		},

		// smonitor
		{
			URID ("/smonitor"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(screenBg)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(border)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(monColors)},
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(BStyles::greys)}
			})
		},

		// mmonitor
		{
			URID ("/mmonitor"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(BStyles::blackFill)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(BStyles::noBorder)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(monColors)},
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(BStyles::greys)}
			})
		},

		// redbutton
		{
			URID ("/redbutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(bgColors)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(BStyles::reds)}
			})
		},

		// invbutton
		{
			URID ("/invbutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>({BStyles::noFill})}
			})
		},

		// halobutton
		{
			URID ("/halobutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>({BStyles::noFill})},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(border)}
			})
		},

		// blendbutton
		{
			URID ("/blendbutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>({BStyles::noFill})},
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>({BStyles::invisible})},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// label
		{
			URID ("/label"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(defaultFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// smlabel
		{
			URID ("/smlabel"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(smFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// dial
		{
			URID ("/dial"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(BStyles::darks)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(fgColors)},
				{URID ("/dial/label"), BUtilities::makeAny<BStyles::Style>
					({
						{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(smFont)},
						{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
					})}
			})
		},

		// menu
		{
			URID ("/menu"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(screenBg)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(border)}
			})
		},

		// menu
		{
			URID ("/menu/listbox"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(screenBg)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(border)}
			})
		}

	};
};

#endif /* BCHOPPR_GUI_HPP_ */
