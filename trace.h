#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>
#include <stdio.h>

/* One memory access read from the trace file. */
typedef struct {
    char     op;   /* 'R' for read, 'W' for write */
    uint64_t addr;
} TraceEntry;

/* Opaque handle to an open trace file. */
typedef struct {
    FILE *fp;
    int   line_num;  /* for error reporting */
} TraceReader;

/* Open a trace file. Returns NULL and prints an error on failure. */
TraceReader *trace_open(const char *path);

/*
 * Read the next trace entry into *entry.
 * Returns 1 on success, 0 on EOF, -1 on parse error.
 */
int trace_next(TraceReader *reader, TraceEntry *entry);

/* Close the trace file and free the reader. */
void trace_close(TraceReader *reader);

#endif /* TRACE_H */
