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

/*
* Client: rctSigs.cpp.c -> get_pre_mlsag_hash
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
int electroneum_apdu_mlsag_prehash_init() {
    if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {
        if (G_electroneum_vstate.io_p2 == 1) {
            electroneum_sha256_outkeys_final(NULL);
            electroneum_sha256_outkeys_init();
            electroneum_sha256_commitment_init();
            electroneum_keccak_init_H();
        }
    }
    electroneum_keccak_update_H(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset,
                          G_electroneum_vstate.io_length-G_electroneum_vstate.io_offset);
    if ((G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) &&(G_electroneum_vstate.io_p2==1)) {
        // skip type
        electroneum_io_fetch_u8();
        // fee str
        electroneum_vamount2str(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, G_electroneum_vstate.ux_amount, 15);
         //ask user
        electroneum_io_discard(1);
        ui_menu_fee_validation_display(0);
        return 0;
    } else {
        electroneum_io_discard(1);
        return SW_OK;
    }
}

/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_mlsag_prehash_update() {
    unsigned char is_subaddress;
    unsigned char *Aout;
    unsigned char *Bout;
    unsigned char is_change;
    unsigned char AKout[32];
    unsigned char C[32];
    unsigned char v[32];
    unsigned char k[32];

    #define aH AKout
    unsigned char kG[32];

    //fetch destination
    is_subaddress = electroneum_io_fetch_u8();
    if (G_electroneum_vstate.io_protocol_version == 2) {
        is_change =  electroneum_io_fetch_u8();
    } else {
        is_change = 0;
    }
    Aout = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    Bout = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset; electroneum_io_fetch(NULL,32);
    electroneum_io_fetch_decrypt(AKout,32);
    electroneum_io_fetch(C, 32);
    electroneum_io_fetch(k, 32);
    electroneum_io_fetch(v, 32);

    electroneum_io_discard(0);

    //update MLSAG prehash
    if ((G_electroneum_vstate.options&0x03) == 0x02) {
        electroneum_keccak_update_H(v,8);
    } else {
        electroneum_keccak_update_H(k,32);
        electroneum_keccak_update_H(v,32);
    }

    if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {
        if (is_change == 0) {
            //encode dest adress
            electroneum_base58_public_key(&G_electroneum_vstate.ux_address[0], Aout, Bout, is_subaddress, NULL);
        }
        //update destination hash control
        if (G_electroneum_vstate.io_protocol_version == 2) {
            electroneum_sha256_outkeys_update(Aout,32);
            electroneum_sha256_outkeys_update(Bout,32);
            electroneum_sha256_outkeys_update(&is_change,1);
            electroneum_sha256_outkeys_update(AKout,32);
        }

        //check C = aH+kG
        electroneum_unblind(v,k, AKout, G_electroneum_vstate.options&0x03);
        electroneum_ecmul_G(kG, k);
        if (!cx_math_is_zero(v, 32)) {
            electroneum_ecmul_H(aH, v);
            electroneum_ecadd(aH, kG, aH);
        } else {
            os_memmove(aH, kG, 32);
        }
        if (os_memcmp(C, aH, 32)) {
            THROW(SW_SECURITY_COMMITMENT_CONTROL);
        }
        //update commitment hash control
        electroneum_sha256_commitment_update(C,32);


        if ((G_electroneum_vstate.options & IN_OPTION_MORE_COMMAND)==0) {
            if (G_electroneum_vstate.io_protocol_version == 2) {
                //finalize and check destination hash_control
                electroneum_sha256_outkeys_final(k);
                if (os_memcmp(k, G_electroneum_vstate.OUTK, 32)) {
                    THROW(SW_SECURITY_OUTKEYS_CHAIN_CONTROL);
                }
            }
            //finalize commitment hash control
            electroneum_sha256_commitment_final(NULL);
            electroneum_sha256_commitment_init();
        }

        //ask user
        uint64_t amount;
        amount = electroneum_bamount2uint64(v);
        if (amount) {
            electroneum_amount2str(amount, G_electroneum_vstate.ux_amount, 15);
            if (!is_change) {
                ui_menu_validation_display(0);
            } else  {
                ui_menu_change_validation_display(0);
            }
            return 0;
        }
    }
    return SW_OK;

    #undef aH
}


/* ----------------------------------------------------------------------- */
/* ---                                                                 --- */
/* ----------------------------------------------------------------------- */
int electroneum_apdu_mlsag_prehash_finalize() {
    unsigned char message[32];
    unsigned char proof[32];
    unsigned char H[32];

    if (G_electroneum_vstate.options & IN_OPTION_MORE_COMMAND) {
        //accumulate
        electroneum_io_fetch(H,32);
        electroneum_io_discard(1);
        electroneum_keccak_update_H(H,32);
        electroneum_sha256_commitment_update(H,32);
#ifdef DEBUG_HWDEVICE
        electroneum_io_insert(H, 32);
#endif

    } else {
        //Finalize and check commitment hash control
        if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {
            electroneum_sha256_commitment_final(H);
            if (os_memcmp(H,G_electroneum_vstate.C,32)) {
                THROW(SW_SECURITY_COMMITMENT_CHAIN_CONTROL);
            }
        }
        //compute last H
        electroneum_keccak_final_H(H);
        //compute last prehash
        electroneum_io_fetch(message,32);
        electroneum_io_fetch(proof,32);
        electroneum_io_discard(1);
        electroneum_keccak_init_H();
        electroneum_keccak_update_H(message,32);
        electroneum_keccak_update_H(H,32);
        electroneum_keccak_update_H(proof,32);
        electroneum_keccak_final_H(NULL);
#ifdef DEBUG_HWDEVICE
        electroneum_io_insert(G_electroneum_vstate.H, 32);
        electroneum_io_insert(H, 32);
#endif
    }

    return SW_OK;
}

