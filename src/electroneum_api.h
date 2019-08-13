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

#ifndef electroneum_API_H
#define  electroneum_API_H


void electroneum_install(unsigned char netId);
void electroneum_init(void);
void electroneum_init_private_key(void);
void electroneum_wipe_private_key(void);

void electroneum_init_ux(void);
int electroneum_dispatch(void);

int electroneum_apdu_put_key(void);
int electroneum_apdu_get_key(void);
int electroneum_apdu_manage_seedwords() ;
int electroneum_apdu_verify_key(void);
int electroneum_apdu_get_chacha8_prekey(void);
int electroneum_apdu_sc_add(void);
int electroneum_apdu_sc_sub(void);
int electroneum_apdu_scal_mul_key(void);
int electroneum_apdu_scal_mul_base(void);
int electroneum_apdu_generate_keypair(void);
int electroneum_apdu_secret_key_to_public_key(void);
int electroneum_apdu_generate_key_derivation(void);
int electroneum_apdu_derivation_to_scalar(void);
int electroneum_apdu_derive_public_key(void);
int electroneum_apdu_derive_secret_key(void);
int electroneum_apdu_generate_key_image(void);
int electroneum_apdu_derive_subaddress_public_key(void);
int electroneum_apdu_get_subaddress(void);
int electroneum_apdu_get_subaddress_spend_public_key(void);
int electroneum_apdu_get_subaddress_secret_key(void);

int electroneum_apdu_get_tx_proof(void);

int electroneum_apdu_open_tx(void);
void electroneum_reset_tx(void);
int electroneum_apdu_open_subtx(void) ;
int electroneum_apdu_set_signature_mode(void) ;
int electroneum_apdu_stealth(void);
int electroneum_apdu_blind(void);
int electroneum_apdu_unblind(void);
int electroneum_apdu_gen_commitment_mask(void);

int electroneum_apdu_mlsag_prehash_init(void);
int electroneum_apdu_mlsag_prehash_update(void);
int electroneum_apdu_mlsag_prehash_finalize(void);
int electroneum_apu_generate_txout_keys(void);

int electroneum_apdu_mlsag_prepare(void);
int electroneum_apdu_mlsag_hash(void);
int electroneum_apdu_mlsag_sign(void);
int electroneum_apdu_close_tx(void);

/* ----------------------------------------------------------------------- */
/* ---                               MISC                             ---- */
/* ----------------------------------------------------------------------- */
#define OFFSETOF(type, field)    ((unsigned int)&(((type*)NULL)->field))

int electroneum_base58_public_key( char* str_b58, unsigned char *view, unsigned char *spend, unsigned char is_subbadress);

/** unsigned varint amount to uint64 */
uint64_t electroneum_vamount2uint64(unsigned char *binary);
/** binary little endian unsigned  int amount to uint64 */
uint64_t electroneum_bamount2uint64(unsigned char *binary);
/** unsigned varint amount to str */
int electroneum_vamount2str(unsigned char *binary,  char *str, unsigned int str_len);
/** binary little endian unsigned  int amount to str */
int electroneum_bamount2str(unsigned char *binary,  char *str, unsigned int str_len);
/** uint64  amount to str */
int electroneum_amount2str(uint64_t xmr,  char *str, unsigned int str_len);

int electroneum_abort_tx() ;
int electroneum_unblind(unsigned char *v, unsigned char *k, unsigned char *AKout, unsigned int short_amount);
void ui_menu_validation_display(unsigned int value) ;
void ui_menu_fee_validation_display(unsigned int value) ;
void ui_menu_change_validation_display(unsigned int value) ;

/* ----------------------------------------------------------------------- */
/* ---                          KEYS & ADDRESS                        ---- */
/* ----------------------------------------------------------------------- */
void electroneum_sc_add(unsigned char *r, unsigned char *s1, unsigned char *s2);
void electroneum_hash_to_scalar(unsigned char *scalar, unsigned char *raw, unsigned int len);
void electroneum_hash_to_ec(unsigned char *ec, unsigned char *ec_pub);
void electroneum_generate_keypair(unsigned char *ec_pub, unsigned char *ec_priv);
/*
 *  compute s = 8 * (k*P)
 *
 * s [out] 32 bytes derivation value
 * P [in]  point in 02 y or 04 x y format
 * k [in]  32 bytes scalar
 */
void electroneum_generate_key_derivation(unsigned char *drv_data, unsigned char *P, unsigned char *scalar);
void electroneum_derivation_to_scalar(unsigned char *scalar, unsigned char *drv_data, unsigned int out_idx);
/*
 *  compute x = Hps(drv_data,out_idx) + ec_pv
 *
 * x        [out] 32 bytes private key
 * drv_data [in]  32 bytes derivation data (point)
 * ec_pv    [in]  32 bytes private key
 */
void electroneum_derive_secret_key(unsigned char *x, unsigned char *drv_data, unsigned int out_idx, unsigned char *ec_priv);
/*
 *  compute x = Hps(drv_data,out_idx)*G + ec_pub
 *
 * x        [out] 32 bytes public key
 * drv_data [in]  32 bytes derivation data (point)
 * ec_pub   [in]  32 bytes public key
 */
void electroneum_derive_public_key(unsigned char *x, unsigned char* drv_data, unsigned int out_idx, unsigned char *ec_pub);
void electroneum_secret_key_to_public_key(unsigned char *ec_pub, unsigned char *ec_priv);
void electroneum_generate_key_image(unsigned char *img, unsigned char *P, unsigned char* x);

void electroneum_derive_subaddress_public_key(unsigned char *x, unsigned char *pub, unsigned char* drv_data, unsigned int index);
void electroneum_get_subaddress_spend_public_key(unsigned char *x,unsigned char *index);
void electroneum_get_subaddress(unsigned char *C, unsigned char *D, unsigned char *index);
void electroneum_get_subaddress_secret_key(unsigned char *sub_s, unsigned char *s, unsigned char *index);

void electroneum_clear_words();
/* ----------------------------------------------------------------------- */
/* ---                              CRYPTO                            ---- */
/* ----------------------------------------------------------------------- */
extern const unsigned char C_ED25519_ORDER[];


void electroneum_aes_derive(cx_aes_key_t *sk, unsigned char *seed32, unsigned char *a, unsigned char *b);
void electroneum_aes_generate(cx_aes_key_t *sk);

/* Compute electroneum-Hash of data*/
void electroneum_hash_init_keccak(cx_hash_t * hasher);
void electroneum_hash_init_sha256(cx_hash_t * hasher);
void electroneum_hash_update(cx_hash_t * hasher, unsigned char* buf, unsigned int len) ;
int  electroneum_hash_final(cx_hash_t * hasher, unsigned char* out);
int  electroneum_hash(unsigned int algo, cx_hash_t * hasher, unsigned char* buf, unsigned int len, unsigned char* out);

#define electroneum_keccak_init_F() \
    electroneum_hash_init_keccak((cx_hash_t *)&G_electroneum_vstate.keccakF)
#define electroneum_keccak_update_F(buf,len)  \
    electroneum_hash_update((cx_hash_t *)&G_electroneum_vstate.keccakF,(buf), (len))
#define electroneum_keccak_final_F(out) \
    electroneum_hash_final((cx_hash_t *)&G_electroneum_vstate.keccakF, (out))
#define electroneum_keccak_F(buf,len,out) \
    electroneum_hash(CX_KECCAK, (cx_hash_t *)&G_electroneum_vstate.keccakF, (buf),(len), (out))

#define electroneum_keccak_init_H() \
    electroneum_hash_init_keccak((cx_hash_t *)&G_electroneum_vstate.keccakH)
#define electroneum_keccak_update_H(buf,len)  \
    electroneum_hash_update((cx_hash_t *)&G_electroneum_vstate.keccakH,(buf), (len))
#define electroneum_keccak_final_H(out) \
    electroneum_hash_final((cx_hash_t *)&G_electroneum_vstate.keccakH, (out)?(out):G_electroneum_vstate.H)
#define electroneum_keccak_H(buf,len,out) \
    electroneum_hash(CX_KECCAK, (cx_hash_t *)&G_electroneum_vstate.keccakH, (buf),(len), (out)?(out):G_electroneum_vstate.H)

#define electroneum_sha256_commitment_init() \
    electroneum_hash_init_sha256((cx_hash_t *)&G_electroneum_vstate.sha256_commitment)
#define electroneum_sha256_commitment_update(buf,len) \
    electroneum_hash_update((cx_hash_t *)&G_electroneum_vstate.sha256_commitment,(buf), (len))
#define electroneum_sha256_commitment_final(out) \
    electroneum_hash_final((cx_hash_t *)&G_electroneum_vstate.sha256_commitment, (out)?(out):G_electroneum_vstate.C)

#define electroneum_sha256_outkeys_init() \
    electroneum_hash_init_sha256((cx_hash_t *)&G_electroneum_vstate.sha256_out_keys)
#define electroneum_sha256_outkeys_update(buf,len) \
    electroneum_hash_update((cx_hash_t *)&G_electroneum_vstate.sha256_out_keys, (buf), (len))
#define electroneum_sha256_outkeys_final(out) \
    electroneum_hash_final((cx_hash_t *)&G_electroneum_vstate.sha256_out_keys, (out)?(out):G_electroneum_vstate.OUTK)


/**
 * LE-7-bits encoding. High bit set says one more byte to decode.
 */
unsigned int electroneum_encode_varint(unsigned char varint[8], unsigned int out_idx);

/** */
void electroneum_reverse32(unsigned char *rscal, unsigned char *scal);

/**
 * Hps: keccak(drv_data|varint(out_idx))
 */
void electroneum_derivation_to_scalar(unsigned char *scalar, unsigned char *drv_data, unsigned int out_idx);


/*
 * W = k.P
 */
void electroneum_ecmul_k(unsigned char *W, unsigned char *P, unsigned char *scalar32);
/*
 * W = 8k.P
 */
void electroneum_ecmul_8k(unsigned char *W, unsigned char *P, unsigned char *scalar32);

/*
 * W = 8.P
 */
void electroneum_ecmul_8(unsigned char *W, unsigned char *P);

/*
 * W = k.G
 */
void electroneum_ecmul_G(unsigned char *W, unsigned char *scalar32);

/*
 * W = k.H
 */
void electroneum_ecmul_H(unsigned char *W, unsigned char *scalar32);


/**
 *  keccak("amount"|sk)
 */
void electroneum_ecdhHash(unsigned char *x, unsigned char *k);

/**
 * keccak("commitment_mask"|sk) %order
 */
void electroneum_genCommitmentMask(unsigned char *c,  unsigned char *sk);

/*
 * W = P+Q
 */
void electroneum_ecadd(unsigned char *W, unsigned char *P, unsigned char *Q);
/*
 * W = P-Q
 */
void electroneum_ecsub(unsigned char *W, unsigned char *P, unsigned char *Q);

/* r = (a+b) %order */
void electroneum_addm(unsigned char *r, unsigned char *a, unsigned char *b);

/* r = (a-b) %order */
void electroneum_subm(unsigned char *r, unsigned char *a, unsigned char *b);

/* r = (a*b) %order */
void electroneum_multm(unsigned char *r, unsigned char *a, unsigned char *b);

/* r = (a*8) %order */
void electroneum_multm_8(unsigned char *r, unsigned char *a);

/* */
void electroneum_reduce(unsigned char *r, unsigned char *a);


void electroneum_rng(unsigned char *r,  int len) ;
/* ----------------------------------------------------------------------- */
/* ---                                IO                              ---- */
/* ----------------------------------------------------------------------- */

void electroneum_io_discard(int clear) ;
void electroneum_io_clear(void);
void electroneum_io_set_offset(unsigned int offset) ;
void electroneum_io_mark(void) ;
void electroneum_io_rewind(void) ;
void electroneum_io_hole(unsigned int sz) ;
void electroneum_io_inserted(unsigned int len);
void electroneum_io_insert(unsigned char const * buffer, unsigned int len) ;
void electroneum_io_insert_encrypt(unsigned char* buffer, int len);

void electroneum_io_insert_u32(unsigned  int v32) ;
void electroneum_io_insert_u24(unsigned  int v24) ;
void electroneum_io_insert_u16(unsigned  int v16) ;
void electroneum_io_insert_u8(unsigned int v8) ;
void electroneum_io_insert_t(unsigned int T) ;
void electroneum_io_insert_tl(unsigned int T, unsigned int L) ;
void electroneum_io_insert_tlv(unsigned int T, unsigned int L, unsigned char const *V) ;

void electroneum_io_fetch_buffer(unsigned char  * buffer, unsigned int len) ;
unsigned int electroneum_io_fetch_u32(void) ;
unsigned int electroneum_io_fetch_u24(void) ;
unsigned int electroneum_io_fetch_u16(void) ;
unsigned int electroneum_io_fetch_u8(void) ;
int electroneum_io_fetch_t(unsigned int *T) ;
int electroneum_io_fetch_l(unsigned int *L) ;
int electroneum_io_fetch_tl(unsigned int *T, unsigned int *L) ;
int electroneum_io_fetch_nv(unsigned char* buffer, int len) ;
int electroneum_io_fetch(unsigned char* buffer, int len) ;
int electroneum_io_fetch_decrypt(unsigned char* buffer, int len);
int electroneum_io_fetch_decrypt_key(unsigned char* buffer);

int electroneum_io_do(unsigned int io_flags) ;
/* ----------------------------------------------------------------------- */
/* ---                                DEBUG                           ---- */
/* ----------------------------------------------------------------------- */
#ifdef electroneum_DEBUG

#include "electroneum_debug.h"

#else

#define electroneum_nvm_write   nvm_write
#define electroneum_io_exchange io_exchange

#endif

#endif
