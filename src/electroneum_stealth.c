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
int electroneum_apdu_stealth() {
    int i ;
    unsigned char pub[32];
    unsigned char sec[32];
    unsigned char drv[33];
    unsigned char payID[8];
    
    //fetch pub
    electroneum_io_fetch(pub,32);
    //fetch sec
    electroneum_io_fetch_decrypt_key(sec);
    //fetch paymentID
    electroneum_io_fetch(payID,8);

    electroneum_io_discard(0);

    //Compute Dout
    electroneum_generate_key_derivation(drv, pub, sec);
    
    //compute mask
    drv[32] = ENCRYPTED_PAYMENT_ID_TAIL;
    electroneum_keccak_F(drv,33,sec);
    
    //stealth!
    for (i=0; i<8; i++) {
        payID[i] = payID[i] ^ sec[i];
    }
    
    electroneum_io_insert(payID,8);

    return SW_OK;
}