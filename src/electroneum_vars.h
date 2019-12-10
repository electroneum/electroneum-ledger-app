/* Copyright 2017 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef electroneum_VARS_H
#define electroneum_VARS_H

#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "electroneum_types.h"
#include "electroneum_api.h"


extern electroneum_v_state_t  G_electroneum_vstate;

#ifdef TARGET_NANOX
extern const electroneum_nv_state_t N_state_pic;
#define N_electroneum_pstate  ((volatile  electroneum_nv_state_t *)PIC(&N_state_pic))
#else
extern electroneum_nv_state_t N_state_pic;
#define N_electroneum_pstate  ((WIDE  electroneum_nv_state_t *)PIC(&N_state_pic))
#endif

#ifdef electroneum_DEBUG_MAIN
extern int apdu_n;
#endif

extern ux_state_t ux;
#endif
