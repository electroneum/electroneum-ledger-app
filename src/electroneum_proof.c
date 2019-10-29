/* Copyright 2019 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS
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
#include "electroneum_vars.h"

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
/*
 * pick random k
 * if B:
 *   compute X = k*B
 * else:
 *   compute X = k*G
 * compute Y = k*A
 * sig.c = Hs(Msg || D || X || Y)
 * sig.r = k - sig.c*r
 */
int electroneum_apdu_get_tx_proof() {
    unsigned char *msg;
    unsigned char *R;
    unsigned char *A;
    unsigned char *B;
    unsigned char *D;
    unsigned char r[32];
    unsigned char XY[32];
    unsigned char sig_c[32];
    unsigned char sig_r[32];
    #define k (G_electroneum_vstate.tmp+256)

    msg = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    R = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    A = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    B = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    D = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    electroneum_io_fetch_decrypt_key(r);

    electroneum_io_discard(0);

    electroneum_rng(k,32);
    electroneum_reduce(k,k);
    os_memmove(G_electroneum_vstate.tmp+32*0, msg, 32);
    os_memmove(G_electroneum_vstate.tmp+32*1, D, 32);

    if(G_electroneum_vstate.options&1) {
        electroneum_ecmul_k(XY,B,k);
    } else {
        electroneum_ecmul_G(XY,k);
    }
    os_memmove(G_electroneum_vstate.tmp+32*2,  XY, 32);

    electroneum_ecmul_k(XY,A,k);
    os_memmove(G_electroneum_vstate.tmp+32*3, XY, 32);

    electroneum_hash_to_scalar(sig_c, &G_electroneum_vstate.tmp[0],32*4);

    electroneum_multm(XY, sig_c, r);
    electroneum_subm(sig_r, k, XY);

    electroneum_io_insert(sig_c, 32);
    electroneum_io_insert(sig_r, 32);

    return SW_OK;
}
