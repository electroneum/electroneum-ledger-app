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
#include "electroneum_vars.h"


/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
void electroneum_reset_tx() {
    os_memset(G_electroneum_vstate.r, 0, 32);
    os_memset(G_electroneum_vstate.R, 0, 32);
    cx_rng(G_electroneum_vstate.hmac_key, 32);

    electroneum_keccak_init_H();
    electroneum_sha256_commitment_init();
    electroneum_sha256_outkeys_init();
    G_electroneum_vstate.tx_in_progress = 0;
    G_electroneum_vstate.tx_output_cnt = 0;

 }


/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
/*
 * HD wallet not yet supported : account is assumed to be zero
 */
int electroneum_apdu_open_tx() {

    unsigned int account;

    account = electroneum_io_fetch_u32();

    electroneum_io_discard(1);

    electroneum_reset_tx();
    G_electroneum_vstate.tx_in_progress = 1;

    #ifdef DEBUG_HWDEVICE
    os_memset(G_electroneum_vstate.hmac_key, 0xab, 32);
    #else
    cx_rng(G_electroneum_vstate.hmac_key, 32);
    #endif

    electroneum_rng_mod_order(G_electroneum_vstate.r);
    electroneum_ecmul_G(G_electroneum_vstate.R, G_electroneum_vstate.r);

    electroneum_io_insert(G_electroneum_vstate.R,32);
    electroneum_io_insert_encrypt(G_electroneum_vstate.r,32);
    electroneum_io_insert(C_FAKE_SEC_VIEW_KEY,32);
    electroneum_io_insert_hmac_for((void*)C_FAKE_SEC_VIEW_KEY,32);
    electroneum_io_insert(C_FAKE_SEC_SPEND_KEY,32);
    electroneum_io_insert_hmac_for((void*)C_FAKE_SEC_SPEND_KEY,32);
    return SW_OK;
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_close_tx() {
   electroneum_io_discard(1);
   electroneum_reset_tx();
   G_electroneum_vstate.tx_in_progress = 0;
   return SW_OK;
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
/*
 * Sub dest address not yet supported: P1 = 2 not supported
 */
int electroneum_abort_tx() {
    electroneum_reset_tx();
    return 0;
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
/*
 * Sub dest address not yet supported: P1 = 2 not supported
 */
int electroneum_apdu_set_signature_mode() {
    unsigned int sig_mode;

    G_electroneum_vstate.sig_mode = TRANSACTION_CREATE_FAKE;

    sig_mode = electroneum_io_fetch_u8();
    electroneum_io_discard(0);
    switch(sig_mode) {
    case TRANSACTION_CREATE_REAL:
    case TRANSACTION_CREATE_FAKE:
        break;
    default:
        THROW(SW_WRONG_DATA);
    }
    G_electroneum_vstate.sig_mode = sig_mode;

    electroneum_io_insert_u32( G_electroneum_vstate.sig_mode );
    return SW_OK;
}
