// ====================================================================
// This file is part of FlexibleSUSY.
//
// FlexibleSUSY is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// FlexibleSUSY is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FlexibleSUSY.  If not, see
// <http://www.gnu.org/licenses/>.
// ====================================================================

// File generated at Mon 5 Oct 2015 12:42:27

/**
 * @file SSDM_two_scale_model.cpp
 * @brief implementation of the SSDM model class
 *
 * Contains the definition of the SSDM model class methods
 * which solve EWSB and calculate pole masses and mixings from DRbar
 * parameters.
 *
 * This file was generated at Mon 5 Oct 2015 12:42:27 with FlexibleSUSY
 * 1.1.0 (git commit: v1.1.0-3-ga4e5ebd) and SARAH 4.5.6 .
 */

#include "SSDM_two_scale_model.hpp"

namespace flexiblesusy {

using namespace SSDM_info;

#define CLASSNAME SSDM<Two_scale>

CLASSNAME::SSDM(const SSDM_input_parameters& input_)
   : Two_scale_model()
   , SSDM_mass_eigenstates(input_)
{
}

CLASSNAME::~SSDM()
{
}

void CLASSNAME::calculate_spectrum()
{
   SSDM_mass_eigenstates::calculate_spectrum();
}

void CLASSNAME::clear_problems()
{
   SSDM_mass_eigenstates::clear_problems();
}

std::string CLASSNAME::name() const
{
   return SSDM_mass_eigenstates::name();
}

void CLASSNAME::run_to(double scale, double eps)
{
   SSDM_mass_eigenstates::run_to(scale, eps);
}

void CLASSNAME::print(std::ostream& out) const
{
   SSDM_mass_eigenstates::print(out);
}

void CLASSNAME::set_precision(double p)
{
   SSDM_mass_eigenstates::set_precision(p);
}

} // namespace flexiblesusy