/* B.Choppr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 - 2021 Sven Jähnichen
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

#ifndef SHAREDDATA_HPP
#define SHAREDDATA_HPP

#include "definitions.hpp"
#include "Ports.hpp"
#include <vector>
#include <algorithm>

class BChoppr; // Forward declaration

class SharedData
{
protected:
        float controllers_[NrControllers];
        BChoppr* instances_[MAXINSTANCES];

public:
        SharedData () : controllers_ {0}, instances_ {0} {}

        bool isLinked (const BChoppr* inst) const {return std::find (instances_, instances_ + MAXINSTANCES, inst) != instances_ + MAXINSTANCES;}

        bool empty() const
        {
                for (int i = 0; i < MAXINSTANCES ; ++i)
                {
                        if (instances_[i] != nullptr) return false;
                }
                return true;
        }

        void link (BChoppr* inst)
        {
                if (isLinked (inst)) return;
                BChoppr** it = std::find (instances_, instances_ + MAXINSTANCES, nullptr);
                if (it != instances_ + MAXINSTANCES) *it = inst;
        }

        void unlink (BChoppr* inst)
        {
                BChoppr** it = std::find (instances_, instances_ + MAXINSTANCES, inst);
                if (it != instances_ + MAXINSTANCES) *it = nullptr;
        }

        void set (const int nr, const float value) {controllers_[nr] = value;}

        float get (const int nr) const {return controllers_[nr];}
};

#endif /* SHAREDDATA_HPP */
