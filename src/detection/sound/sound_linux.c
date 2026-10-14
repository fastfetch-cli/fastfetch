#include "sound.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/io.h"
#include "common/strutil.h"

#include <errno.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

// Linux sound comes from one of two places: the sound server a desktop runs, and the hardware
// underneath it.
//
// The server is PulseAudio -- PipeWire speaks its protocol too -- and it is asked first, because
// it is the only one of the two that knows what the desktop's volume is: the mixer the card
// exposes and the slider on screen are not the same thing. When no server answers, the cards are
// read from ALSA instead; that part lives in its own section at the end of this file.
//
// ---------------------------------------------------------------------------
// the server: PulseAudio's native protocol ("protocol-native")
// ---------------------------------------------------------------------------
//
// Instead of linking or dlopen-ing libpulse, fastfetch talks to the daemon
// directly: the daemon speaks a small length prefixed binary protocol on its
// Unix socket (TCP works as well) and the read only introspection commands used
// here need no event loop, no shared memory and no subscription.
//
// A frame is a 20 byte big endian header (payload length, channel, offset high,
// offset low, flags) followed by a "tagstruct" payload: a stream of one byte
// type tags, each followed by its value. All integers are big endian. The header
// of control packets always uses the channel 0xFFFFFFFF with a zero offset and
// flag word.
//
// The command numbers, the tag characters and the field orders below are the ones in pulseaudio's
// own sources -- pulsecore/native-common.h, pulsecore/tagstruct.{c,h} and
// pulsecore/protocol-native.c -- and the place each of them was read from is named where it is used.

typedef enum FFSoundPulseCommand : uint32_t {
    FF_SOUND_PA_COMMAND_ERROR = 0,
    FF_SOUND_PA_COMMAND_REPLY = 2,
    FF_SOUND_PA_COMMAND_AUTH = 8,
    FF_SOUND_PA_COMMAND_SET_CLIENT_NAME = 9,
    FF_SOUND_PA_COMMAND_GET_SERVER_INFO = 20,
    FF_SOUND_PA_COMMAND_GET_SINK_INFO_LIST = 22,
} FFSoundPulseCommand;

typedef enum FFSoundPulseTag : uint8_t {
    FF_SOUND_PA_TAG_STRING = 't',
    FF_SOUND_PA_TAG_STRING_NULL = 'N',
    FF_SOUND_PA_TAG_U32 = 'L',
    FF_SOUND_PA_TAG_U8 = 'B',
    FF_SOUND_PA_TAG_U64 = 'R',
    FF_SOUND_PA_TAG_S64 = 'r',
    FF_SOUND_PA_TAG_SAMPLE_SPEC = 'a',
    FF_SOUND_PA_TAG_ARBITRARY = 'x',
    FF_SOUND_PA_TAG_BOOLEAN_TRUE = '1',
    FF_SOUND_PA_TAG_BOOLEAN_FALSE = '0',
    FF_SOUND_PA_TAG_TIMEVAL = 'T',
    FF_SOUND_PA_TAG_USEC = 'U',
    FF_SOUND_PA_TAG_CHANNEL_MAP = 'm',
    FF_SOUND_PA_TAG_CVOLUME = 'v',
    FF_SOUND_PA_TAG_PROPLIST = 'P',
    FF_SOUND_PA_TAG_VOLUME = 'V',
    FF_SOUND_PA_TAG_FORMAT_INFO = 'f',
} FFSoundPulseTag;

typedef enum FFSoundPulseError {
    FF_SOUND_PA_ERROR_ACCESS = 1,
    FF_SOUND_PA_ERROR_VERSION = 17,
} FFSoundPulseError;

enum {
    FF_SOUND_PA_VERSION = 35,          // PA_PROTOCOL_VERSION
    FF_SOUND_PA_VERSION_MASK = 0xFFFF, // PA_PROTOCOL_VERSION_MASK
    FF_SOUND_PA_COOKIE_LENGTH = 256,   // PA_NATIVE_COOKIE_LENGTH
    FF_SOUND_PA_FRAME_HEADER_SIZE = 20,
    FF_SOUND_PA_FRAME_LENGTH_MAX = 1024 * 1024 * 16, // FRAME_SIZE_MAX_ALLOW
    FF_SOUND_PA_CHANNELS_MAX = 32,                   // PA_CHANNELS_MAX
    FF_SOUND_PA_VOLUME_NORM = 0x10000,               // PA_VOLUME_NORM, i.e. 100%
    FF_SOUND_PA_PORT_AVAILABLE_NO = 2,               // PA_PORT_AVAILABLE_NO
    FF_SOUND_PA_RECV_TIMEOUT_MS = 2000,
};

#define FF_SOUND_PA_DEFAULT_PORT "4713" // PA_NATIVE_DEFAULT_PORT
#define FF_SOUND_PA_SOCKET_NAME "native"

#ifdef MSG_NOSIGNAL
    #define FF_SOUND_PA_SEND_FLAGS MSG_NOSIGNAL
#else
    #define FF_SOUND_PA_SEND_FLAGS 0
#endif

#if defined(__linux__) || defined(__ANDROID__)
    #define FF_SOUND_PA_HAS_CREDENTIALS 1
#else
    #define FF_SOUND_PA_HAS_CREDENTIALS 0
#endif

typedef struct FFSoundPulseConn {
    int fd;
    uint32_t version; // protocol version negotiated with the server
    uint32_t nextTag;
    FFstrbuf frame; // payload of the last received frame
} FFSoundPulseConn;

typedef struct FFSoundPulseReader {
    const char* data;
    uint32_t length;
    uint32_t offset;
    bool error;
} FFSoundPulseReader;

// Everything this module needs to know about one sink. The fields point into the
// frame buffer of the connection.
typedef struct FFSoundPulseSinkInfo {
    const char* name;        // the sink name, reported as the device identifier
    const char* description; // the sink's own description field, reported as the device name
    uint32_t volume;         // cvolume of the first channel
    bool mute;
    bool main;   // the sink equals the server's default sink
    bool active; // the sink has an active port and that port is available
} FFSoundPulseSinkInfo;

// ---------------------------------------------------------------------------
// tagstruct
// ---------------------------------------------------------------------------

static void putTagU32(FFstrbuf* buffer, uint32_t value) {
    ffStrbufAppendC(buffer, FF_SOUND_PA_TAG_U32);
    value = FF_READ_BE(value);
    ffStrbufAppendNS(buffer, sizeof(value), (const char*) &value);
}

static void putTagString(FFstrbuf* buffer, const char* value) {
    if (!value) {
        ffStrbufAppendC(buffer, FF_SOUND_PA_TAG_STRING_NULL);
        return;
    }

    ffStrbufAppendC(buffer, FF_SOUND_PA_TAG_STRING);
    ffStrbufAppendS(buffer, value);
    ffStrbufAppendC(buffer, '\0');
}

static void putTagArbitrary(FFstrbuf* buffer, const void* value, uint32_t length) {
    ffStrbufAppendC(buffer, FF_SOUND_PA_TAG_ARBITRARY);
    uint32_t lengthData = FF_READ_BE(length);
    ffStrbufAppendNS(buffer, sizeof(lengthData), (const char*) &lengthData);
    ffStrbufAppendNS(buffer, length, value);
}

// A proplist is a series of "string key, u32 length, arbitrary value" triples
// terminated by a NULL string. The length is a tagged u32 like any other, only
// the length inside the arbitrary value is raw. Because the terminator uses the
// same tag as a NULL string, a proplist may only appear at the end of a packet --
// which is the case for every packet we send.
static void putTagProplist(FFstrbuf* buffer, const char* key, const char* value) {
    ffStrbufAppendC(buffer, FF_SOUND_PA_TAG_PROPLIST);
    if (key) {
        uint32_t length = (uint32_t) strlen(value);
        putTagString(buffer, key);
        putTagU32(buffer, length);
        putTagArbitrary(buffer, value, length);
    }
    putTagString(buffer, nullptr);
}

static bool readerEof(const FFSoundPulseReader* reader) {
    return reader->offset >= reader->length;
}

static bool skipBytes(FFSoundPulseReader* reader, uint32_t count) {
    if ((uint64_t) reader->offset + count > reader->length) {
        reader->error = true;
        return false;
    }

    reader->offset += count;
    return true;
}

static uint8_t readU8(FFSoundPulseReader* reader) {
    if (!skipBytes(reader, 1)) {
        return 0;
    }

    return (uint8_t) reader->data[reader->offset - 1];
}

static uint32_t readU32(FFSoundPulseReader* reader) {
    if (!skipBytes(reader, 4)) {
        return 0;
    }

    uint32_t value;
    memcpy(&value, reader->data + reader->offset - 4, sizeof(value));
    return FF_READ_BE(value);
}

static bool readU32Value(FFSoundPulseReader* reader, uint32_t* value) {
    if (readU8(reader) != FF_SOUND_PA_TAG_U32) {
        reader->error = true;
        return false;
    }

    *value = readU32(reader);
    return !reader->error;
}

static bool readU8Value(FFSoundPulseReader* reader, uint8_t* value) {
    if (readU8(reader) != FF_SOUND_PA_TAG_U8) {
        reader->error = true;
        return false;
    }

    *value = readU8(reader);
    return !reader->error;
}

// Skips a NUL terminated string whose tag has already been consumed.
static bool skipString(FFSoundPulseReader* reader) {
    while (reader->offset < reader->length && reader->data[reader->offset]) {
        reader->offset++;
    }

    if (reader->offset >= reader->length) {
        reader->error = true;
        return false;
    }

    reader->offset++; // trailing NUL
    return true;
}

// Reads a string tag and returns a pointer to the NUL terminated string inside
// the frame. A NULL string ('N') is a valid value, not an error. The pointer
// stays valid as long as the frame buffer is not reused.
static const char* readStringRef(FFSoundPulseReader* reader) {
    uint8_t tag = readU8(reader);
    if (reader->error) {
        return nullptr;
    }

    if (tag == FF_SOUND_PA_TAG_STRING_NULL) {
        return nullptr;
    }

    if (tag != FF_SOUND_PA_TAG_STRING) {
        reader->error = true;
        return nullptr;
    }

    uint32_t start = reader->offset;
    if (!skipString(reader)) {
        return nullptr;
    }

    return reader->data + start;
}

static bool readString(FFSoundPulseReader* reader, FFstrbuf* value) {
    const char* string = readStringRef(reader);
    if (reader->error) {
        return false;
    }

    if (string) {
        ffStrbufSetS(value, string);
    } else {
        ffStrbufClear(value);
    }

    return true;
}

static bool readBoolean(FFSoundPulseReader* reader, bool* value) {
    uint8_t tag = readU8(reader);
    if (tag == FF_SOUND_PA_TAG_BOOLEAN_TRUE) {
        *value = true;
        return true;
    }

    if (tag == FF_SOUND_PA_TAG_BOOLEAN_FALSE) {
        *value = false;
        return true;
    }

    reader->error = true;
    return false;
}

// Reads a cvolume tag and returns the volume of its first channel. The channel
// count is validated like pa_tagstruct_get_cvolume() does, otherwise a malformed
// packet would make us walk out of the payload.
static bool readCVolume(FFSoundPulseReader* reader, uint32_t* volume) {
    if (readU8(reader) != FF_SOUND_PA_TAG_CVOLUME) {
        reader->error = true;
        return false;
    }

    uint8_t channels = readU8(reader);
    if (reader->error || channels > FF_SOUND_PA_CHANNELS_MAX) {
        reader->error = true;
        return false;
    }

    *volume = 0;
    for (uint8_t channel = 0; channel < channels; channel++) {
        uint32_t value = readU32(reader);
        if (reader->error) {
            return false;
        }

        if (channel == 0) {
            *volume = value;
        }
    }

    return true;
}

// Skips the value that follows the already consumed tag. It dispatches on the
// tag it actually reads, so it can not run out of sync even if the layout
// assumed for the surrounding packet is wrong.
static bool skipValue(FFSoundPulseReader* reader) {
    uint8_t tag = readU8(reader);
    if (reader->error) {
        return false;
    }

    switch (tag) {
        case FF_SOUND_PA_TAG_STRING_NULL:
        case FF_SOUND_PA_TAG_BOOLEAN_TRUE:
        case FF_SOUND_PA_TAG_BOOLEAN_FALSE:
            return true;
        case FF_SOUND_PA_TAG_STRING:
            return skipString(reader); // the tag is already consumed
        case FF_SOUND_PA_TAG_U8:
            return skipBytes(reader, 1);
        case FF_SOUND_PA_TAG_U32:
        case FF_SOUND_PA_TAG_VOLUME:
            return skipBytes(reader, 4);
        case FF_SOUND_PA_TAG_U64:
        case FF_SOUND_PA_TAG_S64:
        case FF_SOUND_PA_TAG_USEC:
        case FF_SOUND_PA_TAG_TIMEVAL:
            return skipBytes(reader, 8);
        case FF_SOUND_PA_TAG_SAMPLE_SPEC:
            return skipBytes(reader, 6); // format, channels, rate
        case FF_SOUND_PA_TAG_ARBITRARY: {
            uint32_t length = readU32(reader);
            return !reader->error && skipBytes(reader, length);
        }
        case FF_SOUND_PA_TAG_CHANNEL_MAP: {
            uint8_t channels = readU8(reader);
            return !reader->error && skipBytes(reader, channels);
        }
        case FF_SOUND_PA_TAG_CVOLUME: {
            uint8_t channels = readU8(reader);
            return !reader->error && skipBytes(reader, (uint32_t) channels * 4);
        }
        case FF_SOUND_PA_TAG_PROPLIST:
            while (!reader->error && !readerEof(reader) && reader->data[reader->offset] != FF_SOUND_PA_TAG_STRING_NULL) {
                if (!skipValue(reader) || !skipValue(reader) || !skipValue(reader)) { // key, length, value
                    return false;
                }
            }
            return skipValue(reader); // the NULL string terminating the list
        case FF_SOUND_PA_TAG_FORMAT_INFO:
            return skipValue(reader) && skipValue(reader); // encoding, proplist
        default:
            FF_DEBUG("Unknown pulseaudio tag 0x%02X", tag);
            reader->error = true;
            return false;
    }
}

// ---------------------------------------------------------------------------
// transport
// ---------------------------------------------------------------------------

static bool writeAll(int fd, const char* data, uint32_t length) {
    while (length > 0) {
        ssize_t written = send(fd, data, length, FF_SOUND_PA_SEND_FLAGS);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }

            FF_DEBUG("Failed to write to the pulseaudio socket: %s", strerror(errno));
            return false;
        }

        data += written;
        length -= (uint32_t) written;
    }

    return true;
}

static bool readAll(int fd, char* data, uint32_t length) {
    while (length > 0) {
        ssize_t received = recv(fd, data, length, 0);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            FF_DEBUG("Failed to read from the pulseaudio socket: %s", strerror(errno));
            return false;
        }

        if (received == 0) { // the server closed the connection
            FF_DEBUG("The pulseaudio server closed the connection");
            return false;
        }

        data += received;
        length -= (uint32_t) received;
    }

    return true;
}

static void applyReceiveTimeout(int fd) {
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &(struct timeval){ .tv_sec = FF_SOUND_PA_RECV_TIMEOUT_MS / 1000, .tv_usec = (FF_SOUND_PA_RECV_TIMEOUT_MS % 1000) * 1000 }, sizeof(struct timeval));
}

static int connectUnixSocket(const char* path) {
    struct sockaddr_un address = { .sun_family = AF_UNIX };
    size_t length = strlen(path);
    if (length >= sizeof(address.sun_path)) {
        FF_DEBUG("The pulseaudio socket path is too long: %s", path);
        return -1;
    }

    memcpy(address.sun_path, path, length + 1);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    if (connect(fd, (struct sockaddr*) &address, (socklen_t) (offsetof(struct sockaddr_un, sun_path) + length + 1)) < 0) {
        FF_DEBUG("Failed to connect to %s: %s", path, strerror(errno));
        close(fd);
        return -1;
    }

    applyReceiveTimeout(fd);
    return fd;
}

static int connectTcpSocket(const char* address, int family) {
    // "host", "host:port" or "[host]:port"
    FF_STRBUF_AUTO_DESTROY host = ffStrbufCreate();
    const char* port = FF_SOUND_PA_DEFAULT_PORT;

    if (*address == '[') {
        const char* end = strchr(address, ']');
        if (!end) {
            return -1;
        }

        ffStrbufSetNS(&host, (uint32_t) (end - address - 1), address + 1);
        if (end[1] == ':') {
            port = end + 2;
        }
    } else {
        const char* colon = strrchr(address, ':');
        if (colon && strchr(address, ':') == colon) { // an unbracketed IPv6 address has several colons
            ffStrbufSetNS(&host, (uint32_t) (colon - address), address);
            port = colon + 1;
        } else {
            ffStrbufSetS(&host, address);
        }
    }

    if (host.length == 0) {
        ffStrbufSetS(&host, "localhost");
    }

    if (*port == '\0') {
        port = FF_SOUND_PA_DEFAULT_PORT;
    }

    struct addrinfo hints = { .ai_family = family, .ai_socktype = SOCK_STREAM };
    struct addrinfo* result = nullptr;
    if (getaddrinfo(host.chars, port, &hints, &result) != 0) {
        FF_DEBUG("Failed to resolve the pulseaudio server address %s", address);
        return -1;
    }

    int fd = -1;
    for (struct addrinfo* entry = result; entry; entry = entry->ai_next) {
        fd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (fd < 0) {
            continue;
        }

        if (connect(fd, entry->ai_addr, entry->ai_addrlen) == 0) {
            break;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(result);

    if (fd < 0) {
        FF_DEBUG("Failed to connect to the pulseaudio server %s", address);
        return -1;
    }

    applyReceiveTimeout(fd);
    return fd;
}

// $PULSE_SERVER overrides the whole address and accepts "unix:", "tcp:", "tcp4:"
// and "tcp6:" prefixes as well as a plain socket path.
static int connectServer(const char* server, const FFstrbuf* defaultSocketPath) {
    if (ffStrStartsWith(server, "unix:")) {
        const char* path = server + strlen("unix:");
        return ffStrSet(path) ? connectUnixSocket(path) : connectUnixSocket(defaultSocketPath->chars);
    }

    if (ffStrStartsWith(server, "tcp4:")) {
        return connectTcpSocket(server + strlen("tcp4:"), AF_INET);
    }

    if (ffStrStartsWith(server, "tcp6:")) {
        return connectTcpSocket(server + strlen("tcp6:"), AF_INET6);
    }

    if (ffStrStartsWith(server, "tcp:")) {
        return connectTcpSocket(server + strlen("tcp:"), AF_UNSPEC);
    }

    return connectUnixSocket(server);
}

// $PULSE_RUNTIME_PATH is used as is, otherwise $XDG_RUNTIME_DIR/pulse. The socket
// is always named "native". The directory is deliberately not created: only the
// daemon owns it.
static void getDefaultSocketPath(FFstrbuf* path) {
    const char* runtimePath = getenv("PULSE_RUNTIME_PATH");
    if (ffStrSet(runtimePath)) {
        ffStrbufSetF(path, "%s/%s", runtimePath, FF_SOUND_PA_SOCKET_NAME);
        return;
    }

    const char* xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");
    if (ffStrSet(xdgRuntimeDir)) {
        ffStrbufSetF(path, "%s/pulse/%s", xdgRuntimeDir, FF_SOUND_PA_SOCKET_NAME);
        return;
    }

    const char* home = getenv("HOME");
    if (ffStrSet(home)) {
        ffStrbufSetF(path, "%s/.pulse/%s", home, FF_SOUND_PA_SOCKET_NAME);
        return;
    }

    ffStrbufClear(path);
}

// The daemon always compares 256 bytes, so the buffer is sent in full. When no
// cookie file is found the daemon still authorizes us by the credentials attached
// to the AUTH frame.
static void getCookie(char cookie[FF_SOUND_PA_COOKIE_LENGTH]) {
    memset(cookie, 0, FF_SOUND_PA_COOKIE_LENGTH);

    FF_STRBUF_AUTO_DESTROY paths = ffStrbufCreate();
    const char* cookiePath = getenv("PULSE_COOKIE");
    if (ffStrSet(cookiePath)) {
        ffStrbufSetS(&paths, cookiePath);
    } else {
        const char* configHome = getenv("XDG_CONFIG_HOME");
        const char* home = getenv("HOME");
        if (ffStrSet(configHome)) {
            ffStrbufAppendF(&paths, "%s/pulse/cookie:", configHome);
        }
        if (ffStrSet(home)) {
            ffStrbufAppendF(&paths, "%s/.config/pulse/cookie:", home);
            ffStrbufAppendF(&paths, "%s/.pulse-cookie", home);
        }
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    for (const char* path = paths.chars; ffStrSet(path);) {
        const char* end = strchr(path, ':');
        FF_STRBUF_AUTO_DESTROY candidate = ffStrbufCreateNS(end ? (uint32_t) (end - path) : (uint32_t) strlen(path), path);
        if (ffReadFileBuffer(candidate.chars, &buffer) && buffer.length > 0) {
            memcpy(cookie, buffer.chars, buffer.length < FF_SOUND_PA_COOKIE_LENGTH ? buffer.length : FF_SOUND_PA_COOKIE_LENGTH);
            FF_DEBUG("Using the pulseaudio cookie from %s", candidate.chars);
            return;
        }

        path = end ? end + 1 : nullptr;
    }

    FF_DEBUG("No pulseaudio cookie found, relying on the credentials of the AUTH frame");
}

// ---------------------------------------------------------------------------
// frames
// ---------------------------------------------------------------------------

// The daemon authorizes same user clients by the credentials of the last packet it
// read before AUTH, so they must not be omitted on Unix sockets. They are attached
// to the whole frame instead of the header alone: with two packets the daemon can
// read the second one without credentials and forget the ones it took from the
// first, which then fails AUTH with PA_ERR_ACCESS. Whether it does depends on how
// it happens to buffer the two writes, which makes that layout flaky.
static bool sendWithCredentials(int fd, const char* data, uint32_t length) {
#if FF_SOUND_PA_HAS_CREDENTIALS
    struct iovec iov = { .iov_base = (void*) data, .iov_len = length };
    char control[CMSG_SPACE(sizeof(struct ucred))];
    struct msghdr message = {
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = control,
        .msg_controllen = sizeof(control),
    };

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&message);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_CREDENTIALS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(struct ucred));
    memcpy(CMSG_DATA(cmsg), &(struct ucred){ .pid = getpid(), .uid = getuid(), .gid = getgid() }, sizeof(struct ucred));

    ssize_t sent;
    do {
        sent = sendmsg(fd, &message, FF_SOUND_PA_SEND_FLAGS);
    } while (sent < 0 && errno == EINTR);

    if (sent < 0) {
        FF_DEBUG("Failed to send the pulseaudio frame: %s", strerror(errno));
        return false;
    }

    return writeAll(fd, data + sent, length - (uint32_t) sent); // the remainder carries no credentials
#else
    return writeAll(fd, data, length);
#endif
}

static bool sendFrame(FFSoundPulseConn* conn, const FFstrbuf* payload, bool withCredentials) {
    FF_STRBUF_AUTO_DESTROY frame = ffStrbufCreate();
    ffStrbufAppendNC(&frame, FF_SOUND_PA_FRAME_HEADER_SIZE, '\0');
    frame.chars[0] = (char) (payload->length >> 24);
    frame.chars[1] = (char) (payload->length >> 16);
    frame.chars[2] = (char) (payload->length >> 8);
    frame.chars[3] = (char) payload->length;
    frame.chars[4] = frame.chars[5] = frame.chars[6] = frame.chars[7] = (char) 0xFF; // channel: control packet
    ffStrbufAppend(&frame, payload);

    if (withCredentials) {
        return sendWithCredentials(conn->fd, frame.chars, frame.length);
    }

    return writeAll(conn->fd, frame.chars, frame.length);
}

static bool recvFrame(FFSoundPulseConn* conn) {
    alignas(uint32_t) char header[FF_SOUND_PA_FRAME_HEADER_SIZE];
    if (!readAll(conn->fd, header, sizeof(header))) {
        return false;
    }

    uint32_t length = FF_READ_BE(*(uint32_t*) header);
    if (length == 0 || length > FF_SOUND_PA_FRAME_LENGTH_MAX) {
        FF_DEBUG("Refusing a pulseaudio frame of %u bytes", length);
        return false;
    }

    ffStrbufEnsureFixedLengthFree(&conn->frame, length);
    if (!readAll(conn->fd, conn->frame.chars, length)) {
        return false;
    }

    conn->frame.chars[length] = '\0';
    conn->frame.length = length;
    return true;
}

// Waits for the reply to `tag`. On success the reader is positioned after the
// command and tag fields.
static const char* recvReply(FFSoundPulseConn* conn, uint32_t tag, FFSoundPulseReader* reader) {
    while (true) {
        if (!recvFrame(conn)) {
            FF_DEBUG("Failed to read the pulseaudio reply to tag %u", tag);
            return "Failed to read a pulseaudio reply";
        }

        FFSoundPulseReader frame = { .data = conn->frame.chars, .length = conn->frame.length };
        uint32_t command = 0;
        uint32_t replyTag = 0;
        // Both the command and the tag are tagged u32s, so they must be read as
        // values. Reading four raw bytes here would swallow the 'L' of the tag.
        if (!readU32Value(&frame, &command) || !readU32Value(&frame, &replyTag)) {
            FF_DEBUG("Malformed pulseaudio reply to tag %u: %u payload bytes", tag, frame.length);
            return "Malformed pulseaudio reply";
        }

        if (replyTag != tag) {
            // Server initiated commands use the tag (uint32_t) -1 and are only
            // sent to subscribed clients. Ignore everything that is not ours.
            continue;
        }

        if (command == FF_SOUND_PA_COMMAND_ERROR) {
            uint32_t error = 0;
            if (!readU32Value(&frame, &error)) {
                FF_DEBUG("Malformed pulseaudio error reply to tag %u: %u payload bytes", tag, frame.length);
                return "Malformed pulseaudio error reply";
            }

            FF_DEBUG("The pulseaudio server returned the error %u", error);
            if (error == FF_SOUND_PA_ERROR_ACCESS) {
                FF_DEBUG("The pulseaudio server denied access to the AUTH request");
                return "Access to the pulseaudio server was denied";
            }

            if (error == FF_SOUND_PA_ERROR_VERSION) {
                FF_DEBUG("The pulseaudio server rejected our protocol version %u", FF_SOUND_PA_VERSION);
                return "The pulseaudio server is too old";
            }

            return "The pulseaudio server refused the request";
        }

        if (command != FF_SOUND_PA_COMMAND_REPLY) {
            FF_DEBUG("Unexpected pulseaudio command %u in the reply to tag %u", command, tag);
            return "Unexpected pulseaudio reply";
        }

        *reader = frame;
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// handshake
// ---------------------------------------------------------------------------

static const char* handshake(FFSoundPulseConn* conn, FFstrbuf* serverName, FFstrbuf* defaultSinkName) {
    {
        FF_STRBUF_AUTO_DESTROY payload = ffStrbufCreate();
        uint32_t tag = conn->nextTag++;
        putTagU32(&payload, FF_SOUND_PA_COMMAND_AUTH);
        putTagU32(&payload, tag);
        putTagU32(&payload, FF_SOUND_PA_VERSION); // neither PA_PROTOCOL_FLAG_SHM nor PA_PROTOCOL_FLAG_MEMFD

        char cookie[FF_SOUND_PA_COOKIE_LENGTH];
        getCookie(cookie);
        putTagArbitrary(&payload, cookie, sizeof(cookie));

        if (!sendFrame(conn, &payload, true)) {
            FF_DEBUG("Failed to send the pulseaudio AUTH request (tag %u)", tag);
            return "Failed to send the pulseaudio AUTH request";
        }

        FFSoundPulseReader reader;
        const char* error = recvReply(conn, tag, &reader);
        if (error) {
            return error;
        }

        uint32_t version = 0;
        if (!readU32Value(&reader, &version)) {
            return "Malformed pulseaudio AUTH reply";
        }

        conn->version = version & FF_SOUND_PA_VERSION_MASK;
        if (conn->version > FF_SOUND_PA_VERSION) {
            conn->version = FF_SOUND_PA_VERSION;
        }

        FF_DEBUG("Negotiated pulseaudio protocol version %u", conn->version);
    }

    {
        FF_STRBUF_AUTO_DESTROY payload = ffStrbufCreate();
        uint32_t tag = conn->nextTag++;
        putTagU32(&payload, FF_SOUND_PA_COMMAND_SET_CLIENT_NAME);
        putTagU32(&payload, tag);

        if (conn->version >= 13) {
            putTagProplist(&payload, "application.name", "fastfetch");
        } else {
            putTagString(&payload, "fastfetch");
        }

        if (!sendFrame(conn, &payload, false)) {
            FF_DEBUG("Failed to send the pulseaudio SET_CLIENT_NAME request (tag %u)", tag);
            return "Failed to send the pulseaudio SET_CLIENT_NAME request";
        }

        FFSoundPulseReader reader;
        const char* error = recvReply(conn, tag, &reader);
        if (error) {
            return error;
        }
    }

    {
        FF_STRBUF_AUTO_DESTROY payload = ffStrbufCreate();
        uint32_t tag = conn->nextTag++;
        putTagU32(&payload, FF_SOUND_PA_COMMAND_GET_SERVER_INFO);
        putTagU32(&payload, tag);

        if (!sendFrame(conn, &payload, false)) {
            FF_DEBUG("Failed to send the pulseaudio GET_SERVER_INFO request (tag %u)", tag);
            return "Failed to send the pulseaudio GET_SERVER_INFO request";
        }

        FFSoundPulseReader reader;
        const char* error = recvReply(conn, tag, &reader);
        if (error) {
            return error;
        }

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
        if (!readString(&reader, &name) || !readString(&reader, &version)) {
            return "Malformed pulseaudio GET_SERVER_INFO reply";
        }

        if (!skipValue(&reader) || !skipValue(&reader)) { // user name, host name
            return "Malformed pulseaudio GET_SERVER_INFO reply";
        }

        if (!skipValue(&reader)) { // sample spec
            return "Malformed pulseaudio GET_SERVER_INFO reply";
        }

        if (!readString(&reader, defaultSinkName)) { // default sink name, may be a NULL string
            return "Malformed pulseaudio GET_SERVER_INFO reply";
        }

        FF_DEBUG("Default pulseaudio sink: %s", defaultSinkName->chars);

        // pipewire-pulse reports itself as "PulseAudio (on PipeWire x.y.z)"
        const char* on = strstr(name.chars, "(on ");
        if (on) {
            ffStrbufSetS(serverName, on + strlen("(on "));
            ffStrbufTrimRight(serverName, ')');
        } else {
            ffStrbufSetF(serverName, "%s %s", name.chars, version.chars);
        }
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// sinks
// ---------------------------------------------------------------------------

// The port that is reported as active decides whether a sink counts as active.
// Since its name is sent after the port list, the list is scanned a second time
// to look up the availability of that port.
static bool isPortAvailable(const FFSoundPulseReader* reader, uint32_t version, uint32_t portCount, uint32_t portsOffset, const char* activePort) {
    if (version < 24) {
        return true; // ports carry no availability information before v24
    }

    FFSoundPulseReader scan = *reader;
    scan.offset = portsOffset;
    scan.error = false;

    for (uint32_t port = 0; port < portCount; port++) {
        const char* name = readStringRef(&scan);
        if (scan.error || !skipValue(&scan) || !skipValue(&scan)) { // description, priority
            return true;
        }

        uint32_t available = 0;
        if (!readU32Value(&scan, &available)) {
            return true;
        }

        if (version >= 34) { // availability group, type
            if (!skipValue(&scan) || !skipValue(&scan)) {
                return true;
            }
        }

        if (name && ffStrEquals(name, activePort)) {
            return available != FF_SOUND_PA_PORT_AVAILABLE_NO;
        }
    }

    return true;
}

// Sink objects are sent in the order of sink_fill_tagstruct(). Everything that is
// not needed is skipped; the fields that are read are tag checked so that a
// layout mismatch is reported instead of silently shifting the whole list.
static bool readSinkInfo(FFSoundPulseReader* reader, const FFSoundPulseConn* conn, const FFstrbuf* defaultSinkName, FFSoundPulseSinkInfo* sink) {
    uint32_t version = conn->version;

    if (!skipValue(reader)) { // index
        return false;
    }

    sink->name = readStringRef(reader);
    if (reader->error || !sink->name) {
        return false;
    }

    sink->description = readStringRef(reader);
    if (reader->error) {
        return false;
    }

    if (!skipValue(reader)) { // sample spec
        return false;
    }

    if (!skipValue(reader)) { // channel map
        return false;
    }

    if (!skipValue(reader)) { // owner module
        return false;
    }

    if (!readCVolume(reader, &sink->volume)) {
        return false;
    }

    if (!readBoolean(reader, &sink->mute)) {
        return false;
    }

    if (!skipValue(reader)) { // monitor source
        return false;
    }

    if (!skipValue(reader)) { // monitor source name
        return false;
    }

    if (!skipValue(reader)) { // latency
        return false;
    }

    if (!skipValue(reader)) { // driver
        return false;
    }

    if (!skipValue(reader)) { // flags
        return false;
    }

    if (version >= 13) {
        if (!skipValue(reader)) { // proplist
            return false;
        }

        if (!skipValue(reader)) { // configured latency
            return false;
        }
    }

    if (version >= 15) {
        for (uint32_t field = 0; field < 4; field++) { // base volume, state, volume steps, card
            if (!skipValue(reader)) {
                return false;
            }
        }
    }

    const char* activePort = nullptr;
    uint32_t portCount = 0;
    uint32_t portsOffset = 0;

    if (version >= 16) {
        if (!readU32Value(reader, &portCount)) {
            return false;
        }

        portsOffset = reader->offset;
        for (uint32_t port = 0; port < portCount; port++) {
            if (!skipValue(reader)) { // name
                return false;
            }

            if (!skipValue(reader)) { // description
                return false;
            }

            if (!skipValue(reader)) { // priority
                return false;
            }

            if (version >= 24 && !skipValue(reader)) { // availability
                return false;
            }

            if (version >= 34) {
                if (!skipValue(reader) || !skipValue(reader)) { // availability group, type
                    return false;
                }
            }
        }

        activePort = readStringRef(reader); // the last field of the port section
        if (reader->error) {
            return false;
        }
    }

    if (version >= 21) {
        uint8_t formats = 0;
        if (!readU8Value(reader, &formats)) { // number of format infos
            return false;
        }

        for (uint8_t format = 0; format < formats; format++) {
            if (!skipValue(reader)) { // format info
                return false;
            }
        }
    }

    sink->main = ffStrbufEqualS(defaultSinkName, sink->name);
    sink->active = activePort && isPortAvailable(reader, version, portCount, portsOffset, activePort);
    return true;
}

static void appendSink(FFlist* devices, const FFSoundPulseSinkInfo* sink, const FFstrbuf* serverName) {
    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *devices);
    ffStrbufInitS(&device->identifier, sink->name);
    ffStrbufTrimRightSpace(&device->identifier);
    ffStrbufInitCopy(&device->platformApi, serverName);
    ffStrbufInitS(&device->name, sink->description ? sink->description : "");
    ffStrbufTrimRightSpace(&device->name);
    ffStrbufTrimLeft(&device->name, ' ');
    device->volume = sink->mute ? 0 : (uint8_t) (((uint64_t) sink->volume * 100 + FF_SOUND_PA_VOLUME_NORM / 2) / FF_SOUND_PA_VOLUME_NORM);
    device->type = (sink->main ? FF_SOUND_TYPE_MAIN : FF_SOUND_TYPE_NONE) | (sink->active ? FF_SOUND_TYPE_ACTIVE : FF_SOUND_TYPE_NONE);
}

static const char* fetchSinkList(FFSoundPulseConn* conn, const FFSoundOptions* options, FFlist* devices, const FFstrbuf* serverName, const FFstrbuf* defaultSinkName) {
    FF_STRBUF_AUTO_DESTROY payload = ffStrbufCreate();
    uint32_t tag = conn->nextTag++;
    putTagU32(&payload, FF_SOUND_PA_COMMAND_GET_SINK_INFO_LIST);
    putTagU32(&payload, tag);

    if (!sendFrame(conn, &payload, false)) {
        FF_DEBUG("Failed to send the pulseaudio GET_SINK_INFO_LIST request (tag %u)", tag);
        return "Failed to send the pulseaudio GET_SINK_INFO_LIST request";
    }

    FFSoundPulseReader reader;
    const char* error = recvReply(conn, tag, &reader);
    if (error) {
        return error;
    }

    // All sinks are concatenated in a single reply without a separator, a count
    // or a terminator: parse until the payload is exhausted.
    while (!reader.error && !readerEof(&reader)) {
        FFSoundPulseSinkInfo sink = {};
        if (!readSinkInfo(&reader, conn, defaultSinkName, &sink)) {
            FF_DEBUG("Failed to parse a pulseaudio sink at payload offset %u of %u", reader.offset, reader.length);
            return "Failed to parse the pulseaudio sink list";
        }

        if ((options->soundType & FF_SOUND_TYPE_MAIN) && !sink.main) {
            continue;
        }

        if ((options->soundType & FF_SOUND_TYPE_ACTIVE) && !sink.active) {
            continue;
        }

        appendSink(devices, &sink, serverName);
    }

    if (reader.error) {
        FF_DEBUG("The pulseaudio sink list ended at payload offset %u of %u", reader.offset, reader.length);
        return "Failed to parse the pulseaudio sink list";
    }

    FF_DEBUG("Parsed %u pulseaudio sinks", devices->length);
    return nullptr;
}

// ---------------------------------------------------------------------------
// the hardware: ALSA
// ---------------------------------------------------------------------------

// A machine that runs no sound server still has ALSA, and this is what answers for it: the cards
// are enumerated from /proc and their volumes read from the control device of each card.
//
// Nothing is linked and nothing is loaded. The values are behind four ioctls of the kernel's own
// control interface, and both the structs and the request numbers are the kernel's uapi, which
// every libc ships as <sound/asound.h>. The request numbers are `_IOWR`-style, so they carry the
// size of the struct they pass -- a layout that does not match the kernel's is refused as an
// unknown ioctl instead of being read as garbage.
//
// Android is left out: it has no ALSA of the kind this reads ( /dev/snd and /proc/asound are
// closed to an app's SELinux domain ) and a session that does have a desktop on top of the device
// is running a sound server, which is the branch below. A libc that does not carry the header says
// so here, and the fallback is simply not compiled in -- the same shape as the other optional
// backends in this tree.
#if defined(__linux__) && !defined(__ANDROID__) && __has_include(<sound/asound.h>)
    #define FF_SOUND_ALSA 1
#endif

#ifdef FF_SOUND_ALSA

    #include <fcntl.h>
    #include <stdio.h>
    // <sys/ioctl.h> before <sound/asound.h>, in this order: the uapi header spells its request
    // numbers with _IOR and _IOWR and does not define either of them itself.
    #include <sys/ioctl.h>
    #include <sound/asound.h>

// What a mixer control *is* is written down nowhere in the control interface. The kernel describes
// a control with a type (integer, boolean, ...) and an access bitmask, and that bitmask has no
// volume and no switch bit at all -- the uapi header defines READ, WRITE, VOLATILE, TLV_*, INACTIVE,
// LOCK, OWNER and USER, and nothing else. What carries it is the control's name, which a driver
// builds by appending a suffix to the name of what is being controlled:
//
//     "Master Playback Volume"   the output volume of the control named "Master"
//     "Capture Volume"           the input volume of a control that has no name of its own
//
// So the suffix is what is matched, on one condition: it has to be the whole of the name or the
// start of a word in it. Without that, "Capture Volume" would end in " Volume" and be read as the
// directionless " Volume" -- which is the volume of a card that has only one direction, and so an
// output.
//
// The set of suffixes is the one libasound's simple mixer layer carries; the entries sit one after
// another in that library's .rodata. The order below is the order they have to be tried in, the
// longest first, or "Master Playback Volume" would be taken for a " Volume".
//
// One deliberate difference from libasound: it reads "Capture Volume" as a directionless volume,
// which is why a desktop mixer shows a "Capture" slider. This module reports output devices, and a
// capture gain is not one of them -- so a card that carries nothing else has no output to report.
typedef struct FFSoundAlsaSuffix {
    const char* suffix;
    bool isSwitch; // ... the suffix names a switch rather than a volume
    bool playback; // ... and one of the output direction
} FFSoundAlsaSuffix;

static const FFSoundAlsaSuffix FF_SOUND_ALSA_SUFFIXES[] = {
    { "Playback Volume", false, true },
    { "Playback Switch", true, true },
    { "Playback Enum", false, false },
    { "Playback Route", false, false },
    { "Capture Volume", false, false },
    { "Capture Switch", false, false },
    { "Capture Enum", false, false },
    { "Capture Route", false, false },
    { "Volume", false, true },
    { "Switch", true, true },
    { "Enum", false, false },
    { "Route", false, false },
};

// Which control is "the volume" of a card. A desktop mixer shows one slider per output and the name
// on it is one of these. A card that has none of them -- a USB interface, a virtual card -- still
// has exactly one playback volume, and there the first one the kernel enumerates is taken. This list
// exists so that the choice does not have to come out of that order, which is the order the driver
// registered its controls in and not a ranking at all.
static const char* const FF_SOUND_ALSA_MAIN_CONTROLS[] = { "Master", "Master Mono", "PCM", "Speaker", "Headphone", "Headphones", "Front" };

enum {
    // Ids asked for per ELEM_LIST call. The list is walked in batches rather than copied in one go,
    // so that nothing has to be sized after a number the card reports; 64 ids is 4 KiB of stack, and
    // the far side of the ioctl is a table lookup either way.
    FF_SOUND_ALSA_ID_BATCH = 64,
    FF_SOUND_ALSA_PRIORITY_NONE = UINT32_MAX,
};

// Whether the control name is that of a playback volume or switch, and how much of it the suffix
// takes up. Both suffixes that are accepted end in the same six character tail -- "Volume" or
// "Switch" -- which is the only thing that pairs a card's volume with its mute; nothing else in the
// control interface says the two belong together.
static bool stripAlsaNameSuffix(const char* name, uint32_t length, bool* isSwitch, uint32_t* baseLength) {
    for (size_t index = 0; index < ARRAY_SIZE(FF_SOUND_ALSA_SUFFIXES); index++) {
        const FFSoundAlsaSuffix* suffix = &FF_SOUND_ALSA_SUFFIXES[index];
        const uint32_t suffixLength = (uint32_t) strlen(suffix->suffix);
        if (length < suffixLength || memcmp(name + length - suffixLength, suffix->suffix, suffixLength) != 0) {
            continue;
        }

        // A suffix is a word of the name: either all of it, or preceded by a space. Anything else
        // is a coincidence of letters -- "Capture Volume" ends in " Volume" and is not one.
        if (length > suffixLength && name[length - suffixLength - 1] != ' ') {
            continue;
        }

        if (!suffix->playback) {
            return false; // a capture or a routing control is not the card's output
        }

        // The space that separated the base name from the suffix belongs to neither, and a name
        // that is nothing but the suffix has no base name at all.
        uint32_t base = length - suffixLength;
        while (base > 0 && name[base - 1] == ' ') {
            base--;
        }

        *isSwitch = suffix->isSwitch;
        *baseLength = base;
        return true;
    }

    return false;
}

// The rank of a control's base name, or FF_SOUND_ALSA_PRIORITY_NONE when the list does not know it.
static uint32_t getAlsaControlPriority(const char* name, uint32_t baseLength) {
    for (size_t index = 0; index < ARRAY_SIZE(FF_SOUND_ALSA_MAIN_CONTROLS); index++) {
        if (strlen(FF_SOUND_ALSA_MAIN_CONTROLS[index]) == baseLength && memcmp(name, FF_SOUND_ALSA_MAIN_CONTROLS[index], baseLength) == 0) {
            return (uint32_t) index;
        }
    }

    return FF_SOUND_ALSA_PRIORITY_NONE;
}

// Walks the controls of one card. ELEM_LIST holds no state -- every call carries the offset it wants
// and the buffer to fill -- so a second walk of the same card is simply a second one of these.
typedef struct FFSoundAlsaControls {
    int fd;
    uint32_t offset;
    uint32_t index;
    uint32_t batch;
    bool done;
    struct snd_ctl_elem_id ids[FF_SOUND_ALSA_ID_BATCH];
} FFSoundAlsaControls;

// The next mixer control of the card, or nullptr at the end. A list that cannot be read ends the
// walk rather than failing it: what to do about "this card says nothing" belongs to the caller, and
// everything below treats it exactly like a card that has no volume control. The other interfaces a
// card carries (PCM, timer, raw MIDI) are skipped here so that no caller has to know about them.
static const struct snd_ctl_elem_id* nextAlsaControl(FFSoundAlsaControls* controls) {
    for (;;) {
        if (controls->index < controls->batch) {
            const struct snd_ctl_elem_id* id = &controls->ids[controls->index++];
            if (id->iface != SNDRV_CTL_ELEM_IFACE_MIXER) {
                continue;
            }

            return id;
        }

        if (controls->done) {
            return nullptr;
        }

        struct snd_ctl_elem_list list = {
            .offset = controls->offset,
            .space = FF_SOUND_ALSA_ID_BATCH,
            .pids = controls->ids,
        };

        if (ioctl(controls->fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) < 0) {
            FF_DEBUG("Listing the controls of an ALSA card failed: %s", strerror(errno));
            controls->done = true;
            return nullptr;
        }

        controls->offset += list.used;
        controls->index = 0;
        controls->batch = list.used;

        if (list.used == 0) {
            controls->done = true;
            return nullptr;
        }
    }
}

// A card's volume control and its name without the suffix -- which is what a desktop mixer puts on
// the slider, "Master" rather than "Master Playback Volume".
typedef struct FFSoundAlsaVolume {
    struct snd_ctl_elem_id id;
    char base[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
    uint32_t baseLength;
} FFSoundAlsaVolume;

static bool findAlsaVolume(FFSoundAlsaControls* controls, FFSoundAlsaVolume* volume) {
    uint32_t priority = FF_SOUND_ALSA_PRIORITY_NONE;
    bool found = false;

    const struct snd_ctl_elem_id* id = nullptr;
    while ((id = nextAlsaControl(controls)) != nullptr) {
        // The name is an array of SNDRV_CTL_ELEM_ID_NAME_MAXLEN bytes with the terminator inside
        // it, so its length is bounded rather than taken on trust.
        const uint32_t length = (uint32_t) strnlen((const char*) id->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN);
        bool isSwitch = false;
        uint32_t baseLength = 0;
        if (!stripAlsaNameSuffix((const char*) id->name, length, &isSwitch, &baseLength) || isSwitch) {
            continue;
        }

        const uint32_t candidate = getAlsaControlPriority((const char*) id->name, baseLength);
        if (found && candidate >= priority) {
            continue; // a control the list knows beats one it does not, and the first of a rank wins
        }

        volume->id = *id;
        volume->baseLength = baseLength;
        memcpy(volume->base, id->name, baseLength);
        volume->base[baseLength] = '\0';
        priority = candidate;
        found = true;
    }

    return found;
}

// The switch that belongs to the volume control: same base name, same index, same device. It takes
// a second walk because which base name wins is only known once the first walk is over. A card with
// no switch at all leaves the caller with "not muted", which is the answer a control without a mute
// state deserves.
static bool findAlsaSwitch(FFSoundAlsaControls* controls, const FFSoundAlsaVolume* volume, struct snd_ctl_elem_id* result) {
    const struct snd_ctl_elem_id* id = nullptr;
    while ((id = nextAlsaControl(controls)) != nullptr) {
        if (id->index != volume->id.index || id->device != volume->id.device || id->subdevice != volume->id.subdevice) {
            continue;
        }

        const uint32_t length = (uint32_t) strnlen((const char*) id->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN);
        bool isSwitch = false;
        uint32_t baseLength = 0;
        if (!stripAlsaNameSuffix((const char*) id->name, length, &isSwitch, &baseLength) || !isSwitch) {
            continue;
        }

        if (baseLength != volume->baseLength || memcmp(id->name, volume->base, baseLength) != 0) {
            continue;
        }

        *result = *id;
        return true;
    }

    return false;
}

// The output of a card that has no volume control: any playback switch, the first one the kernel
// enumerates. HDMI and DP cards are typically this shape, with an "IEC958 Playback Switch" and no
// volume to go with it, so the switch is the only thing that says the card plays anything at all.
// Returns whether the card has a playback switch, so a caller can tell "no output" from "a switch".
static bool findAlsaPlaybackSwitch(FFSoundAlsaControls* controls, FFSoundAlsaVolume* output) {
    const struct snd_ctl_elem_id* id = nullptr;
    while ((id = nextAlsaControl(controls)) != nullptr) {
        const uint32_t length = (uint32_t) strnlen((const char*) id->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN);
        bool isSwitch = false;
        uint32_t baseLength = 0;
        if (!stripAlsaNameSuffix((const char*) id->name, length, &isSwitch, &baseLength) || !isSwitch) {
            continue;
        }

        output->id = *id;
        output->baseLength = baseLength;
        memcpy(output->base, id->name, baseLength);
        output->base[baseLength] = '\0';
        return true;
    }

    return false;
}

// One value of a control, and the range that control's own scale puts it in. A switch is a boolean
// and shares the same union; a few DSP cards use the 64 bit integer type.
typedef struct FFSoundAlsaValue {
    long long value;
    long long minimum;
    long long maximum;
} FFSoundAlsaValue;

static bool readAlsaValue(int fd, const struct snd_ctl_elem_id* id, FFSoundAlsaValue* result) {
    struct snd_ctl_elem_info info = { .id = *id };
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0) {
        FF_DEBUG("Reading the type of an ALSA control failed: %s", strerror(errno));
        return false;
    }

    struct snd_ctl_elem_value value = { .id = *id };
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &value) < 0) {
        FF_DEBUG("Reading the value of an ALSA control failed: %s", strerror(errno));
        return false;
    }

    // Only the first channel is read, the way the PulseAudio backend reports the first channel of a
    // sink's volume. A card whose channels disagree is left to the mixer to sort out.
    if (info.type == SNDRV_CTL_ELEM_TYPE_INTEGER64) {
        result->value = value.value.integer64.value[0];
        result->minimum = info.value.integer64.min;
        result->maximum = info.value.integer64.max;
        return true;
    }

    if (info.type != SNDRV_CTL_ELEM_TYPE_INTEGER && info.type != SNDRV_CTL_ELEM_TYPE_BOOLEAN) {
        FF_DEBUG("An ALSA control carries the value type %u, which is not a volume", (unsigned) info.type);
        return false;
    }

    result->value = value.value.integer.value[0];
    result->minimum = info.value.integer.min;
    result->maximum = info.value.integer.max;
    return true;
}

// The control's own scale as a percentage, which is the number `amixer` prints as `[n%]`. A range
// that is empty or inverted says nothing at all, and is reported as unknown rather than as 0% --
// 0% is a state a control is really in, and it is the muted one.
static uint8_t getAlsaPercent(const FFSoundAlsaValue* value) {
    if (value->maximum <= value->minimum) {
        return FF_SOUND_VOLUME_UNKNOWN;
    }

    if (value->value <= value->minimum) {
        return 0;
    }

    if (value->value >= value->maximum) {
        return 100;
    }

    return (uint8_t) (((value->value - value->minimum) * 100 + (value->maximum - value->minimum) / 2) / (value->maximum - value->minimum));
}

// Appends the output of one card. Returns whether the card has an output to report at all, which is
// not the same as whether a device was appended: `options` filters what is kept, while the main
// device of a machine is the first card that has an output whatever the filter then does with it.
//
// A card whose control device cannot be opened is reported without a volume rather than dropped.
// The devices are root:audio behind an ACL for the seat, so a session over ssh can name a card it
// may not read, and the name comes from /proc/asound/cards, which is world readable.
//
// The reported name is the card's own name from /proc/asound/cards followed by the control's base
// name -- "HDA Intel PCH Master" rather than a bare "Master" -- so the list says which card each
// volume belongs to. A card that *can* be read and has no playback control at all is dropped
// instead, because that is a card with nothing to play through. An HDMI or DP card often has no
// volume control but does have an "IEC958 Playback Switch"; the switch alone is still an output, so
// such a card is reported, with no percentage to go with it.
static bool appendAlsaCard(uint32_t card, const FFstrbuf* cardName, bool main, const FFSoundOptions* options, FFlist* devices) {
    // "/dev/snd/controlC", at most ten digits of the card index, and the terminator.
    char path[sizeof("/dev/snd/controlC") + 11];
    snprintf(path, sizeof(path), "/dev/snd/controlC%u", card);

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateCopy(cardName);
    // A card whose control device cannot be opened is not a card without an output, so it still
    // gets a device -- without a percentage.
    uint8_t volume = FF_SOUND_VOLUME_UNKNOWN;
    bool found = false;
    bool active = true;

    const int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        // Only read access is asked for: a volume is being read, never written, and read permission
        // is the smaller of the two things to need.
        FF_DEBUG("Opening \"%s\" failed: %s", path, strerror(errno));
        found = true;
    } else {
        FFSoundAlsaControls controls = { .fd = fd };
        FFSoundAlsaVolume control = {};

        if (findAlsaVolume(&controls, &control)) {
            found = true;
            // A control whose name is nothing but the suffix -- a codec that has one volume and no
            // name for it registers "Playback Volume" -- leaves the card's own name in place, which
            // is the only thing left to call the device.
            if (control.baseLength > 0) {
                ffStrbufAppendC(&name, ' ');
                ffStrbufAppendNS(&name, control.baseLength, control.base);
            }

            FFSoundAlsaValue value = {};
            if (readAlsaValue(fd, &control.id, &value)) {
                volume = getAlsaPercent(&value);
            }

            FFSoundAlsaControls switches = { .fd = fd };
            struct snd_ctl_elem_id switchId = {};
            FFSoundAlsaValue switchValue = {};
            if (findAlsaSwitch(&switches, &control, &switchId) && readAlsaValue(fd, &switchId, &switchValue)) {
                active = switchValue.value != 0;
                if (!active) {
                    // A muted control is reported as 0% rather than as the value it still holds,
                    // which is what the PulseAudio backend does with a muted sink.
                    volume = 0;
                }
            }
        } else {
            // No volume control: an HDMI or DP card is usually this shape, its only output control
            // an "IEC958 Playback Switch". The switch alone is still an output to play through, so
            // the card is reported -- without a percentage, because there is no volume to read.
            FFSoundAlsaControls switches = { .fd = fd };
            FFSoundAlsaVolume switchControl = {};
            if (findAlsaPlaybackSwitch(&switches, &switchControl)) {
                found = true;
                if (switchControl.baseLength > 0) {
                    ffStrbufAppendC(&name, ' ');
                    ffStrbufAppendNS(&name, switchControl.baseLength, switchControl.base);
                }

                FFSoundAlsaValue switchValue = {};
                if (readAlsaValue(fd, &switchControl.id, &switchValue)) {
                    active = switchValue.value != 0;
                    if (!active) {
                        volume = 0;
                    }
                }
            }
        }

        close(fd);
    }

    if (!found) {
        FF_DEBUG("Card %u has no playback control", card);
        return false;
    }

    const FFSoundType type = (FFSoundType) ((main ? FF_SOUND_TYPE_MAIN : FF_SOUND_TYPE_NONE) | (active ? FF_SOUND_TYPE_ACTIVE : FF_SOUND_TYPE_NONE));
    if ((options->soundType & FF_SOUND_TYPE_MAIN) && !(type & FF_SOUND_TYPE_MAIN)) {
        return true;
    }

    if ((options->soundType & FF_SOUND_TYPE_ACTIVE) && !(type & FF_SOUND_TYPE_ACTIVE)) {
        return true;
    }

    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *devices);
    // "hw:0" is how the card is named everywhere else -- `amixer -c 0`, and the name after a colon
    // in every ALSA configuration -- so it is what identifies the device here.
    ffStrbufInitF(&device->identifier, "hw:%u", card);
    ffStrbufInitMove(&device->name, &name);
    ffStrbufInitStatic(&device->platformApi, "ALSA");
    device->volume = volume;
    device->type = type;
    FF_DEBUG("Reporting \"%s\" (hw:%u) at %u%%%s", device->name.chars, card, device->volume, active ? "" : ", muted");
    return true;
}

static const char* detectAlsa(FFSoundOptions* options, FFlist* devices) {
    FF_STRBUF_AUTO_DESTROY cards = ffStrbufCreate();
    if (!ffReadFileBuffer("/proc/asound/cards", &cards)) {
        FF_DEBUG("Reading \"/proc/asound/cards\" failed: %s", strerror(errno));
        return "Failed to read the ALSA card list";
    }

    bool main = true; // the machine's main output is the first card that has one
    bool output = false;

    for (const char* line = cards.chars; *line;) {
        const char* newline = strchr(line, '\n');
        const uint32_t length = newline ? (uint32_t) (newline - line) : (uint32_t) strlen(line);
        FF_STRBUF_AUTO_DESTROY record = ffStrbufCreateNS(length, line);
        line = newline ? newline + 1 : line + length;

        //      0 [PCH            ]: HDA-Intel - HDA Intel PCH
        //                            HDA Intel PCH at 0xdf240000 irq 125
        //
        // One line per card, with the index in the first column, and the card's long name on the
        // line below -- which does not start with a digit, and is what this skips. The name that is
        // reported is the one behind the dash: the driver's name for the card, without the
        // "HDA-Intel - " in front of it.
        char* indexEnd = nullptr;
        const long number = strtol(record.chars, &indexEnd, 10);
        if (indexEnd == record.chars || number < 0) {
            continue;
        }

        const char* close = strchr(indexEnd, ']');
        const char* dash = close ? strstr(close, " - ") : nullptr;
        if (dash == nullptr) {
            FF_DEBUG("The ALSA card list line \"%s\" is not in the expected form", record.chars);
            continue;
        }

        FF_STRBUF_AUTO_DESTROY cardName = ffStrbufCreateS(dash + strlen(" - "));
        ffStrbufTrimRightSpace(&cardName);

        if (appendAlsaCard((uint32_t) number, &cardName, main, options, devices)) {
            main = false;
            output = true;
        }
    }

    if (!output) {
        return "No ALSA card with a playback control was found";
    }

    return nullptr;
}

#endif // FF_SOUND_ALSA

// ---------------------------------------------------------------------------
// the server
// ---------------------------------------------------------------------------

static const char* detectPulseAudio(FFSoundOptions* options, FFlist* devices) {
    FF_STRBUF_AUTO_DESTROY defaultSocketPath = ffStrbufCreate();
    getDefaultSocketPath(&defaultSocketPath);
    FF_DEBUG("Default pulseaudio socket path: %s", defaultSocketPath.chars);

    FFSoundPulseConn conn = {
        .fd = -1,
        .version = FF_SOUND_PA_VERSION,
        .nextTag = 0,
        .frame = ffStrbufCreate(),
    };

    const char* serverList = getenv("PULSE_SERVER");
    if (ffStrSet(serverList)) {
        // $PULSE_SERVER may list several whitespace separated candidates; the
        // first one that accepts a connection wins, like libpulse does.
        for (const char* cursor = serverList; *cursor && conn.fd < 0;) {
            while (*cursor == ' ' || *cursor == '\t') {
                cursor++;
            }

            const char* end = cursor;
            while (*end && *end != ' ' && *end != '\t') {
                end++;
            }

            if (end == cursor) {
                break;
            }

            FF_STRBUF_AUTO_DESTROY server = ffStrbufCreateNS((uint32_t) (end - cursor), cursor);
            conn.fd = connectServer(server.chars, &defaultSocketPath);
            cursor = end;
        }
    } else if (defaultSocketPath.length > 0) {
        conn.fd = connectUnixSocket(defaultSocketPath.chars);
    }

    if (conn.fd < 0) {
        FF_DEBUG("Failed to connect to the pulseaudio server on %s",
            defaultSocketPath.length > 0 ? defaultSocketPath.chars : "no known socket path");
        ffStrbufDestroy(&conn.frame);
        return "Failed to connect to the pulseaudio server";
    }

    FF_STRBUF_AUTO_DESTROY serverName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY defaultSinkName = ffStrbufCreate();
    FF_LIST_AUTO_DESTROY parsed = ffListCreate();

    const char* error = handshake(&conn, &serverName, &defaultSinkName);
    if (!error) {
        error = fetchSinkList(&conn, options, &parsed, &serverName, &defaultSinkName);
    }

    close(conn.fd);
    ffStrbufDestroy(&conn.frame);

    if (error) {
        FF_LIST_FOR_EACH (FFSoundDevice, device, parsed) {
            ffStrbufDestroy(&device->identifier);
            ffStrbufDestroy(&device->name);
            ffStrbufDestroy(&device->platformApi);
        }

        return error;
    }

    FF_LIST_FOR_EACH (FFSoundDevice, device, parsed) {
        FFSoundDevice* target = FF_LIST_ADD(FFSoundDevice, *devices);
        target->identifier = ffStrbufCreateMove(&device->identifier);
        target->name = ffStrbufCreateMove(&device->name);
        target->platformApi = ffStrbufCreateMove(&device->platformApi);
        target->volume = device->volume;
        target->type = device->type;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------

// Android compiles this file next to sound_android.c, which owns ffDetectSound and calls this one
// only while the display server is not SurfaceFlinger -- a Termux:X11 or Wayland session runs a real
// desktop on top of the device, and PulseAudio is the sound of that desktop. So the entry point has
// to step aside there. Same arrangement as wallpaper_linux.c and terminalfont_linux.c.
const char*
#ifdef __ANDROID__
ffDetectSoundLinux
#else
ffDetectSound
#endif
    (FFSoundOptions* options, FFlist* devices) {
    const char* error = detectPulseAudio(options, devices);
    if (error == nullptr) {
        return nullptr;
    }

#ifdef FF_SOUND_ALSA
    // No server answered. A machine that runs none still has the hardware, and that is what is
    // asked here -- but only ever after the server, never instead of it: on a machine that runs
    // one, the server's answer is the desktop's, and the mixer behind it is not the same number.
    FF_DEBUG("PulseAudio is not answering (%s), so the ALSA cards are read instead", error);

    const uint32_t detected = devices->length;
    const char* alsaError = detectAlsa(options, devices);
    if (alsaError == nullptr || devices->length > detected) {
        // ALSA answered. Its list may still be empty after the filter, which is the same answer
        // the server path gives when nothing matches, and not a failure.
        return nullptr;
    }

    FF_DEBUG("Reading the ALSA cards failed as well: %s", alsaError);
#endif

    return error;
}
