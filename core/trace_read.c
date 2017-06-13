#include "trace_read.h"
#include "plugin.h"
#include "msg.h"
#include "rdram.h"
#include "rdp.h"

#include <stdio.h>
#include <stdlib.h>

static FILE* fp;

bool trace_read_open(const char* path)
{
    fp = fopen(path, "rb");

    return trace_read_is_open();
}

bool trace_read_is_open(void)
{
    return fp != NULL;
}

void trace_read_header(size_t* rdram_size)
{
    char header[4];
    fread(header, sizeof(header), 1, fp);
    if (memcmp(header, TRACE_HEADER, sizeof(header))) {
        msg_error("trace_read_header: Invalid trace file header or unsupported version.");
    }

    fread(rdram_size, sizeof(rdram_size), 1, fp);
}

char trace_read_id(void)
{
    char id = fgetc(fp);
    switch (id) {
        case TRACE_EOF:
        case TRACE_CMD:
        case TRACE_RDRAM:
        case TRACE_VI:
            break;
        default:
            msg_error("trace_read_id: Invalid trace packet ID %d at offset %d", id, ftell(fp));
    }
    return id;
}

void trace_read_cmd(uint32_t* cmd, size_t* length)
{
    *length = fgetc(fp);

    if (*length > CMD_MAX_INTS) {
        msg_error("trace_read_cmd: Command is too long (%d)", *length);
    }

    fread(cmd, sizeof(uint32_t), *length, fp);
}

void trace_read_rdram(void)
{
    size_t offset;
    size_t length;
    fread(&offset, sizeof(offset), 1, fp);
    fread(&length, sizeof(length), 1, fp);

    uint32_t v;
    for (size_t i = offset; i < offset + length; i++) {
        fread(&v, sizeof(v), 1, fp);
        rdram_write_idx32(i, v);
    }
}

void trace_read_vi(void)
{
    for (int32_t i = 0; i < VI_NUM_REG; i++) {
        fread(plugin_vi_register(i), sizeof(uint32_t), 1, fp);
    }
}

void trace_read_close(void)
{
    fclose(fp);
    fp = NULL;
}
