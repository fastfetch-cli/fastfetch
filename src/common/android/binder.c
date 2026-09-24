#include "common/android/binder.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

const char* ffBinderOpen(FFBinder* binder) {
    *binder = (FFBinder) { .fd = -1, .shared = nullptr, .sharedSize = 0, .protocolVersion = 0 };

    const int fd = open(FF_BINDER_DEVICE, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        return "Failed to open " FF_BINDER_DEVICE;
    }

    struct binder_version version = { .protocol_version = 0 };
    if (ioctl(fd, BINDER_VERSION, &version) != 0 || version.protocol_version != BINDER_CURRENT_PROTOCOL_VERSION) {
        close(fd);
        return "Unsupported binder protocol version";
    }

    // The region is read-only: the kernel writes reply payloads into it and hands out their
    // addresses in BR_REPLY, so it has to stay mapped for as long as the handle is used.
    void* shared = mmap(nullptr, FF_BINDER_SHARED_SIZE, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
    if (shared == MAP_FAILED) {
        close(fd);
        return "Failed to mmap " FF_BINDER_DEVICE;
    }

    *binder = (FFBinder) {
        .fd = fd,
        .shared = (uint8_t*) shared,
        .sharedSize = FF_BINDER_SHARED_SIZE,
        .protocolVersion = version.protocol_version,
    };
    return nullptr;
}

void ffBinderClose(FFBinder* binder) {
    if (binder->shared != nullptr) {
        munmap(binder->shared, binder->sharedSize);
    }
    if (binder->fd >= 0) {
        close(binder->fd);
    }
    binder->fd = -1;
    binder->shared = nullptr;
    binder->sharedSize = 0;
}

const char* ffBinderTransact(FFBinder* binder, uint32_t handle, uint32_t code, uint32_t flags, const FFBinderParcel* parcel, FFBinderReply* reply) {
    reply->size = 0;
    reply->code = 0;
    reply->flags = 0;
    reply->handleCount = 0;
    reply->fdCount = 0;

    if (parcel->truncated) {
        return "Binder parcel does not fit the caller buffer";
    }

    uint8_t writeBuffer[sizeof(uint32_t) + sizeof(struct binder_transaction_data)];
    const struct binder_transaction_data transaction = {
        .target = { .handle = handle },
        .cookie = 0,
        .code = code,
        .flags = flags,
        .sender_pid = 0,
        .sender_euid = 0,
        .data_size = parcel->size,
        .offsets_size = 0,
        .data = { .ptr = { .buffer = (binder_uintptr_t) (uintptr_t) parcel->data, .offsets = 0 } },
    };
    const uint32_t transactionCommand = BC_TRANSACTION;
    memcpy(writeBuffer, &transactionCommand, sizeof(uint32_t));
    memcpy(writeBuffer + sizeof(uint32_t), &transaction, sizeof(transaction));

    // The read buffer only has to hold the command words and the binder_transaction_data that
    // follows BR_REPLY; the payload itself lives in the mmap region.
    uint8_t readBuffer[FF_BINDER_REPLY_BUFFER_SIZE];
    struct binder_write_read exchange = {
        .write_size = sizeof(writeBuffer),
        .write_consumed = 0,
        .write_buffer = (binder_uintptr_t) (uintptr_t) writeBuffer,
        .read_size = sizeof(readBuffer),
        .read_consumed = 0,
        .read_buffer = (binder_uintptr_t) (uintptr_t) readBuffer,
    };
    if (ioctl(binder->fd, BINDER_WRITE_READ, &exchange) != 0) {
        return "BINDER_WRITE_READ failed";
    }

    const uint8_t* cursor = readBuffer;
    const uint8_t* end = readBuffer + (size_t) exchange.read_consumed;
    while ((size_t) (end - cursor) >= sizeof(uint32_t)) {
        uint32_t command = 0;
        memcpy(&command, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        switch (command) {
            case BR_NOOP:
            case BR_TRANSACTION_COMPLETE:
            case BR_SPAWN_LOOPER:
                break;

            case BR_FAILED_REPLY:
                return "Binder transaction failed";
            case BR_DEAD_REPLY:
                return "Binder service is not running";

            case BR_INCREFS:
            case BR_ACQUIRE:
            case BR_RELEASE:
            case BR_DECREFS:
                cursor += sizeof(struct binder_ptr_cookie);
                break;
            case BR_DEAD_BINDER:
            case BR_CLEAR_DEATH_NOTIFICATION_DONE:
                cursor += sizeof(binder_uintptr_t);
                break;
            case BR_ERROR:
                cursor += sizeof(int32_t);
                break;

            case BR_REPLY: {
                if ((size_t) (end - cursor) < sizeof(struct binder_transaction_data)) {
                    return "Truncated binder reply";
                }
                struct binder_transaction_data data;
                memcpy(&data, cursor, sizeof(data));
                cursor += sizeof(data);

                reply->code = data.code;
                reply->flags = data.flags;

                const uint8_t* payload = (const uint8_t*) (uintptr_t) data.data.ptr.buffer;
                const size_t payloadSize = (size_t) data.data_size;
                const uint8_t* offsets = (const uint8_t*) (uintptr_t) data.data.ptr.offsets;
                const size_t offsetsSize = (size_t) data.offsets_size;

                const char* error = nullptr;
                if (payloadSize > 0) {
                    if (payloadSize > reply->capacity) {
                        error = "Binder reply does not fit the caller buffer";
                    } else {
                        memcpy(reply->data, payload, payloadSize);
                        reply->size = payloadSize;
                    }
                }

                // Walk the offset table for the flat objects the service sent us. A reply may carry
                // both kinds at once and they are not interchangeable: a handle is a reference that
                // has to be acquired (see below) to outlive the reply buffer, while a file
                // descriptor was installed in our own fd table by the kernel and is simply ours.
                for (size_t offset = 0; offset + sizeof(binder_size_t) <= offsetsSize; offset += sizeof(binder_size_t)) {
                    binder_size_t objectOffset = 0;
                    memcpy(&objectOffset, offsets + offset, sizeof(objectOffset));
                    if (objectOffset + sizeof(struct flat_binder_object) > payloadSize) {
                        continue;
                    }
                    struct flat_binder_object object;
                    memcpy(&object, payload + objectOffset, sizeof(object));
                    if (object.hdr.type == BINDER_TYPE_HANDLE) {
                        if (reply->handleCount < FF_BINDER_MAX_HANDLES) {
                            reply->handles[reply->handleCount++] = object.handle;
                        }
                    } else if (object.hdr.type == BINDER_TYPE_FD) {
                        if (reply->fdCount < FF_BINDER_MAX_FDS) {
                            reply->fds[reply->fdCount++] = (int32_t) object.handle;
                        } else {
                            // Nothing is going to hold this one and nothing is going to close it.
                            close((int) object.handle);
                        }
                    }
                }

                // The kernel gave us a new reference whose only strong reference belongs to the
                // reply buffer, so it has to be acquired before that buffer is freed -- otherwise
                // the handle silently degrades to a weak reference and every later transaction on
                // it fails with BR_FAILED_REPLY. libbinder does the same in acquire_object().
                uint8_t tail[FF_BINDER_MAX_HANDLES * 4 * sizeof(uint32_t) + sizeof(uint32_t) + sizeof(binder_uintptr_t)];
                size_t tailSize = 0;
                for (uint32_t i = 0; i < reply->handleCount; i++) {
                    const uint32_t acquire = BC_ACQUIRE;
                    const uint32_t increfs = BC_INCREFS;
                    memcpy(tail + tailSize, &acquire, sizeof(uint32_t));
                    tailSize += sizeof(uint32_t);
                    memcpy(tail + tailSize, &reply->handles[i], sizeof(uint32_t));
                    tailSize += sizeof(uint32_t);
                    memcpy(tail + tailSize, &increfs, sizeof(uint32_t));
                    tailSize += sizeof(uint32_t);
                    memcpy(tail + tailSize, &reply->handles[i], sizeof(uint32_t));
                    tailSize += sizeof(uint32_t);
                }
                const uint32_t freeCommand = BC_FREE_BUFFER;
                const binder_uintptr_t freeBuffer = data.data.ptr.buffer;
                memcpy(tail + tailSize, &freeCommand, sizeof(uint32_t));
                tailSize += sizeof(uint32_t);
                memcpy(tail + tailSize, &freeBuffer, sizeof(freeBuffer));
                tailSize += sizeof(freeBuffer);

                struct binder_write_read release = {
                    .write_size = tailSize,
                    .write_consumed = 0,
                    .write_buffer = (binder_uintptr_t) (uintptr_t) tail,
                    .read_size = 0,
                    .read_consumed = 0,
                    .read_buffer = 0,
                };
                // This one ioctl carries the whole tail, so its failure is not cosmetic: without
                // BC_ACQUIRE the handles handed to the caller are only weakly referenced and every
                // later transaction on them fails with BR_FAILED_REPLY, which says nothing about the
                // real cause. Reported, but only when nothing more specific is already at hand.
                if (ioctl(binder->fd, BINDER_WRITE_READ, &release) != 0 && error == nullptr) {
                    error = "Releasing the binder reply failed";
                }

                return error;
            }

            default:
                return "Unexpected binder command";
        }
    }

    return "No reply from binder";
}

const char* ffBinderLookupService(FFBinder* binder, const char* name, uint32_t transactionCode, uint32_t* handle) {
    uint8_t parcelBuffer[256];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_BINDER_SM_DESCRIPTOR);
    ffBinderParcelPutString16(&parcel, name);

    uint8_t replyBuffer[256];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, FF_BINDER_SERVICE_MANAGER_HANDLE, transactionCode, 0, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        return "Service manager rejected the request";
    }
    if (ffBinderReadI32(reply.data, reply.size, 0) != 0) {
        return "Service manager raised an exception";
    }
    if (reply.handleCount == 0) {
        return "Service is not registered";
    }

    *handle = reply.handles[0];
    return nullptr;
}

void ffBinderServiceHandleRelease(FFBinderServiceHandle* service) {
    if (service->binder == nullptr || service->handle == 0) {
        return;
    }

    // The mirror of the acquire at the end of ffBinderTransact(): one strong and one weak reference
    // each, dropped in the same order they were taken. The kernel keeps the service node alive until
    // the last one goes, so this is what stops a short-lived fastfetch from holding one for the whole
    // of its life.
    const uint32_t commands[] = { BC_RELEASE, BC_DECREFS };
    uint8_t tail[2 * 2 * sizeof(uint32_t)];
    size_t tailSize = 0;
    for (size_t i = 0; i < 2; i++) {
        memcpy(tail + tailSize, &commands[i], sizeof(uint32_t));
        tailSize += sizeof(uint32_t);
        memcpy(tail + tailSize, &service->handle, sizeof(uint32_t));
        tailSize += sizeof(uint32_t);
    }

    struct binder_write_read release = {
        .write_size = tailSize,
        .write_consumed = 0,
        .write_buffer = (binder_uintptr_t) (uintptr_t) tail,
        .read_size = 0,
        .read_consumed = 0,
        .read_buffer = 0,
    };
    // Not reported: this runs from a cleanup attribute, where there is no caller left to report to,
    // and the only way for it to fail is a binder that is already dead -- which every earlier call on
    // the same binder would have said already.
    ioctl(service->binder->fd, BINDER_WRITE_READ, &release);
    service->handle = 0;
}
