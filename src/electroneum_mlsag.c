// Copyright (c) Electroneum Limited 2019-2020
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

#include "os.h"
#include "cx.h"
#include "electroneum_types.h"
#include "electroneum_api.h"
#include "electroneum_ux_nanos.h"
#include "electroneum_vars.h"

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_mlsag_prepare() {
    int options;
    unsigned char Hi[32];
    unsigned char xin[32];
    unsigned char alpha[32];
    unsigned char mul[32];


    if (G_electroneum_vstate.io_length>1) {
        electroneum_io_fetch(Hi,32);
        if(G_electroneum_vstate.options &0x40) {
            electroneum_io_fetch(xin,32);
        } else {
           electroneum_io_fetch_decrypt(xin,32);
        }
        options = 1;
    }  else {
        options = 0;
    }

    electroneum_io_discard(1);

    //ai
    electroneum_rng_mod_order(alpha);
    electroneum_reduce(alpha, alpha);
    electroneum_io_insert_encrypt(alpha, 32);

    //ai.G
    electroneum_ecmul_G(mul, alpha);
    electroneum_io_insert(mul,32);

    if (options) {
        //ai.Hi
        electroneum_ecmul_k(mul, Hi, alpha);
        electroneum_io_insert(mul,32);
        //IIi = xin.Hi
        electroneum_ecmul_k(mul, Hi, xin);
        electroneum_io_insert(mul,32);
    }

    return SW_OK;
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_mlsag_hash() {
    unsigned char msg[32];
    unsigned char c[32];
    if (G_electroneum_vstate.io_p2 == 1) {
        electroneum_keccak_init_H();
        os_memmove(msg, G_electroneum_vstate.H, 32);
    } else {
        electroneum_io_fetch(msg, 32);
    }
    electroneum_io_discard(1);

    electroneum_keccak_update_H(msg, 32);
    if ((G_electroneum_vstate.options&0x80) == 0 ) {
        electroneum_keccak_final_H(c);
        electroneum_reduce(c,c);
        electroneum_io_insert(c,32);
        os_memmove(G_electroneum_vstate.c, c, 32);
    }
    return SW_OK;
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_mlsag_sign() {
    unsigned char xin[32];
    unsigned char alpha[32];
    unsigned char ss[32];
    unsigned char ss2[32];

    if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_FAKE) {
        electroneum_io_fetch(xin,32);
        electroneum_io_fetch(alpha,32);
    } else if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {
        electroneum_io_fetch_decrypt(xin,32);
        electroneum_io_fetch_decrypt(alpha,32);
    } else {
        THROW(SW_WRONG_DATA);
    }
    electroneum_io_discard(1);


    electroneum_reduce(ss, G_electroneum_vstate.c);
    electroneum_reduce(xin,xin);
    electroneum_multm(ss, ss, xin);

    electroneum_reduce(alpha, alpha);
    electroneum_subm(ss2, alpha, ss);

    electroneum_io_insert(ss2,32);
    electroneum_io_insert_u32(G_electroneum_vstate.sig_mode);
    return SW_OK;
}
