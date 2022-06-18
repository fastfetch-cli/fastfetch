#include "fastfetch.h"

#include <unistd.h>

#define FF_CPU_USAGE_MODULE_NAME "CPU Usage"
#define FF_CPU_USAGE_NUM_FORMAT_ARGS 1

void ffPrintCPUUsage(FFinstance* instance)
{
    long user, nice, system, idle, iowait, irq, softirq;
    FILE* procStat = fopen("/proc/stat", "r");
    if(procStat == NULL)
    {
        ffPrintError(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpu, "fopen(\"""/proc/stat\", \"r\") == NULL");
        return;
    }
    if (fscanf(procStat, "cpu%ld%ld%ld%ld%ld%ld%ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) < 0) goto exit;
    long workJiffies1 = user + nice + system;
    long totalJiffies1 = workJiffies1 + idle + iowait + irq + softirq;

    sleep(1);

    rewind(procStat);
    if (fscanf(procStat, "cpu%ld%ld%ld%ld%ld%ld%ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) < 0) goto exit;
    long workJiffies2 = user + nice + system;
    long totalJiffies2 = workJiffies2 + idle + iowait + irq + softirq;

    // https://stackoverflow.com/questions/3017162/how-to-get-total-cpu-usage-in-linux-using-c#answer-3017438
    long workOverPeriod = workJiffies2 - workJiffies1;
    long totalOverPeriod = totalJiffies2 - totalJiffies1;
    double cpuPercent = (double)workOverPeriod / (double)totalOverPeriod * 100;

    if(instance->config.cpuUsage.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage.key);

        printf("%.2lf%%\n", cpuPercent);
    }
    else
    {
        ffPrintFormat(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage, FF_CPU_USAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpuPercent}
        });
    }

exit:
    fclose(procStat);
}
