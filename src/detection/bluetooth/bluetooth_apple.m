#include "bluetooth.h"
#include "common/percent.h"
#include "common/strutil.h"

#import <CoreBluetooth/CoreBluetooth.h>
#import <IOBluetooth/IOBluetooth.h>

@interface IOBluetoothDevice()
    @property (nonatomic) uint8_t batteryPercentCase;
    @property (nonatomic) uint8_t batteryPercentCombined;
    @property (nonatomic) uint8_t batteryPercentLeft;
    @property (nonatomic) uint8_t batteryPercentRight;
    @property (nonatomic) uint8_t batteryPercentSingle;
    // Not in the public header. The class is a wrapper around the private Core Bluetooth peer
    // objects, and this is the one that carries the `NSUUID` the LE side hands out: `CBClassicPeer`
    // and `CBPeripheral` both derive from `CBPeer`, which has one `identifier` and a separate
    // `connectedTransport` byte -- one identity per device, not one per stack, which is the same idea
    // as the container id the Windows backend joins on. The wrapper's own `identifier` below answers
    // nil on macOS 27, so it is kept only as a second source.
    @property (nonatomic) NSUUID* identifier;
    @property (nonatomic) CBPeripheral* peripheral;
@end

@interface CBPeripheral()
    // Not in the public header. `state` and `isConnected` both answer *disconnected* for a
    // peripheral the system has connected but this process has not -- which is every peripheral
    // this module ever sees, because it reads the list and never connects to anything. Measured on
    // macOS 27 across the seven peripherals `retrieveConnectedPeripheralsWithServices:` returned,
    // two of them connected: `state` was 0 on all seven, `isConnected` 0 on all seven, and
    // `isConnectedToSystem` 1 on exactly the two.
    @property (readonly, nonatomic) BOOL isConnectedToSystem;
@end

// ---------------------------------------------------------------------------------------------
// The LE stack
// ---------------------------------------------------------------------------------------------
//
// Core Bluetooth is the only public route to a peripheral that speaks Low Energy rather than BR/EDR.
// What it will and will not answer was measured on macOS 27 with `.workbuddy-ai/probes/ble/le_probe`
// (12 paired devices, one of them connected), because none of it follows from the headers:
//
// * `retrieveConnectedPeripheralsWithServices:@[]` answers with **nothing** -- 0 devices in 0.1 ms.
//   The empty list is not "everything", so the assigned 16-bit UUIDs have to be spelled out. With
//   them the same call answers in 0.5 ms, which is the reason this is the entry point that is used.
//
// * Scanning is not an option. `scanForPeripheralsWithServices:nil` turned up **57** peripherals in
//   five seconds and 54 of them had no name at all: it reports every phone, watch and beacon in
//   radio range, not the devices the machine knows. It is also the only way to see a peripheral that
//   is not connected, so `showDisconnected` cannot be honoured on this platform -- there is no
//   "paired LE peripherals" query the way `IOBluetoothDevice.pairedDevices` is one for classic.
//
// * The two stacks are joined on `NSUUID`, and the join is exact rather than a guess. The LE side
//   hands one out as `CBPeripheral.identifier`, and the classic side has the same value on the
//   `CBPeripheral` that `IOBluetoothDevice` wraps, the class being a wrapper around the private Core
//   Bluetooth peer objects. A `CBPeer` carries a single `identifier` and a separate
//   `connectedTransport` byte, so a dual-mode device is one identity rather than two -- the same
//   shape as the container id the Windows backend joins on. It is not the device address: no LE
//   identifier carried any of the twelve classic addresses.
//   Two sources were measured on macOS 27 and only one of them is safe. `IOBluetoothDevice`'s own
//   `identifier` answers nil for all twelve paired devices, and `peer.identifier` disagrees with
//   `peripheral.identifier` on three of them (ERAZER G501: `23F3EDAC-...` against `18BDC152-...`),
//   so `peer` is the classic peer and `peripheral` the LE one. Only `peripheral` is read here, and
//   only where it is non-nil; the name is the fallback for the rest, trusted only when exactly one
//   classic entry carries it. The machine this was written against has two devices called
//   "ERAZER N500", and guessing between them would put one device's LE link on the other.
//
// * The object that would make this trivially exact -- the private `CBDevice`, which holds
//   `bleAddressData` and `btAddressData` at once -- is out of reach: `+[CBDiscovery
//   devicesWithDiscoveryFlags:error:]` answers `CBErrorDomain -71168` for every flag tried, which is
//   the entitlement it needs and a plain command line tool does not have.
//
// * `pairedDevices` hands a dual-mode device out **twice** on macOS 27, once per stack, under the
//   same name and the same address: the headset this was written against comes back as two entries,
//   both carrying the same `peer` and `peripheral`. The second copy is therefore folded into the
//   first as it is detected, rather than appended and removed again, both because printing one
//   headset twice is wrong and because the duplicate makes the name ambiguous, which would leave the
//   fallback above unusable. It matters that the fold happens while walking the list rather than
//   afterwards: the identifier registered for the LE join comes from the *second* copy, which is the
//   one holding the `CBPeripheral`.
//
// * The classic half cannot be folded into this one, and was measured rather than assumed. Core
//   Bluetooth's public surface has no paired-device query at all: `CBCentralManager` enumerates
//   *connected* peripherals only, which on this machine is 2 of the 12. The paired query does exist,
//   as `CBClassicManager.retrievePairedPeersWithOptions:`, but it is private and it is what
//   `IOBluetoothDevice.pairedDevices` already calls -- `IOBluetoothCoreBluetoothCoordinator`, which
//   lives in IOBluetooth.framework, holds the manager and is the caller. Both Core-Bluetooth-only
//   routes were measured and neither replaces it: a `CBClassicManager` this process creates never
//   fills its peer map (0 entries after two seconds, `retrievePairedPeersWithOptions:` answers nil),
//   and `CBCentralManager.sharedPairingAgent.retrievePairedPeers` answers with 7 of the 11 devices
//   as bare `CBPeripheral` objects carrying no address, no battery and no class of device.
//   `.workbuddy-ai/probes/ble/cb_only_probe.m` is the probe; the numbers are in `bug.md`.
//
// * `CBPeripheral.state` is not the connection state the caller means. It answers `Disconnected` for
//   every peripheral the retrieval above returns, and so does `isConnected`; only the private
//   `isConnectedToSystem` reflects that the system has the device. Reading `state` reported every LE
//   peripheral as disconnected, which both suppressed the signal strength below and dropped a
//   connected LE-only peripheral whenever `showDisconnected` was off.
//
// * The signal strength is a `readRSSI` round trip. It is asked for only where the system says the
//   link is up, and the callbacks are waited for as a group, so a machine with no LE link pays
//   nothing.
//
// * `showType` is honoured here rather than in the module, because the two stacks are two separate
//   walks with two separate permission surfaces: a stack the user switched off is not asked about
//   at all. The LE pass keeps working without the classic one, it only loses the address join and
//   falls back to matching by name.
//
// Note on the permission model, which is what the first bullet above cannot show: any process that
// touches Bluetooth is subject to the `kTCCServiceBluetoothAlways` service, and the request is
// attributed to the *responsible* process. A host application that does not declare
// `NSBluetoothAlwaysUsageDescription` in its own Info.plist does not get a prompt -- the process is
// killed with SIGABRT instead. Terminal.app is an Apple platform binary and iTerm2, Ghostty and
// kitty all declare the key, so a normal terminal run prompts and remembers the answer. fastfetch
// deliberately does not embed a usage description of its own: that would give it a second TCC
// identity, which for an unsigned binary changes on every rebuild and would re-prompt each time.

#define FF_BLUETOOTH_LE_STATE_WAIT_MS 500 // The state callback lands in 18.7 ms when powered on
// A cap, not a measured latency. `didReadRSSI` was never reached on macOS 27 until the
// `isConnectedToSystem` fix below, so what one round trip costs is still unmeasured; this is ten
// times the 18.7 ms the state callback was measured at, and it is only ever paid when the system
// reports a live LE link.
#define FF_BLUETOOTH_LE_RSSI_WAIT_MS 200

@interface FFBluetoothLeCentral : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>
@property(nonatomic, strong) CBCentralManager* central;
@property(nonatomic, strong) NSMutableDictionary<NSString*, NSNumber*>* signalStrength;
@property(nonatomic) NSUInteger signalStrengthAsked;
@property(nonatomic) NSUInteger signalStrengthAnswered;
@property(nonatomic) BOOL stateKnown;
@end

@implementation FFBluetoothLeCentral

- (instancetype) init {
    self = [super init];
    _signalStrength = [NSMutableDictionary dictionary];
    return self;
}

- (void) centralManagerDidUpdateState:(CBCentralManager*)central {
    self.stateKnown = YES;
}

- (void) peripheral:(CBPeripheral*)peripheral didReadRSSI:(NSNumber*)RSSI error:(NSError*)error {
    ++self.signalStrengthAnswered;
    if (!error && RSSI) {
        self.signalStrength[peripheral.identifier.UUIDString] = RSSI;
    }
}

@end

// The assigned 16-bit UUIDs, plus the 0xFEE0-0xFEFF block, which is where the vendor-specific ones
// live. `retrieveConnectedPeripheralsWithServices:` filters on this list, so anything exposing only
// a 128-bit custom service is out of reach -- that is a limit of the API, not of this list.
static NSArray<CBUUID*>* leServiceUuids(void) {
    static NSArray<CBUUID*>* uuids;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSMutableArray<CBUUID*>* list = [NSMutableArray array];
        for (uint32_t i = 0x1800; i <= 0x18FF; ++i) {
            [list addObject: [CBUUID UUIDWithString: [NSString stringWithFormat: @"%04X", i]]];
        }
        for (uint32_t i = 0xFEE0; i <= 0xFEFF; ++i) {
            [list addObject: [CBUUID UUIDWithString: [NSString stringWithFormat: @"%04X", i]]];
        }
        uuids = list;
    });
    return uuids;
}

// See the note on `CBPeripheral.isConnectedToSystem` at the top of the file. The fallback keeps the
// module working on a system that does not answer the private property.
static bool leIsConnected(CBPeripheral* peripheral) {
    if ([peripheral respondsToSelector: @selector(isConnectedToSystem)]) {
        return peripheral.isConnectedToSystem;
    }
    return peripheral.state == CBPeripheralStateConnected;
}

// The address is what a device is looked up by, both while the classic list is walked and by the LE
// pass, and it is the only field the two copies of a dual-mode device are guaranteed to agree on.
static FFBluetoothResult* findByAddress(FFlist* devices, const char* address) {
    FF_LIST_FOR_EACH (FFBluetoothResult, device, *devices) {
        if (ffStrbufEqualS(&device->address, address)) {
            return device;
        }
    }
    return nullptr;
}

// The class of device, as the classic stack reports it. It is built into a scratch buffer rather than
// straight into the entry because the entry may already exist: see the merge in `ffDetectBluetooth`,
// where the second copy of a dual-mode device only fills what the first copy left empty.
static void setClassOfDevice(FFstrbuf* type, IOBluetoothDevice* ioDevice) {
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorLimitedDiscoverableMode)
        ffStrbufAppendS(type, "Limited Discoverable Mode, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorReserved1)
        ffStrbufAppendS(type, "LE audio, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorReserved2)
        ffStrbufAppendS(type, "Reserved for future use, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorPositioning)
        ffStrbufAppendS(type, "Positioning, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorNetworking)
        ffStrbufAppendS(type, "Networking, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorRendering)
        ffStrbufAppendS(type, "Rendering, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorCapturing)
        ffStrbufAppendS(type, "Capturing, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorObjectTransfer)
        ffStrbufAppendS(type, "Object Transfer, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorAudio)
        ffStrbufAppendS(type, "Audio, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorTelephony)
        ffStrbufAppendS(type, "Telephony, ");
    if(ioDevice.serviceClassMajor & kBluetoothServiceClassMajorInformation)
        ffStrbufAppendS(type, "Information, ");

    if(type->length == 0)
    {
        switch(ioDevice.deviceClassMajor)
        {
            case kBluetoothDeviceClassMajorMiscellaneous:
                ffStrbufAppendS(type, "Miscellaneous");
                break;
            case kBluetoothDeviceClassMajorComputer:
                ffStrbufAppendS(type, "Computer");
                break;
            case kBluetoothDeviceClassMajorPhone:
                ffStrbufAppendS(type, "Phone");
                break;
            case kBluetoothDeviceClassMajorLANAccessPoint:
                ffStrbufAppendS(type, "LAN/Network Access point");
                break;
            case kBluetoothDeviceClassMajorAudio:
                ffStrbufAppendS(type, "Audio/Video");
                break;
            case kBluetoothDeviceClassMajorPeripheral:
                ffStrbufAppendS(type, "Peripheral");
                break;
            case kBluetoothDeviceClassMajorImaging:
                ffStrbufAppendS(type, "Imaging");
                break;
            case kBluetoothDeviceClassMajorWearable:
                ffStrbufAppendS(type, "Wearable");
                break;
            case kBluetoothDeviceClassMajorToy:
                ffStrbufAppendS(type, "Toy");
                break;
            case kBluetoothDeviceClassMajorHealth:
                ffStrbufAppendS(type, "Health");
                break;
            case kBluetoothDeviceClassMajorUnclassified:
                ffStrbufAppendS(type, "Uncategorized");
                break;
            default:
                ffStrbufAppendS(type, "Unknown");
                break;
        }
    }
    else
    {
        ffStrbufTrimRight(type, ' ');
        ffStrbufTrimRight(type, ',');
    }
}

// The fallback for a system whose `IOBluetoothDevice` does not answer `identifier`. A name that is
// carried by more than one classic entry cannot identify anything, so an ambiguous name joins
// nothing and the peripheral is reported as its own device instead of being merged.
static FFBluetoothResult* findByName(FFlist* devices, const char* name) {
    FFBluetoothResult* found = nullptr;
    FF_LIST_FOR_EACH (FFBluetoothResult, device, *devices) {
        if (!ffStrEqualsIgnCase(device->name.chars, name)) {
            continue;
        }
        if (found) {
            return nullptr;
        }
        found = device;
    }
    return found;
}

static const char* detectLe(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */, NSDictionary<NSString*, NSString*>* classicAddressByIdentifier) {
    CBManagerAuthorization authorization = CBCentralManager.authorization;
    if (authorization == CBManagerAuthorizationDenied || authorization == CBManagerAuthorizationRestricted) {
        return nullptr; // The user said no. `NotDetermined` is left to the manager below, which asks.
    }

    FFBluetoothLeCentral* delegate = [FFBluetoothLeCentral new];
    delegate.central = [[CBCentralManager alloc] initWithDelegate: delegate queue: dispatch_get_main_queue()];

    // Every CoreBluetooth entry point answers on a delegate callback, so the run loop has to be
    // pumped until the state callback arrives. It is bounded because a module cannot wait forever.
    NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow: (double) FF_BLUETOOTH_LE_STATE_WAIT_MS / 1000];
    while (!delegate.stateKnown && [deadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
    }

    if (delegate.central.state != CBManagerStatePoweredOn) {
        return nullptr; // Powered off, unauthorized or no radio: the classic list stands on its own
    }

    NSArray<CBPeripheral*>* peripherals = [delegate.central retrieveConnectedPeripheralsWithServices: leServiceUuids()];
    if (peripherals.count == 0) {
        return nullptr;
    }

    for (CBPeripheral* peripheral in peripherals) {
        if (leIsConnected(peripheral)) {
            peripheral.delegate = delegate;
            ++delegate.signalStrengthAsked;
            [peripheral readRSSI];
        }
    }

    if (delegate.signalStrengthAsked > 0) {
        deadline = [NSDate dateWithTimeIntervalSinceNow: (double) FF_BLUETOOTH_LE_RSSI_WAIT_MS / 1000];
        while (delegate.signalStrengthAnswered < delegate.signalStrengthAsked && [deadline timeIntervalSinceNow] > 0) {
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
        }
    }

    for (CBPeripheral* peripheral in peripherals) {
        const char* name = peripheral.name.length > 0 ? peripheral.name.UTF8String : nullptr;
        bool connected = leIsConnected(peripheral);

        // The classic half wins wherever it has an answer: its name, address and type come from the
        // class of device rather than from a name the two stacks may spell differently. The
        // identifier is what makes the join exact; the name only stands in when it is missing.
        FFBluetoothResult* target = nullptr;
        NSString* address = classicAddressByIdentifier[peripheral.identifier.UUIDString];
        if (address) {
            target = findByAddress(devices, address.UTF8String);
        }
        if (!target && name) {
            target = findByName(devices, name);
        }

        if (!target && !connected && !options->showDisconnected) {
            continue; // An LE-only peripheral nobody asked to see
        }

        if (!target) {
            target = FF_LIST_ADD(FFBluetoothResult, *devices);
            ffStrbufInit(&target->name);
            ffStrbufInit(&target->address);
            ffStrbufInit(&target->type);
            target->deviceType = FF_BLUETOOTH_DEVICE_TYPE_NONE;
            target->battery = 0;
            target->signalQuality = -DBL_MAX;
            target->connected = false;

            if (name) {
                ffStrbufSetS(&target->name, name);
            } else {
                ffStrbufSetStatic(&target->name, "Unknown Device");
            }
            // Core Bluetooth never hands out the device address, so the identifier is the only
            // identity there is. It is stable on the host but is not an address, and it is not the
            // 17-character form the other backends report.
            ffStrbufSetS(&target->address, peripheral.identifier.UUIDString.UTF8String);
        }

        target->deviceType = (FFBluetoothDeviceType) (target->deviceType | FF_BLUETOOTH_DEVICE_TYPE_LE_BIT);
        target->connected |= connected;

        NSNumber* rssi = delegate.signalStrength[peripheral.identifier.UUIDString];
        if (rssi) {
            target->signalQuality = ffRssiToSignalQuality(rssi.intValue);
        }
    }

    return nullptr;
}

// The classic (BR/EDR) half. `classicAddressByIdentifier` is an output rather than an input: the
// classic list is the only place a device address can be had from, and the LE pass needs those
// addresses to recognise a peripheral that has already been listed.
static const char* detectClassic(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */, NSMutableDictionary<NSString*, NSString*>* classicAddressByIdentifier)
{
    NSArray<IOBluetoothDevice*>* ioDevices = IOBluetoothDevice.pairedDevices;
    if(!ioDevices)
        return "IOBluetoothDevice.pairedDevices failed";

    for(IOBluetoothDevice* ioDevice in ioDevices)
    {
        if (!options->showDisconnected && !ioDevice.isConnected)
            continue;

        // Normalised before the lookup rather than after the entry is created, because the address is
        // what decides whether this is a device already seen: `pairedDevices` reports a dual-mode one
        // once per stack, under one name and one address.
        FF_STRBUF_AUTO_DESTROY address = ffStrbufCreateS(ioDevice.addressString.UTF8String);
        ffStrbufReplaceAllC(&address, '-', ':');
        ffStrbufUpperCase(&address);

        // Registered before the duplicate is folded away, because it is the *second* copy of a
        // dual-mode device that carries the `CBPeripheral` -- the first one has `peripheral` nil. The
        // wrapper's own `identifier` answers nil here, but that `CBPeripheral` carries the same
        // `NSUUID` the LE side hands out. Both accesses are guarded because both are private: a system
        // that answers neither falls back to matching by name.
        if (address.length > 0 && [ioDevice respondsToSelector: @selector(peripheral)] && ioDevice.peripheral) {
            NSUUID* identifier = ioDevice.peripheral.identifier;
            if (identifier)
                classicAddressByIdentifier[identifier.UUIDString] = [NSString stringWithUTF8String: address.chars];
        }

        FF_STRBUF_AUTO_DESTROY type = ffStrbufCreate();
        setClassOfDevice(&type, ioDevice);

        uint8_t battery = 0;
        if (ioDevice.batteryPercentSingle)
            battery = ioDevice.batteryPercentSingle;
        else if (ioDevice.batteryPercentCombined)
            battery = ioDevice.batteryPercentCombined;
        else if (ioDevice.batteryPercentCase)
            battery = ioDevice.batteryPercentCase;

        // A device with no address cannot be recognised as a repeat, so it is always its own entry.
        FFBluetoothResult* device = address.length > 0 ? findByAddress(devices, address.chars) : nullptr;
        if (!device)
        {
            device = FF_LIST_ADD(FFBluetoothResult, *devices);
            ffStrbufInit(&device->name);
            ffStrbufInit(&device->address);
            ffStrbufInit(&device->type);
            device->deviceType = FF_BLUETOOTH_DEVICE_TYPE_NONE;
            device->battery = 0;
            device->signalQuality = -DBL_MAX;
            device->connected = false;
            ffStrbufSetS(&device->name, ioDevice.name.UTF8String);
            ffStrbufSet(&device->address, &address);
        }

        // The two copies of a dual-mode device are not identical -- one is the classic peer and the
        // other the LE one -- so each field is taken wherever the entry already kept has nothing,
        // rather than the second copy simply being dropped. The name is not merged: both copies spell
        // it the same way, and the first one is the one the LE pass looks up by.
        if (device->battery == 0)
            device->battery = battery;
        if (device->type.length == 0)
            ffStrbufSet(&device->type, &type);
        device->deviceType = (FFBluetoothDeviceType) (device->deviceType | FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT); // IOBluetooth is the BR/EDR stack
        device->connected |= !!ioDevice.isConnected;
    }

    return nullptr;
}

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */)
{
    // `showType` selects which stacks are walked, not merely which entries are printed: a stack the
    // user switched off is not asked about at all. The classic pass is the one that produces the
    // identifier map the LE pass joins on, so it is nil when that pass does not run -- the LE pass
    // then has nothing to match against and reports every peripheral as a device of its own.
    NSMutableDictionary<NSString*, NSString*>* classicAddressByIdentifier = nil;

    if (options->showType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT) {
        classicAddressByIdentifier = [NSMutableDictionary dictionary];

        const char* error = detectClassic(options, devices, classicAddressByIdentifier);
        if (error) {
            return error;
        }
    }

    // Last, so that a peripheral answering on both stacks is matched against the classic list and
    // ends up as one entry rather than two.
    if (options->showType & FF_BLUETOOTH_DEVICE_TYPE_LE_BIT) {
        detectLe(options, devices, classicAddressByIdentifier);
    }

    return nullptr;
}
