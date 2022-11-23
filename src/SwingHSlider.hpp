/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 - 2022 by Sven Jähnichen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SWINGHSLIDER_HPP_
#define SWINGHSLIDER_HPP_

#include "BWidgets/BWidgets/ValueHSlider.hpp"

#ifndef DEFAULT_SWINGHSLIDER_WIDTH
#define DEFAULT_SWINGHSLIDER_WIDTH 80.0
#endif

#ifndef DEFAULT_SWINGHSLIDER_HEIGHT
#define DEFAULT_SWINGHSLIDER_HEIGHT 40.0
#endif

/**
 *  @brief  A ValueHSlider with its center as the base.
 */
class SwingHSlider : public BWidgets::ValueHSlider
{
public:

	/**
	 *  @brief  Constructs a default %SwingHSlider object.
	 */
	SwingHSlider ();

	/**
	 *  @brief  Constructs a default %SwingHSlider object.
	 *  @param URID  URID.
	 *  @param title  %Widget title.
	 */
	SwingHSlider (const uint32_t urid, const std::string& title);

	/**
	 *  @brief  Creates a %SwingHSlider with default size.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param urid  Optional, URID (default = URID_UNKNOWN_URID).
	 *  @param title  Optional, %Widget title (default = "").
	 */
	SwingHSlider	(const double value, const double min, const double max, double step = 0.0, 
					 uint32_t urid = URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a %SwingHSlider.
	 *  @param x  %SwingHSlider X origin coordinate.
	 *  @param y  %SwingHSlider Y origin coordinate.
	 *  @param width  %SwingHSlider width.
	 *  @param height  %SwingHSlider height.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param transferFunc  Optinonal, function to transfer a value from an
	 *  external context to the internal context.
	 *  @param reTransferFunc  Optinonal, function to transfer a value from the
	 *  internal context to an external context.
	 *  @param displayFunc  Optional, function to convert the value to a string
	 *  which will be displayed as a label.
	 *  @param reDisplayFunc  Optional, function to convert the string from
	 *  the (edited) label to the value.
	 *  @param urid  Optional, URID (default = URID_UNKNOWN_URID).
	 *  @param title  Optional, %SwingHSlider title (default = "").
	 *
	 *  The optional parameters @a displayFunc and @a reDisplayFunc can be used
	 *  as powerful tools to visualize the value in any way (including units,
	 *  prefixes, postfixes, text substitution, ...) and to parse it. By 
	 *  default, %SwingHSlider displays the value via @c valueToString() and
	 *  thus shows a decimal floating point representation with up to 3 digits
	 *  after the point.
	 */
	SwingHSlider	(const double x, const double y, const double width, const double height, 
					 const double value, const double min, const double max, double step = 0.0,
					 std::function<double (const double& x)> transferFunc = ValueTransferable<double>::noTransfer,
					 std::function<double (const double& x)> reTransferFunc = ValueTransferable<double>::noTransfer,
					 std::function<std::string (const double& x)> displayFunc = valueToString,
					 std::function<double (const std::string& s)> reDisplayFunc = stringToValue,
					 uint32_t urid = URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a clone of the %SwingHSlider. 
	 *  @return  Pointer to the new %SwingHSlider.
	 *
	 *  Creates a clone of this %SwingHSlider by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual Widget* clone () const override; 

	/**
	 *  @brief  Copies from another %SwingHSlider. 
	 *  @param that  Other %SwingHSlider.
	 *
	 *  Copies all properties from another %SwingHSlider. But NOT its linkage.
	 */
	void copy (const SwingHSlider* that);

protected:

	/**
     *  @brief  Unclipped draw a %SwingHSlider to the surface.
     */
    virtual void draw () override;

    /**
     *  @brief  Clipped draw a %SwingHSlider to the surface.
     *  @param x0  X origin of the clipped area. 
     *  @param y0  Y origin of the clipped area. 
     *  @param width  Width of the clipped area.
     *  @param height  Height of the clipped area. 
     */
    virtual void draw (const double x0, const double y0, const double width, const double height) override;

    /**
     *  @brief  Clipped draw a %SwingHSlider to the surface.
     *  @param area  Clipped area. 
     */
    virtual void draw (const BUtilities::Area<>& area) override;
};

inline SwingHSlider::SwingHSlider () :
	SwingHSlider	(
					 0.0, 0.0, BWIDGETS_DEFAULT_VALUEHSLIDER_WIDTH, BWIDGETS_DEFAULT_VALUEHSLIDER_HEIGHT, 
					 1.0, 0.0, 2.0, 0.0, 
					 ValueTransferable<double>::noTransfer, 
					 ValueTransferable<double>::noTransfer, 
					 valueToString,
					 stringToValue,
					 URID_UNKNOWN_URID, "")
{

}

inline SwingHSlider::SwingHSlider (const uint32_t urid, const std::string& title) : 
	SwingHSlider	(0.0, 0.0, BWIDGETS_DEFAULT_VALUEHSLIDER_WIDTH, BWIDGETS_DEFAULT_VALUEHSLIDER_HEIGHT, 
					 1.0, 0.0, 2.0, 0.0, 
					 ValueTransferable<double>::noTransfer, 
					 ValueTransferable<double>::noTransfer,
					 valueToString,
					 stringToValue, 
				 	 urid, title) 
{

}

inline SwingHSlider::SwingHSlider (double value, const double min, const double max, double step, uint32_t urid, std::string title) : 
	SwingHSlider	(0.0, 0.0, BWIDGETS_DEFAULT_VALUEHSLIDER_WIDTH, BWIDGETS_DEFAULT_VALUEHSLIDER_HEIGHT, 
					 value, min, max, step, 
					 ValueTransferable<double>::noTransfer, 
					 ValueTransferable<double>::noTransfer,
					 valueToString,
					 stringToValue, 
				 	 urid, title) 
{

}

inline SwingHSlider::SwingHSlider	(const double  x, const double y, const double width, const double height, 
								 double value, const double min, const double max, double step, 
								 std::function<double (const double& x)> transferFunc,
					 			 std::function<double (const double& x)> reTransferFunc,
								 std::function<std::string (const double& x)> displayFunc,
								 std::function<double (const std::string& s)> reDisplayFunc,
								 uint32_t urid, std::string title) :
	ValueHSlider (x, y, width, height, value, min, max, step, transferFunc, reTransferFunc, displayFunc, reDisplayFunc, urid, title)
{

}

inline BWidgets::Widget* SwingHSlider::clone () const 
{
	Widget* f = new SwingHSlider (urid_, title_);
	f->copy (this);
	return f;
}

inline void SwingHSlider::copy (const SwingHSlider* that)
{
	SwingHSlider::copy (that);
}

inline void SwingHSlider::draw ()
{
	draw (0, 0, getWidth(), getHeight());
}

inline void SwingHSlider::draw (const double x0, const double y0, const double width, const double height)
{
	draw (BUtilities::Area<> (x0, y0, width, height));
}

inline void SwingHSlider::draw (const BUtilities::Area<>& area)
{
	if ((!surface_) || (cairo_surface_status (surface_) != CAIRO_STATUS_SUCCESS)) return;

	// Draw only if minimum requirements satisfied
	if ((getHeight () >= 1) && (getWidth () >= 1))
	{
		cairo_t* cr = cairo_create (surface_);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
			cairo_clip (cr);

			const BStyles::Color fgColor = getFgColors()[getStatus()];
			const BStyles::Color bgColor = getBgColors()[getStatus()];
			const double rval = getRatioFromValue (getValue());

			if (step_ >= 0.0) drawHBar(cr, scale_.getX(), scale_.getY(), scale_.getWidth(), scale_.getHeight(), 0.5, rval, fgColor, bgColor);
			else drawHBar(cr, scale_.getX(), scale_.getY(), scale_.getWidth(), scale_.getHeight(), 1.0 - rval, 0.5, fgColor, bgColor);

			if (step_ >= 0.0) drawKnob	(cr, 
										 scale_.getX() + rval * scale_.getWidth(), 
										 scale_.getY() + 0.5 * scale_.getHeight() + 0.5, 
										 scale_.getHeight() - 1.0, 
										 1.0, 
										 bgColor, bgColor);

			else drawKnob				(cr, 
										 scale_.getX() + (1.0 - rval) * scale_.getWidth(), 
										 scale_.getY() + 0.5 * scale_.getHeight() + 0.5, 
										 scale_.getHeight() - 1.0, 
										 1.0, 
										 bgColor, bgColor);
		}

		cairo_destroy (cr);
	}
}

/*
	SwingHSlider ();
	SwingHSlider (const double x, const double y, const double width, const double height, const std::string& name,
		      const double value, const double min, const double max, const double step, std::string format = "%1.2f",
	      	      std::function<double (const double val, const double min, const double max)> valfunc = [] (const double val, const double min, const double max)
		      {return (val >= 1.0 ? 0.5 + 0.5 * (val - 1.0) / (max - 1.0) : 0.5 - 0.5 * (1.0 / val - 1.0) / (1.0 / min - 1.0));},
	      	      std::function<double (const double frac, const double min, const double max)> fracfunc = [] (const double frac, const double min, const double max)
		      {
			      return
		      		(
		      			frac >= 0.5 ?
		      			(2.0 * frac - 1.0) * (max - 1.0) + 1.0 :
		      			1.0 / ((1.0 - 2.0 * frac) * (1.0 / min - 1.0) + 1.0)
		      		);
		      }) :
	HSliderValue ( x, y, width, height, name, value, min, max, step, format),
	valueToFraction_ (valfunc),
	fractionToValue_ (fracfunc)
	{
		valueDisplay.setCallbackFunction(BEvents::EventType::MESSAGE_EVENT, displayMessageCallback);
	}

	virtual Widget* clone () const override {return new SwingHSlider (*this);}

	virtual void setValue (const double val) override
	{
		{
			HSlider::setValue (val);
			std::string valstr =
			(
				val < 1.0 ?
				"1 : " + BUtilities::to_string (1.0 / getValue(), valFormat) :
				(
					val == 1.0 ?
					"1 : 1" :
					BUtilities::to_string (getValue(), valFormat) + " : 1"
				)
			);
			valueDisplay.setText (valstr);
			focusLabel.setText (valstr);

		}
	}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override
	{
		if
		(
			main_ &&
			isVisible () &&
			(getHeight () >= 1) &&
			(getWidth () >= 1) &&
			(scaleArea.getWidth () > 0) &&
			(event->getButton() == BDevices::LEFT_BUTTON)
		)
		{
			double min = getMin ();
			double max = getMax ();

			// Use pointer coords directly if hardSetable , otherwise apply only
			// X movement (drag mode)
			if (hardChangeable)
			{
				double frac = (event->getPosition ().x - scaleArea.getX ()) / scaleArea.getWidth ();
				if (getStep () < 0) frac = 1 - frac;
				double hardValue = fractionToValue_ (frac, getMin(), getMax());
				softValue = 0;
				setValue (hardValue);
			}
			else
			{
				if (min != max)
				{
					double deltaFrac = event->getDelta ().x / scaleArea.getWidth ();
					if (getStep () < 0) deltaFrac = -deltaFrac;
					softValue += deltaFrac;
					double newValue = fractionToValue_ (valueToFraction_ (getValue(), getMin(), getMax()) + softValue, getMin(), getMax());
					setValue (newValue);
				}
			}
		}
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		double min = getMin ();
		double max = getMax ();

		if (min != max)
		{
			double step = (getStep () != 0.0 ? getStep () : 1.0 / scaleArea.getWidth ());
			double frac = valueToFraction_ (getValue(), getMin(), getMax()) + event->getDelta ().y * step;
			double newValue = fractionToValue_ (frac, getMin(), getMax());
			setValue (newValue);
		}
	}

	virtual void update () override
	{
		HSliderValue::update();
		std::string valstr =
		(
			value < 1.0 ?
			"1 : " + BUtilities::to_string (1/value, valFormat) :
			(
				value == 1.0 ?
				"1 : 1" :
				BUtilities::to_string (value, valFormat) + " : 1"
			)
		);
		valueDisplay.setText (valstr);
		focusLabel.setText (valstr);
	}

protected:
	std::function<double (const double val, const double min, const double max)> valueToFraction_;
	std::function<double (const double frac, const double min, const double max)> fractionToValue_;

	virtual void updateCoords () override
	{
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight () / 2;

		knobRadius = (h < w / 2 ? h / 2 : w / 4);
		scaleArea = BUtilities::RectArea
		(
			getXOffset () + knobRadius,
			getYOffset () + h + knobRadius / 2,
			w - 2 * knobRadius,
			knobRadius
		);

		scaleXValue = scaleArea.getX() + valueToFraction_ (getValue(), getMin(), getMax()) * scaleArea.getWidth();

		knobPosition = BUtilities::Point (scaleXValue, scaleArea.getY() + scaleArea.getHeight() / 2);

		double dh = knobRadius * 2;
		double dw = 4.0 * dh;
		double dy = getYOffset () + h - dh;
		double dx = LIMIT (scaleXValue - dw / 2, getXOffset (), getXOffset () + getEffectiveWidth () - dw);
		displayArea = BUtilities::RectArea (dx, dy, dw, dh);
	}

	static void displayMessageCallback (BEvents::Event* event)
	{
		if (event && event->getWidget())
		{
			BWidgets::Label* l = (BWidgets::Label*)event->getWidget();
			SwingHSlider* d = (SwingHSlider*)l->getParent();
			if (d)
			{
				const std::string s = l->getText();
				const size_t p = s.find (":");
				if ((p == std::string::npos) || (p >= s.size() - 1))
				{
					fprintf (stderr, "Invalid ratio format for %s\n", s.c_str());
					d->update();
					return;
				}

				double v1;
				try {v1 = BUtilities::stof (s);}
				catch (std::invalid_argument &ia)
				{
					fprintf (stderr, "%s\n", ia.what());
					d->update();
					return;
				}

				double v2;
				try {v2 = BUtilities::stof (s.substr (p + 2));}
				catch (std::invalid_argument &ia)
				{
					fprintf (stderr, "%s\n", ia.what());
					d->update();
					return;
				}

				if (v2 == 0)
				{
					fprintf (stderr, "Division by zero\n");
					d->update();
					return;
				}

				d->setValue (v1 / v2);
				d->update();
			}
		}
	}
};
*/

#endif /* SWINGHSLIDER_HPP_ */
