#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trace.h"

TraceReader *trace_open(const char *path) {
    TraceReader *r = malloc(sizeof(TraceReader));
    if (!r) { perror("malloc"); exit(EXIT_FAILURE); }

    r->fp = fopen(path, "r");
    if (!r->fp) {
        perror(path);
        free(r);
        return NULL;
    }
    r->line_num = 0;
    return r;
}

int trace_next(TraceReader *reader, TraceEntry *entry) {
    char line[64];

    /* Skip blank lines and comment lines (starting with '#') */
    while (fgets(line, sizeof(line), reader->fp)) {
        reader->line_num++;

        /* Strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0' || line[0] == '#')
            continue;

        char op;
        unsigned long long addr;
        if (sscanf(line, " %c 0x%llx", &op, &addr) == 2) {
            if (op != 'R' && op != 'W') {
                fprintf(stderr, "trace:%d: unknown operation '%c'\n",
                        reader->line_num, op);
                return -1;
            }
            entry->op   = op;
            entry->addr = (uint64_t)addr;
            return 1;
        } else {
            fprintf(stderr, "trace:%d: malformed line: %s\n",
                    reader->line_num, line);
            return -1;
        }
    }

    return 0; /* EOF */
}

void trace_close(TraceReader *reader) {
    if (!reader) return;
    fclose(reader->fp);
    free(reader);
}
