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

#if defined(IODUMMYCRYPT)
  #warning IODUMMYCRYPT activated
#endif
#if defined(IONOCRYPT)
  #warning IONOCRYPT activated
#endif


/*
 * io_buff: contains current message part
 * io_off: offset in current message part
 * io_length: length of current message part
 */



/* ----------------------------------------------------------------------- */
/* MISC                                                                    */
/* ----------------------------------------------------------------------- */
void electroneum_io_set_offset(unsigned int offset) {
  if (offset == IO_OFFSET_END) {
    G_electroneum_vstate.io_offset = G_electroneum_vstate.io_length;
  }
  else if (offset == IO_OFFSET_MARK) {
    G_electroneum_vstate.io_offset = G_electroneum_vstate.io_mark;
  }
  else if (offset < G_electroneum_vstate.io_length) {
    G_electroneum_vstate.io_offset = offset;
  }
  else {
    THROW(ERROR_IO_OFFSET);
    return ;
  }
}


void electroneum_io_mark() {
  G_electroneum_vstate.io_mark = G_electroneum_vstate.io_offset;
}


void electroneum_io_inserted(unsigned int len) {
  G_electroneum_vstate.io_offset += len;
  G_electroneum_vstate.io_length += len;
}

void electroneum_io_discard(int clear) {
  G_electroneum_vstate.io_length = 0;
  G_electroneum_vstate.io_offset = 0;
  G_electroneum_vstate.io_mark = 0;
  if (clear) {
    electroneum_io_clear();
  }
}

void electroneum_io_clear() {
  os_memset(G_electroneum_vstate.io_buffer, 0 , ELECTRONEUM_IO_BUFFER_LENGTH);
}

/* ----------------------------------------------------------------------- */
/* INSERT data to be sent                                                  */
/* ----------------------------------------------------------------------- */

void electroneum_io_hole(unsigned int sz) {
  if ((G_electroneum_vstate.io_length + sz) > ELECTRONEUM_IO_BUFFER_LENGTH) {
    THROW(ERROR_IO_FULL);
    return ;
  }
  os_memmove(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset+sz,
             G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset,
             G_electroneum_vstate.io_length-G_electroneum_vstate.io_offset);
  G_electroneum_vstate.io_length += sz;
}

void electroneum_io_insert(unsigned char const *buff, unsigned int len) {
  electroneum_io_hole(len);
  os_memmove(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, buff, len);
  G_electroneum_vstate.io_offset += len;
}

void electroneum_io_insert_hmac_for(unsigned char* buffer, int len) {
  unsigned char hmac[32];
  cx_hmac_sha256(G_electroneum_vstate.hmac_key, 32, buffer, len, hmac, 32);
  electroneum_io_insert(hmac,32);
}

void electroneum_io_insert_encrypt(unsigned char* buffer, int len) {
  //for now, only 32bytes block are allowed
  if (len != 32) {
    THROW(SW_WRONG_DATA);
    return ;
  }

  electroneum_io_hole(len);

#if defined(IODUMMYCRYPT)
  for (int i = 0; i<len; i++) {
       G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+i] = buffer[i] ^ 0x55;
    }
#elif defined(IONOCRYPT)
  os_memmove(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, buffer, len);
#else
  cx_aes(&G_electroneum_vstate.spk, CX_ENCRYPT|CX_CHAIN_CBC|CX_LAST|CX_PAD_NONE,
         buffer, len,
         G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len);
#endif
  G_electroneum_vstate.io_offset += len;
  if (G_electroneum_vstate.tx_in_progress) {
    electroneum_io_insert_hmac_for(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset-len, len);
  }

}

void electroneum_io_insert_u32(unsigned  int v32) {
  electroneum_io_hole(4);
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] = v32>>24;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] = v32>>16;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+2] = v32>>8;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+3] = v32>>0;
  G_electroneum_vstate.io_offset += 4;
}

void electroneum_io_insert_u24(unsigned  int v24) {
  electroneum_io_hole(3);
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] = v24>>16;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] = v24>>8;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+2] = v24>>0;
  G_electroneum_vstate.io_offset += 3;
}

void electroneum_io_insert_u16(unsigned  int v16) {
  electroneum_io_hole(2);
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] = v16>>8;
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] = v16>>0;
  G_electroneum_vstate.io_offset += 2;
}

void electroneum_io_insert_u8(unsigned int v8) {
  electroneum_io_hole(1);
  G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] = v8;
  G_electroneum_vstate.io_offset += 1;
}

void electroneum_io_insert_t(unsigned int T) {
  if (T &0xFF00) {
     electroneum_io_insert_u16(T);
  } else {
     electroneum_io_insert_u8(T);
  }
}

void electroneum_io_insert_tl(unsigned int T, unsigned int L) {
  electroneum_io_insert_t(T);
  if (L < 128) {
    electroneum_io_insert_u8(L);
  } else if (L < 256) {
    electroneum_io_insert_u16(0x8100|L);
  } else {
    electroneum_io_insert_u8(0x82);
    electroneum_io_insert_u16(L);
  }
}

void electroneum_io_insert_tlv(unsigned int T, unsigned int L, unsigned char const *V) {
  electroneum_io_insert_tl(T,L);
  electroneum_io_insert(V, L);
}

/* ----------------------------------------------------------------------- */
/* FECTH data from received buffer                                         */
/* ----------------------------------------------------------------------- */
void electroneum_io_assert_available(int sz) {
  if ((G_electroneum_vstate.io_length-G_electroneum_vstate.io_offset) < sz) {
    THROW(SW_WRONG_LENGTH + (sz&0xFF));
  }
}

int electroneum_io_fetch(unsigned char* buffer, int len) {
  electroneum_io_assert_available(len);
  if (buffer) {
    os_memmove(buffer, G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len);
  }
  G_electroneum_vstate.io_offset += len;
  return len;
}



static void electroneum_io_verify_hmac_for(const unsigned char* buffer, int len, unsigned char *expected_hmac) {
  unsigned char  hmac[32];

  cx_hmac_sha256(G_electroneum_vstate.hmac_key, 32,
                 buffer, len,
                 hmac, 32);
  if (os_memcmp(hmac, expected_hmac, 32)) {
      THROW(SW_SECURITY_TRUSTED_INPUT);
    }
}

int electroneum_io_fetch_decrypt(unsigned char* buffer, int len) {


  //for now, only 32bytes block allowed
  if (len != 32) {
    THROW(SW_SECURE_MESSAGING_NOT_SUPPORTED);
    return 0;
  }

  if (G_electroneum_vstate.tx_in_progress) {
    electroneum_io_assert_available(len+32);
    electroneum_io_verify_hmac_for(G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len,
                              G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset + len);
  } else {
    electroneum_io_assert_available(len);
  }

  if (buffer) {
#if defined(IODUMMYCRYPT)
    for (int i = 0; i<len; i++) {
      buffer[i] = G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+i] ^ 0x55;
    }
#elif defined(IONOCRYPT)
     os_memmove(buffer, G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len);
#else //IOCRYPT
    cx_aes(&G_electroneum_vstate.spk, CX_DECRYPT|CX_CHAIN_CBC|CX_LAST|CX_PAD_NONE,
           G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len,
           buffer, len);
#endif
  }
  G_electroneum_vstate.io_offset += len;
  if (G_electroneum_vstate.tx_in_progress) {
    G_electroneum_vstate.io_offset += 32;
  }
  return len;
}

int electroneum_io_fetch_decrypt_key(unsigned char* buffer) {
  unsigned char* k;
  electroneum_io_assert_available(32);

  k = G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset;
  //view?
  if (os_memcmp(k, C_FAKE_SEC_VIEW_KEY, 32)==0) {
    G_electroneum_vstate.io_offset += 32;
    if (G_electroneum_vstate.tx_in_progress) {
      electroneum_io_assert_available(32);
      electroneum_io_verify_hmac_for(C_FAKE_SEC_VIEW_KEY, 32, G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset);
      G_electroneum_vstate.io_offset += 32;
    }
    os_memmove(buffer, G_electroneum_vstate.a,32);
    return 32;
  }
  //spend?
  else if (os_memcmp(k, C_FAKE_SEC_SPEND_KEY, 32)==0) {
    G_electroneum_vstate.io_offset += 32;
    if (G_electroneum_vstate.tx_in_progress) {
      electroneum_io_assert_available(32);
      electroneum_io_verify_hmac_for(C_FAKE_SEC_SPEND_KEY, 32, G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset);
    }
    os_memmove(buffer, G_electroneum_vstate.b,32);
    return 32;
  }
  //else
  else {
    return electroneum_io_fetch_decrypt(buffer, 32);
  }
}

unsigned int electroneum_io_fetch_u32() {
  unsigned int  v32;
  electroneum_io_assert_available(4);
  v32 =  ( (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] << 24) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] << 16) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+2] << 8) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+3] << 0) );
  G_electroneum_vstate.io_offset += 4;
  return v32;
}

unsigned int electroneum_io_fetch_u24() {
  unsigned int  v24;
  electroneum_io_assert_available(3);
  v24 =  ( (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] << 16) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] << 8) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+2] << 0) );
  G_electroneum_vstate.io_offset += 3;
  return v24;
}

unsigned int electroneum_io_fetch_u16() {
  unsigned int  v16;
  electroneum_io_assert_available(2);
  v16 =  ( (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+0] << 8) |
           (G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset+1] << 0) );
  G_electroneum_vstate.io_offset += 2;
  return v16;
}

unsigned int electroneum_io_fetch_u8() {
  unsigned int  v8;
  electroneum_io_assert_available(1);
  v8 = G_electroneum_vstate.io_buffer[G_electroneum_vstate.io_offset] ;
  G_electroneum_vstate.io_offset += 1;
  return v8;
}

int electroneum_io_fetch_t(unsigned int *T) {
  *T = electroneum_io_fetch_u8();
  if ((*T & 0x1F) == 0x1F) {
    *T = (*T << 8) | electroneum_io_fetch_u8();
  }
  return 0;
}

int electroneum_io_fetch_l(unsigned int *L) {
  *L = electroneum_io_fetch_u8();

  if ((*L & 0x80) != 0) {
    *L &= 0x7F;
    if (*L == 1) {
      *L =  electroneum_io_fetch_u8();
    } else if (*L == 2) {
      *L =  electroneum_io_fetch_u16() ;
    } else {
      *L = -1;
    }
  }
  return 0;
}

int electroneum_io_fetch_tl(unsigned int *T, unsigned int *L) {
  electroneum_io_fetch_t(T);
  electroneum_io_fetch_l(L);
  return 0;
}

int electroneum_io_fetch_nv(unsigned char* buffer, int len) {
  electroneum_io_assert_available(len);
  electroneum_nvm_write(buffer, G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, len);
  G_electroneum_vstate.io_offset += len;
  return len;
}




/* ----------------------------------------------------------------------- */
/* REAL IO                                                                 */
/* ----------------------------------------------------------------------- */

#define MAX_OUT ELECTRONEUM_APDU_LENGTH

int electroneum_io_do(unsigned int io_flags) {


  // if IO_ASYNCH_REPLY has been  set,
  //  electroneum_io_exchange will return when  IO_RETURN_AFTER_TX will set in ui
  if (io_flags & IO_ASYNCH_REPLY) {
    electroneum_io_exchange(CHANNEL_APDU | IO_ASYNCH_REPLY, 0);
  }
  //else send data now
  else {
    G_electroneum_vstate.io_offset = 0;
    if(G_electroneum_vstate.io_length > MAX_OUT) {
      THROW(SW_FILE_FULL);
      return SW_FILE_FULL;
    }
    os_memmove(G_io_apdu_buffer,  G_electroneum_vstate.io_buffer+G_electroneum_vstate.io_offset, G_electroneum_vstate.io_length);

    if (io_flags & IO_RETURN_AFTER_TX) {
      electroneum_io_exchange(CHANNEL_APDU |IO_RETURN_AFTER_TX, G_electroneum_vstate.io_length);
      return 0;
    } else {
      electroneum_io_exchange(CHANNEL_APDU,  G_electroneum_vstate.io_length);
    }
  }

  //--- set up received data  ---
  G_electroneum_vstate.io_offset = 0;
  G_electroneum_vstate.io_length = 0;
  G_electroneum_vstate.io_protocol_version = G_io_apdu_buffer[0];
  G_electroneum_vstate.io_ins = G_io_apdu_buffer[1];
  G_electroneum_vstate.io_p1  = G_io_apdu_buffer[2];
  G_electroneum_vstate.io_p2  = G_io_apdu_buffer[3];
  G_electroneum_vstate.io_lc  = 0;
  G_electroneum_vstate.io_le  = 0;
  G_electroneum_vstate.io_lc  = G_io_apdu_buffer[4];
  os_memmove(G_electroneum_vstate.io_buffer, G_io_apdu_buffer+5, G_electroneum_vstate.io_lc);
  G_electroneum_vstate.io_length =  G_electroneum_vstate.io_lc;

  return 0;
}


