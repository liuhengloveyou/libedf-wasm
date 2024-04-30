#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <string>
#include "edflib.h"

typedef struct edfMatedata
{
    double fileDuration;       // 文件时长(second)
    double dataRecordDuration; // 数据记录时长(second)
    int signals;               // 通道数
    int sampleFrequency;       //  采样频率
    int dataRecordsInFile;     // 数据记录个数
    int annotationsInFile;     // 标注个数

} edfMatedata;

typedef struct edf
{
    double ys[EDFLIB_MAXSIGNALS];
    char *labels[EDFLIB_MAXSIGNALS];
    char *indexs[EDFLIB_MAXSIGNALS];

    // 数据缓存，一个datarecord
    double *data = nullptr;
    int data_row = 0;
    int data_col = 0;

    edflib_hdr_t hdr;
} edf;

int edf_init();
void edf_destroy();

edfMatedata edf_open(std::string path);
void edf_close();
int edf_read_datarecord(int sec);
double *edf_read(int channel, int sec_pos, int sec_count, double *buf);
char **get_labels();