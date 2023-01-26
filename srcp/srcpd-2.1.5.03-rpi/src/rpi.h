/* $Id: rpi.h 1176 2008-01-29 22:42:17Z schmischi $ */

#ifndef _RPI_H
#define _RPI_H

#include <libxml/tree.h> /*xmlDocPtr, xmlNodePtr*/

typedef struct _RPI_DATA {
    int number_ga;
    int number_gl;
    int number_fb;
    /* RJA */
    int nmra_addr;
    ga_state_t tga[50];
} RPI_DATA;

int readconfig_RPI(xmlDocPtr doc, xmlNodePtr node, bus_t busnumber);
int init_bus_RPI(bus_t );
int init_gl_RPI(gl_state_t *);
int init_ga_RPI(ga_state_t *);
int getDescription_RPI(char *reply);
void* thr_sendrec_RPI(void *);

#endif
