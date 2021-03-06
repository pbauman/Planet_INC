//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// Planet - An atmospheric code for planetary bodies, adapted to Titan
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-

#ifndef PLANET_ATMOSPHERE_TEMPERATURE_H
#define PLANET_ATMOSPHERE_TEMPERATURE_H

//Antioch
#include "antioch/antioch_asserts.h"

//Planet
#include "planet/math_functions.h"

//C++

namespace Planet
{
  template<typename CoeffType, typename VectorCoeffType>
  class AtmosphericTemperature
  {
      private:
        //!no default constructor
        AtmosphericTemperature(){antioch_error();return;}

        VectorCoeffType _neutral_altitude;
        VectorCoeffType _neutral_temperature;
        VectorCoeffType _ionic_altitude;
        VectorCoeffType _ionic_temperature;
        VectorCoeffType _electronic_altitude;
        VectorCoeffType _electronic_temperature;

      public:
        AtmosphericTemperature(const VectorCoeffType &neu, const VectorCoeffType &ion, const VectorCoeffType &alt_neu, const VectorCoeffType &alt_ion);
        ~AtmosphericTemperature();

        //!\return neutral temperature
        const VectorCoeffType &neutral_temperature() const;

        //!\return neutral temperature altitude
        const VectorCoeffType &neutral_altitude() const;

        //!\return ionic temperature
        const VectorCoeffType &ionic_temperature()   const;

        //!\return ionic temperature altitude
        const VectorCoeffType &ionic_altitude()   const;

        //!\return electronic temperature
        const VectorCoeffType &electronic_temperature()   const;

        //!\return electronic temperature altitude
        const VectorCoeffType &electronic_altitude()   const;

        //!\return neutral temperature at custom altitude
        template <typename StateType>
        const CoeffType neutral_temperature(const StateType &z) const;

        //!\return ionic temperature at custom altitude
        template <typename StateType>
        const CoeffType ionic_temperature(const StateType &z)   const;

        //!\return electronic temperature at custom altitude
        template <typename StateType>
        const CoeffType electronic_temperature(const StateType &z)   const;

        //!
        template<typename VectorStateType>
        void set_neutral_temperature(const VectorStateType &neu);

        //!
        template<typename VectorStateType>
        void set_ionic_temperature(const VectorStateType &ion);

        template<typename VectorStateType>
        void set_electronic_temperature(const VectorStateType &electron);

        //!
        void initialize();

        //!\return derivative at an altitude
        template<typename StateType>
        const CoeffType dneutral_temperature_dz(const StateType &z) const;

        //!\return derivative at an altitude
        template<typename StateType>
        const CoeffType dionic_temperature_dz(const StateType &z) const;

        //!\return derivative at an altitude
        template<typename StateType>
        const CoeffType delectronic_temperature_dz(const StateType &z) const;

  };

  template<typename CoeffType, typename VectorCoeffType>
  inline
  AtmosphericTemperature<CoeffType,VectorCoeffType>::~AtmosphericTemperature()
  {
    return;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  AtmosphericTemperature<CoeffType,VectorCoeffType>::AtmosphericTemperature(const VectorCoeffType &neu, 
                                                                            const VectorCoeffType &ion, 
                                                                            const VectorCoeffType &alt_neu,
                                                                            const VectorCoeffType &alt_ion):
      _neutral_altitude(alt_neu),
      _neutral_temperature(neu),
      _ionic_altitude(alt_ion),
      _ionic_temperature(ion)
  {
    _electronic_altitude = _neutral_altitude;
    _electronic_temperature.resize(_electronic_altitude.size());
    for(unsigned int iz = 0; iz < _electronic_altitude.size(); iz++)
    {
      if(_electronic_altitude[iz] < 900.)
      {
         _electronic_temperature[iz] = 180.L;
      }else if(_electronic_altitude[iz] < 1400.)
      {
         _electronic_temperature[iz] = CoeffType(180.L) + (_electronic_altitude[iz] - CoeffType(900.L))/CoeffType(500.L) * CoeffType(1150.L - 180.L) ;
      }else
      {
         _electronic_temperature[iz] = CoeffType(1150.L) + CoeffType(0.1L) * (_electronic_altitude[iz] - CoeffType(1400.L));
      }
    }
    return;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  void AtmosphericTemperature<CoeffType,VectorCoeffType>::initialize()
  {
     return;//nothing right now
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::ionic_temperature() const
  {
    return _ionic_temperature;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::ionic_altitude() const
  {
    return _ionic_altitude;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::neutral_temperature() const
  {
    return _neutral_temperature;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::neutral_altitude() const
  {
    return _neutral_altitude;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::electronic_temperature() const
  {
    return _electronic_temperature;
  }

  template<typename CoeffType, typename VectorCoeffType>
  inline
  const VectorCoeffType &AtmosphericTemperature<CoeffType,VectorCoeffType>::electronic_altitude() const
  {
    return _electronic_altitude;
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::ionic_temperature(const StateType &z) const
  {
    return Functions::linear_evaluation(_ionic_altitude,_ionic_temperature,z);
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::neutral_temperature(const StateType &z) const
  {
    return Functions::linear_evaluation(_neutral_altitude,_neutral_temperature,z);
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::electronic_temperature(const StateType &z) const
  {
    return Functions::linear_evaluation(_electronic_altitude,_electronic_temperature,z);
  }


  template<typename CoeffType, typename VectorCoeffType>
  template<typename VectorStateType>
  inline
  void AtmosphericTemperature<CoeffType,VectorCoeffType>::set_neutral_temperature(const VectorStateType &neu)
  {
     _neutral_temperature = neu;
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename VectorStateType>
  inline
  void AtmosphericTemperature<CoeffType,VectorCoeffType>::set_ionic_temperature(const VectorStateType &ion)
  {
     _ionic_temperature = ion;
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename VectorStateType>
  inline
  void AtmosphericTemperature<CoeffType,VectorCoeffType>::set_electronic_temperature(const VectorStateType &electron)
  {
     _electronic_temperature = electron;
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::dneutral_temperature_dz(const StateType &z) const
  {
     return Functions::linear_evaluation_dz(_neutral_altitude,_neutral_temperature,z);
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::dionic_temperature_dz(const StateType &z) const
  {
     return Functions::linear_evaluation_dz(_ionic_altitude,_ionic_temperature,z);
  }

  template<typename CoeffType, typename VectorCoeffType>
  template<typename StateType>
  inline
  const CoeffType AtmosphericTemperature<CoeffType,VectorCoeffType>::delectronic_temperature_dz(const StateType &z) const
  {
     return Functions::linear_evaluation_dz(_electronic_altitude,_electronic_temperature,z);
  }

}

#endif
