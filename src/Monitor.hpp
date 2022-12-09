/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 - 2022 by Sven Jähnichen
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

#ifndef MONITOR_HPP_
#define MONITOR_HPP_

#include "BWidgets/BStyles/Status.hpp"
#include "BWidgets/BStyles/Types/Color.hpp"
#include "BWidgets/BWidgets/Widget.hpp"
#include "BWidgets/BWidgets/Supports/Clickable.hpp"
#include "BWidgets/BWidgets/Supports/Draggable.hpp"
#include "BWidgets/BWidgets/Supports/Scrollable.hpp"
#include "BWidgets/BEvents/WheelEvent.hpp"
#include "BWidgets/BWidgets/Label.hpp"
#include "BWidgets/BUtilities/to_string.hpp"
#include "definitions.hpp"
#include <cairo/cairo.h>
#include <cstdint>


#ifndef DEFAULT_MONITOR_WIDTH
#define DEFAULT_MONITOR_WIDTH 10.0
#endif

#ifndef DEFAULT_MONITOR_HEIGHT
#define DEFAULT_MONITOR_HEIGHT 20.0
#endif

/**
 *  @brief A %Monitor is a Widget that draws a visual content. I also
 *  supports and handles pointer clicking, dragging or wheel scrolling.
 *  (e. g., via callbacks).
 */
class Monitor :	public BWidgets::Widget, 
				public BWidgets::Clickable, 
				public BWidgets::Draggable, 
				public BWidgets::Scrollable
{
protected:
   std::array<BChopprNotifications, MONITORBUFFERSIZE> data_;
   std::vector<double> steps_;
   uint32_t horizon_;
   double scale_;

public:

	/**
	 * @brief  Constructs a default %Monitor object.
	 * 
	 */
	Monitor ();

	/**
	 *  @brief  Constructs a default %Monitor object.
	 *  @param URID  URID.
	 *  @param title  %Widget title.
	 */
	Monitor (const uint32_t urid, const std::string& title);

	/**
	 *  @brief  Creates a %Monitor.
	 *  @param x  %Monitor X origin coordinate.
	 *  @param y  %Monitor Y origin coordinate.
	 *  @param width  %Monitor width.
	 *  @param height  %Monitor height.
	 *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %Monitor title (default = "").
	 */
	Monitor	(const double x, const double y, const double width, const double height,
			 uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a clone of the %Monitor. 
	 *  @return  Pointer to the new %Monitor.
	 *
	 *  Creates a clone of this %Monitor by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual Widget* clone () const override; 

	/**
	 *  @brief  Copies from another %Monitor. 
	 *  @param that  Other %Monitor.
	 *
	 *  Copies all properties from another %Monitor. But NOT its linkage.
	 */
	void copy (const Monitor* that);

	/**
     *  @brief  Method called upon pointer drag.
     *  @param event  Passed Event.
     *
     *  Overridable method called from the main window event scheduler upon
     *  a pointer drag. Changes the value and calls the widget static callback
	 *  function.
     */
    virtual void onPointerDragged (BEvents::Event* event) override;
	
	/**
     *  @brief  Method called upon (mouse) wheel scroll.
     *  @param event  Passed Event.
     *
     *  Overridable method called from the main window event scheduler upon
     *  a (mouse) wheel scroll. Increases or decreases the value and calls the
	 *  widget static callback function.
     */
    virtual void onWheelScrolled (BEvents::Event* event) override;

	/**
	 * @brief Set the position of the steps
	 * 
	 * @param steps Vector containing step positions within the range [0,1]
	 */
	virtual void setSteps (const std::vector<double>& steps);

	/**
	 * @brief  Pushes raw data to the monitor to be displayed.
	 * 
	 * @param data  Raw data.
	 */
	virtual void pushData (const std::vector<BChopprNotifications>& data);

protected:

	/**
     *  @brief  Unclipped draw a %Monitor to the surface.
     */
    virtual void draw () override;

    /**
     *  @brief  Clipped draw a %Monitor to the surface.
     *  @param x0  X origin of the clipped area. 
     *  @param y0  Y origin of the clipped area. 
     *  @param width  Width of the clipped area.
     *  @param height  Height of the clipped area. 
     */
    virtual void draw (const double x0, const double y0, const double width, const double height) override;

    /**
     *  @brief  Clipped draw a %Monitor to the surface.
     *  @param area  Clipped area. 
     */
    virtual void draw (const BUtilities::Area<>& area) override;
};

inline Monitor::Monitor () :
	Monitor	(0.0, 0.0, DEFAULT_MONITOR_WIDTH, DEFAULT_MONITOR_HEIGHT, BUTILITIES_URID_UNKNOWN_URID, "")
{

}

inline Monitor::Monitor (const uint32_t urid, const std::string& title) : 
	Monitor	(0.0, 0.0, DEFAULT_MONITOR_WIDTH, DEFAULT_MONITOR_HEIGHT, urid, title) 
{

}

inline Monitor::Monitor	(const double  x, const double y, const double width, const double height, uint32_t urid, std::string title) :
	Widget (x, y, width, height, urid, title),
	Clickable(),
	Draggable(),
	Scrollable(),
	data_(),
	steps_(),
	horizon_(0),
	scale_(1.0)
{

}

inline BWidgets::Widget* Monitor::clone () const 
{
	BWidgets::Widget* f = new Monitor (urid_, title_);
	f->copy (this);
	return f;
}

inline void Monitor::copy (const Monitor* that)
{
	data_ = that->data_;
	steps_ = that->steps_;
	horizon_ = that->horizon_;
	scale_ = that->scale_;
	Scrollable::operator= (*that);
	Draggable::operator= (*that);
	Clickable::operator= (*that);
	Widget::copy (that);
}

inline void Monitor::onPointerDragged (BEvents::Event* event)
{
	BEvents::PointerEvent* pev = dynamic_cast<BEvents::PointerEvent*> (event);
	if (!pev) return;

	scale_ += 0.01 * pev->getDelta().y * scale_;
	if (scale_ < 0.0001f) scale_ = 0.0001f;
	update ();
}
	
inline void Monitor::onWheelScrolled (BEvents::Event* event)
{
	BEvents::WheelEvent* wev = dynamic_cast<BEvents::WheelEvent*> (event);
	if (!wev) return;

	scale_ += 0.1 * wev->getDelta().y * scale_;
	if (scale_ < 0.0001f) scale_ = 0.0001f;
	update ();
}

inline void Monitor::setSteps (const std::vector<double>& steps)
{
	if (steps_ != steps)
	{
		steps_ = steps;
		update();
	}
}

inline void Monitor::pushData (const std::vector<BChopprNotifications>& data)
{
	for (const BChopprNotifications& note : data)
	{
		int pos = std::min (std::max (note.position, 0.0f), MONITORBUFFERSIZE - 1.0f);

		data_[pos].input1 = note.input1;
		data_[pos].input2 = note.input2;
		data_[pos].output1 = note.output1;
		data_[pos].output2 = note.output2;
		horizon_ = pos;
	}

	update();
}

inline void Monitor::draw ()
{
	draw (0, 0, getWidth(), getHeight());
}

inline void Monitor::draw (const double x0, const double y0, const double width, const double height)
{
	draw (BUtilities::Area<> (x0, y0, width, height));
}

inline void Monitor::draw (const BUtilities::Area<>& area)
{
	if ((!cairoSurface()) || (cairo_surface_status (cairoSurface()) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (area);

	const double x0 = getXOffset();
	const double y0 = getYOffset();
	const double heff = getEffectiveHeight ();
	const double weff = getEffectiveWidth ();

	// Draw monitor
	// only if minimum requirements satisfied
	if ((heff >= 10) && (heff >= 10))
	{
		cairo_t* cr = cairo_create (cairoSurface());

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
			cairo_clip (cr);

			// Colors uses within this method
			const BStyles::Color inColor = getFgColors()[BStyles::STATUS_NORMAL];
			const BStyles::Color outColor = getFgColors()[BStyles::STATUS_ACTIVE];
			const BStyles::Color bgColor = getBgColors()[BStyles::STATUS_NORMAL];

			// Create individual surfaces for all 4 data blocks
			cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getWidth(), getHeight());
			cairo_t* cr1 = cairo_create (surface);
			cairo_t* cr2 = cairo_create (surface);
			cairo_t* cr3 = cairo_create (surface);
			cairo_t* cr4 = cairo_create (surface);

			// Create patterns
			cairo_pattern_t* pat1 = cairo_pattern_create_linear (0, y0 + heff, 0, y0);
			cairo_pattern_add_color_stop_rgba (pat1, 0.1, CAIRO_RGB (inColor), 1);
			cairo_pattern_add_color_stop_rgba (pat1, 0.6, CAIRO_RGB (inColor), 0);
			cairo_pattern_t* pat2 = cairo_pattern_create_linear (0, y0 + heff, 0, y0);
			cairo_pattern_add_color_stop_rgba (pat2, 0.1, CAIRO_RGB (outColor), 1);
			cairo_pattern_add_color_stop_rgba (pat2, 0.6, CAIRO_RGB (outColor), 0);
			cairo_pattern_t* pat3 = cairo_pattern_create_linear (0, y0, 0, y0 + heff);
			cairo_pattern_add_color_stop_rgba (pat3, 0.1, CAIRO_RGB (inColor), 1);
			cairo_pattern_add_color_stop_rgba (pat3, 0.6, CAIRO_RGB (inColor), 0);
			cairo_pattern_t* pat4 = cairo_pattern_create_linear (0, y0, 0, y0 + heff);
			cairo_pattern_add_color_stop_rgba (pat4, 0.1, CAIRO_RGB (outColor), 1);
			cairo_pattern_add_color_stop_rgba (pat4, 0.6, CAIRO_RGB (outColor), 0);

			// Draw grid
			cairo_set_line_width (cr, 1);
			cairo_set_source_rgba (cr, CAIRO_RGBA (bgColor));
			cairo_move_to (cr, x0, y0 + 0.1 * heff);
			cairo_line_to (cr, x0 + weff, y0 + 0.1 * heff);
			cairo_move_to (cr, x0, y0 + 0.5 * heff);
			cairo_line_to (cr, x0 + weff, y0 + 0.5 * heff);
			cairo_move_to (cr, x0, x0 + 0.9 * heff);
			cairo_line_to (cr, x0 + weff, y0 + 0.9 * heff);

			// Draw steps
			for (const double s : steps_)
			{
				cairo_move_to (cr, x0 + s * weff, y0);
				cairo_line_to (cr, x0 + s * weff, y0 + heff);
			}

			cairo_stroke (cr);

			// Draw curves

			// Draw input (cr1, cr3) and output (cr2, cr4) curves
			cairo_move_to (cr1, x0, y0 + heff * (0.5  + 0.4 * std::min (std::max (data_[0].input2 / scale_, 0.0), 1.0)));
			cairo_move_to (cr2, x0, y0 + heff * (0.5  + 0.4 * std::min (std::max (data_[0].output2 / scale_, 0.0), 1.0)));
			cairo_move_to (cr3, x0, y0 + heff * (0.5  - 0.4 * std::min (std::max (data_[0].input1 / scale_, 0.0), 1.0)));
			cairo_move_to (cr4, x0, y0 + heff * (0.5  - 0.4 * std::min (std::max (data_[0].output1 / scale_, 0.0), 1.0)));

			for (int i = 0; i < MONITORBUFFERSIZE; ++i)
			{
				double pos = ((double) i) / (MONITORBUFFERSIZE - 1.0f);
				cairo_line_to (cr1, x0 + pos * weff, y0 + heff * (0.5  + 0.4 * std::min (std::max (data_[i].input2 / scale_, 0.0), 1.0)));
				cairo_line_to (cr2, x0 + pos * weff, y0 + heff * (0.5  + 0.4 * std::min (std::max (data_[i].output2 / scale_, 0.0), 1.0)));
				cairo_line_to (cr3, x0 + pos * weff, y0 + heff * (0.5  - 0.4 * std::min (std::max (data_[i].input1 / scale_, 0.0), 1.0)));
				cairo_line_to (cr4, x0 + pos * weff, y0 + heff * (0.5  - 0.4 * std::min (std::max (data_[i].output1 / scale_, 0.0), 1.0)));
			}

			// Visualize input (cr, cr3) and output (cr2, cr4) curves
			cairo_set_source_rgba (cr1, CAIRO_RGB (inColor), 1.0);
			cairo_set_line_width (cr1, 3);
			cairo_set_source_rgba (cr2, CAIRO_RGB (outColor), 1.0);
			cairo_set_line_width (cr2, 3);
			cairo_stroke_preserve (cr1);
			cairo_stroke_preserve (cr2);
			cairo_set_source_rgba (cr3, CAIRO_RGB (inColor), 1.0);
			cairo_set_line_width (cr3, 3);
			cairo_set_source_rgba (cr4, CAIRO_RGB (outColor), 1.0);
			cairo_set_line_width (cr4, 3);
			cairo_stroke_preserve (cr3);
			cairo_stroke_preserve (cr4);

			// Visualize input (cr, cr3) and output (cr2, cr4) areas under the curves
			cairo_line_to (cr1, x0 + weff, y0 + 0.5 * heff);
			cairo_line_to (cr1, x0, y0 + 0.5 * heff);
			cairo_close_path (cr1);
			cairo_line_to (cr2, x0 + weff, y0 + 0.5 * heff);
			cairo_line_to (cr2, x0, y0 + 0.5 * heff);
			cairo_close_path (cr2);
			cairo_set_source (cr1, pat1);
			cairo_set_line_width (cr1, 0);
			cairo_set_source (cr2, pat2);
			cairo_set_line_width (cr2, 0);
			cairo_fill (cr1);
			cairo_fill (cr2);
			cairo_line_to (cr3, x0 + weff, y0 + 0.5 * heff);
			cairo_line_to (cr3, x0, y0 + 0.5 * heff);
			cairo_close_path (cr3);
			cairo_line_to (cr4, x0 + weff, y0 + 0.5 * heff);
			cairo_line_to (cr4, x0, y0 + 0.5 * heff);
			cairo_close_path (cr4);
			cairo_set_source (cr3, pat3);
			cairo_set_line_width (cr3, 0);
			cairo_set_source (cr4, pat4);
			cairo_set_line_width (cr4, 0);
			cairo_fill (cr3);
			cairo_fill (cr4);

			// Draw fade out
			double r = double (horizon_) / (MONITORBUFFERSIZE - 1.0);
			if (x0 + r * weff > x0 + weff - 63) r -= 1.0;
			cairo_pattern_t* pat6 = cairo_pattern_create_linear (x0 + r * weff, y0, x0 + r * weff + 63, y0);
			if (cairo_pattern_status (pat6) == CAIRO_STATUS_SUCCESS)
			{
				cairo_pattern_add_color_stop_rgba (pat6, 0.0, CAIRO_RGBA(BStyles::black));
				cairo_pattern_add_color_stop_rgba (pat6, 1.0, CAIRO_RGBA(BStyles::invisible));
				cairo_set_line_width (cr1, 0.0);
				cairo_set_source (cr1, pat6);
				cairo_rectangle (cr1, x0 + r * weff, y0, 63, y0 + heff);
				cairo_fill (cr1);
				cairo_pattern_destroy (pat6);
			}

			// Draw horizon line
			cairo_set_source_rgba (cr1, CAIRO_RGBA(BStyles::white));
			cairo_set_line_width (cr1, 1);
			cairo_move_to (cr1, x0 + r * weff, y0);
			cairo_line_to (cr1, x0 + r * weff, y0 + heff);
			cairo_stroke (cr1);

			// Write content to widget surface
			cairo_set_source_surface (cr, surface, 0, 0);
			cairo_paint (cr);

			// Destroy patterns
			cairo_pattern_destroy (pat4);
			cairo_pattern_destroy (pat3);
			cairo_pattern_destroy (pat2);
			cairo_pattern_destroy (pat1);

			// Destroy contents and surface
			cairo_destroy (cr4);
			cairo_destroy (cr3);
			cairo_destroy (cr2);
			cairo_destroy (cr1);
			cairo_surface_destroy (surface);
		}

		cairo_destroy (cr);
	}
}

#endif /* MONITOR_HPP_ */
