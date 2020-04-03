#ifndef TVM_RUNTIME_MICRO_PACKET_H_
#define TVM_RUNTIME_MICRO_PACKET_H_

#include <stdint.h>
#include <stddef.h>

typedef struct zynq_packet {
    enum {
        READ,
        WRITE,
        EXECUTE,
    } type;
    union {
        struct {
            uint64_t addr;
            uint64_t len;
        } rw;
        struct {
            uint64_t addr;
            uint64_t stop;
        } exec;
    };
} zynq_packet_t;

// random number
#define ACK 0x4c3f2baf

#endif // TVM_RUNTIME_MICRO_PACKET_H_
