/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
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

#include "BWidgets/HSliderValue.hpp"
#include "BUtilities/to_string.hpp"

class SwingHSlider : public BWidgets::HSliderValue
{
public:
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
	{}

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
};

#endif /* SWINGHSLIDER_HPP_ */
