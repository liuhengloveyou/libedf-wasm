#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "edf.h"

#include <emscripten/bind.h>
using namespace emscripten;

edf *global_edf = NULL;

static int calloc_data(edf *p, size_t size)
{
  if (p->data_size >= size)
  {
    return 0;
  }
  p->data_size = size;

  for (int i = 0; i < EDFLIB_MAXSIGNALS; i++)
  {

    p->data[i] = (double *)calloc(p->data_size, sizeof(double));
    if (p->data[i] == NULL)
    {
      printf("\nmalloc error\n");
      return -1;
    }
  }

  return 0;
}

int edf_open(std::string path)
{
  if (global_edf)
  {
    edf_close();
  }

  edf *p = (edf *)malloc(sizeof(edf));
  if (p == NULL)
  {
    printf("\nmalloc error\n");
    return -1;
  }

  global_edf = p;

  const char *fn = path.c_str();
  if (edfopen_file_readonly(fn, &p->hdr, EDFLIB_READ_ALL_ANNOTATIONS))
  {
    switch (p->hdr.filetype)
    {
    case EDFLIB_MALLOC_ERROR:
      printf("\nmalloc error\n\n");
      break;
    case EDFLIB_NO_SUCH_FILE_OR_DIRECTORY:
      printf("\ncannot open file, no such file or directory\n\n");
      break;
    case EDFLIB_FILE_CONTAINS_FORMAT_ERRORS:
      printf("\nthe file is not EDF(+) or BDF(+) compliant\n(it contains format errors)\n\n");
      break;
    case EDFLIB_MAXFILES_REACHED:
      printf("\nto many files opened\n\n");
      break;
    case EDFLIB_FILE_READ_ERROR:
      printf("\na read error occurred\n\n");
      break;
    case EDFLIB_FILE_ALREADY_OPENED:
      printf("\nfile has already been opened\n\n");
      break;
    default:
      printf("\nunknown error\n\n");
      break;
    }

    return 0;
  }

  int hdl = p->hdr.handle;

  printf("\nlibrary version: %i.%02i\n", edflib_version() / 100, edflib_version() % 100);
  printf("\ngeneral header:\n\n");
  printf("filetype: %i\n", p->hdr.filetype);
  printf("edfsignals: %i\n", p->hdr.edfsignals);
  printf("file duration: %lli seconds\n", p->hdr.file_duration / EDFLIB_TIME_DIMENSION);
  printf("采集开始时间: %i-%i-%i %i:%02i:%02i.%07lli\n", p->hdr.startdate_day, p->hdr.startdate_month, p->hdr.startdate_year, p->hdr.starttime_hour, p->hdr.starttime_minute, p->hdr.starttime_second, p->hdr.starttime_subsecond);

  printf("患者: %s\n", p->hdr.patient);
  printf("采集记录: %s\n", p->hdr.recording);
  printf("患者号: %s\n", p->hdr.patientcode);
  printf("性别: %s\n", p->hdr.sex);
  printf("生日: %s\n", p->hdr.birthdate);
  printf("患者姓名: %s\n", p->hdr.patient_name);
  printf("患者附加信息: %s\n", p->hdr.patient_additional);
  printf("数据记录时长: %f seconds\n", ((double)p->hdr.datarecord_duration) / EDFLIB_TIME_DIMENSION);
  printf("数据记录个数: %lli\n", p->hdr.datarecords_in_file);
  printf("标注个数: %lli\n", p->hdr.annotations_in_file);

  edflib_annotation_t annot;

  printf("\n");

  for (int i = 0; i < p->hdr.annotations_in_file; i++)
  {
    if (edf_get_annotation(hdl, i, &annot))
    {
      printf("\nerror: edf_get_annotations()\n");
      edfclose_file(hdl);
      return NULL;
    }
    else
    {
      printf("annotation: onset is %lli.%07lli sec    duration is %s    description is \"%s\"\n",
             annot.onset / EDFLIB_TIME_DIMENSION,
             annot.onset % EDFLIB_TIME_DIMENSION,
             annot.duration,
             annot.annotation);
    }
  }

  for (int i = 0; i < EDFLIB_MAXSIGNALS; i++)
  {
    p->ys[i] = i;

    char *t = (char *)calloc(5, sizeof(char));
    (void)snprintf(t, 5, "%d", i);
    p->indexs[i] = t;
  }

  return 0;
}

void edf_close()
{
  int hdl = global_edf->hdr.handle;

  edfclose_file(hdl);

  for (int i = 0; i < EDFLIB_MAXSIGNALS; i++)
  {
    free(global_edf->data[i]);
  }

  free(global_edf);
  global_edf = NULL;

  return;
}

void edf_read_datarecord(int s, int n)
{
  edf *p = global_edf;

  calloc_data(p, n);
  printf("edfsignals: %d %d %lu\n", p->hdr.edfsignals, n, p->data_size);

  for (int i = 0; i < p->hdr.edfsignals; i++)
  {
    printf("label: %d %s \n", i, p->hdr.signalparam[i].label);
    printf("smp_in_datarecord: %i\n", p->hdr.signalparam[i].smp_in_datarecord);
    printf("samplefrequency: %f\n", ((double)p->hdr.signalparam[i].smp_in_datarecord / (double)p->hdr.datarecord_duration) * EDFLIB_TIME_DIMENSION);

    p->labels[i] = &p->hdr.signalparam[i].label[0];
    p->labelss[i] = std::string(p->hdr.signalparam[i].label);

    edf_read(i, s, n, p->data[i]);
  }

  // DEMO
  // for (int i = p->hdr.edfsignals; i < EDFLIB_MAXSIGNALS; i++)
  // {
  //   for (int j = 0; j < n; j++)
  //   {
  //     p->buf[i][j] = p->buf[1][j] + 100 * i;
  //   }
  // }
}

/*
  channel: 通道
  s: start reading x seconds from start of file
  n: 读多少
*/
double *edf_read(int channel, int s, int n, double *buf)
{
  edf *p = global_edf;
  int hdl = p->hdr.handle;

  // for (int i = 0; i < p->hdr.edfsignals; i++)
  // {
  //   printf("label: %d %s \n", i, p->hdr.signalparam[i].label);
  //   printf("samples in file: %lli\n", p->hdr.signalparam[channel].smp_in_file);
  //   printf("samples in datarecord: %i\n", p->hdr.signalparam[channel].smp_in_datarecord);
  //   printf("physical maximum: %f\n", p->hdr.signalparam[channel].phys_max);
  //   printf("physical minimum: %f\n", p->hdr.signalparam[channel].phys_min);
  //   printf("digital maximum: %i\n", p->hdr.signalparam[channel].dig_max);
  //   printf("digital minimum: %i\n", p->hdr.signalparam[channel].dig_min);
  //   printf("physical dimension: %s\n", p->hdr.signalparam[channel].physdimension);
  //   printf("prefilter: %s\n", p->hdr.signalparam[channel].prefilter);
  //   printf("transducer: %s\n", p->hdr.signalparam[channel].transducer);
  //   printf("samplefrequency: %f\n\n\n", ((double)p->hdr.signalparam[channel].smp_in_datarecord / (double)p->hdr.datarecord_duration) * EDFLIB_TIME_DIMENSION);
  // }

  edfseek(hdl, channel, (long long)((((double)s) / ((double)p->hdr.file_duration / (double)EDFLIB_TIME_DIMENSION)) * ((double)p->hdr.signalparam[channel].smp_in_file)), EDFSEEK_SET);

  n = edfread_physical_samples(hdl, channel, n, buf);
  if (n == (-1))
  {
    printf("\nerror: edf_read_physical_samples()\n");
    // edfclose_file(hdl);
    free(buf);
    return NULL;
  }

  // printf("\nread %i samples, started at %i seconds from start of file:\n\n", n, s);

  for (int i = 0; i < n; i++)
  {
    buf[i] += channel * 100;
    // printf("%.0f  ", buf[i]);
  }
  // printf("\n\n");

  return buf;
}

char **get_labels()
{
  return global_edf->labels;
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("edf_open", &edf_open);
  function("edf_close", &edf_close);
  function("edf_read_datarecord", &edf_read_datarecord);
  function("get_labels", &get_labels, allow_raw_pointers());
}