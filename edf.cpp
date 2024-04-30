#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "edf.h"

using namespace emscripten;

static edf *global_edf = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

  extern void transpost_data_to_js(int rid, void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

static int calloc_data(edf *p, int row, int col)
{
  if (p->data)
  {
    free(p->data);
  }

  p->data_row = row;
  p->data_col = col;

  p->data = (double *)calloc(p->data_row * p->data_col, sizeof(double));
  if (p->data == NULL)
  {
    printf("calloc error\n");
    return -1;
  }

  return 0;
}

edfMatedata edf_open(std::string path)
{
  edfMatedata mate;
  mate.fileDuration = 0;       // 文件时长(second)
  mate.dataRecordDuration = 0; // 数据记录时长(second)
  mate.dataRecordsInFile = 0;  // 数据记录个数
  mate.annotationsInFile = 0;  // 标注个数
  mate.signals = 0;            // 通道数

  if (global_edf == nullptr)
  {
    global_edf = (edf *)malloc(sizeof(edf));
    if (global_edf == NULL)
    {
      printf("\nmalloc error\n");
      return mate;
    }
  }

  edf *p = global_edf;

  const char *fn = path.c_str();
  if (edfopen_file_readonly(fn, &p->hdr, EDFLIB_READ_ALL_ANNOTATIONS))
  {
    switch (p->hdr.filetype)
    {
    case EDFLIB_MALLOC_ERROR:
      printf("malloc error\n");
      break;
    case EDFLIB_NO_SUCH_FILE_OR_DIRECTORY:
      printf("cannot open file, no such file or directory\n");
      break;
    case EDFLIB_FILE_CONTAINS_FORMAT_ERRORS:
      printf("the file is not EDF(+) or BDF(+) compliant\n(it contains format errors)\n");
      break;
    case EDFLIB_MAXFILES_REACHED:
      printf("to many files opened\n");
      break;
    case EDFLIB_FILE_READ_ERROR:
      printf("a read error occurred\n");
      break;
    case EDFLIB_FILE_ALREADY_OPENED:
      printf("file has already been opened\n");
      break;
    default:
      printf("unknown error\n");
      break;
    }

    return mate;
  }

  // int hdl = p->hdr.handle;

  printf("version: %i.%02i\n", edflib_version() / 100, edflib_version() % 100);
  printf("filetype: %i\n", p->hdr.filetype);
  printf("edfsignals: %i\n", p->hdr.edfsignals);
  printf("file duration: %lli seconds\n", p->hdr.file_duration / EDFLIB_TIME_DIMENSION);
  printf("采集开始时间: %i-%i-%i %i:%02i:%02i.%07lli\n", p->hdr.startdate_day, p->hdr.startdate_month, p->hdr.startdate_year, p->hdr.starttime_hour, p->hdr.starttime_minute, p->hdr.starttime_second, p->hdr.starttime_subsecond);
  printf("数据记录时长: %f seconds\n", ((double)p->hdr.datarecord_duration) / EDFLIB_TIME_DIMENSION);
  printf("数据记录个数: %lli\n", p->hdr.datarecords_in_file);
  printf("标注个数: %lli\n", p->hdr.annotations_in_file);

  // printf("患者: %s\n", p->hdr.patient);
  // printf("采集记录: %s\n", p->hdr.recording);
  // printf("患者号: %s\n", p->hdr.patientcode);
  // printf("性别: %s\n", p->hdr.sex);
  // printf("生日: %s\n", p->hdr.birthdate);
  // printf("患者姓名: %s\n", p->hdr.patient_name);
  // printf("患者附加信息: %s\n", p->hdr.patient_additional);

  mate.fileDuration = p->hdr.file_duration / EDFLIB_TIME_DIMENSION;                                                                      // 文件时长(second)
  mate.dataRecordDuration = ((double)p->hdr.datarecord_duration) / EDFLIB_TIME_DIMENSION;                                                // 数据记录时长(second)
  mate.dataRecordsInFile = p->hdr.datarecords_in_file;                                                                                   // 数据记录个数
  mate.annotationsInFile = p->hdr.annotations_in_file;                                                                                   // 标注个数
  mate.signals = p->hdr.edfsignals;                                                                                                      // 通道数
  mate.sampleFrequency = ((double)p->hdr.signalparam[0].smp_in_datarecord / (double)p->hdr.datarecord_duration) * EDFLIB_TIME_DIMENSION; // TODO signalparam[0]

  // edflib_annotation_t annot;
  // for (int i = 0; i < p->hdr.annotations_in_file; i++)
  // {
  //   if (edf_get_annotation(hdl, i, &annot))
  //   {
  //     printf("\nerror: edf_get_annotations()\n");
  //     edfclose_file(hdl);
  //     return NULL;
  //   }
  //   else
  //   {
  //     printf("annotation: onset is %lli.%07lli sec    duration is %s    description is \"%s\"\n",
  //            annot.onset / EDFLIB_TIME_DIMENSION,
  //            annot.onset % EDFLIB_TIME_DIMENSION,
  //            annot.duration,
  //            annot.annotation);
  //   }
  // }

  for (int i = 0; i < EDFLIB_MAXSIGNALS; i++)
  {
    p->ys[i] = i;

    p->indexs[i] = (char *)calloc(5, sizeof(char));
    (void)snprintf(p->indexs[i], 5, "%d", i);
  }

  for (int i = 0; i < p->hdr.edfsignals; i++)
  {
    p->labels[i] = &p->hdr.signalparam[i].label[0];
  }

  return mate;
}

void edf_close()
{
  edfclose_file(global_edf->hdr.handle);
  global_edf->hdr.handle = -1;
  free(global_edf->data);
  free(global_edf);
  global_edf = NULL;

  return;
}

/*
  读一个数据记录
  sec_pos: 读第几秒数据
*/
int edf_read_datarecord(int sec)
{
  edf *p = global_edf;

  int sampleFrequency = ((double)p->hdr.signalparam[0].smp_in_datarecord / (double)p->hdr.datarecord_duration) * EDFLIB_TIME_DIMENSION;

  calloc_data(p, p->hdr.edfsignals, sampleFrequency);
  for (int i = 0; i < p->hdr.edfsignals; i++)
  {
    edf_read(i, sec, sampleFrequency, p->data + p->data_col * i);
  }

  transpost_data_to_js(sec, p->data, p->data_row * p->data_col * sizeof(double));

  return 0;
}

/*
  channel: 通道
  sec_pos: start reading x seconds from start of file
  count: 读多少数据
*/
double *edf_read(int channel, int sec_pos, int count, double *buf)
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

  edfseek(hdl, channel, (long long)((((double)sec_pos) / ((double)p->hdr.file_duration / (double)EDFLIB_TIME_DIMENSION)) * ((double)p->hdr.signalparam[channel].smp_in_file)), EDFSEEK_SET);

  int rn = edfread_physical_samples(hdl, channel, count, buf);
  if (rn == (-1))
  {
    printf("\nerror: edf_read_physical_samples()\n");
    return NULL;
  }

  return buf;
}

char **get_labels()
{
  return global_edf->labels;
}

EMSCRIPTEN_BINDINGS(my_module)
{
  value_object<edfMatedata>("edfMatedata")
      .field("fileDuration", &edfMatedata::fileDuration)
      .field("signals", &edfMatedata::signals)
      .field("sampleFrequency", &edfMatedata::sampleFrequency)
      .field("dataRecordDuration", &edfMatedata::dataRecordDuration)
      .field("dataRecordsInFile", &edfMatedata::dataRecordsInFile)
      .field("annotationsInFile", &edfMatedata::annotationsInFile);

  function("edf_open", &edf_open);
  function("edf_close", &edf_close);
  function("edf_read_datarecord", &edf_read_datarecord);
  function("get_labels", &get_labels, allow_raw_pointers());
}
