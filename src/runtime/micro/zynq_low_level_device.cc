/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file zynq_low_level_device.cc
 */
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <climits>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "micro_common.h"
#include "low_level_device.h"
#include "zynq_packet.h"

namespace tvm {
namespace runtime {

/*!
 * \brief Zynq low-level device for uTVM micro devices in Zynq PL
 */
class ZynqLowLevelDevice final : public LowLevelDevice {
 public:
  /*!
   * \brief constructor to initialize connection to zynq device
   * \param server_addr address of the zynq server to connect to
   * \param port port of the zynq server to connect to
   */
  explicit ZynqLowLevelDevice(const std::string& server_addr,
                                 int port) {
    server_addr_ = server_addr;
    port_ = port;

	/* dial the socket to the server */
	if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		goto fail;
	}

	struct sockaddr_in serv_addr; 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, server_addr.c_str(), &serv_addr.sin_addr) <= 0) {
		perror("inet_pton");
		goto fail;
	}

	if (connect(sock_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect");
		goto fail;
	}

	return;

fail:
	/* TODO(jsteward): how do we handle failure? */
	fprintf(stderr, "%s failed\n", __func__);
  }

 private:
    ssize_t read_sock(void *dest, size_t len) {
		size_t bytes_read, total_read = 0;
		while (total_read < len) {
			bytes_read =
				read(sock_, (uint8_t*)dest + total_read, len - total_read);
			if (bytes_read < 0) {
				perror("read");
				return -1;
			} else if (bytes_read == 0) {
				// short read
				break;
			}
			total_read += bytes_read;
		}
		return 0;
	}

	ssize_t write_sock(const void *buf, size_t len) {
		size_t bytes_sent, total_sent = 0;
		while (total_sent < len) {
			bytes_sent = send(sock_, (uint8_t*)buf + total_sent,
					len - total_sent, MSG_NOSIGNAL);
			if (bytes_sent < 0) {
				perror("send");
				return -1;
			} else if (bytes_sent == 0) {
				// short write
				break;
			}
			total_sent += bytes_sent;
		}
		return 0;
	}
  
 public:
  void Read(DevPtr addr, void* buf, size_t num_bytes) {
	zynq_packet_t packet;
	packet.type = zynq_packet_t::READ;
	packet.rw.addr = addr.cast_to<uint64_t>();
	packet.rw.len = num_bytes;
	CHECK_EQ(write_sock((void*)&packet, sizeof(packet)), 0);
	CHECK_EQ(read_sock(buf, num_bytes), 0);
  }

  void Write(DevPtr addr, const void* buf, size_t num_bytes) {
	zynq_packet_t packet;
	packet.type = zynq_packet_t::WRITE;
	packet.rw.addr = addr.cast_to<uint64_t>();
	packet.rw.len = num_bytes;
	CHECK_EQ(write_sock((void*)&packet, sizeof(packet)), 0);
	CHECK_EQ(write_sock(buf, num_bytes), 0);

	uint32_t ack;
	CHECK_EQ(read_sock((void*)&ack, sizeof(ack)), 0);
	CHECK_EQ(ack, ACK);
  }

  void Execute(DevPtr func_addr, DevPtr breakpoint_addr) {
	zynq_packet_t packet;
	packet.type = zynq_packet_t::EXECUTE;
	packet.exec.addr = func_addr.cast_to<uint64_t>();
	packet.exec.stop = breakpoint_addr.cast_to<uint64_t>();
	CHECK_EQ(write_sock((void*)&packet, sizeof(packet)), 0);

	uint32_t ack;
	CHECK_EQ(read_sock((void*)&ack, sizeof(ack)), 0);
	CHECK_EQ(ack, ACK);
  }

  const char* device_type() const final {
    return "zynq";
  }

 private:
  /*! \brief socket used to communicate with the device */
  int sock_;
  /*! \brief address of Zynq server */
  std::string server_addr_;
  /*! \brief port of Zynq server */
  int port_;

  /*! \brief number of bytes in a word on the target device (64-bit) */
  static const constexpr ssize_t kWordSize = 8;
  /* NOTE: this is an artificial limit and does not exist for Zynq servers */
  /*! \brief maximum number of bytes allowed in a single memory transfer */
  static const constexpr ssize_t kMemTransferLimit = SSIZE_MAX;
  /*! \brief number of milliseconds to wait for function execution to halt */
  static const constexpr int kWaitTime = 10000;
};

const std::shared_ptr<LowLevelDevice> ZynqLowLevelDeviceCreate(const std::string& server_addr,
                                                                  int port) {
  std::shared_ptr<LowLevelDevice> lld =
      std::make_shared<ZynqLowLevelDevice>(server_addr, port);
  return lld;
}

}  // namespace runtime
}  // namespace tvm
