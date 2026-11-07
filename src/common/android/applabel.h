#pragma once

#include "fastfetch.h"

// The name an app presents to the user, which is not its package name: an app installed as
// `com.termux` presents itself as `Termux:Launcher` on the launcher, in its notifications and on
// the settings screen.
//
// Every route that answers with an *already resolved* label is closed to an app UID, and each one
// was measured on the test device rather than assumed:
//
//   * `dumpsys package` needs android.permission.DUMP.
//   * `cmd package dump` is filtered down to 929 bytes for an app UID, against 1.2 MB for the shell.
//   * The task descriptions the activity service hands out only ever describe the caller's own task.
//   * `getApplicationInfo` answers with `labelRes` -- a resource id -- and leaves `nonLocalizedLabel`
//     null, so there is no string in the reply to read.
//
// What is open is the APK itself. `getApplicationInfo` carries its path (`sourceDir`), APKs are
// world readable, and the `resources.arsc` inside one is *stored* rather than deflated -- the
// platform expects to mmap it -- so resolving an id out of it needs no decompressor. That is the
// route this takes: one transaction, one mapping, no fork and no child process.
//
// The label is the resource id in the reply whose type is `string`. That is a rule about the reply
// rather than about the parcel's field order, and it was audited against the manifest's own
// `android:label` over every package installed on the test device: nine in ten have exactly one
// `string` candidate and it is the right one every time, a handful more have two and the first is
// still the right one, and the rest have none because their label is a literal string rather than a
// reference (`android:label="Example"`), so the reply carries no id at all. No package resolved to
// a wrong string; the counts behind that are in doc/app-label.md.
//
// Writes the label into `result` when it can be resolved, and leaves `result` alone otherwise --
// which includes the cases above where there is nothing to resolve. The caller sets its fallback
// first, so that an unresolved label costs nothing but the query.
//
// Returns nullptr on success, a static message when the query itself could not be made. An error is
// not a fallback: it says the label was not looked for, not that the app has none.
[[gnu::nonnull(1, 2), nodiscard]] const char* ffAndroidGetAppLabel(const char* packageName, FFstrbuf* result);
