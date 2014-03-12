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

//Antioch
#include "antioch/vector_utils_decl.h"
#include "antioch/metaprogramming.h"
#include "antioch/vector_utils.h"

//Planet
#include "planet/diffusion_evaluator.h"
#include "planet/atmospheric_kinetics.h"

// libMesh
#include "libmesh/libmesh_common.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/getpot.h"

namespace Planet
{

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  class PlanetPhysicsHelper
  {
  public:

    PlanetPhysicsHelper( const GetPot& input );

    PlanetPhysicsHelper(AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType> *compo,
                        AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType > *kinetics = NULL,
                        DiffusionEvaluator <CoeffType,VectorCoeffType,MatrixCoeffType > *diffusion = NULL);

    ~PlanetPhysicsHelper();

    libMesh::Real diffusion_term(unsigned int s) const;

    libMesh::Real chemical_term(unsigned int s)  const;

    //computes omega_dot and omega
    template<typename StateType, typename VectorStateType>
    void compute(const VectorStateType & molar_concentrations,
                 const VectorStateType & dmolar_concentrations_dz,
                 const StateType & z);

    //!fills molar_concentrations_first_guess with barometric equation
    template<typename StateType, typename VectorStateType>
    void first_guess(VectorStateType & molar_concentrations_first_guess, const StateType z) const;

    //!fills lower boundary conditions
    template<typename VectorStateType>
    void lower_boundary_dirichlet(VectorStateType & lower_boundary) const;

    //!fills upper boundary conditions
    template<typename VectorStateType>
    void upper_boundary_neumann(VectorStateType & upper_boundary, const VectorStateType &molar_densities) const;

    const AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType>& composition() const;

  private:
    /*! \todo This should really be const. Need to fix up ParticleFlux stuff. */
    Antioch::ReactionSet<CoeffType>& neutral_reaction_set();

    AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType>*  _composition; //for first guess
    AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType>* _kinetics;
    DiffusionEvaluator<CoeffType,VectorCoeffType,MatrixCoeffType>* _diffusion;
    const Antioch::ReactionSet<CoeffType>& ionic_reaction_set() const;

    template<typename VectorStateType, typename StateType>
    void update_cache(const VectorStateType &molar_concentrations, const StateType &z);
    const PhotonOpacity<CoeffType,VectorCoeffType>& tau() const;

    void cache_recompute();
    const std::vector<std::vector<BinaryDiffusion<CoeffType> > >& bin_diff_coeff() const;

    //! uses compo.barometric_density(z);
    template <typename StateType>
    const VectorCoeffType get_cache(const StateType &z) const;
    const AtmosphericTemperature<CoeffType,VectorCoeffType>& temperature() const;

    VectorCoeffType _omegas;
    VectorCoeffType _omegas_dots;
    MatrixCoeffType _cache_composition;
    VectorCoeffType _cache_altitudes;
    std::map<CoeffType,VectorCoeffType> _cache;
    //const & lambda_hv() const;

    //const & phy1AU() const;

    //const & medium() const;


    // Additional data structures that need to be cached
    AtmosphericTemperature<CoeffType,VectorCoeffType>* _temperature;
    Antioch::ChemicalMixture<CoeffType>* _neutral_species;
    Antioch::ChemicalMixture<CoeffType>* _ionic_species;

    Antioch::ReactionSet<CoeffType>* _neutral_reaction_set;
    Antioch::ReactionSet<CoeffType>* _ionic_reaction_set;
    Antioch::ReactionSet<CoeffType>* _neut_reac_theo;

    Chapman<CoeffType>* _chapman;

    std::vector<std::vector<BinaryDiffusion<CoeffType> > > _bin_diff_coeff;

    //! Parameter for diffusion
    /*! Needs to be cached because some Evaluators depend on this value */
    CoeffType _K0;

    PhotonOpacity<CoeffType,VectorCoeffType>* _tau;

    /*! Convenience method to hide all the construction code for
        composition, kinetics, and diffusion */
    void build( const GetPot& input );

    /*! Convenience method within a convenience method */
    void build_temperature( const GetPot& input );

    /*! Convenience method within a convenience method */
    void build_species( const GetPot& input );

    /*! Convenience method within a convenience method */
    void build_reaction_sets( const GetPot& input );

    /*! Convenience method within a convenience method */
    void build_composition( const GetPot& input );

    // Helper functions for parsing data
    void read_temperature(VectorCoeffType& T0, VectorCoeffType& Tz, const std::string& file) const;

  };

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::PlanetPhysicsHelper( const GetPot& input )
    : _composition(NULL),
      _kinetics(NULL),
      _diffusion(NULL),
      _temperature(NULL),
      _neutral_species(NULL),
      _ionic_species(NULL),
      _neutral_reaction_set(NULL),
      _ionic_reaction_set(NULL),
      _neut_reac_theo(NULL),
      _chapman(NULL),
      _tau(NULL)
  {
    this->build(input);

    _omegas.resize(_kinetics->neutral_kinetics().reaction_set().n_species());
    _omegas_dots.resize(_kinetics->neutral_kinetics().reaction_set().n_species());

    return;
  }


  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template<typename StateType, typename VectorStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::compute(const VectorStateType & molar_concentrations,
                                                               const VectorStateType & dmolar_concentrations_dz,
                                                               const StateType & z)
  {
   _diffusion->diffusion(molar_concentrations,dmolar_concentrations_dz,z,_omegas);
   _kinetics->chemical_rate(molar_concentrations,this->get_cache(z),z,_omegas_dots);

   this->update_cache(molar_concentrations,z);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::PlanetPhysicsHelper(AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType> *compo,
                                                        AtmosphericKinetics<CoeffType,VectorCoeffType,MatrixCoeffType > *kinetics,
                                                        DiffusionEvaluator <CoeffType,VectorCoeffType,MatrixCoeffType > *diffusion):
        _kinetics(kinetics),
        _diffusion(diffusion),
        _composition(compo)
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

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template <typename StateType>
  const VectorCoeffType PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::get_cache(const StateType &z) const
  {
     if(!_cache.count(z))
     {
        VectorCoeffType first_sum_guess = Antioch::zero_clone(_composition->neutral_molar_fraction_bottom());
        _composition->first_guess_densities_sum(z,first_sum_guess);
        return first_sum_guess;
     }else
     {
        return _cache.at(z);
     }
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template <typename VectorStateType, typename StateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::update_cache(const VectorStateType &molar_concentrations, const StateType &z)
  {
    bool recompute(true);
    if(!_cache.count(z))
    {
        recompute = false;
       _cache[z] = get_cache(z);
    }

     _cache_composition.push_back(molar_concentrations);
     _cache_altitudes.push_back(z);
     if(recompute && _cache_composition.size() == _cache.size())this->cache_recompute();
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::cache_recompute()
  {
   //from highest altitude to lowest altitude
   unsigned int istart(0);
   int istep(1);
   if(_cache_altitudes.back() > _cache_altitudes.front())
   {
      istep = -1;
      istart = _cache_altitudes.size() - 1;
   }
 

   //sum densities are sdens_{i} = n(z_{i+1}) * (z_{i+1} - z_{i}), top composition is useless
   for(unsigned int i = 1; i < _cache_altitudes.size(); i++)
   {
      unsigned int j = istart + istep * i;
      unsigned int jbottom = istart + istep * (i - 1);
      for(unsigned int s = 0; s < _cache_composition[j].size(); s++)
      {
        _cache.at(_cache_altitudes[j])[s] = _cache.at(_cache_altitudes[jbottom])[s] + 
                                           _cache_composition[j][s] * (_cache_altitudes[j] - _cache_altitudes[jbottom]);
      }
   }

    _cache_composition.clear();
    _cache_altitudes.clear();
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template<typename StateType, typename VectorStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::first_guess(VectorStateType & molar_concentrations_first_guess, const StateType z) const
  {
      _composition->first_guess_densities(z,molar_concentrations_first_guess);
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template<typename VectorStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::lower_boundary_dirichlet(VectorStateType & lower_boundary) const
  {
      _composition->lower_boundary_concentrations(lower_boundary);
  }

  template <typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  template<typename VectorStateType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::upper_boundary_neumann(VectorStateType & upper_boundary, const VectorStateType &molar_densities) const
  {
      _composition->upper_boundary_fluxes(upper_boundary, molar_densities);
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::build(const GetPot& input)
  {
    this->build_temperature(input);

    this->build_species(input);

    // Must be called after: build_species
    this->build_reaction_sets(input);

    // Must be called after: build_temperature, build_species
    this->build_composition(input);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::build_temperature( const GetPot& input )
  {
    std::string input_T = input( "Planet/temperature_file", "DIE!" );
    std::vector<CoeffType> T0,Tz;
    this->read_temperature(T0,Tz,input_T);
    _temperature = new AtmosphericTemperature<CoeffType,VectorCoeffType>(T0,T0,Tz,Tz);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::build_species( const GetPot& input )
  {
    // Read neutral and ionic species from input
    unsigned int n_neutral = input.vector_variable_size("Planet/neutral_species");
    unsigned int n_ionic = input.vector_variable_size("Planet/ionic_species");

    std::vector<std::string> neutrals(n_neutral);
    std::vector<std::string> ions(n_ionic);

    for( unsigned int s = 0; s < n_neutral; s++ )
      {
        neutrals[s] = input("Planet/neutral_species", "DIE!", s);
      }

    for( unsigned int s = 0; s < n_neutral; s++ )
      {
        ions[s] = input("Planet/ionic_species", "DIE!", s);
      }

    _neutral_species = new Antioch::ChemicalMixture<CoeffType>(neutrals);
    _ionic_species = new Antioch::ChemicalMixture<CoeffType>(ions);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::build_reaction_sets( const GetPot& input )
  {
    // Build up reaction sets
    _neutral_reaction_set = new Antioch::ReactionSet<CoeffType>(*_neutral_species);
    _ionic_reaction_set = new Antioch::ReactionSet<CoeffType>(*_ionic_species);
    _neut_reac_theo = new Antioch::ReactionSet<CoeffType>(*_neutral_species);

    if( !input.have_variable("Planet/input_reactions_elem") )
      {
        std::cerr << "Error: could not find input_reactions_elem filename!" << std::endl;
        antioch_error();
      }

    if( !input.have_variable("Planet/input_reactions_fall") )
      {
        std::cerr << "Error: could not find input_reactions_fall filename!" << std::endl;
        antioch_error();
      }

    if( !input.have_variable("Planet/input_N2") )
      {
        std::cerr << "Error: could not find input_N2 filename!" << std::endl;
        antioch_error();
      }

    if( !input.have_variable("Planet/input_CH4") )
      {
        std::cerr << "Error: could not find input_CH4 filename!" << std::endl;
        antioch_error();
      }

    std::string input_reactions_elem = input( "Planet/input_reactions_elem", "DIE!" );
    std::string input_reactions_fall = input( "Planet/input_reactions_fall", "DIE!" );
    std::string input_N2  = input( "Planet/input_N2", "DIE!" );
    std::string input_CH4 = input( "Planet/input_CH4", "DIE!" );

    // here read the reactions, Antioch will take care of it once hdf5, no ionic reactions there
    //Kooij / Arrhenius + photochem
    this->fill_neutral_reactions_elementary(input_reactions_elem, input_N2, input_CH4, *_neutral_reaction_set);

    this->fill_neutral_reactions_falloff(input_reactions_fall, *_neutral_reaction_set);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::build_composition( const GetPot& input )
  {

    // Build AtmosphericMixture
    _composition = new AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType>( _neutral_species, _ionic_species, _temperature );

    if( !input.have_variable("Planet/zmin") )
      {
        std::cerr << "Error: zmin not found in input file!" << std::endl;
        antioch_error();
      }

    if( !input.have_variable("Planet/zmax") )
      {
        std::cerr << "Error: zmax not found in input file!" << std::endl;
        antioch_error();
      }

    CoeffType zmin = input("Planet/zmin", 0.0 );
    CoeffType zmax = input("Planet/zmin", 0.0 );

    //_composition->init_composition(molar_frac, dens_tot, zmin, zmax);
    //_composition->set_thermal_coefficient(tc);

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  void PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::read_temperature(VectorCoeffType& T0, VectorCoeffType& Tz, const std::string& file) const
  {
    T0.clear();
    Tz.clear();
    std::string line;
    std::ifstream temp(file);
    getline(temp,line);
    while(!temp.eof())
      {
        CoeffType t,tz,dt,dtz;
        temp >> t >> tz >> dt >> dtz;
        T0.push_back(t);
        Tz.push_back(tz);
      }
    temp.close();

    return;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  const AtmosphericMixture<CoeffType,VectorCoeffType,MatrixCoeffType>& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::composition() const
  {
    return *_composition;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  Antioch::ReactionSet<CoeffType>& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::neutral_reaction_set()
  {
    return *_neutral_reaction_set;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  const Antioch::ReactionSet<CoeffType>& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::ionic_reaction_set() const
  {
    return *_ionic_reaction_set;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  const PhotonOpacity<CoeffType,VectorCoeffType>& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::tau() const
  {
    return *_tau;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  const std::vector<std::vector<BinaryDiffusion<CoeffType> > >& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::bin_diff_coeff() const
  {
    return _bin_diff_coeff;
  }

  template<typename CoeffType, typename VectorCoeffType, typename MatrixCoeffType>
  const AtmosphericTemperature<CoeffType,VectorCoeffType>& PlanetPhysicsHelper<CoeffType,VectorCoeffType,MatrixCoeffType>::temperature() const
  {
    return *_temperature;
  }

} // end namespace Planet

#endif // PLANET_PLANET_PHYSICS_HELPER_H
