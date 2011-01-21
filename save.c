#include <math.h>
#include <stdlib.h>

#include "allvars.h"
#include "proto.h"

void write_particle_data(void)
{
    printf("\nWriting IC-file\n");
    
#define BUFFER 10
  size_t bytes;
  float *block;
  int *blockid;
  int blkheadsize = sizeof(int) + 4 * sizeof(char);
  int nextblock;
  long long *blocklongid;
  int blockmaxlen, maxidlen, maxlongidlen;
  int4byte dummy;
  FILE *fd;
  char buf[300];
  int i, k, pc;
#ifdef  PRODUCEGAS
  double meanspacing, shift_gas, shift_dm;
#endif


  if(NumPart == 0)
    return;

    sprintf(buf, "%s/%s", OutputDir, FileBase);

  if(!(fd = fopen(buf, "w")))
    {
      printf("Error. Can't write in file '%s'\n", buf);
      exit(10);
    }

  for(i = 0; i < 6; i++)
    {
      header.npart[i] = 0;
      header.npartTotal[i] = 0;
      header.mass[i] = 0;
    }


#ifdef MULTICOMPONENTGLASSFILE
  qsort(P, NumPart, sizeof(struct part_data), compare_type);	/* sort particles by type, because that's how they should be stored in a gadget binary file */

  for(i = 0; i < 3; i++)
    header.npartTotal[i] = header1.npartTotal[i] * GlassTileFac * GlassTileFac * GlassTileFac;

  for(i = 0; i < NumPart; i++)
    header.npart[P[i].Type]++;

  if(header.npartTotal[0])
    header.mass[0] =
      (OmegaBaryon) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / (header.npartTotal[0]);

  if(header.npartTotal[1])
    header.mass[1] =
      (Omega - OmegaBaryon - OmegaDM_2ndSpecies) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box,3) /
      (header.npartTotal[1]);

  if(header.npartTotal[2])
    header.mass[2] =
      (OmegaDM_2ndSpecies) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / (header.npartTotal[2]);


#else

  header.npart[1] = NumPart;
  header.npartTotal[1] = TotNumPart;
  header.npartTotalHighWord[1] = (TotNumPart >> 32);
  header.mass[1] = (Omega) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / TotNumPart;

#ifdef  PRODUCEGAS
  header.npart[0] = NumPart;
  header.npartTotal[0] = TotNumPart;
  header.npartTotalHighWord[0] = (TotNumPart >> 32);
  header.mass[0] = (OmegaBaryon) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / TotNumPart;
  header.mass[1] = (Omega - OmegaBaryon) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / TotNumPart;
#endif
#endif

#ifdef NEUTRINO_PAIRS
  header.npart[2] *= 2;
  header.npartTotal[2] *= 2;
  header.mass[2] /= 2;
#endif

  header.time = InitTime;
  header.redshift = 1.0 / InitTime - 1;

  header.flag_sfr = 0;
  header.flag_feedback = 0;
  header.flag_cooling = 0;
  header.flag_stellarage = 0;
  header.flag_metals = 0;

  /*FIXME*/
  header.num_files = 1;

  header.BoxSize = Box;
  header.Omega0 = Omega;
  header.OmegaLambda = OmegaLambda;
  header.HubbleParam = HubbleParam;

  header.flag_stellarage = 0;
  header.flag_metals = 0;
  header.flag_entropy_instead_u=0;	/*!< flags that IC-file contains entropy instead of u */
  header.flag_doubleprecision=0;	/*!< flags that snapshot contains double-precision instead of single precision */

  header.flag_ic_info=1;             /*!< flag to inform whether IC files are generated with ordinary Zeldovich approximation,*/
  header.lpt_scalingfactor=1;      /*!< scaling factor for 2lpt initial conditions */
  dummy = sizeof(header);
#ifdef FORMAT_TWO
      /*Write format 2 header header*/
      my_fwrite(&blkheadsize,sizeof(dummy),1,fd);
      my_fwrite("HEAD", sizeof(char), 4, fd);
      nextblock = dummy + 2 * sizeof(int);
      my_fwrite(&nextblock, sizeof(int), 1, fd);
      my_fwrite(&blkheadsize,sizeof(dummy),1,fd);
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  my_fwrite(&header, sizeof(header), 1, fd);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);


#ifdef  PRODUCEGAS
  meanspacing = Box / pow(TotNumPart, 1.0 / 3);
  shift_gas = -0.5 * (Omega - OmegaBaryon) / (Omega) * meanspacing;
  shift_dm = +0.5 * OmegaBaryon / (Omega) * meanspacing;
#endif


  if(!(block = malloc(bytes = BUFFER * 1024 * 1024)))
    {
      printf("failed to allocate memory for `block' (%g bytes).\n", (double) bytes);
      exit(24);
    }

  blockmaxlen = bytes / (3 * sizeof(float));

  blockid = (int *) block;
  blocklongid = (long long *) block;
  maxidlen = bytes / (sizeof(int));
  maxlongidlen = bytes / (sizeof(long long));

  /* write coordinates */
  dummy = sizeof(float) * 3 * NumPart;
#ifdef  PRODUCEGAS
  dummy *= 2;
#endif
#ifdef NEUTRINO_PAIRS
  dummy =
    sizeof(float) * 3 * (header.npart[0] + header.npart[1] + header.npart[2] + header.npart[3] +
			 header.npart[4] + header.npart[5]);
#endif
#ifdef FORMAT_TWO
          /*Write position header*/
	  blkheadsize = sizeof(int) + 4 * sizeof(char);
      	  my_fwrite(&blkheadsize,sizeof(int),1,fd);
      	  my_fwrite("POS ", sizeof(char), 4, fd);
	  nextblock=dummy+2*sizeof(int);
	  my_fwrite(&nextblock, sizeof(int), 1, fd);
	  my_fwrite(&blkheadsize, sizeof(int), 1, fd);
	  /*Done writing position header*/
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	{
	  block[3 * pc + k] = P[i].Pos[k];
#ifdef  PRODUCEGAS
	  block[3 * pc + k] = periodic_wrap(P[i].Pos[k] + shift_gas);
#endif
	}
      pc++;

#ifdef NEUTRINO_PAIRS
      if(P[i].Type == 2)
	{
	  for(k = 0; k < 3; k++)
	    block[3 * pc + k] = P[i].Pos[k];
	  pc++;
	}
#endif

      if(pc >= (blockmaxlen - 1))
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
#ifdef  PRODUCEGAS
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	{
	  block[3 * pc + k] = periodic_wrap(P[i].Pos[k] + shift_dm);
	}

      pc++;

      if(pc == blockmaxlen)
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);



  /* write velocities */
  dummy = sizeof(float) * 3 * NumPart;
#ifdef  PRODUCEGAS
  dummy *= 2;
#endif
#ifdef NEUTRINO_PAIRS
  dummy =
    sizeof(float) * 3 * (header.npart[0] + header.npart[1] + header.npart[2] + header.npart[3] +
			 header.npart[4] + header.npart[5]);
#endif
#ifdef FORMAT_TWO
          /*Write velocity header*/
	  blkheadsize = sizeof(int) + 4 * sizeof(char);
      	  my_fwrite(&blkheadsize,sizeof(int),1,fd);
      	  my_fwrite("VEL ", sizeof(char), 4, fd);
	  nextblock=dummy+2*sizeof(int);
	  my_fwrite(&nextblock, sizeof(int), 1, fd);
	  my_fwrite(&blkheadsize, sizeof(int), 1, fd);
	  /*Done writing velocity header*/
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	block[3 * pc + k] = P[i].Vel[k];

#ifdef MULTICOMPONENTGLASSFILE
      if(WDM_On == 1 && WDM_Vtherm_On == 1 && P[i].Type == 1)
	add_WDM_thermal_speeds(&block[3 * pc]);
#ifdef NEUTRINOS

#ifdef NEUTRINO_PAIRS
      if(NU_On == 1 && NU_Vtherm_On == 1 && P[i].Type == 2)
	{
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
      if(NU_On == 1 && NU_Vtherm_On == 1 && P[i].Type == 2)
	add_NU_thermal_speeds(&block[3 * pc]);
#endif

#endif
#else
#ifndef PRODUCEGAS
      if(WDM_On == 1 && WDM_Vtherm_On == 1)
	add_WDM_thermal_speeds(&block[3 * pc]);
#endif
#endif

      pc++;

      if(pc >= (blockmaxlen - 1))
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
#ifdef PRODUCEGAS
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	block[3 * pc + k] = P[i].Vel[k];

      if(WDM_On == 1 && WDM_Vtherm_On == 1)
	add_WDM_thermal_speeds(&block[3 * pc]);

      pc++;

      if(pc == blockmaxlen)
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);


  /* write particle ID */
#ifdef NO64BITID
  dummy = sizeof(int) * NumPart;
#else
  dummy = sizeof(long long) * NumPart;
#endif
#ifdef  PRODUCEGAS
  dummy *= 2;
#endif
#ifdef NEUTRINO_PAIRS
  dummy =
    sizeof(int) * (header.npart[0] + header.npart[1] + header.npart[2] + header.npart[3] + header.npart[4] +
		   header.npart[5]);
#endif
#ifdef FORMAT_TWO
          /*Write ID header*/
	  blkheadsize = sizeof(int) + 4 * sizeof(char);
      	  my_fwrite(&blkheadsize,sizeof(int),1,fd);
      	  my_fwrite("ID  ", sizeof(char), 4, fd);
	  nextblock=dummy+2*sizeof(int);
	  my_fwrite(&nextblock, sizeof(int), 1, fd);
	  my_fwrite(&blkheadsize, sizeof(int), 1, fd);
	  /*Done writing ID header*/
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
#ifdef NO64BITID
      blockid[pc] = P[i].ID;
#else
      blocklongid[pc] = P[i].ID;
#endif

      pc++;

#ifdef NEUTRINO_PAIRS
      if(P[i].Type == 2)
	{
#ifdef NO64BITID
	  blockid[pc] =
	    P[i].ID + header.npartTotal[0] + header.npartTotal[1] + header.npartTotal[2] +
	    header.npartTotal[3] + header.npartTotal[4] + header.npartTotal[5];
#else
	  blocklongid[pc] =
	    P[i].ID + header.npartTotal[0] + header.npartTotal[1] + header.npartTotal[2] +
	    header.npartTotal[3] + header.npartTotal[4] + header.npartTotal[5];
#endif
	  pc++;
	}
#endif

      if(pc >= (maxlongidlen - 1))
	{
#ifdef NO64BITID
	  my_fwrite(blockid, sizeof(int), pc, fd);
#else
	  my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
	  pc = 0;
	}
    }
  if(pc > 0)
    {
#ifdef NO64BITID
      my_fwrite(blockid, sizeof(int), pc, fd);
#else
      my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
    }

#ifdef PRODUCEGAS
  for(i = 0, pc = 0; i < NumPart; i++)
    {
#ifdef NO64BITID
      blockid[pc] = P[i].ID + TotNumPart;
#else
      blocklongid[pc] = P[i].ID + TotNumPart;
#endif

      pc++;

      if(pc == maxlongidlen)
	{
#ifdef NO64BITID
	  my_fwrite(blockid, sizeof(int), pc, fd);
#else
	  my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
	  pc = 0;
	}
    }
  if(pc > 0)
    {
#ifdef NO64BITID
      my_fwrite(blockid, sizeof(int), pc, fd);
#else
      my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
    }
#endif

  my_fwrite(&dummy, sizeof(dummy), 1, fd);





  /* write zero temperatures if needed */
#ifdef  PRODUCEGAS
  dummy = sizeof(float) * NumPart;
#ifdef FORMAT_TWO
          /*Write temperature header*/
	  blkheadsize = sizeof(int) + 4 * sizeof(char);
      	  my_fwrite(&blkheadsize,sizeof(int),1,fd);
      	  my_fwrite("U   ", sizeof(char), 4, fd);
	  nextblock=dummy+2*sizeof(int);
	  my_fwrite(&nextblock, sizeof(int), 1, fd);
	  my_fwrite(&blkheadsize, sizeof(int), 1, fd);
	  /*Done writing temperature header*/
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      block[pc] = 0;

      pc++;

      if(pc == blockmaxlen)
	{
	  my_fwrite(block, sizeof(float), pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), pc, fd);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
#endif


  /* write zero temperatures if needed */
#ifdef  MULTICOMPONENTGLASSFILE
  if(header.npart[0])
    {
      dummy = sizeof(float) * header.npart[0];
#ifdef FORMAT_TWO
          /*Write temperature header*/
	  blkheadsize = sizeof(int) + 4 * sizeof(char);
      	  my_fwrite(&blkheadsize,sizeof(int),1,fd);
      	  my_fwrite("U   ", sizeof(char), 4, fd);
	  nextblock=dummy+2*sizeof(int);
	  my_fwrite(&nextblock, sizeof(int), 1, fd);
	  my_fwrite(&blkheadsize, sizeof(int), 1, fd);
	  /*Done writing temperature header*/
#endif
      my_fwrite(&dummy, sizeof(dummy), 1, fd);

      for(i = 0, pc = 0; i < header.npart[0]; i++)
	{
	  block[pc] = 0;

	  pc++;

	  if(pc == blockmaxlen)
	    {
	      my_fwrite(block, sizeof(float), pc, fd);
	      pc = 0;
	    }
	}
      if(pc > 0)
	my_fwrite(block, sizeof(float), pc, fd);
      my_fwrite(&dummy, sizeof(dummy), 1, fd);
    }
#endif



  free(block);

  fclose(fd);
  printf("Finished writing IC file.\n");
  return;
}


/* This catches I/O errors occuring for my_fwrite(). In this case we better stop.
 */
size_t my_fwrite(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  size_t nwritten;

  if((nwritten = fwrite(ptr, size, nmemb, stream)) != nmemb)
    {
      printf("I/O error (fwrite) has occured.\n");
      fflush(stdout);
      exit(777);
    }
  return nwritten;
}


/* This catches I/O errors occuring for fread(). In this case we better stop.
 */
size_t my_fread(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  size_t nread;

  if((nread = fread(ptr, size, nmemb, stream)) != nmemb)
    {
      printf("I/O error (fread) has occured.\n");
      fflush(stdout);
      exit(778);
    }
  return nread;
}


#ifdef MULTICOMPONENTGLASSFILE
int compare_type(const void *a, const void *b)
{
  if(((struct part_data *) a)->Type < (((struct part_data *) b)->Type))
    return -1;

  if(((struct part_data *) a)->Type > (((struct part_data *) b)->Type))
    return +1;

  return 0;
}
#endif
