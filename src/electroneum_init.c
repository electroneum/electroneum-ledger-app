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


/* ----------------------*/
/* -- A Kind of Magic -- */
/* ----------------------*/
const unsigned char C_MAGIC[8] = {
 'M','O','N','E','R','O','H','W'
};

/* ----------------------------------------------------------------------- */
/* --- Boot                                                            --- */
/* ----------------------------------------------------------------------- */
void electroneum_init() {
  os_memset(&G_electroneum_vstate, 0, sizeof(electroneum_v_state_t));

  //first init ?
  if (os_memcmp(N_electroneum_pstate->magic, (void*)C_MAGIC, sizeof(C_MAGIC)) != 0) {
    electroneum_install(MAINNET);
  }

  G_electroneum_vstate.protocol = 0xff;

  //load key
  electroneum_init_private_key();
  //ux conf
  electroneum_init_ux();
  // Let's go!
  G_electroneum_vstate.state = STATE_IDLE;
}


/* ----------------------------------------------------------------------- */
/* --- init private keys                                               --- */
/* ----------------------------------------------------------------------- */
void electroneum_wipe_private_key() {
 os_memset(G_electroneum_vstate.a,  0, 32);
 os_memset(G_electroneum_vstate.b,  0, 32);
 os_memset(G_electroneum_vstate.A,  0, 32);
 os_memset(G_electroneum_vstate.B,  0, 32);
 os_memset(&G_electroneum_vstate.spk, 0, sizeof(G_electroneum_vstate.spk));
 G_electroneum_vstate.key_set = 0;
}

void electroneum_init_private_key() {
  unsigned int  path[5];
  unsigned char seed[32];
  unsigned char chain[32];

  //generate account keys

  // m / purpose' / coin_type' / account' / change / address_index
  // m / 44'      / 128'       / 0'       / 0      / 0
  path[0] = 0x8000002C;
  path[1] = 0x80000080;
  path[2] = 0x80000000;
  path[3] = 0x00000000;
  path[4] = 0x00000000;
  os_perso_derive_node_bip32(CX_CURVE_SECP256K1, path, 5 , seed, chain);

  switch(N_electroneum_pstate->key_mode) {
  case KEY_MODE_SEED:

    electroneum_keccak_F(seed,32,G_electroneum_vstate.b);
    electroneum_reduce(G_electroneum_vstate.b,G_electroneum_vstate.b);
    electroneum_keccak_F(G_electroneum_vstate.b,32,G_electroneum_vstate.a);
    electroneum_reduce(G_electroneum_vstate.a,G_electroneum_vstate.a);
    break;

  case KEY_MODE_EXTERNAL:
    os_memmove(G_electroneum_vstate.a,  N_electroneum_pstate->a, 32);
    os_memmove(G_electroneum_vstate.b,  N_electroneum_pstate->b, 32);
    break;

  default :
    THROW(SW_SECURITY_LOAD_KEY);
    return;
  }

  electroneum_ecmul_G(G_electroneum_vstate.A, G_electroneum_vstate.a);
  electroneum_ecmul_G(G_electroneum_vstate.B, G_electroneum_vstate.b);

  //generate key protection
  electroneum_aes_derive(&G_electroneum_vstate.spk,chain,G_electroneum_vstate.a,G_electroneum_vstate.b);


  G_electroneum_vstate.key_set = 1;
}

/* ----------------------------------------------------------------------- */
/* ---  Set up ui/ux                                                   --- */
/* ----------------------------------------------------------------------- */
void electroneum_init_ux() {
}

/* ----------------------------------------------------------------------- */
/* ---  Install/ReInstall electroneum app                                   --- */
/* ----------------------------------------------------------------------- */
void electroneum_install(unsigned char netId) {
  unsigned char c;

  //full reset data
  electroneum_nvm_write(N_electroneum_pstate, NULL, sizeof(electroneum_nv_state_t));

  //set mode key
  c = KEY_MODE_SEED;
  nvm_write(&N_electroneum_pstate->key_mode, &c, 1);

  //set net id
  electroneum_nvm_write(&N_electroneum_pstate->network_id, &netId, 1);

  //write magic
  electroneum_nvm_write(N_electroneum_pstate->magic, (void*)C_MAGIC, sizeof(C_MAGIC));
}

/* ----------------------------------------------------------------------- */
/* --- Reset                                                           --- */
/* ----------------------------------------------------------------------- */
#define ELECTRONEUM_SUPPORTED_CLIENT_SIZE 1
const char * const electroneum_supported_client[ELECTRONEUM_SUPPORTED_CLIENT_SIZE] = {
  "3.0.0.0"
};

int electroneum_apdu_reset() {

  unsigned int client_version_len;
  char client_version[10];
  client_version_len = G_electroneum_vstate.io_length - G_electroneum_vstate.io_offset;
  if (client_version_len > 10) {
    THROW(SW_CLIENT_NOT_SUPPORTED+1);
  }
  electroneum_io_fetch((unsigned char*)&client_version[0], client_version_len);

  unsigned int i = 0;
  while(i < ELECTRONEUM_SUPPORTED_CLIENT_SIZE) {
    if ((strlen((char*)PIC(electroneum_supported_client[i])) == client_version_len) &&
        (os_memcmp((char*)PIC(electroneum_supported_client[i]), client_version, client_version_len)==0) ) {
      break;
    }
    i++;
  }
  if (i == ELECTRONEUM_SUPPORTED_CLIENT_SIZE) {
    THROW(SW_CLIENT_NOT_SUPPORTED);
  }

  electroneum_io_discard(0);
  electroneum_init();
  electroneum_io_insert_u8(electroneum_VERSION_MAJOR);
  electroneum_io_insert_u8(electroneum_VERSION_MINOR);
  electroneum_io_insert_u8(electroneum_VERSION_MICRO);
  return 0x9000;
}
