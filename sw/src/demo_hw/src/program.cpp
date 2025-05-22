#include "common.h"

#include <sim.h>

#include <cstring>
#include <cstdlib>
#include <vector>

int nb_triangles;
bool rasterizer_ena = true;

int read_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_read_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

int write_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_write_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

int main(void)
{
    if (!sd_init()) {
        printf("SD card initialization failed.\r\n");
        return 1;
    }

    fl_init();

    // Attach media access functions to library
    if (fl_attach_media(read_sector, write_sector) != FAT_INIT_OK)
    {
        printf("Failed to init file system\r\n");
        return 1;
    }

    graphite_init();

    sim_run(nullptr);

    fl_shutdown();    

    return 0;
}
