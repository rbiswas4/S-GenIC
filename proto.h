#ifndef __PROTO_H
#define __PROTO_H

#include <gsl/gsl_rng.h>

#ifdef __cplusplus
#include <gadgetreader.hpp>
#include <gadgetwriter.hpp>
#include "part_data.hpp"

#ifdef PRINT_SPEC
void   print_spec(int type);
#endif
int    FatalError(int errnum);
void displacement_fields(const int type, const int64_t NumPart, part_data& P, const int Nmesh, bool RayleighScatter);
void   initialize_ffts(void);
unsigned int * initialize_rng(int Seed);
void   set_units(void);
double fnl(double x);

double displacement_read_out(float * Disp, const int order, const int64_t NumPart, part_data& P, const int Nmesh, const int axes);

double periodic_wrap(double x);

int64_t write_particle_data(GadgetWriter::GWriteSnap & snap, int type, part_data&  P, int64_t NumPart, int64_t FirstId);

gadget_header generate_header(std::valarray<int64_t> & npart);


extern "C" {
#endif

void   read_power_table(void);
int    compare_logk(const void *a, const void *b);
double PowerSpec(double kmag, int Type);
double PowerSpec_Efstathiou(double k);
double PowerSpec_EH(double k);
double PowerSpec_Tabulated(double k,int Type);
double PowerSpec_DM_2ndSpecies(double k);

void   initialize_powerspectrum(void);
double GrowthFactor(double astart, double aend);
double growth(double a);
double growth_int(double, void *);
double sigma2_int(double k, void * params);
double TopHatSigma2(double R);
double F_Omega(double a);
double F2_Omega(double a);

void  read_parameterfile(char *fname);
double tk_eh(double k);


void add_WDM_thermal_speeds(float *vel);
void add_NU_thermal_speeds(float *vel);
double get_fermi_dirac_vel_nu(void);
void fermi_dirac_init_nu(void);
#ifdef __cplusplus
}
#endif

#endif //__PROTO_H
