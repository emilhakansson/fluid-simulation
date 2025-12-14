# Project
add_executable (EDAN35_FluidSimulation)
target_sources (
	EDAN35_FluidSimulation
	PRIVATE
		[[fluidsimulation.hpp]]
		[[fluidsimulation.cpp]] "neighborhood_search.cpp" "Particle.h")

target_link_libraries (EDAN35_FluidSimulation PRIVATE assignment_setup parametric_shapes)

install (TARGETS EDAN35_FluidSimulation DESTINATION bin)
