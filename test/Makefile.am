BUILT_SOURCES   = .license.stamp

EXTRA_DIST = input_files

check_PROGRAMS  = 
check_PROGRAMS += binary_diffusion_unit
check_PROGRAMS += chapman_unit
check_PROGRAMS += temperature_unit
check_PROGRAMS += photon_opacity_unit
check_PROGRAMS += atmospheric_mixture_unit
check_PROGRAMS += photon_evaluator_unit
check_PROGRAMS += eddy_diffusion_evaluator_unit
check_PROGRAMS += molecular_diffusion_evaluator_unit
check_PROGRAMS += diffusion_evaluator_unit
check_PROGRAMS += physics_helper_unit
check_PROGRAMS += solver_test

AM_CPPFLAGS  = 
AM_CPPFLAGS += -I$(top_srcdir)/src/core/include
AM_CPPFLAGS += -I$(top_srcdir)/src/atmosphere/include
AM_CPPFLAGS += -I$(top_srcdir)/src/kinetics/include
AM_CPPFLAGS += -I$(top_srcdir)/src/temperature/include
AM_CPPFLAGS += -I$(top_srcdir)/src/photon_flux/include
AM_CPPFLAGS += -I$(top_srcdir)/src/absorption/include
AM_CPPFLAGS += -I$(top_srcdir)/src/sampling/include
AM_CPPFLAGS += -I$(top_srcdir)/src/diffusion/include
AM_CPPFLAGS += -I$(top_srcdir)/src/utilities/include
AM_CPPFLAGS += -I$(top_srcdir)/src/grins_interface/include
AM_CPPFLAGS += -I$(top_builddir)/src/utilities/include #planet_version.h

AM_LDFLAGS = 

LIBS = 

#Planet
LIBS += $(LIBPLANET_LIBS)

#Antioch
AM_CPPFLAGS += $(ANTIOCH_CPPFLAGS)

#GRINS
AM_CPPFLAGS += $(GRINS_CPPFLAGS)
AM_LDFLAGS += $(GRINS_LDFLAGS)
LIBS += $(LIBGRINS_LIBS)

# Sources for these tests
binary_diffusion_unit_SOURCES = binary_diffusion_unit.C
chapman_unit_SOURCES = chapman_unit.C
temperature_unit_SOURCES = temperature_unit.C
photon_opacity_unit_SOURCES = photon_opacity_unit.C
atmospheric_mixture_unit_SOURCES = atmospheric_mixture_unit.C
photon_evaluator_unit_SOURCES = photon_evaluator_unit.C
eddy_diffusion_evaluator_unit_SOURCES = eddy_diffusion_evaluator_unit.C
molecular_diffusion_evaluator_unit_SOURCES = molecular_diffusion_evaluator_unit.C
diffusion_evaluator_unit_SOURCES = diffusion_evaluator_unit.C
physics_helper_unit_SOURCES = physics_helper_unit.C
solver_test_SOURCES = solver_test.C

#Define tests to actually be run
TESTS = 
TESTS += binary_diffusion_unit 
TESTS += chapman_unit
TESTS += temperature_unit.sh
TESTS += photon_opacity_unit.sh
TESTS += atmospheric_mixture_unit.sh
TESTS += photon_evaluator_unit.sh
TESTS += eddy_diffusion_evaluator_unit.sh
TESTS += molecular_diffusion_evaluator_unit.sh
TESTS += diffusion_evaluator_unit.sh
TESTS += physics_helper_unit.sh
TESTS += solver_test.sh

CLEANFILES =
if CODE_COVERAGE_ENABLED
  CLEANFILES += *.gcda *.gcno
endif

.license.stamp: $(top_srcdir)/LICENSE
	$(top_srcdir)/src/common/lic_utils/update_license.pl $(top_srcdir)/LICENSE \
	$(top_srcdir)/test/*.C
	echo 'updated source license headers' >$@

CLEANFILES += .license.stamp

# Required for AX_AM_MACROS
###@INC_AMINCLUDE@
