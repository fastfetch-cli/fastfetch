#include "gamepad.h"
#include "common/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <GameController/GameController.h>

#include <math.h>

// macOS offers two overlapping views of the same controller, and neither replaces the other:
//
// * `IOHIDManager` sees every HID device whose primary usage is a joystick or a gamepad, third
//   party pads included. It is the only source of the manufacturer, the product and the serial
//   number.
// * `GCController` is the GameController framework's view of the pads Apple has whitelisted -- the
//   official Sony, Microsoft, Nintendo and MFi ones. It is the only source of a battery level
//   (`GCDeviceBattery`, macOS 11.0+), and it is what the system's own games read.
//
// Neither view replaces the other, so each contributes whole entries rather than a name from one and
// a battery from the other. Nothing on the public surface can pair them: `GCDevice` carries only the
// display strings `vendorName` and `productCategory`, `+[GCController supportsHIDDevice:]` answers in
// the other direction only, and SDL -- the reference implementation for all of this -- does not pair
// them either ("we don't have an easy way to know if those devices correspond to a specific
// GCController", `SDL_mfijoystick.m`).
//
// The serial is the one field both views carry, so it is also the one field that would otherwise be
// spelled two different ways for the same pad depending on which pass contributed the entry. It is
// joined through a private property -- see the `GCController()` category below for what was measured
// -- and the join only *borrows* the serial. The entries themselves stay separate.
//
// ⚠️ `[GCController controllers]` is filled in by the private `_GCControllerManager` once it is
// told that the application became active, which a process that never activates -- a command line
// tool -- never tells it. SDL records the symptom for macOS 11.0.1 ("this always returns an empty
// array"). The array may therefore legitimately be empty here, so the IOHID pass only skips a
// device when the framework actually contributed something: with an empty array there is no
// duplicate to avoid, and skipping would drop the device from the output altogether.
//
// The array also does not fill on its own. Measured on this machine with a Pro Controller attached:
// it reads 0 at process start, is *still* 0 after 20 ms of unrelated work with no run loop, and
// only becomes non-empty ~10.7 ms after the first `CFRunLoopRunInMode` call -- the request is sent
// by the run loop turn, not by the property read. Since nothing else in this file pumps, waiting for
// the framework has to be explicit, which is what `GAMEPAD_GAMECONTROLLER_TIMEOUT_SECONDS` and the
// pump loop in `findGameControllerDevices()` are for. Without them this whole pass contributes
// nothing and macOS never shows a battery.
//
// Merely reading `GCController.controllers` is what starts the framework -- it connects to
// `gamecontrollerd` on first touch -- so that read is *not* hoisted to the top of
// `ffDetectGamepad()`. It is therefore paid only on the path where the framework has something to
// answer for. Measured on this machine with no controller attached, module time via `--stat`:
//
//     nothing hoisted .................  0.3 ms
//     the read only ...................  8.3 ms
//     the read plus the run loop turn . 14.3 ms
//
// Both halves are expensive and neither is optional: the read is the ignition and the turn is what
// carries the request. The turn is nominally bounded by 0.5 ms, but `returnAfterSourceHandled` does
// not bound the handler, so it adds ~6 ms of its own. Paying 14 ms on a machine with no pad at all
// -- to learn what `supportsHIDDevice:` below already answered -- is why the kick is gone rather
// than merely deferred.
#define GAMEPAD_GAMECONTROLLER_TIMEOUT_SECONDS 0.05

@interface GCController()
    // Not in the public header, and the only identity the framework exposes at all: the public
    // `GCDevice` surface has just the two display strings, so two pads of the same model are
    // indistinguishable through it -- no vendor id, no product id, no serial.
    //
    // On the Bluetooth pad this was measured with, the description is the transport address, which is
    // the very string IOKit reports as `kIOHIDSerialNumberKey`:
    //
    //     GCController.identifier   LOGICAL_DEVICE(5c-52-1e-88-7e-70)
    //     IOHIDDevice serial        5C:52:1E:88:7E:70
    //
    // `persistentIdentifier` answered the same string on that pad and is deliberately not used: its
    // name promises stability across launches, which is a different contract from "the address this
    // pad has right now". `physicalDeviceUniqueID` is nil there, and `deviceHash` is a per-process
    // hash, so neither is something that could be looked up in the IOKit set.
    @property (readonly, nonatomic) NSString* identifier;
@end

// ---------------------------------------------------------------------------------------------
// Joining the two views
// ---------------------------------------------------------------------------------------------
//
// The serial is borrowed rather than guessed. The framework's own spelling of the address is the join
// key, and both sides are normalised before they are compared.
//
// Only that one pad has been measured: the wrapper format is private, so a pad that reports its
// identity in some other shape -- a wired pad with a real serial number, say -- fails to match. That
// is the intended failure mode, and it is why this only ever fills in a field that would otherwise be
// empty: no match means an empty serial, never a wrong one.

// `LOGICAL_DEVICE(5c-52-1e-88-7e-70)` -> `5c-52-1e-88-7e-70`. Anything without a parenthesised part
// is rejected outright rather than guessed at, because the wrapper is private formatting that can
// change without notice.
static bool extractIdentityToken(NSString* identifier, FFstrbuf* result) {
    // The property is declared opaque in the framework's metadata -- `NSObject<NSCopying,
    // NSSecureCoding>` -- but the value is a plain string, so it is used as-is: measured on this pad
    // the object is an `__NSCFString` inheriting `NSMutableString`, and `description` returns the
    // same 33 characters. Anything that is not a string is rejected rather than stringified, because
    // the wrapper format is private either way and a non-string value would mean it changed.
    if (![identifier isKindOfClass: NSString.class]) {
        return false;
    }
    NSRange open = [identifier rangeOfString: @"("];
    if (open.location == NSNotFound) {
        return false;
    }
    NSRange close = [identifier rangeOfString: @")" options: 0 range: NSMakeRange(open.location + 1, identifier.length - open.location - 1)];
    if (close.location == NSNotFound) {
        return false;
    }
    NSString* token = [identifier substringWithRange: NSMakeRange(open.location + 1, close.location - open.location - 1)];
    ffStrbufSetS(result, token.UTF8String);
    return result->length > 0;
}

// Drop every separator and fold to upper case, so `5c-52-1e-88-7e-70` and `5C:52:1E:88:7E:70` -- the
// two spellings of one address above -- compare equal.
static void stripIdentitySeparators(FFstrbuf* strbuf) {
    uint32_t out = 0;
    for (uint32_t i = 0; i < strbuf->length; ++i) {
        char c = strbuf->chars[i];
        if (c >= '0' && c <= '9') {
            strbuf->chars[out++] = c;
        } else if (c >= 'a' && c <= 'f') {
            strbuf->chars[out++] = (char) (c - 'a' + 'A');
        } else if (c >= 'A' && c <= 'F') {
            strbuf->chars[out++] = c;
        }
    }
    strbuf->chars[out] = '\0';
    strbuf->length = out;
}

// The IOKit device the framework is describing, or `nullptr` when the two cannot be matched. The
// returned ref is owned by `set`, which the caller holds for the rest of the call.
static IOHIDDeviceRef findHIDDeviceForController(GCController* controller, NSSet* set) {
    if (set == nil || ![controller respondsToSelector: @selector(identifier)]) {
        return nullptr;
    }

    FF_STRBUF_AUTO_DESTROY identity = ffStrbufCreate();
    if (!extractIdentityToken(controller.identifier, &identity)) {
        return nullptr;
    }
    stripIdentitySeparators(&identity);

    for (id value in set) {
        IOHIDDeviceRef device = (__bridge IOHIDDeviceRef) value;
        FF_STRBUF_AUTO_DESTROY serial = ffStrbufCreate();
        ffCfStrGetString(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey)), &serial);
        stripIdentitySeparators(&serial);
        if (serial.length > 0 && ffStrbufEqual(&serial, &identity)) {
            return device;
        }
    }
    return nullptr;
}

typedef struct FFFindGamepadContext {
    FFlist* devices;
    uint32_t gameControllerCount;
} FFFindGamepadContext;

// The GameController framework's own devices, and how many of them there were. `set` is the IOKit
// side, used only to borrow the serial -- see `findHIDDeviceForController()`.
static uint32_t findGameControllerDevices(FFlist* devices, NSSet* set) {
    if (@available(macOS 11.0, *)) {
        // See the note at the top: the list stays empty until the run loop has been pumped once, and
        // the timeout only bounds the case where the framework has nothing to report at all. The
        // caller checks `supportsHIDDevice:` first, so a pad the framework does not claim never
        // reaches this wait.
        //
        // The turn is handed the time that is left rather than 0, so it blocks until the framework's
        // source fires or the deadline passes. A zero timeout returns as soon as the run loop has
        // nothing pending, which turns this into a busy spin: measured with no pad and the loop
        // forced to run its full timeout, `CFRunLoopRunInMode(mode, 0, true)` burned 50.4 ms of CPU
        // in 50.0 ms of wall time (30k-40k turns), where passing the remaining time burned 0.08 ms
        // in a single turn. The deadline is a soft bound either way -- the blocking turn overshot it
        // by 3-5 ms in the same measurement.
        NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow: GAMEPAD_GAMECONTROLLER_TIMEOUT_SECONDS];
        while (GCController.controllers.count == 0 && deadline.timeIntervalSinceNow > 0) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, deadline.timeIntervalSinceNow, true);
        }

        uint32_t count = 0;
        @autoreleasepool {
            for (GCController* controller in GCController.controllers) {
                FFGamepadDevice* device = FF_LIST_ADD(FFGamepadDevice, *devices);
                ffStrbufInit(&device->serial);
                ffStrbufInit(&device->name);
                device->battery = 0;

                // The framework knows the pad but not its serial -- the public surface has no such
                // field -- so it is borrowed from the IOKit device its private identity points at.
                // Both passes then spell the same serial the same way, which is what makes the two
                // entries comparable in the output.
                IOHIDDeviceRef hidDevice = findHIDDeviceForController(controller, set);
                if (hidDevice) {
                    ffCfStrGetString(IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDSerialNumberKey)), &device->serial);
                }

                if (controller.productCategory) {
                    ffStrbufSetS(&device->name, controller.productCategory.UTF8String);
                } else if (controller.vendorName) {
                    ffStrbufSetS(&device->name, controller.vendorName.UTF8String);
                } else {
                    ffStrbufSetS(&device->name, "MFi Gamepad");
                }

                // A wired pad has no battery object at all, and `batteryLevel` is `0.0` by default,
                // so an unknown level cannot be told apart from a drained one. Both become the
                // module's "0 means unknown", which hides the number rather than printing a wrong 0%.
                GCDeviceBattery* battery = controller.battery;
                if (battery) {
                    int percent = (int) lroundf(battery.batteryLevel * 100.f);
                    device->battery = (uint8_t) (percent < 0 ? 0 : (percent > 100 ? 100 : percent));
                }

                ++count;
            }
        }
        return count;
    }
    return 0;
}

// Whether the framework would claim any of these devices. Unlike the list itself this is a local
// predicate on the HID descriptor, not a daemon round trip, so it is the right thing to ask *before*
// paying for the wait above. Measured on this machine: 0.3 ms warm, 0.9 ms as the very first
// GameController call in a process -- and reading `GCController.controllers` immediately afterwards
// still cost 6 ms. That is the point: the gate answers without starting the framework, so the
// ~14 ms above is only ever paid by a pad Apple actually claims. A Logitech pad, or any pad outside
// Apple's whitelist, fails this and skips the wait entirely instead of sitting through a timeout for
// a list that will never contain it.
static bool anyDeviceIsClaimedByGameController(NSSet* set) {
    if (@available(macOS 11.0, *)) {
        for (id value in set) {
            if ([GCController supportsHIDDevice: (__bridge IOHIDDeviceRef) value]) {
                return true;
            }
        }
    }
    return false;
}

static void enumSet(IOHIDDeviceRef value, FFFindGamepadContext* ctx) {
    // A device the framework owns is already in the list under its own name; see the note at the
    // top for why this is conditional on the framework having contributed anything at all.
    if (ctx->gameControllerCount > 0) {
        if (@available(macOS 11.0, *)) {
            if ([GCController supportsHIDDevice:value]) {
                return;
            }
        }
    }

    FFGamepadDevice* device = FF_LIST_ADD(FFGamepadDevice, *ctx->devices);
    ffStrbufInit(&device->serial);
    ffStrbufInit(&device->name);
    device->battery = 0;

    CFStringRef manufacturer = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDManufacturerKey));
    ffCfStrGetString(manufacturer, &device->name);

    CFStringRef product = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDProductKey));
    if (device->name.length) {
        ffCfStrGetString(product, &device->serial);
        ffStrbufAppendC(&device->name, ' ');
        ffStrbufAppend(&device->name, &device->serial);
    } else {
        ffCfStrGetString(product, &device->name);
    }

    CFStringRef serialNumber = IOHIDDeviceGetProperty(value, CFSTR(kIOHIDSerialNumberKey));
    ffCfStrGetString(serialNumber, &device->serial);
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */) {
    FF_CFTYPE_AUTO_RELEASE IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
        return "IOHIDManagerOpen() failed";
    }

    // Built as Foundation collections and bridged for the call. The C spelling of this is four lines
    // of `CFDictionaryCreate` with inline array casts around two numbers each; a literal says the same
    // thing in two. `kIOHIDDevice*Key` are C string macros -- the old spelling had to wrap them in
    // `CFSTR()` -- and `@()` boxes them straight to `NSString`.
    //
    // The old spelling was *not* leaking, tempting as that is to assume: the four `ffCfCreateInt()`
    // numbers do hand a +1 reference to a dictionary that retains them and never give it back, but
    // `CFNumberCreate()` answers with a tagged pointer for every int32 -- measured, all seven test
    // values came back pointer-identical and 200k stranded references grew RSS by 0 KB. So this is a
    // readability change, not a fix.
    NSArray* matchings = @[
        @{ @(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_Joystick) },
        @{ @(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad) },
    ];
    IOHIDManagerSetDeviceMatchingMultiple(manager, (__bridge CFArrayRef) matchings);

    // `IOHIDManagerCopyDevices()` follows the Create rule, so the set is handed to ARC rather than
    // tracked with a cleanup attribute. It may also return NULL, which bridging turns into nil.
    NSSet* set = CFBridgingRelease(IOHIDManagerCopyDevices(manager));

    // The framework's entries come first, and the wait for them happens only when the framework has
    // a reason to answer -- see `anyDeviceIsClaimedByGameController()`.
    FFFindGamepadContext ctx = {
        .devices = devices,
        .gameControllerCount = anyDeviceIsClaimedByGameController(set) ? findGameControllerDevices(devices, set) : 0,
    };

    for (id value in set) {
        enumSet((__bridge IOHIDDeviceRef) value, &ctx);
    }
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);

    return nullptr;
}
