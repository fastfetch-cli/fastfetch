#include "chassis.h"

const char* ffChassisTypeToString(uint32_t chassisType)
{
    // https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.2.0.pdf
    // 7.4.1 System Enclosure or Chassis Types
    switch (chassisType)
    {
        case 0x01: return "Other";
        case 0x02: return "Unknown";
        case 0x03: return "Desktop";
        case 0x04: return "Low Profile Desktop";
        case 0x05: return "Pizza Box";
        case 0x06: return "Mini Tower";
        case 0x07: return "Tower";
        case 0x08: return "Portable";
        case 0x09: return "Laptop";
        case 0x0A: return "Notebook";
        case 0x0B: return "Hand Held";
        case 0x0C: return "Docking Station";
        case 0x0D: return "All in One";
        case 0x0E: return "Sub Notebook";
        case 0x0F: return "Space-saving";
        case 0x10: return "Lunch Box";
        case 0x11: return "Main Server Chassis";
        case 0x12: return "Expansion Chassis";
        case 0x13: return "SubChassis";
        case 0x14: return "Bus Expansion Chassis";
        case 0x15: return "Peripheral Chassis";
        case 0x16: return "RAID Chassis";
        case 0x17: return "Rack Mount Chassis";
        case 0x18: return "Sealed-case PC";
        case 0x19: return "Multi-system chassis";
        case 0x1A: return "Compact PCI";
        case 0x1B: return "Advanced TCA";
        case 0x1C: return "Blade";
        case 0x1E: return "Tablet";
        case 0x1F: return "Convertible";
        case 0x20: return "Detachable";
        case 0x21: return "IoT Gateway";
        case 0x22: return "Embedded PC";
        case 0x23: return "Mini PC";
        case 0x24: return "Stick PC";
        default: return NULL;
    }
}
