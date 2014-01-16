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

#ifndef PLANET_PLANET_PHYSICS_HELPER_H
#define PLANET_PLANET_PHYSICS_HELPER_H

//Planet
#include "planet/diffusion_evaluator.h"
#include "planet/atmospheric_kinetics.h"

// libMesh
#include "libmesh/libmesh_common.h"

namespace Planet
{

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  class PlanetPhysicsHelper
  {
  public:

    PlanetPhysicsHelper(AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType > *kinetics = NULL,
                        DiffusionEvaluator <CoeffType,VectorCoeffType,MatrixCoeffType > *diffusion = NULL);

    ~PlanetPhysicsHelper();

    template <typename StateType, typename VectorStateType, typename MatrixStateType>
    void set_kinetics(AtmosphericKinetics<StateType,VectorStateType,MatrixStateType> *kinetics);

    template <typename StateType, typename VectorStateType, typename MatrixStateType>
    void set_diffusion(DiffusionEvaluator<StateType,VectorStateType,MatrixStateType> *diffusion);

    libMesh::Real diffusion_term(unsigned int s) const;

    libMesh::Real chemical_term(unsigned int s)  const;

    template<typename StateType, typename VectorStateType, typename MatrixStateType>
    void compute(const VectorStateType & molar_concentrations,
                 const VectorStateType & dmolar_concentrations_dz,
                 const VectorStateType & other_altitudes,
                 const MatrixStateType & other_concentrations,
                 const StateType & z);

  private:

    AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType> *_kinetics;
    DiffusionEvaluator <CoeffType,VectorCoeffType,MatrixCoeffType> *_diffusion;

    VectorCoeffType _omegas;
    VectorCoeffType _omegas_dots;

  };

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template<typename StateType, typename VectorStateType, typename MatrixStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::compute(const VectorStateType & molar_concentrations,
                                                               const VectorStateType & dmolar_concentrations_dz,
                                                               const VectorStateType & other_altitudes,
                                                               const MatrixStateType & other_concentrations,
                                                               const StateType & z)
  {
// here we supposed altitudes from bottom to top, approximation is
// that concentration is constant between altitudes, bottom-up,
// thus concentrations at top are useless here
   VectorStateType sum_concentration;
   sum_concentration.resize(molar_concentrations.size(),0.L);
   for(unsigned int s = 0; s < sum_concentration.size(); s++)
   {
     sum_concentration[s] += molar_concentrations[s] * (other_altitudes[0] - z);
   }
   for(unsigned int iz = 1; iz < other_concentrations.size(); iz++)
   {
      for(unsigned int s = 0; s < sum_concentration.size(); s++)
      {
        sum_concentration[s] += other_concentrations[iz][s] * (other_altitudes[iz-1] - other_altitudes[iz]);
      }
   }
// is r_max always there?
/*
   for(unsigned int s = 0; s < sum_concentration.size(); s++)
   {
     sum_concentration[s] += other_concentrations.back()[s] * (r_max - other_altitudes.back());
   }
*/

   _diffusion->diffusion(molar_concentrations,dmolar_concentrations_dz,z,_omegas);
   _kinetics->chemical_rate(molar_concentrations,sum_concentration,z,_omegas_dots);

    return;
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template <typename StateType, typename VectorStateType, typename MatrixStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::set_kinetics(AtmosphericKinetics<StateType,VectorStateType,MatrixStateType> *kinetics)
  {
     _kinetics = kinetics;
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template <typename StateType, typename VectorStateType, typename MatrixStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::set_diffusion(DiffusionEvaluator <StateType,VectorStateType,MatrixStateType> *diffusion)
  {
     _diffusion = diffusion;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::PlanetPhysicsHelper(AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType > *kinetics,
                                                                      DiffusionEvaluator <CoeffType,VectorCoeffType,MatrixCoeffType > *diffusion):
        _kinetics(kinetics),
        _diffusion(diffusion)
  {
    _omegas.resize(_kinetics->neutral_kinetics().reaction_set().n_species());
    _omegas_dots.resize(_kinetics->neutral_kinetics().reaction_set().n_species());
    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::~PlanetPhysicsHelper()
  {
    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  libMesh::Real PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::diffusion_term(unsigned int s) const
  {
    return _omegas[s];
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  libMesh::Real PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::chemical_term(unsigned int s) const
  {
    return _omegas_dots[s];
  }



} // end namespace Planet

#endif // PLANET_PLANET_PHYSICS_HELPER_H
