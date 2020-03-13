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

int electroneum_apdu_tx_prompt_amount() {
    
    if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {
        uint64_t total = 0;

        if(G_electroneum_vstate.tx_total_amount > 0) {
            total = G_electroneum_vstate.tx_total_amount;

            electroneum_base58_public_key(&G_electroneum_vstate.ux_address[0], G_electroneum_vstate.dest_Aout, G_electroneum_vstate.dest_Bout, G_electroneum_vstate.dest_is_subaddress, NULL);
            electroneum_amount2str(total, G_electroneum_vstate.ux_amount, 15);

            electroneum_io_discard(1);
            ui_menu_validation_display(0);

        } else { //loopback tx
            total = G_electroneum_vstate.tx_outs_amount;
            electroneum_amount2str(total, G_electroneum_vstate.ux_amount, 15);
            electroneum_amount2str(G_electroneum_vstate.tx_ins_count*100, G_electroneum_vstate.ux_inputs, 15);

            electroneum_io_discard(1);
            ui_menu_validation_loopback_display(0);
        }

        return 0;
        

    }
    electroneum_io_discard(1);
    return SW_OK;
}

int electroneum_apdu_tx_prompt_fee() {
    if (G_electroneum_vstate.sig_mode == TRANSACTION_CREATE_REAL) {

        G_electroneum_vstate.tx_fee = G_electroneum_vstate.tx_ins_amount-G_electroneum_vstate.tx_outs_amount;
        electroneum_amount2str(G_electroneum_vstate.tx_fee, G_electroneum_vstate.ux_amount, 15);

        electroneum_io_discard(1);
        ui_menu_fee_validation_display(0);
        return 0;
    }
    electroneum_io_discard(1);
    return SW_OK;
}

void add_to_tx_prefix(uint64_t num) {
    unsigned char varint[10] = {0};
    unsigned int size = electroneum_encode_varint(varint, num);
    electroneum_keccak_update_H(varint, size);

    electroneum_io_insert(varint, size);
}

int electroneum_apdu_tx_prefix_start() {
    unsigned int version = electroneum_io_fetch_u32();
    unsigned int unlock_time = electroneum_io_fetch_u32();
    unsigned int vins_size = electroneum_io_fetch_u32();

    electroneum_io_discard(1);

    G_electroneum_vstate.tx_ins_count = 0;
    G_electroneum_vstate.tx_ins_amount = 0;
    G_electroneum_vstate.tx_outs_amount = 0;
    G_electroneum_vstate.tx_fee = 0;
    G_electroneum_vstate.tx_outs_current_index = 0;
    G_electroneum_vstate.tx_total_amount = 0;

    electroneum_keccak_init_H();

    add_to_tx_prefix(version);
    add_to_tx_prefix(unlock_time);
    add_to_tx_prefix(vins_size);
    
    return SW_OK;
}

int electroneum_apdu_tx_prefix_inputs() {
    uint64_t amount = electroneum_io_fetch_u64();

    //unsigned char buffer[sizeof(amount)];
    //memcpy(buffer, &amount, sizeof(amount));
    //PRINTF("64 Bit Amount:\n %.*H \n\n", sizeof(buffer), buffer);

    unsigned int key_offset = electroneum_io_fetch_u32();
    unsigned char k_image[32];
    
    electroneum_io_fetch(k_image,32);

    electroneum_io_discard(0);

    G_electroneum_vstate.tx_ins_amount += amount;
    G_electroneum_vstate.tx_ins_count++;

    add_to_tx_prefix(2);
    add_to_tx_prefix(amount);
    add_to_tx_prefix(1);
    add_to_tx_prefix(key_offset);

    electroneum_keccak_update_H(k_image, 32);
    electroneum_io_insert(k_image, 32);

    return SW_OK;
}

int electroneum_apdu_tx_prefix_outputs() {
    uint64_t amount = electroneum_io_fetch_u64();
    unsigned char key[32];
    electroneum_io_fetch(key,32);
    unsigned int output_index = electroneum_io_fetch_u32();

    electroneum_io_discard(0);

    //read tx_change_idx bit-by-bit
    unsigned int memblock = G_electroneum_vstate.tx_outs_current_index / 8;
    uint8_t shift = G_electroneum_vstate.tx_outs_current_index == 0 ? 0 : (G_electroneum_vstate.tx_outs_current_index - 8*memblock) % 8;

    bool is_change = G_electroneum_vstate.tx_change_idx[memblock] & (1 << shift);

    // To ensure the outputs are really going to their intended destination,
    // we need to check that the stealth addresses correspond to the destination A&B. So recompute P=H(rA)G+B.
    unsigned char drvpub[32]; // stealth address P to be computed
    unsigned char derivation[32]; // r*A
    if(!is_change) {
        electroneum_generate_key_derivation(derivation, G_electroneum_vstate.dest_Aout, G_electroneum_vstate.r);
        electroneum_derive_public_key(drvpub, derivation, output_index, G_electroneum_vstate.dest_Bout); // H(rA)+B
        if(!(memcmp(drvpub, key, sizeof(drvpub)))){
            //reset indexes
            os_memset(G_electroneum_vstate.tx_change_idx, 0, 50);
            return false;
        }
        G_electroneum_vstate.tx_total_amount += amount;
    }
    else{
        electroneum_generate_key_derivation(derivation, G_electroneum_vstate.A, G_electroneum_vstate.r);
        electroneum_derive_public_key(drvpub, derivation, output_index, G_electroneum_vstate.B);
        if(!(memcmp(drvpub, key, sizeof(drvpub)))){
            //reset indexes
            os_memset(G_electroneum_vstate.tx_change_idx, 0, 50);
            return false;
        }
    }
    G_electroneum_vstate.tx_outs_amount += amount;
    G_electroneum_vstate.tx_outs_current_index++;

    add_to_tx_prefix(amount);
    add_to_tx_prefix(2);

    electroneum_keccak_update_H(key, 32);
    electroneum_io_insert(key, 32);

    return SW_OK;
}

int electroneum_apdu_tx_prefix_outputs_size() {
    unsigned int vins_size = electroneum_io_fetch_u32();

    electroneum_io_discard(0);

    add_to_tx_prefix(vins_size);

    return SW_OK;
}

int electroneum_apdu_tx_prefix_extra() {
    unsigned int extra_size = electroneum_io_fetch_u32();
    unsigned char* extra = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset;
    electroneum_io_fetch(NULL,extra_size);

    electroneum_io_discard(0);

    add_to_tx_prefix(extra_size);

    electroneum_keccak_update_H(extra, extra_size);
    electroneum_io_insert(extra, extra_size);

    unsigned char  h[32];
    electroneum_keccak_final_H(h);

    os_memcpy(G_electroneum_vstate.tx_prefix_hash, h, 32);

    //reset indexes
    os_memset(G_electroneum_vstate.tx_change_idx, 0, 50);

    return SW_OK;
}

