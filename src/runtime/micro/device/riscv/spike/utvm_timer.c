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
 * \file utvm_timer.c
 * \brief uTVM timer API stubs for RISC-V Spike
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "utvm_runtime.h"

#define read_csr(reg) ({ unsigned long __tmp; \
		  __asm__ volatile ("csrr %0, " #reg : "=r"(__tmp)); \
		    __tmp; })
#define rdcycle() read_csr(cycle)

static uint64_t start_cycle = 0;
static uint64_t stop_cycle = 0;

int32_t UTVMTimerStart() {
  start_cycle = rdcycle();
  return 0;
}

void UTVMTimerStop() {
  stop_cycle = rdcycle();
}

void UTVMTimerReset() {
  start_cycle = 0;
  stop_cycle = 0;
}

uint32_t UTVMTimerRead() {
  if (start_cycle > stop_cycle) {
	// something happened
	return 0;
  }
  return stop_cycle - start_cycle;
}

#ifdef __cplusplus
}  // TVM_EXTERN_C
#endif
