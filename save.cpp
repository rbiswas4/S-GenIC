#include "allvars.h"
#include "proto.h"
#include <gadgetwriter.hpp>

using namespace GadgetWriter;
using namespace std;

#define BUFFER 10

int64_t write_particle_data(GWriteSnap & snap, int type, struct part_data * P, int64_t NumPart, int64_t FirstId)
{
  size_t bytes;
  float *block;
  id_type *blockid;
  int blockmaxlen;
  int64_t maxidlen,written=0;
  int i,k,pc;
    
  printf("\nWriting IC-file\n");

  if(!(block = (float *) malloc(bytes = BUFFER * 1024 * 1024)))
    {
      printf("failed to allocate memory for `block' (%g bytes).\n", (double) bytes);
      exit(24);
    }

  blockmaxlen = bytes / (3 * sizeof(float));

  /*We are about to write the POS block*/
  for(i = 0, pc = 0; i < NumPart; i++){
      for(k = 0; k < 3; k++)
	  block[3 * pc + k] = P[i].Pos[k];
      pc++;

#ifdef NEUTRINO_PAIRS
      /*Add an extra copy of the position vector for the double neutrino*/
      if(type == NEUTRINO_TYPE) {
	  for(k = 0; k < 3; k++)
	    block[3 * pc + k] = P[i].Pos[k];
	  pc++;
      }
#endif //NEUTRINO_PAIRS

      if(pc > blockmaxlen){
	  snap.WriteBlocks(string("POS "),type, block, pc,written);
          written+=pc;
	  pc = 0;
	}
  }
  if(pc > 0)
	  snap.WriteBlocks(string("POS "),type, block, pc,written);
  /*Done writing POS block*/
  written=0;

  /* write velocities: sizes are the same as for positions */
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	block[3 * pc + k] = P[i].Vel[k];

      if(WDM_On == 1 && WDM_Vtherm_On == 1 && type == 1)
	add_WDM_thermal_speeds(&block[3 * pc]);
#ifdef NEUTRINOS

#ifdef NEUTRINO_PAIRS
      if(NU_On == 1 && NU_Vtherm_On == 1 && type == 2) {
	  float vtherm[3];
	  for(k = 0; k < 3; k++)
	    vtherm[k] = 0;
	  add_NU_thermal_speeds(vtherm);
	  for(k = 0; k < 3; k++)
	    block[3 * pc + k] = P[i].Vel[k] + vtherm[k];
	  pc++;
	  for(k = 0; k < 3; k++)
	    block[3 * pc + k] = P[i].Vel[k] - vtherm[k];
	}
#else
      if(NU_On == 1 && NU_Vtherm_On == 1 && type == 2)
	add_NU_thermal_speeds(&block[3 * pc]);
#endif //NEUTRINO_PAIRS

#endif //NEUTRINOS
      pc++;

      if(pc > blockmaxlen){
	  snap.WriteBlocks(string("VEL "),type, block, pc,written);
          written+=pc;
	  pc = 0;
	}
    }
  if(pc > 0)
	  snap.WriteBlocks(string("VEL "),type, block, pc,written);

  /* write particle ID */
  written=0;
  blockid = (id_type *) block;
  maxidlen = bytes / (sizeof(id_type));
  
  for(i = 0, pc = 0; i < NumPart; i++){
      blockid[pc] = i+FirstId;
      pc++;

#ifdef NEUTRINO_PAIRS 
      if(type == 2) {
	  blockid[pc] = i+FirstId+NumPart;
	  pc++;
	}
#endif //NEUTRINO_PAIRS

      if(pc > blockmaxlen){
	  snap.WriteBlocks(string("ID  "),type, block, pc,written);
          written+=pc;
	  pc = 0;
      }
  }
  if(pc > 0)
	  snap.WriteBlocks(string("ID  "),type, block, pc,written);

  /*Done writing IDs*/

  /* write zero temperatures if needed */
  if(type== 0) {
          written=0;

      for(i = 0, pc = 0; i < NumPart; i++){
	  block[pc] = 0;
	  pc++;
          if(pc > blockmaxlen){
              snap.WriteBlocks(string("U   "),type, block, pc,written);
              written+=pc;
              pc = 0;
          }
      }
      if(pc > 0)
	  snap.WriteBlocks(string("U   "),type, block, pc,written);
  }
  /*Done writing temperatures*/

  free(block);
  printf("Finished writing IC file.\n");
  FirstId+=NumPart;
#ifdef NEUTRINO_PAIRS
  if(type==2)
          FirstId+=NumPart;
#endif
  return FirstId;
}
