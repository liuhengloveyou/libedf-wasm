#pragma once

#include <string>
#include "edflib.h"

typedef struct edf
{
    edflib_hdr_t hdr;

    double ys[EDFLIB_MAXSIGNALS];
    char *labels[EDFLIB_MAXSIGNALS];
    char *indexs[EDFLIB_MAXSIGNALS];

    std::string labelss[EDFLIB_MAXSIGNALS];
    double *data[EDFLIB_MAXSIGNALS];
    size_t data_size;
} edf;
    
int edf_open(std::string path);
void edf_close();
void edf_read_datarecord(int s, int n);
double *edf_read(int channel, int s, int n, double *buf);
char **get_labels();