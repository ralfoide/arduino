#ifndef __INC_CAM_STATS_H
#define __INC_CAM_STATS_H

struct CamStats {
    long __frame_last_ms;
    int __frame_delta_ms;
    int __process_delta_ms;
    int __process_malloc_ms;
    int __process_copy_ms;
    int __frame_grab_count;
    int __frame_share_count;
    int __frame_convert_count;
};

#endif // __INC_CAM_STATS_H
