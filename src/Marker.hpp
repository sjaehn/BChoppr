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

#ifndef MARKER_HPP_
#define MARKER_HPP_

#include "BWidgets/BStyles/Types/Color.hpp"
#include "BWidgets/BWidgets/Widget.hpp"
#include "BWidgets/BWidgets/Supports/Validatable.hpp"
#include "BWidgets/BWidgets/Supports/ValueableTyped.hpp"
#include "BWidgets/BWidgets/Supports/ValidatableRange.hpp"
#include "BWidgets/BWidgets/Supports/ValueTransferable.hpp"
#include "BWidgets/BWidgets/Supports/Clickable.hpp"
#include "BWidgets/BWidgets/Supports/Draggable.hpp"
#include "BWidgets/BWidgets/Supports/Scrollable.hpp"
#include "BWidgets/BEvents/WheelEvent.hpp"
#include "BWidgets/BWidgets/Label.hpp"
#include "BWidgets/BUtilities/to_string.hpp"


#ifndef DEFAULT_MARKER_WIDTH
#define DEFAULT_MARKER_WIDTH 10.0
#endif

#ifndef DEFAULT_MARKER_HEIGHT
#define DEFAULT_MARKER_HEIGHT 20.0
#endif

/**
 *  @brief A %Marker is a Widget that draws a visual marker and supports
 *  @c double values (including range validation and value transfer). I also
 *  supports but NOT handles (!) pointer clicking, dragging or wheel 
 *  scrolling. Thus, the respective events need to be controlled externally
 *  (e. g., via callbacks).
 */
class Marker :	public BWidgets::Widget, 
				public BWidgets::ValueableTyped<double>, 
				public BWidgets::ValidatableRange<double>, 
				public BWidgets::ValueTransferable<double>,
				public BWidgets::Clickable, 
				public BWidgets::Draggable, 
				public BWidgets::Scrollable
{
protected:
    bool hasValue_;

public:

	/**
	 * @brief  Constructs a default %Marker object.
	 * 
	 */
	Marker ();

	/**
	 *  @brief  Constructs a default %Marker object.
	 *  @param URID  URID.
	 *  @param title  %Widget title.
	 */
	Marker (const uint32_t urid, const std::string& title);

	/**
	 *  @brief  Creates a %Marker with default size.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param urid  Optional, URID (default = URID_UNKNOWN_URID).
	 *  @param title  Optional, %Widget title (default = "").
	 */
	Marker	(const double value, const double min, const double max, double step = 0.0, 
			 uint32_t urid = URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a %Marker.
	 *  @param x  %Marker X origin coordinate.
	 *  @param y  %Marker Y origin coordinate.
	 *  @param width  %Marker width.
	 *  @param height  %Marker height.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param transferFunc  Optinonal, function to transfer a value from an
	 *  external context to the internal context.
	 *  @param reTransferFunc  Optinonal, function to transfer a value from the
	 *  internal context to an external context.
	 *  @param urid  Optional, URID (default = URID_UNKNOWN_URID).
	 *  @param title  Optional, %Marker title (default = "").
	 */
	Marker	(const double x, const double y, const double width, const double height, 
			 const double value, const double min, const double max, double step = 0.0,
			 std::function<double (const double& x)> transferFunc = ValueTransferable<double>::noTransfer,
			 std::function<double (const double& x)> reTransferFunc = ValueTransferable<double>::noTransfer,
			 uint32_t urid = URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a clone of the %Marker. 
	 *  @return  Pointer to the new %Marker.
	 *
	 *  Creates a clone of this %Marker by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual Widget* clone () const override; 

	/**
	 *  @brief  Copies from another %Marker. 
	 *  @param that  Other %Marker.
	 *
	 *  Copies all properties from another %Marker. But NOT its linkage.
	 */
	void copy (const Marker* that);

	/**
	 *  @brief  Defines if the marker has got a defined value or not.
	 *  @param hasValue  True if the marker has got a defined value, otherwise
	 *  false.
	 */
	void setHasValue (bool hasValue = true);

	/**
	 *  @brief  Checks if the marker has got a defined value or not.
	 *  @return  True if the marker has got a defined value, otherwise false.
	 */
	bool hasValue () const;

	/**
     *  @brief  Method to be called following an object state change.
     */
    virtual void update () override;

protected:

	/**
     *  @brief  Unclipped draw a %Marker to the surface.
     */
    virtual void draw () override;

    /**
     *  @brief  Clipped draw a %Marker to the surface.
     *  @param x0  X origin of the clipped area. 
     *  @param y0  Y origin of the clipped area. 
     *  @param width  Width of the clipped area.
     *  @param height  Height of the clipped area. 
     */
    virtual void draw (const double x0, const double y0, const double width, const double height) override;

    /**
     *  @brief  Clipped draw a %Marker to the surface.
     *  @param area  Clipped area. 
     */
    virtual void draw (const BUtilities::Area<>& area) override;
};

inline Marker::Marker () :
	Marker	(0.0, 0.0, DEFAULT_MARKER_WIDTH, DEFAULT_MARKER_HEIGHT, 
			0.0, 0.0, 1.0, 0.0, 
			ValueTransferable<double>::noTransfer, ValueTransferable<double>::noTransfer, 
			URID_UNKNOWN_URID, "")
{

}

inline Marker::Marker (const uint32_t urid, const std::string& title) : 
	Marker	(0.0, 0.0, DEFAULT_MARKER_WIDTH, DEFAULT_MARKER_HEIGHT, 
			 0.0, 0.0, 1.0, 0.0, 
			 ValueTransferable<double>::noTransfer, ValueTransferable<double>::noTransfer, 
			 urid, title) 
{

}

inline Marker::Marker (double value, const double min, const double max, double step, uint32_t urid, std::string title) : 
	Marker	(0.0, 0.0, DEFAULT_MARKER_WIDTH, DEFAULT_MARKER_HEIGHT, 
			 value, min, max, step, 
			 ValueTransferable<double>::noTransfer, ValueTransferable<double>::noTransfer, 
			 urid, title) 
{

}

inline Marker::Marker	(const double  x, const double y, const double width, const double height, 
						 double value, const double min, const double max, double step, 
						 std::function<double (const double& x)> transferFunc,
			 			 std::function<double (const double& x)> reTransferFunc,
						 uint32_t urid, std::string title) :
	Widget (x, y, width, height, urid, title),
	ValueableTyped<double> (value),
	ValidatableRange<double> (min, max, step),
	ValueTransferable<double> (transferFunc, reTransferFunc),
	Clickable(),
	Draggable(),
	Scrollable(),
	hasValue_ (false)
{

}

inline BWidgets::Widget* Marker::clone () const 
{
	BWidgets::Widget* f = new Marker (urid_, title_);
	f->copy (this);
	return f;
}

inline void Marker::copy (const Marker* that)
{
	hasValue_ = that->hasValue_;
	ValueTransferable<double>::operator= (*that);
	ValidatableRange<double>::operator= (*that);
	ValueableTyped<double>::operator= (*that);
	Scrollable::operator= (*that);
	Draggable::operator= (*that);
	Clickable::operator= (*that);
	Widget::copy (that);
}

inline void Marker::setHasValue (const bool hasValue)
{
	if (hasValue != hasValue_)
	{
		hasValue_ = hasValue;
		update();
	}
}

inline bool Marker::hasValue () const 
{
	return hasValue_;
}

inline void Marker::update ()
{
	BWidgets::Label* f = dynamic_cast<BWidgets::Label*>(focus_);
	if (f)
	{
		f->setText (getTitle() + ": " + (hasValue() ? std::to_string (this->getValue()) : "Auto"));
		f->resize();
	}

	Widget::update();
}

inline void Marker::draw ()
{
	draw (0, 0, getWidth(), getHeight());
}

inline void Marker::draw (const double x0, const double y0, const double width, const double height)
{
	draw (BUtilities::Area<> (x0, y0, width, height));
}

inline void Marker::draw (const BUtilities::Area<>& area)
{
	if ((!surface_) || (cairo_surface_status (surface_) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (area);

	double heff = getEffectiveHeight ();
	double weff = getEffectiveWidth ();

	// Draw knob
	// only if minimum requirements satisfied
	if ((getHeight () >= 1) && (getWidth () >= 1))
	{
		cairoplus_surface_clear (surface_);
		cairo_t* cr = cairo_create (surface_);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
			cairo_clip (cr);

			// Colors uses within this method
			BStyles::Color cLo = getBgColors()[getStatus()].illuminate (!hasValue_ ? BStyles::Color::normalLighted : 2.0 * BStyles::Color::illuminated);
			BStyles::Color cMid = getBgColors()[getStatus()].illuminate (0.5 * (BStyles::Color::normalLighted + BStyles::Color::illuminated));
			BStyles::Color cHi = getBgColors()[getStatus()].illuminate (BStyles::Color::illuminated);

			cairo_set_line_width (cr, 0.0);
			cairo_set_source_rgba (cr, CAIRO_RGBA (cMid));
			cairo_move_to (cr, 0.5 * weff, 0);
			cairo_line_to (cr, 0, 0.25 * heff);
			cairo_line_to (cr, 0, heff);
			cairo_line_to (cr, weff, heff);
			cairo_line_to (cr, weff, 0.25 * heff);
			cairo_close_path (cr);
			cairo_fill (cr);

			cairo_set_line_width (cr, 1.0);
			cairo_set_source_rgba (cr, CAIRO_RGBA (cLo));
			cairo_move_to (cr, 0.25 * weff, 0.5 * heff);
			cairo_line_to (cr, 0.75 * weff, 0.5 * heff);
			cairo_move_to (cr, 0.25 * weff, 0.6 * heff);
			cairo_line_to (cr, 0.75 * weff, 0.6 * heff);
			cairo_move_to (cr, 0.25 * weff, 0.7 * heff);
			cairo_line_to (cr, 0.75 * weff, 0.7 * heff);
			cairo_stroke (cr);

			cairo_set_line_width (cr, 2.0);
			cairo_set_source_rgba (cr, CAIRO_RGBA (cHi));
			cairo_move_to (cr, 0, heff);
			cairo_line_to (cr, weff, heff);
			cairo_line_to (cr, weff, 0.25 * heff);
			cairo_stroke (cr);
		}

		cairo_destroy (cr);
	}
}

#endif /* MARKER_HPP_ */
