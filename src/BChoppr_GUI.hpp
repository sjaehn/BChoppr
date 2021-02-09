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

#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include "BWidgets/Widget.hpp"
#include "BWidgets/Window.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "BWidgets/HSwitch.hpp"
#include "BWidgets/VSlider.hpp"
#include "BWidgets/VSliderValue.hpp"
#include "BWidgets/HSliderValue.hpp"
#include "BWidgets/DialValue.hpp"
#include "BWidgets/ListBox.hpp"
#include "BWidgets/ToggleButton.hpp"
#include "BWidgets/TextButton.hpp"
#include "Ports.hpp"
#include "Marker.hpp"
#include "LightButton.hpp"
#include "SwingHSlider.hpp"
#include "HaloButton.hpp"
#include "HaloToggleButton.hpp"
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

#define SCALEMIN -60
#define SCALEMAX 30
#define CAIRO_BG_COLOR 0.0, 0.0, 0.0, 1.0
#define CAIRO_BG_COLOR2 0.2, 0.2, 0.2, 1.0
#define CAIRO_TRANSPARENT 0.0, 0.0, 0.0, 0.0
#define CAIRO_FG_COLOR 1.0, 1.0, 1.0, 1.0
#define CAIRO_INK1 0.0, 1.0, 0.4
#define CAIRO_INK2 0.8, 0.6, 0.2

#define BG_FILE "surface.png"
#define HELP_URL "https://github.com/sjaehn/BChoppr/blob/master/README.md"
#define YT_URL " https://youtu.be/PuzoxiAs-h8"
#define OPEN_CMD "xdg-open"

#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
#define CO_DB(g) ((g) > 0.0001f ? logf((g)) / 0.05f : -90.0f)
#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))
#define INT(g) (int) (g + 0.5)
#define RESIZE(widget, x, y, w, h, sz) (widget).moveTo ((x) * (sz), (y) * (sz)); (widget).resize ((w) * (sz), (h) * (sz));

const std::string messageStrings[MAX_MSG + 1] =
{
	"",
	"*** Jack transport off or halted. ***"
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
	virtual void onConfigureRequest (BEvents::ExposeEvent* event) override;
	void applyTheme (BStyles::Theme& theme) override;

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
	void resizeGUI ();
        float setController (const int nr, const double value);
        void setMarker (const int markerNr, double value);
        void setAutoMarkers ();
	void rearrange_controllers ();
	static void valueChangedCallback (BEvents::Event* event);
        static void markerClickedCallback (BEvents::Event* event);
	static void markerDraggedCallback (BEvents::Event* event);
        static void monitorScrolledCallback (BEvents::Event* event);
        static void monitorDraggedCallback (BEvents::Event* event);
        static void listBoxChangedCallback (BEvents::Event* event);
        static void markersAutoClickedCallback (BEvents::Event* event);
        static void buttonClickedCallback (BEvents::Event* event);
        static void sharedDataClickedCallback (BEvents::Event* event);
        static void helpButtonClickedCallback (BEvents::Event* event);
	static void ytButtonClickedCallback (BEvents::Event* event);
	static void stepControlLabelMessageCallback (BEvents::Event* event);
	bool init_Stepshape ();
	void destroy_Stepshape ();
	void redrawStepshape ();
	bool init_mainMonitor ();
	void destroy_mainMonitor ();
	void add_monitor_data (BChopprNotifications* notifications, uint32_t notificationsCount, uint32_t& end);
	void redrawMainMonitor ();
	void redrawSContainer ();
        void redrawButtons ();


	BWidgets::Widget mContainer;
        BWidgets::Widget rContainer;
	BWidgets::DrawingSurface sContainer;
	BWidgets::HSwitch monitorSwitch;
	BWidgets::Label monitorLabel;
        LightButton bypassButton;
        BWidgets::Label bypassLabel;
        BWidgets::DialValue drywetDial;
        BWidgets::Label drywetLabel;
        HaloButton helpButton;
        HaloButton ytButton;
	BWidgets::DrawingSurface monitorDisplay;
        BWidgets::RangeWidget blendControl;
        BWidgets::DrawingSurface rectButton;
        BWidgets::DrawingSurface sinButton;
	BWidgets::DrawingSurface stepshapeDisplay;
	BWidgets::DialValue attackControl;
	BWidgets::Label attackLabel;
	BWidgets::DialValue releaseControl;
	BWidgets::Label releaseLabel;
	BWidgets::HSliderValue sequencesperbarControl;
	BWidgets::Label sequencesperbarLabel;
        SwingHSlider ampSwingControl;
	BWidgets::Label ampSwingLabel;
        SwingHSlider swingControl;
	BWidgets::Label swingLabel;
        BWidgets::TextButton markersAutoButton;
        BWidgets::Label markersAutoLabel;
	BWidgets::HSliderValue nrStepsControl;
	BWidgets::Label nrStepsLabel;
	BWidgets::Label stepshapeLabel;
	BWidgets::Label sequencemonitorLabel;
	BWidgets::Label messageLabel;
	std::array<BWidgets::VSlider, MAXSTEPS> stepControl;
        std::array<BWidgets::Label, MAXSTEPS> stepControlLabel;
	std::array<Marker, MAXSTEPS - 1> markerWidgets;
        BWidgets::ListBox markerListBox;
        BWidgets::RangeWidget sharedDataSelection;
        std::array<HaloToggleButton, 4> sharedDataButtons;


	cairo_surface_t* surface;
	cairo_t* cr1;
	cairo_t* cr2;
	cairo_t* cr3;
	cairo_t* cr4;
	cairo_pattern_t* pat1;
	cairo_pattern_t* pat2;
	cairo_pattern_t* pat3;
	cairo_pattern_t* pat4;
	cairo_pattern_t* pat5;

	struct
	{
		bool record_on;
		uint32_t width;
		uint32_t height;
		std::array<BChopprNotifications, MONITORBUFFERSIZE> data;
		uint32_t horizonPos;
	}  mainMonitor;

	std::string pluginPath;
	double sz;
	cairo_surface_t* bgImageSurface;

        std::array<BWidgets::ValueWidget*, NrControllers> controllers;
        float scale;

	LV2_Atom_Forge forge;
	BChopprURIs uris;
	LV2_URID_Map* map;



	// Definition of styles
	BColors::ColorSet fgColors = {{{0.0, 0.75, 0.2, 1.0}, {0.2, 1.0, 0.6, 1.0}, {0.0, 0.2, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
        BColors::ColorSet rdColors = {{{0.75, 0.0, 0.0, 1.0}, {1.0, 0.25, 0.25, 1.0}, {0.2, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{{0.0, 1.0, 0.4, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.5, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::Color ink = {0.0, 0.75, 0.2, 1.0};

	BStyles::Border border = {{ink, 1.0}, 0.0, 2.0, 0.0};
        BStyles::Border actborder = {{{CAIRO_INK1, 1.0}, 2.0}, 0.0, 1.0, 0.0};
        BStyles::Border inactborder = {{BColors::darkgrey, 1.0}, 0.0, 2.0, 0.0};
	BStyles::Border blindborder = {{{0.0, 0.0, 0.0, 0.0}, 1.0}, 0.0, 2.0, 0.0};
        BStyles::Border labelBorder = BStyles::Border (BStyles::noLine, 0.0, 4.0);
        BStyles::Border focusborder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.5), 2.0));
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.75));
	BStyles::Border screenBorder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.75), 4.0));
	BStyles::Font defaultFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
        BStyles::Font mdFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 10.0,
                                         BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
        BStyles::Font smFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 8.0,
                                              BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);

        BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
						       {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
						    {"border", STYLEPTR (&BStyles::noBorder)},
						    {"textcolors", STYLEPTR (&txColors)},
						    {"font", STYLEPTR (&defaultFont)}}};
        BStyles::StyleSet mlStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                                 {"textcolors", STYLEPTR (&fgColors)},
                                                 {"font", STYLEPTR (&mdFont)}}};
        BStyles::StyleSet smStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
                                                 {"border", STYLEPTR (&BStyles::noBorder)},
                                                 {"textcolors", STYLEPTR (&txColors)},
                                                 {"font", STYLEPTR (&smFont)}}};
        BStyles::StyleSet focusStyles = {"labels", {{"background", STYLEPTR (&screenBg)},
                        			    {"border", STYLEPTR (&focusborder)},
                        			    {"textcolors", STYLEPTR (&txColors)},
                        			    {"font", STYLEPTR (&defaultFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"B.Choppr", 	{{"background", STYLEPTR (&BStyles::blackFill)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"main", 	{{"background", STYLEPTR (&widgetBg)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"widget", 	{{"uses", STYLEPTR (&defaultStyles)}}},
		{"mmonitor", 	{{"background", STYLEPTR (&BStyles::blackFill)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
 		{"smonitor", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&border)}}},
 		{"nbutton", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&inactborder)}}},
 		{"abutton", 	{{"background", STYLEPTR (&BStyles::blackFill)},
 				 {"border", STYLEPTR (&actborder)}}},
		{"rcontainer", 	{{"background", STYLEPTR (&BStyles::noFill)},
				 {"border", STYLEPTR (&border)}}},
		{"scontainer", 	{{"background", STYLEPTR (&BStyles::noFill)},
				 {"border", STYLEPTR (&BStyles::noBorder)}}},
                {"listbox",	{{"border", STYLEPTR (&border)},
 				 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"listbox/item",{{"uses", STYLEPTR (&defaultStyles)},
 				 {"border", STYLEPTR (&labelBorder)},
 				 {"textcolors", STYLEPTR (&BColors::whites)},
 				 {"font", STYLEPTR (&defaultFont)}}},
 		{"listbox/button",{{"border", STYLEPTR (&BColors::darks)},
 				 {"background", STYLEPTR (&BStyles::blackFill)},
 			 	 {"bgcolors", STYLEPTR (&BColors::darks)}}},
		{"dial", 	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"redbutton", 	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&rdColors)},
				 {"bgcolors", STYLEPTR (&bgColors)}}},
                {"halobutton", 	{{"uses", STYLEPTR (&defaultStyles)},
 				 {"fgcolors", STYLEPTR (&bgColors)}}},
 		{"halobutton/focus", {{"uses", STYLEPTR (&focusStyles)}}},
		{"dial/focus", 	{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"slider",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)},
				 {"textcolors", STYLEPTR (&fgColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"slider/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"switch",	{{"uses", STYLEPTR (&defaultStyles)},
				 {"fgcolors", STYLEPTR (&fgColors)},
				 {"bgcolors", STYLEPTR (&bgColors)}}},
		{"switch/focus",{{"background", STYLEPTR (&screenBg)},
				 {"border", STYLEPTR (&screenBorder)},
				 {"textcolors", STYLEPTR (&txColors)},
				 {"font", STYLEPTR (&defaultFont)}}},
		{"label",	{{"uses", STYLEPTR (&labelStyles)}}},
                {"mlabel",	{{"uses", STYLEPTR (&mlStyles)}}},
                {"smlabel",	{{"uses", STYLEPTR (&smStyles)}}},
		{"hilabel",	{{"uses", STYLEPTR (&labelStyles)},
				 {"textcolors", STYLEPTR (&BColors::whites)}}},
	});
};

#endif /* BCHOPPR_GUI_HPP_ */
