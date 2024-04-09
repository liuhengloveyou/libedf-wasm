/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2017- 2024 Teunis van Beelen
*
* Email: teuniz@protonmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
***************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "../edflib.h"


#define  JUMP_TO_EXIT_ERROR_PROC   {line = __LINE__; goto OUT_ERROR;}

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

typedef struct raw_header_struct
{
  int total_chans;
  int regular_chans;
  int annot_chans;
  int regular_chans_idx_list[EDFLIB_MAXSIGNALS];
  int annot_chans_idx_list[EDFLIB_MAXSIGNALS];
  int bytes_in_datrec[EDFLIB_MAXSIGNALS];
  int datrec_offset[EDFLIB_MAXSIGNALS];
  int hdr_sz;
  int datrecs;
  int datrec_sz;
  int sample_width;
} raw_hdr_t;

int dblcmp(double, double);
int dblcmp_lim(double, double, double);
int get_raw_header(const char *, raw_hdr_t *);
int get_file_offset(int, int, int, raw_hdr_t *);


int main(void)
{
  int i, j,
      tmp,
      hdl=-1,
      chns=2,
      *ibuf=NULL,
      line,
      ival1,
      ival2;

  char *str=NULL,
       *pbuf=NULL,
       *i3buf=NULL;

  short *sbuf=NULL;

  long long l_tmp;

  double *dbuf=NULL;

  raw_hdr_t rawhdr;

  union
  {
    unsigned int one;
    signed int one_signed;
    unsigned short two[2];
    signed short two_signed[2];
    unsigned char four[4];
    signed char four_signed[4];
  } var;

  edflib_hdr_t hdr;

  edflib_annotation_t annot;

  FILE *fp=NULL;

  setlocale(LC_ALL, "C");

  if(edflib_version() != 125)  JUMP_TO_EXIT_ERROR_PROC

  ibuf = (int *)malloc(50000 * sizeof(int));
  if(ibuf == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }

  sbuf = (short *)malloc(50000 * sizeof(short));
  if(sbuf == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }

  dbuf = (double *)malloc(50000 * sizeof(double));
  if(dbuf == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }

  str = (char *)malloc(4096 * sizeof(char));
  if(str == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }
  str[0] = 0;

  pbuf = (char *)malloc(300 * sizeof(char));
  if(pbuf == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }

  i3buf = (char *)malloc(50000 * 3);
  if(i3buf == NULL)
  {
    JUMP_TO_EXIT_ERROR_PROC;
  }

/********************************** BDF writing ******************************/

  hdl = edfopen_file_writeonly_with_params("test.bdf", EDFLIB_FILETYPE_BDFPLUS, 17, 2799, 300000, "uV");

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_annot_chan_idx_pos(hdl, EDF_ANNOT_IDX_POS_START))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_number_of_annotation_signals(hdl, 3))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<2799; i++)
  {
    dbuf[i] = i;
  }

  for(i=0; i<25; i++)
  {
    for(j=0; j<17; j++)
    {
      if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<75; i++)
  {
    snprintf(str, 4096, "test %i", i + 1);

    if(edfwrite_annotation_latin1_hr(hdl, i * 1000000, -1, str))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

/********************************** BDF reading ******************************/

  if(get_raw_header("test.bdf", &rawhdr))  JUMP_TO_EXIT_ERROR_PROC

  if(rawhdr.annot_chans != 3)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<rawhdr.annot_chans; i++)
  {
    if(rawhdr.annot_chans_idx_list[i] != i)  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 3)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 17)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 250000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 10000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 25)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 69975)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 8388607)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -8388608)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 2799)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uV      "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "X"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.annotations_in_file != 75)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** BDF writing ******************************/

  hdl = edfopen_file_writeonly_with_params("test.bdf", EDFLIB_FILETYPE_BDFPLUS, 17, 2799, 300000, "uV");

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_annot_chan_idx_pos(hdl, EDF_ANNOT_IDX_POS_MIDDLE))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_number_of_annotation_signals(hdl, 2))  JUMP_TO_EXIT_ERROR_PROC

  for(j=0; j<17; j++)
  {
    for(i=0; i<2799; i++)
    {
      dbuf[(j * 2799) + i] = i;
    }
  }

  for(i=0; i<5; i++)
  {
    for(j=0; j<17; j++)
    {
      if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(j=0; j<17; j++)
  {
    for(i=0; i<2799; i++)
    {
      ibuf[i + (j * 2799)] = (i * -27.96202667) + 0.5;
    }
  }

  for(i=0; i<5; i++)
  {
    for(j=0; j<17; j++)
    {
      if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(j=0; j<17; j++)
  {
    for(i=0; i<2799; i++)
    {
      var.one_signed = (i * 27.96202667) + 0.5;

      i3buf[(i * 3) + (j * 2799 * 3) + 0] = var.four[0];
      i3buf[(i * 3) + (j * 2799 * 3) + 1] = var.four[1];
      i3buf[(i * 3) + (j * 2799 * 3) + 2] = var.four[2];
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_digital_3byte_samples(hdl, i3buf))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<50; i++)
  {
    snprintf(str, 4096, "test %i", i + 1);

    if(edfwrite_annotation_latin1_hr(hdl, i * 1000000, -1, str))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

/********************************** BDF reading ******************************/

  if(get_raw_header("test.bdf", &rawhdr))  JUMP_TO_EXIT_ERROR_PROC

  if(rawhdr.annot_chans != 2)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<rawhdr.annot_chans; i++)
  {
    if(rawhdr.annot_chans_idx_list[i] != i + 8)  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 3)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 17)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 250000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 10000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 25)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 69975)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 8388607)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -8388608)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 2799)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uV      "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "X"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.annotations_in_file != 50)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF writing ******************************/

  hdl = edfopen_file_writeonly_with_params("test.edf", EDFLIB_FILETYPE_EDFPLUS, 65, 633, 3000, "uV");

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_annot_chan_idx_pos(hdl, EDF_ANNOT_IDX_POS_MIDDLE))  JUMP_TO_EXIT_ERROR_PROC

  for(j=0; j<65; j++)
  {
    for(i=0; i<633; i++)
    {
      dbuf[(j * 633) + i] = i;
    }
  }

  for(i=0; i<5; i++)
  {
    for(j=0; j<65; j++)
    {
      if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(j=0; j<65; j++)
  {
    for(i=0; i<633; i++)
    {
      ibuf[i + (j * 633)] = (i * -10.92266667) + 0.5;
    }
  }

  for(i=0; i<5; i++)
  {
    for(j=0; j<65; j++)
    {
      if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(j=0; j<65; j++)
  {
    for(i=0; i<633; i++)
    {
      sbuf[i + (j * 633)] = (i * -10.92266667) + 0.5;
    }
  }

  for(i=0; i<5; i++)
  {
    for(j=0; j<65; j++)
    {
      if(edfwrite_digital_short_samples(hdl, sbuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  for(i=0; i<5; i++)
  {
    if(edf_blockwrite_digital_short_samples(hdl, sbuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 65)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 300000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 10000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 30)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 18990)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 3000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -3000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 32767)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -32768)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 633)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uV      "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "X"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF writing ******************************/

  hdl = edfopen_file_writeonly_with_params("test.edf", EDFLIB_FILETYPE_EDFPLUS, 65, 633, 3000, "uV");

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientname(hdl, "XY_Z"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientcode(hdl, "X2_3"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_admincode(hdl, "X6_7"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_technician(hdl, "X.Fo_o"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_equipment(hdl, "Xe_q"))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<633; i++)
  {
    dbuf[i] = i;
  }

  for(i=0; i<10; i++)
  {
    for(j=0; j<65; j++)
    {
      if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 65)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 100000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 10000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 10)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 6330)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 3000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -3000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 32767)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -32768)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 633)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uV      "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, "X2 3"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "XY Z"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, "X6 7"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, "X.Fo o"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, "Xe q"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF writing ******************************/

  hdl = edfopen_file_writeonly("test.edf", EDFLIB_FILETYPE_EDFPLUS, 512);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<512; i++)
  {
    if(edf_set_samplefrequency(hdl, i, 10239))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_maximum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_minimum(hdl, i, -30000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_maximum(hdl, i, 10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_minimum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<10239; i++)
  {
    dbuf[i] = 0;
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = edfopen_file_writeonly("test.edf", EDFLIB_FILETYPE_EDFPLUS, 512);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<512; i++)
  {
    if(edf_set_samplefrequency(hdl, i, 10240))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_maximum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_minimum(hdl, i, -30000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_maximum(hdl, i, 10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_minimum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<10240; i++)
  {
    dbuf[i] = 0;
  }

  if(edfwrite_physical_samples(hdl, dbuf) != EDFLIB_DATARECORD_SIZE_TOO_BIG)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl) == 0)
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = edfopen_file_writeonly("test.edf", EDFLIB_FILETYPE_EDFPLUS, chns);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_samplefrequency(hdl, 0, 20))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_samplefrequency(hdl, 1, 23))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 0, 10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 0, -5000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 1, -10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 1, -30000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 0, 10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 0, -10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 1, 30000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 1, 10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_label(hdl, 0, "trace1"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_label(hdl, 1, "trace2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_prefilter(hdl, 0, "qwerty"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_prefilter(hdl, 1, "zxcvbn"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_transducer(hdl, 0, "asdfgh"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_transducer(hdl, 1, "poklhyg"))  JUMP_TO_EXIT_ERROR_PROC

  strcpy(str, "uVxxxxxxxxxxxxxxxxxxxx");

  str[0] = 181;

  if(edf_set_physical_dimension(hdl, 0, str))  JUMP_TO_EXIT_ERROR_PROC

  strcpy(str, "dCxxxxxxxxxxxxxxxxxxxx");

  str[0] = 176;
  str[2] = 248;

  if(edf_set_physical_dimension(hdl, 1, str))  JUMP_TO_EXIT_ERROR_PROC

  if(!edf_set_startdatetime(hdl, 2085, 12, 5, 12, 23, 8))  JUMP_TO_EXIT_ERROR_PROC

  if(!edf_set_startdatetime(hdl, 1984, 12, 5, 12, 23, 8))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_startdatetime(hdl, 2017, 12, 5, 12, 23, 8))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientname(hdl, "John Doü"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientcode(hdl, "01234"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_sex(hdl, 1))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_birthdate(hdl, 2010, 7, 4))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patient_additional(hdl, "nop"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_admincode(hdl, "789"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_technician(hdl, "Rìchard Roë"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_equipment(hdl, "device"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_number_of_annotation_signals(hdl, 3))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_datarecord_duration(hdl, 13000))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1_hr(hdl, 0, -1, "Recording starts"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1_hr(hdl, 900000, 100000, "Test 1"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_utf8_hr(hdl, 9123456, 1123456, "Test 2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1_hr(hdl, 1300000, -1, "Recording ends"))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    dbuf[i] = -5100 + (i * 800);
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
    dbuf[i] = -30100 + (i * 909);
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    dbuf[i] = -5100 + (i * 800);
  }

  for(i=0; i<23; i++)
  {
    dbuf[i + 20] = -30100 + (i * 909);
  }

  if(edf_blockwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    sbuf[i] = -10100 + (i * 1053);
  }

  if(edfwrite_digital_short_samples(hdl, sbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
    sbuf[i] = 9900 + (i * 1053);
  }

  if(edfwrite_digital_short_samples(hdl, sbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    sbuf[i] = -10100 + (i * 1053);
  }

  for(i=0; i<23; i++)
  {
    sbuf[i + 20] = 9900 + (i * 1053);
  }

  if(edf_blockwrite_digital_short_samples(hdl, sbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    ibuf[i] = -10100 + (i * 1053);
  }

  if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
    ibuf[i] = 9900 + (i * 1053);
  }

  if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    ibuf[i] = -10100 + (i * 1053);
  }

  for(i=0; i<23; i++)
  {
    ibuf[i + 20] = 9900 + (i * 1053);
  }

  if(edf_blockwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  ival1 = -10100;

  ival2 = 9900;

  for(j=0; j<4; j++)
  {
    for(i=0; i<20; i++)
    {
      ibuf[i] = ival1;

      ival1 += 253;
    }

    if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

    for(i=0; i<23; i++)
    {
      ibuf[i] = ival2;

      ival2 += 253;
    }

    if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

/********************************** BDF writing ******************************/

  hdl = edfopen_file_writeonly("test.bdf", EDFLIB_FILETYPE_BDFPLUS, 512);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<512; i++)
  {
    if(edf_set_samplefrequency(hdl, i, 10239))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_maximum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_minimum(hdl, i, -30000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_maximum(hdl, i, 10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_minimum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<10239; i++)
  {
    dbuf[i] = 0;
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = edfopen_file_writeonly("test.bdf", EDFLIB_FILETYPE_BDFPLUS, 512);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<512; i++)
  {
    if(edf_set_samplefrequency(hdl, i, 10240))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_maximum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_physical_minimum(hdl, i, -30000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_maximum(hdl, i, 10000))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_set_digital_minimum(hdl, i, -10000))  JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<10240; i++)
  {
    dbuf[i] = 0;
  }

  if(edfwrite_physical_samples(hdl, dbuf) != EDFLIB_DATARECORD_SIZE_TOO_BIG)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl) == 0)
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = edfopen_file_writeonly("test.bdf", EDFLIB_FILETYPE_BDFPLUS, chns);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_samplefrequency(hdl, 0, 20))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_samplefrequency(hdl, 1, 23))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 0, 10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 0, -5000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 1, -10000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 1, -30000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 0, 1000000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 0, -1000000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 1, 3000000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 1, 1000000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_label(hdl, 0, "trace1"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_label(hdl, 1, "trace2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_prefilter(hdl, 0, "qwerty"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_prefilter(hdl, 1, "zxcvbn"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_transducer(hdl, 0, "asdfgh"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_transducer(hdl, 1, "poklhyg"))  JUMP_TO_EXIT_ERROR_PROC

  strcpy(str, "uVxxxxxxxxxxxxxxxxxxxx");

  str[0] = 181;

  if(edf_set_physical_dimension(hdl, 0, str))  JUMP_TO_EXIT_ERROR_PROC

  strcpy(str, "dCxxxxxxxxxxxxxxxxxxxx");

  str[0] = 176;
  str[2] = 248;

  if(edf_set_physical_dimension(hdl, 1, str))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_startdatetime(hdl, 2017, 12, 5, 12, 23, 8))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientname(hdl, "John Doe"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientcode(hdl, "01234"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_sex(hdl, 0))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_birthdate(hdl, 2010, 7, 4))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patient_additional(hdl, "nop"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_admincode(hdl, "789"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_technician(hdl, "Richard Roe"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_equipment(hdl, "device"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_number_of_annotation_signals(hdl, 3))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_datarecord_duration(hdl, 13000))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1(hdl, 0, -1, "Recording starts"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1(hdl, 6000, 2000, "Test 2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfwrite_annotation_latin1(hdl, 11700, -1, "Recording ends"))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    dbuf[i] = -5100 + (i * 800);
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
    dbuf[i] = -30100 + (i * 909);
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    dbuf[i] = -5100 + (i * 800);
  }

  for(i=0; i<23; i++)
  {
    dbuf[i + 20] = -30100 + (i * 909);
  }

  if(edf_blockwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    tmp = -1010000 + (i * 105300);

    pbuf[i * 3] = tmp & 0xff;

    pbuf[i * 3 + 1] = (tmp >> 8) & 0xff;

    pbuf[i * 3 + 2] = (tmp >> 16) & 0xff;
  }

  for(i=0; i<23; i++)
  {
    tmp = 990000 + (i * 105300);

    pbuf[i * 3 + 60] = tmp & 0xff;

    pbuf[i * 3 + 61] = (tmp >> 8) & 0xff;

    pbuf[i * 3 + 62] = (tmp >> 16) & 0xff;
  }

  if(edf_blockwrite_digital_3byte_samples(hdl, pbuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    ibuf[i] = -1010000 + (i * 105300);
  }

  if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
    ibuf[i] = 990000 + (i * 105300);
  }

  if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
    ibuf[i] = -1010000 + (i * 105300);
  }

  for(i=0; i<23; i++)
  {
    ibuf[i + 20] = 990000 + (i * 105300);
  }

  if(edf_blockwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

  ival1 = -1010000;

  ival2 = 990000;

  for(j=0; j<4; j++)
  {
    for(i=0; i<20; i++)
    {
      ibuf[i] = ival1;

      ival1 += 25300;
    }

    if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC

    for(i=0; i<23; i++)
    {
      ibuf[i] = ival2;

      ival2 += 25300;
    }

    if(edfwrite_digital_samples(hdl, ibuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 2)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 13000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_day != 5)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_month != 12)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_year != 2017)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_second != 8)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_minute != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_hour != 12)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_subsecond != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "John Dou"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, "01234"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.sex, "Male"))  JUMP_TO_EXIT_ERROR_PROC
  if(strcmp(hdr.gender, "Male"))  JUMP_TO_EXIT_ERROR_PROC  /* DEPRECATED */

  if(strcmp(hdr.birthdate, "04 jul 2010"))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_day != 4)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_month != 7)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_year != 2010)  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.patient_additional, "nop", 3))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, "789"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, "Richard Roe"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, "device"))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 1300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 10)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.annotations_in_file != 4)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].label, "trace1          "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[1].label, "trace2          "))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 200)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].smp_in_file != 230)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].phys_max != -10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -5000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].phys_min != -30000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].dig_max != 30000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].dig_min != 10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 20)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].smp_in_datarecord != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uVxxxxxx"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[1].physdimension, ".C0xxxxx"))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[0].prefilter, "qwerty   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[1].prefilter, "zxcvbn   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[0].transducer, "asdfgh   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[1].transducer, "poklhyg  ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 0, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != -10000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Recording starts"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 1, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 9000000)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, "0.100000"))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != 1000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Test 1"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 2, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 91234560)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, "1.123456"))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != 11234560LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Test 2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 3, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 13000000)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != -10000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Recording ends"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 1, 400, EDFSEEK_SET) == 400)  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 0, 412, EDFSEEK_SET) == 412)  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 0, 20, EDFSEEK_SET) != 20)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_physical_samples(hdl, 0, 20, dbuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%f   %i\n", dbuf[i], -5100 + (i * 800));

    if(i == 0)
    {
      if(dblcmp(dbuf[i], -5000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 19)
    {
      if(dblcmp(dbuf[i], 10000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp_lim(dbuf[i], -5100 + (i * 800), 0.75))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 23, EDFSEEK_SET) != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_physical_samples(hdl, 1, 23, dbuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%f   %i\n", dbuf[i], -30100 + (i * 909));

    if(i == 0)
    {
      if(dblcmp(dbuf[i], -30000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp(dbuf[i], -30100 + (i * 909)))  JUMP_TO_EXIT_ERROR_PROC
  }

  edfrewind(hdl, 0);

  if(edfread_physical_samples(hdl, 0, 20, dbuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%f   %i\n", dbuf[i], -5100 + (i * 800));

    if(i == 0)
    {
      if(dblcmp(dbuf[i], -5000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 19)
    {
      if(dblcmp(dbuf[i], 10000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp_lim(dbuf[i], -5100 + (i * 800), 0.75))  JUMP_TO_EXIT_ERROR_PROC
  }

  edfrewind(hdl, 1);

  if(edfread_physical_samples(hdl, 1, 23, dbuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%f   %i\n", dbuf[i], -30100 + (i * 909));

    if(i == 0)
    {
      if(dblcmp(dbuf[i], -30000))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp(dbuf[i], -30100 + (i * 909)))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 40, EDFSEEK_SET) != 40)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != -10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -10100 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 46, EDFSEEK_SET) != 46)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if((i == 20) || (i == 21))
    {
      if(ibuf[i] != 30000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 22)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 9900 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 80, EDFSEEK_SET) != 80)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != -10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -10100 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 92, EDFSEEK_SET) != 92)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 20)
    {
      if(ibuf[i] != 30000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 9900 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 60, EDFSEEK_SET) != 60)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != -10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -10100 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 69, EDFSEEK_SET) != 69)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if((i == 20) || (i == 21))
    {
      if(ibuf[i] != 30000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 22)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 9900 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 100, EDFSEEK_SET) != 100)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != -10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -10100 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 115, EDFSEEK_SET) != 115)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 20)
    {
      if(ibuf[i] != 30000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 9900 + (i * 1053))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 0, 80, ibuf) != 80)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<80; i++)
  {
//    printf("%i:   %i   %i\n", i,  ibuf[i], -10100 + (i * 253));

    if(i == 0)
    {
      if(ibuf[i] != -10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -10100 + (i * 253))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 1, 92, ibuf) != 92)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<92; i++)
  {
//    printf("%i:   %i   %i\n", i,  ibuf[i], 9900 + (i * 253));

    if(i == 0)
    {
      if(ibuf[i] != 10000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 80)
    {
      if(ibuf[i] != 30000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 9900 + (i * 253))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 185, EDFSEEK_SET) != 185)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 15)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<15; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + ((i + 65) * 253));

    if(ibuf[i] != -10100 + ((i + 65) * 253))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 1, SEEK_SET);

  fputc('1', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 1, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 16, SEEK_SET);

  fputc(' ', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 16, SEEK_SET);

  fputc('0', fp);

  fseek(fp, 0xaa, SEEK_SET);

  fputc(':', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xaa, SEEK_SET);

  fputc('.', fp);

  fseek(fp, 0xab, SEEK_SET);

  fputc('9', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xab, SEEK_SET);

  fputc('1', fp);

  fseek(fp, 0xac, SEEK_SET);

  fputc('q', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xac, SEEK_SET);

  fputc('2', fp);

  fseek(fp, 0xc4, SEEK_SET);

  fputc('D', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_IS_DISCONTINUOUS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xc4, SEEK_SET);

  fputc('C', fp);

  fseek(fp, 0x12e, SEEK_SET);

  fputc(' ', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x12e, SEEK_SET);

  fputc('s', fp);

  fseek(fp, 0x1ac, SEEK_SET);

  fputc(181, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x1ac, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x308, SEEK_SET);

  fputc(' ', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x308, SEEK_SET);

  fputc('-', fp);

  fseek(fp, 0x30d, SEEK_SET);

  fputc(',', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x30d, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x3a5, SEEK_SET);

  fputc('.', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x3a5, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x3bc, SEEK_SET);

  fputc(207, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x3bc, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x40b, SEEK_SET);

  fputc(247, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x40b, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x560, SEEK_SET);

  fputc(127, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x560, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x5ff, SEEK_SET);

  fputc(13, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x5ff, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0x54a, SEEK_SET);

  fputc('.', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x54a, SEEK_SET);

  fputc(' ', fp);

  fseek(fp, 0xad, SEEK_SET);

  fputc('-', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xad, SEEK_SET);

  fputc('.', fp);

  fseek(fp, 0x5fe, SEEK_SET);

  fputc('z', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x5fe, SEEK_SET);

  fputc(' ', fp);

  fclose(fp);

  if(get_raw_header("test.edf", &rawhdr))  JUMP_TO_EXIT_ERROR_PROC

  fp = fopen("test.edf", "r+b");

//  printf("offset: %i\n", get_file_offset(1, 0, 1, &rawhdr));

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr) + 1, SEEK_SET);

  fwrite("0.12", 4, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr) + 1, SEEK_SET);

  fwrite("0.131", 5, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr) + 1, SEEK_SET);

  fwrite("0.130", 5, 1, fp);

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr), SEEK_SET);

  fputc('0', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr), SEEK_SET);

  fputc('-', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 0, 1, &rawhdr), SEEK_SET);

  fputc('+', fp);

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 24, SEEK_SET);

  fputc(0, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 24, SEEK_SET);

  fputc(0x14, fp);

  fputc(1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 25, SEEK_SET);

  fputc(0, fp);

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 20, SEEK_SET);

  fputc(3, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 20, SEEK_SET);

  fputc('e', fp);

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 32, SEEK_SET);

  fputc(3, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  fp = fopen("test.edf", "r+b");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(0, 2, 1, &rawhdr) + 32, SEEK_SET);

  fputc(0, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("-32769  ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("-10000  ", 8, 1, fp);

  fseek(fp, 0x380, SEEK_SET);

  fwrite("32768   ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x380, SEEK_SET);

  fwrite("10000   ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) != 0)  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

  /****************************************/

  fp = fopen("test.edf", "rb");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x600, SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %f\n", i, ((signed short *)str)[i], ((-5100 + (i * 800)) / 0.75) - 3333.333333);

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(i >= 19)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(dblcmp_lim(((signed short *)str)[i], ((-5100 + (i * 800)) / 0.75) - 3333.333333, 1.0001))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x628, SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], (-30100 + (i * 909)) + 40000);

    if(i == 0)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != (-30100 + (i * 909)) + 40000)
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 0, 0, &rawhdr), SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %f\n", i, ((signed short *)str)[i], ((-5100 + (i * 800)) / 0.75) - 3333.333333);

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(i >= 19)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(dblcmp_lim(((signed short *)str)[i], ((-5100 + (i * 800)) / 0.75) - 3333.333333, 1.0001))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(1, 1, 0, &rawhdr), SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], (-30100 + (i * 909)) + 40000);

    if(i == 0)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != (-30100 + (i * 909)) + 40000)
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(2, 0, 0, &rawhdr), SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != -10100 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(2, 1, 0, &rawhdr), SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], 9900 + (i * 1053));

    if((i == 0) || (i == 22))
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if((i == 20) || (i == 21))
    {
      if(((signed short *)str)[i] != 30000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != 9900 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(3, 0, 0, &rawhdr), SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != -10100 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(3, 1, 0, &rawhdr), SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], 9900 + (i * 1053));

    if((i == 0) || (i == 22))
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if((i == 20) || (i == 21))
    {
      if(((signed short *)str)[i] != 30000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != 9900 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(4, 0, 0, &rawhdr), SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != -10100 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(4, 1, 0, &rawhdr), SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(i >= 20)
    {
      if(((signed short *)str)[i] != 30000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != 9900 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(5, 0, 0, &rawhdr), SEEK_SET);

  if(fread(str, 40, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], -10100 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != -10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != -10100 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.edf", "rb");
  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, get_file_offset(5, 1, 0, &rawhdr), SEEK_SET);

  if(fread(str, 46, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
//    printf("buf[%i]: %i   value: %i\n", i, ((signed short *)str)[i], 9900 + (i * 1053));

    if(i == 0)
    {
      if(((signed short *)str)[i] != 10000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(i >= 20)
    {
      if(((signed short *)str)[i] != 30000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(((signed short *)str)[i] != 9900 + (i * 1053))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

/********************************** BDF reading ******************************/

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 3)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 2)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.file_duration != 11700000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_day != 5)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_month != 12)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.startdate_year != 2017)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_second != 8)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_minute != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_hour != 12)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_subsecond != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patient_name, "John Doe"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.patientcode, "01234"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.sex, "Female"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.birthdate, "04 jul 2010"))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_day != 4)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_month != 7)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.birthdate_year != 2010)  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.patient_additional, "nop", 3))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.admincode, "789"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.technician, "Richard Roe"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.equipment, "device"))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecord_duration != 1300000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.datarecords_in_file != 9)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.annotations_in_file != 3)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].label, "trace1          "))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[1].label, "trace2          "))  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_file != 180)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].smp_in_file != 207)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_max != 10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].phys_max != -10000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].phys_min != -5000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].phys_min != -30000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_max != 1000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].dig_max != 3000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].dig_min != -1000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].dig_min != 1000000)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[0].smp_in_datarecord != 20)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.signalparam[1].smp_in_datarecord != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[0].physdimension, "uVxxxxxx"))  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(hdr.signalparam[1].physdimension, ".C0xxxxx"))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[0].prefilter, "qwerty   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[1].prefilter, "zxcvbn   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[0].transducer, "asdfgh   ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.signalparam[1].transducer, "poklhyg  ", 9))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 0, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != -10000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Recording starts"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 1, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 6000000)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, "0.200000"))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != 2000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Test 2"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_get_annotation(hdl, 2, &annot))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.onset != 11700000)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.duration, ""))  JUMP_TO_EXIT_ERROR_PROC

  if(annot.duration_l != -10000000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(strcmp(annot.annotation, "Recording ends"))  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 1, 500, EDFSEEK_SET) == 500)  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 0, 333, EDFSEEK_SET) == 333)  JUMP_TO_EXIT_ERROR_PROC

  if(edfseek(hdl, 0, 20, EDFSEEK_SET) != 20)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_physical_samples(hdl, 0, 20, dbuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i:   %f   %i\n", i, dbuf[i], -5100 + (i * 800));

    if(i == 0)
    {
      if(dblcmp_lim(dbuf[i], -5000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 19)
    {
      if(dblcmp_lim(dbuf[i], 10000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp_lim(dbuf[i], -5100 + (i * 800), 0.0075))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 23, EDFSEEK_SET) != 23)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_physical_samples(hdl, 1, 23, dbuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %f   %i\n", i, dbuf[i], -30100 + (i * 909));

    if(i == 0)
    {
      if(dblcmp_lim(dbuf[i], -30000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp(dbuf[i], -30100 + (i * 909)))  JUMP_TO_EXIT_ERROR_PROC
  }

  edfrewind(hdl, 0);

  if(edfread_physical_samples(hdl, 0, 20, dbuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i:   %f   %i\n", i, dbuf[i], -5100 + (i * 800));

    if(i == 0)
    {
      if(dblcmp_lim(dbuf[i], -5000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i == 19)
    {
      if(dblcmp_lim(dbuf[i], 10000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp_lim(dbuf[i], -5100 + (i * 800), 0.0075))  JUMP_TO_EXIT_ERROR_PROC
  }

  edfrewind(hdl, 1);

  if(edfread_physical_samples(hdl, 1, 23, dbuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %f   %i\n", i, dbuf[i], -30100 + (i * 909));

    if(i == 0)
    {
      if(dblcmp_lim(dbuf[i], -30000, 0.00001))  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(dblcmp(dbuf[i], -30100 + (i * 909)))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 40, EDFSEEK_SET) != 40)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], -1010000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != -1000000)  JUMP_TO_EXIT_ERROR_PROC
    }
    else
    {
      if(ibuf[i] != -1010000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  if(edfseek(hdl, 1, 46, EDFSEEK_SET) != 46)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 990000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != 1000000)  JUMP_TO_EXIT_ERROR_PROC
    }
    else if(i >= 20)
      {
        if(ibuf[i] != 3000000)  JUMP_TO_EXIT_ERROR_PROC
      }
      else
      {
        if(ibuf[i] != 990000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
      }
  }

  if(edfseek(hdl, 0, 60, EDFSEEK_SET) != 60)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], -1010000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != -1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -1010000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 1, 69, EDFSEEK_SET) != 69)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 990000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != 1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 20)
    {
      if(ibuf[i] != 3000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 990000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 20)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<20; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], -1010000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != -1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -1010000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 1, 23, ibuf) != 23)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<23; i++)
  {
//    printf("%i:   %i   %i\n", i, ibuf[i], 990000 + (i * 105300));

    if(i == 0)
    {
      if(ibuf[i] != 1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 20)
    {
      if(ibuf[i] != 3000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 990000 + (i * 105300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 0, 80, ibuf) != 80)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<80; i++)
  {
//    printf("%i:   %i   %i\n", i,  ibuf[i], -1010000 + (i * 25300));

    if(i == 0)
    {
      if(ibuf[i] != -1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != -1010000 + (i * 25300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfread_digital_samples(hdl, 1, 92, ibuf) != 92)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<92; i++)
  {
//    printf("%i:   %i   %i\n", i,  ibuf[i], 990000 + (i * 25300));

    if(i == 0)
    {
      if(ibuf[i] != 1000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(i >= 80)
    {
      if(ibuf[i] != 3000000)  JUMP_TO_EXIT_ERROR_PROC

      continue;
    }

    if(ibuf[i] != 990000 + (i * 25300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfseek(hdl, 0, 165, EDFSEEK_SET) != 165)  JUMP_TO_EXIT_ERROR_PROC

  if(edfread_digital_samples(hdl, 0, 20, ibuf) != 15)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<15; i++)
  {
//    printf("%i   %i\n", ibuf[i], -10100 + ((i + 65) * 253));

    if(ibuf[i] != -1010000 + ((i + 65) * 25300))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  /****************************************/

  fp = fopen("test.bdf", "rb");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x600, SEEK_SET);

  if(fread(str, 60, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<20; i++)
  {
    var.four[0] = str[i * 3];
    var.four[1] = str[i * 3 + 1];
    var.four[2] = str[i * 3 + 2];

    if(var.four[2]&0x80)
    {
      var.four[3] = 0xff;
    }
    else
    {
      var.four[3] = 0x00;
    }

//    printf("buf[%i]: %i   value: %f\n", i, var.one_signed, ((-5100 + (i * 800)) / 0.0075) - 333333.333333);

    if(i == 0)
    {
      if(var.one_signed != -1000000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(i >= 19)
    {
      if(var.one_signed != 1000000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(dblcmp_lim(var.one_signed, ((-5100 + (i * 800)) / 0.0075) - 333333.333333, 1.0001))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.bdf", "rb");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x63c, SEEK_SET);

  if(fread(str, 69, 1, fp) != 1)
  {
    fclose(fp);

    JUMP_TO_EXIT_ERROR_PROC
  }

  for(i=0; i<23; i++)
  {
    var.four[0] = str[i * 3];
    var.four[1] = str[i * 3 + 1];
    var.four[2] = str[i * 3 + 2];

    if(var.four[2]&0x80)
    {
      var.four[3] = 0xff;
    }
    else
    {
      var.four[3] = 0x00;
    }

//    printf("buf[%i]: %i   value: %f\n", i, var.one_signed, ((-30100 + (i * 909)) / 0.01) + 4000000.0);

    if(i == 0)
    {
      if(var.one_signed != 1000000)
      {
        fclose(fp);

        JUMP_TO_EXIT_ERROR_PROC
      }

      continue;
    }

    if(dblcmp_lim(var.one_signed, ((-30100 + (i * 909)) / 0.01) + 4000000.0, 1.0001))
    {
      fclose(fp);

      JUMP_TO_EXIT_ERROR_PROC
    }
  }

  fclose(fp);

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x37f, SEEK_SET);

  fputc('7', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x37f, SEEK_SET);

  fputc('8', fp);

  fseek(fp, 0x39e, SEEK_SET);

  fputc('6', fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x39e, SEEK_SET);

  fputc('7', fp);

  fseek(fp, 0x318, SEEK_SET);

  fwrite("1 ", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x318, SEEK_SET);

  fwrite("-1", 2, 1, fp);

  fseek(fp, 0x358, SEEK_SET);

  fwrite("2000000 ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("1000000 ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("-1000000", 8, 1, fp);

  fseek(fp, 0xec, SEEK_SET);

  fwrite("+9", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdr.handle))  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xec, SEEK_SET);

  fwrite("-9", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xec, SEEK_SET);

  fwrite("-1", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xec, SEEK_SET);

  fwrite("0 ", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xec, SEEK_SET);

  fwrite(" 9", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xec, SEEK_SET);

  fwrite("9 ", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) != 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdr.handle))  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("-8388609", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x358, SEEK_SET);

  fwrite("-1000000", 8, 1, fp);

  fseek(fp, 0x380, SEEK_SET);

  fwrite("8388608 ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.bdf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0x380, SEEK_SET);

  fwrite("1000000 ", 8, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.bdf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) != 0)  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

    /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xae, SEEK_SET);

  fwrite("15", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) == 0)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.filetype != EDFLIB_FILE_CONTAINS_FORMAT_ERRORS)  JUMP_TO_EXIT_ERROR_PROC

  /****************************************/

  fp = fopen("test.edf", "r+b");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  fseek(fp, 0xae, SEEK_SET);

  fwrite("17 ", 2, 1, fp);

  fclose(fp);

  if(edfopen_file_readonly("test.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS) != 0)  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF writing ******************************/

  hdl = edfopen_file_writeonly("test2.edf", EDFLIB_FILETYPE_EDFPLUS, 1);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_samplefrequency(hdl, 0, 100))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 0, 1000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 0, -1000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 0, 32767))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 0, -32768))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patient_additional(hdl, "Test"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_recording_additional(hdl, "tEST"))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<100; i++)
  {
    dbuf[i] = 0;
  }

  if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test2.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.patient_additional, "Test", 4))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.recording_additional, "tEST", 4))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF writing ******************************/

  hdl = edfopen_file_writeonly("test3.edf", EDFLIB_FILETYPE_EDFPLUS, 1);

  if(hdl < 0)  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_datarecord_duration(hdl, 77777))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_subsecond_starttime(hdl, 1234000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_number_of_annotation_signals(hdl, 3))  JUMP_TO_EXIT_ERROR_PROC

  char str_korea[]={0xeb,0x8c,0x80,0xed,0x95,0x9c,0xeb,0xaf,0xbc,0xea,0xb5,0xad,0x00};

  char str_accented[]={0xc3,0xb1,0xe1,0xb8,0xb1,0xc3,0xb6,0xc3,0xa8,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x00};

  for(i=0; i<60; i++)
  {
    l_tmp = 10000LL * (i + 1);

    snprintf(str, 4096, "test %i sec", (int)(l_tmp / 10000LL));

    if(i != 0)
    {
     if(edfwrite_annotation_latin1(hdl, l_tmp, -1LL, str))  JUMP_TO_EXIT_ERROR_PROC
    }
    else
    {
     if(edfwrite_annotation_utf8(hdl, l_tmp, -1LL, str_accented))  JUMP_TO_EXIT_ERROR_PROC
    }

    l_tmp += 3333LL;

    snprintf(str, 4096, "test %i.%04i sec", (int)(l_tmp / 10000LL), (int)(l_tmp % 10000LL));

    if(i != 0)
    {
      if(edfwrite_annotation_latin1(hdl, l_tmp, -1LL, str))  JUMP_TO_EXIT_ERROR_PROC
    }
    else
    {
     if(edfwrite_annotation_utf8_hr(hdl, l_tmp * 100LL, -1LL, str_korea))  JUMP_TO_EXIT_ERROR_PROC
    }
  }

  if(edf_set_samplefrequency(hdl, 0, 100))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_maximum(hdl, 0, 1000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_physical_minimum(hdl, 0, -1000))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_maximum(hdl, 0, 32767))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_digital_minimum(hdl, 0, -32768))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_startdatetime(hdl, 2008, 12, 31, 23, 59, 58))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientname(hdl, "Ãlpha"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patientcode(hdl, "Bràvó"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_gender(hdl, 1))  JUMP_TO_EXIT_ERROR_PROC  /* DEPRECATED */

  if(edf_set_birthdate(hdl, 2005, 7, 4))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_patient_additional(hdl, "Charlie"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_admincode(hdl, "Dëlta"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_technician(hdl, "Ëcho"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_equipment(hdl, "Foxtröt"))  JUMP_TO_EXIT_ERROR_PROC

  if(edf_set_recording_additional(hdl, "Golf"))  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<100; i++)
  {
    dbuf[i] = 0;
  }

  for(i=0; i<40; i++)
  {
    if(edfwrite_physical_samples(hdl, dbuf))  JUMP_TO_EXIT_ERROR_PROC
  }

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test3.edf", &hdr, EDFLIB_READ_ALL_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 1)  JUMP_TO_EXIT_ERROR_PROC

  for(i=0; i<60; i++)
  {
    if(edf_get_annotation(hdl, i * 2, &annot))  JUMP_TO_EXIT_ERROR_PROC

//    printf("i: %i   onset: %lli\n", i, annot.onset);

    if(annot.onset != 10000000LL * (i + 1))  JUMP_TO_EXIT_ERROR_PROC

    if(edf_get_annotation(hdl, i * 2 + 1, &annot))  JUMP_TO_EXIT_ERROR_PROC

    if(annot.onset != 10000000LL * (i + 1) + 3333000LL)  JUMP_TO_EXIT_ERROR_PROC

//    printf("i: %i   onset: %lli\n", i * 2 + 1, annot.onset);
  }

  if(strncmp(hdr.patientcode, "Bravo", 5))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.sex, "Male", 4))  JUMP_TO_EXIT_ERROR_PROC
  if(strncmp(hdr.gender, "Male", 4))  JUMP_TO_EXIT_ERROR_PROC  /* DEPRECATED */

  if(strncmp(hdr.birthdate, "04 jul 2005", 11))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.patient_additional, "Charlie", 7))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.admincode, "Delta", 5))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.technician, "Echo", 4))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.equipment, "Foxtrot", 7))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(hdr.recording_additional, "Golf", 4))  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

  /****************************************/

  fp = fopen("test3.edf", "rb");

  if(fp == NULL)  JUMP_TO_EXIT_ERROR_PROC

  if(fread(str, 256, 1, fp) != 1)  JUMP_TO_EXIT_ERROR_PROC

  fclose(fp);

  if(strncmp(str + 8, "Bravo M 04-JUL-2005 Alpha Charlie"
    "                                               ", 80))  JUMP_TO_EXIT_ERROR_PROC

  if(strncmp(str + 88, "Startdate 31-DEC-2008 Delta Echo Foxtrot Golf"
    "                                   ", 80))  JUMP_TO_EXIT_ERROR_PROC

/********************************** EDF reading ******************************/

  if(edfopen_file_readonly("test3.edf", &hdr, EDFLIB_DO_NOT_READ_ANNOTATIONS))  JUMP_TO_EXIT_ERROR_PROC

  hdl = hdr.handle;

  if(hdr.filetype != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.edfsignals != 1)  JUMP_TO_EXIT_ERROR_PROC

  if(hdr.starttime_subsecond!=1234000LL)  JUMP_TO_EXIT_ERROR_PROC

  if(edfclose_file(hdl))
  {
    hdl = -1;

    JUMP_TO_EXIT_ERROR_PROC
  }

  hdl = -1;

  /****************************************/

  free(ibuf);
  free(sbuf);
  free(dbuf);
  free(str);
  free(pbuf);
  free(i3buf);

  return EXIT_SUCCESS;

OUT_ERROR:

  if(hdl >= 0)
  {
    edfclose_file(hdl);
  }

  free(ibuf);
  free(sbuf);
  free(dbuf);
  free(str);
  free(pbuf);
  free(i3buf);

  fprintf(stderr, "Error, line %i file %s\n", line, __FILE__);

  return EXIT_FAILURE;
}


int dblcmp(double val1, double val2)
{
  long double diff = (long double)val1 - (long double)val2;

  if(diff > 1e-13)
  {
    return 1;
  }
  else if(-diff > 1e-13)
    {
      return -1;
    }
    else
    {
      return 0;
    }
}


int dblcmp_lim(double val1, double val2, double lim)
{
  long double diff = (long double)val1 - (long double)val2;

  if(diff > lim)
  {
    return 1;
  }
  else if(-diff > lim)
    {
      return -1;
    }
    else
    {
      return 0;
    }
}


int get_raw_header(const char *f_path, raw_hdr_t *hdr)
{
  int i, err=-999;

  char *buf=NULL,
       str[128]={""};

  FILE *f=NULL;

  if((f_path == NULL) || (hdr == NULL))  return -1;

  if(strlen(f_path) < 5)  return -2;

  memset(hdr, 0, sizeof(raw_hdr_t));

  buf = malloc(300);
  if(buf==NULL)
  {
    err = -3;
    goto EXIT_ERROR;
  }

  f = fopen(f_path, "rb");
  if(f==NULL)
  {
    err = -4;
    goto EXIT_ERROR;
  }

  if(fread(buf, 256, 1, f) != 1)
  {
    err = -11;
    goto EXIT_ERROR;
  }

  if(!memcmp(buf, "0       ", 8))
  {
    hdr->sample_width = 2;
  }
  else if(!memcmp(buf, "\xff" "BIOSEMI", 8))
    {
      hdr->sample_width = 3;
    }
    else
    {
      err = -11;
      goto EXIT_ERROR;
    }

  memcpy(str, buf+236, 8);
  str[8] = 0;
  hdr->datrecs = atoi(str);
  if(hdr->datrecs < 1)
  {
    err = -12;
    goto EXIT_ERROR;
  }

  memcpy(str, buf+252, 4);
  str[4] = 0;
  hdr->total_chans = atoi(str);
  if((hdr->total_chans < 1) || (hdr->total_chans > EDFLIB_MAXSIGNALS))
  {
    err = -13;
    goto EXIT_ERROR;
  }

  memcpy(str, buf+184, 8);
  str[8] = 0;
  hdr->hdr_sz = atoi(str);
  if((hdr->hdr_sz < 512) || (hdr->hdr_sz != ((hdr->total_chans + 1) * 256)))
  {
    err = -14;
    goto EXIT_ERROR;
  }

  free(buf);

  buf = malloc(hdr->hdr_sz);
  if(buf==NULL)
  {
    err = -15;
    goto EXIT_ERROR;
  }

  rewind(f);

  if(fread(buf, hdr->hdr_sz, 1, f) != 1)
  {
    err = -16;
    goto EXIT_ERROR;
  }

  for(i=0; i<hdr->total_chans; i++)
  {
    hdr->bytes_in_datrec[i] = atoi(buf+256+(hdr->total_chans*216)+(i*8)) * hdr->sample_width;

    hdr->datrec_offset[i] = hdr->datrec_sz;

    hdr->datrec_sz += hdr->bytes_in_datrec[i];

    if(hdr->sample_width == 2)
    {
      if(memcmp(buf+256+(i*16), "EDF Annotations ", 16))
      {
        hdr->regular_chans_idx_list[hdr->regular_chans] = i;

        hdr->regular_chans++;
      }
      else
      {
        hdr->annot_chans_idx_list[hdr->annot_chans] = i;

        hdr->annot_chans++;
      }
    }
    else
    {
      if(memcmp(buf+256+(i*16), "BDF Annotations ", 16))
      {
        hdr->regular_chans_idx_list[hdr->regular_chans] = i;

        hdr->regular_chans++;
      }
      else
      {
        hdr->annot_chans_idx_list[hdr->annot_chans] = i;

        hdr->annot_chans++;
      }
    }
  }

  if(!hdr->annot_chans)
  {
    err = -17;
    goto EXIT_ERROR;
  }

  if(f)  fclose(f);

  free(buf);

  return 0;

EXIT_ERROR:

  if(f)  fclose(f);

  free(buf);

  memset(hdr, 0, sizeof(raw_hdr_t));

//  printf("get_raw_header(): error: %i\n", err);

  return err;
}

/* datrec and chan are zero based
 * is_annot: 0: regular channel 1: annotation channel
 */
int get_file_offset(int datrec, int idx, int is_annot, raw_hdr_t *hdr)
{
  if((datrec < 0) || (datrec >= hdr->datrecs))  return -1;

  if(!hdr)  return -2;

  if((is_annot < 0) || (is_annot > 1))  return -3;

  if(is_annot)
  {
    if((idx < 0) || (idx >= hdr->annot_chans))  return -4;

    return (hdr->hdr_sz + (datrec * hdr->datrec_sz) + (hdr->datrec_offset[hdr->annot_chans_idx_list[idx]]));
  }
  else
  {
    if((idx < 0) || (idx >= hdr->regular_chans))  return -5;

    return (hdr->hdr_sz + (datrec * hdr->datrec_sz) + (hdr->datrec_offset[hdr->regular_chans_idx_list[idx]]));
  }
}













