#include "top.h"

#include <OS.h>
#include <string.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes) {
    int32 cookie = 0;
    team_info team;
    while (get_next_team_info(&cookie, &team) == B_OK) {
        if (team.team == B_SYSTEM_TEAM) {
            continue; // The kernel team
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        ffStrbufInitS(&item->name, team.name);

        item->pid = (uint32_t) team.team;

        // User and kernel time of all threads of the team combined in microseconds.
        item->cpuTime = 0;
        team_usage_info usage;
        if (get_team_usage_info(team.team, B_TEAM_USAGE_SELF, &usage) == B_OK) {
            item->cpuTime = ((uint64_t) usage.user_time + (uint64_t) usage.kernel_time) / 1000;
        }

        // Sum up the physically resident memory of all areas of the team;
        // ram_size is the amount of physical pages committed to an area,
        // whereas size is just its virtual reservation.
        uint64_t memBytes = 0;
        ssize_t areaCookie = 0;
        area_info area;
        while (get_next_area_info(team.team, &areaCookie, &area) == B_OK) {
            memBytes += area.ram_size;
        }
        item->memBytes = memBytes;

        // start_time is microseconds since boot.
        item->startTime = (uint64_t) team.start_time;

        // Storage io counters are not exposed by Haiku; leave them zeroed.
        item->bytesRead = 0;
        item->bytesWritten = 0;
    }

    return nullptr;
}
