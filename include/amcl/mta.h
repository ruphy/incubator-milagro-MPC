/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

/**
 * @file mta.h
 * @brief MTA declarations
 *
 */

#ifndef MTA_H
#define MTA_H

#include "amcl/amcl.h"
#include "amcl/paillier.h"
#include "amcl/commitments.h"
#include "amcl/ecp_SECP256K1.h"
#include "amcl/ecdh_SECP256K1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MTA_OK 0             /**< Proof successfully verified */
#define MTA_FAIL 61          /**< Invalid proof */
#define MTA_INVALID_ECP 62   /**< Invalid ECP */

/* MTA protocol API */

/*! \brief Client MTA first pass
 *
 *  Encrypt multplicative share, \f$ a \f$, of secret \f$ s = a.b \f$
 *
 *  @param  RNG              Pointer to a cryptographically secure random number generator
 *  @param  PUB              Paillier Public key
 *  @param  A                Multiplicative share of secret
 *  @param  CA               Ciphertext
 *  @param  R                R value for testing. If RNG is NULL then this value is read.
 */
void MPC_MTA_CLIENT1(csprng *RNG, PAILLIER_public_key* PUB, octet* A, octet* CA, octet* R);

/*! \brief Client MtA second pass
 *
 *  Calculate additive share, \f$ \alpha \f$, of secret \f$ s = a.b \f$
 *
 *  <ol>
 *  <li> Choose a random non-zero value \f$ z \in  F_q \f$ where \f$q\f$ is the curve order
 *  <li> \f$ \alpha = D_A(cb) = D_A(E_A(ab + z)) = ab + z \text{ }\mathrm{mod}\text{ }q \f$
 *  </ol>
 *
 *  @param   PRIV             Paillier Private key
 *  @param   CB               Ciphertext
 *  @param   ALPHA            Additive share of secret
 */
void MPC_MTA_CLIENT2(PAILLIER_private_key *PRIV, octet* CB, octet *ALPHA);

/*! \brief Server MtA
 *
 *  Calculate additive share, \f$ \beta \f$, of secret \f$ s = a.b \f$ and
 *  ciphertext allowing client to calculate their additive share.
 *
 *  <ol>
 *  <li> Choose a random non-zero value \f$ z \in  F_q \f$ where \f$q\f$ is the curve order
 *  <li> \f$ \beta = -z\text{ }\mathrm{mod}\text{ }q \f$
 *  <li> \f$ cb = ca \otimes{} b \oplus{} z = E_A(ab + z) \f$
 *  </ol>
 *
 *  @param   RNG              Pointer to a cryptographically secure random number generator
 *  @param   PUB              Paillier Public key
 *  @param   B                Multiplicative share of secret
 *  @param   CA               Ciphertext of client's additive share of secret
 *  @param   Z                Plaintext z value (see above)
 *  @param   R                R value for testing. If RNG is NULL then this value is read.
 *  @param   CB               Ciphertext
 *  @param   BETA             Additive share of secret (see above)
 */
void MPC_MTA_SERVER(csprng *RNG, PAILLIER_public_key *PUB, octet *B, octet *CA, octet *Z, octet *R, octet *CB, octet *BETA);

/** \brief Sum of secret shares
 *
 *  Sum of secret shares generated by multiplicative to additive scheme
 *
 *  <ol>
 *  <li> \f$ sum  = a.b + \alpha + \beta \text{ }\mathrm{mod}\text{ }q \f$
 *  </ol>
 *
 *  @param A                  A1 value
 *  @param B                  B1 value
 *  @param ALPHA              Additive share of A1.B2
 *  @param BETA               Additive share of A2.B1
 *  @param SUM                The sum of all values
 */
void MPC_SUM_MTA(octet *A, octet *B, octet *ALPHA, octet *BETA, octet *SUM);

/* MTA Zero Knowledge Proofs API*/

// The protocols require a BC modulus (Pt, Qt, Nt, h1, h2) and a Paillier PK (N, g)

/** \brief Random challenge for any of the ZK Proofs
 *
 *  Generate a random challenge for any of the ZK Proofs
 *  below. This can be used instead of the deterministic challenges
 *  produced for each specific proof to make any of the proofs
 *  interactive and be interoperable with other implementations.
 *
 *  <ol>
 *  <li> \f$ e \in_R [0, \ldots, q] \f$
 *  </ol>
 *
 *  @param RNG               csprng for random generation
 *  @param E                 Destination octet for the challenge.
 */
void MTA_ZK_random_challenge(csprng *RNG, octet *E);

/* Range Proof API */

/** \brief Secret random values for the Range Proof commitment */
typedef struct
{
    BIG_1024_58 alpha[FFLEN_2048];              /**< Random value in \f$ [0, \ldots, q^3]          \f$ */
    BIG_1024_58 beta[FFLEN_2048];               /**< Random value in \f$ [0, \ldots, N]            \f$ */
    BIG_1024_58 gamma[FFLEN_2048 + HFLEN_2048]; /**< Random value in \f$ [0, \ldots, \tilde{N}q^3] \f$ */
    BIG_1024_58 rho[FFLEN_2048 + HFLEN_2048];   /**< Random value in \f$ [0, \ldots, \tilde{N}q]   \f$ */
} MTA_RP_commitment_rv;

/** \brief Public commitment for the Range Proof */
typedef struct
{
    BIG_1024_58 z[FFLEN_2048];  /**< Commitment to h1, h2, m using rho */
    BIG_512_60  u[FFLEN_4096];  /**< Commitment to paillier PK using alpha and beta */
    BIG_1024_58 w[FFLEN_2048];  /**< Commitment to h1, h2, m using gamma */
} MTA_RP_commitment;

/** \brief Range Proof */
typedef struct
{
    BIG_512_60  s[FFLEN_4096];                /**< Proof of knowledge of the Paillier r value */
    BIG_1024_58 s1[FFLEN_2048];               /**< Proof of knowledge of the message. It must be less than q^3 */
    BIG_1024_58 s2[FFLEN_2048 + HFLEN_2048];  /**< Auxiliary proof of knowledge for the message */
} MTA_RP_proof;

/** \brief Commitment Generation
 *
 *  Generate a commitment for the message M
 *
 *  <ol>
 *  <li> \f$ \alpha \in_R [0, \ldots, q^3]\f$
 *  <li> \f$ \beta  \in_R [0, \ldots, N]\f$
 *  <li> \f$ \gamma \in_R [0, \ldots, q^{3}\tilde{N}]\f$
 *  <li> \f$ \rho   \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ z = h_1^{m}h_2^{\rho}        \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ u = h_1^{\alpha}h_2^{\gamma} \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ w = g^{\alpha}\beta^{N} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  </ol>
 *
 *  @param RNG         csprng for random generation
 *  @param key         Paillier key used to encrypt M
 *  @param mod         Public BC modulus of the verifier
 *  @param M           Message to prove knowledge and range
 *  @param c           Destinaton commitment
 *  @param rv          Random values associated to the commitment. If RNG is NULL this is read
 */
extern void MTA_RP_commit(csprng *RNG, PAILLIER_private_key *key, COMMITMENTS_BC_pub_modulus *mod,  octet *M, MTA_RP_commitment *c, MTA_RP_commitment_rv *rv);

/** \brief Deterministic Challenge generations
 *
 *  Generate a challenge binding together public parameters and commitment
 *
 *  <ol>
 *  <li> \f$ e = H( g | \tilde{N} | h_1 | h_2 | q | CT | z | u | w ) \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Public BC modulus of the verifier
 *  @param CT          Encrypted Message to prove knowledge and range
 *  @param c           Commitment of the prover
 *  @param E           Destination challenge
 */
extern void MTA_RP_challenge(PAILLIER_public_key *key, COMMITMENTS_BC_pub_modulus *mod, octet *CT, MTA_RP_commitment *c, octet *E);

/** \brief Proof generation
 *
 *  Generate a proof of knowledge of m and of its range
 *
 *  <ol>
 *  <li> \f$ s  = \beta r^e \text{ }\mathrm{mod}\text{ }N \f$
 *  <li> \f$ s_1 = em + \alpha \f$
 *  <li> \f$ s_2 = e\rho + \gamma \f$
 *  </ol>
 *
 *  @param key         Private Paillier key of the prover
 *  @param rv          Random values associated to the commitment
 *  @param M           Message to prove knowledge and range
 *  @param R           Random value used in the Paillier encryption of M
 *  @param E           Generated challenge
 *  @param p           Destination proof
 */
extern void MTA_RP_prove(PAILLIER_private_key *key, MTA_RP_commitment_rv *rv, octet *M, octet *R, octet *E, MTA_RP_proof *p);

/** \brief Verify a Proof
 *
 *  Verify the proof of knowledge of m associated to CT and of its range
 *
 *  <ol>
 *  <li> \f$ s1 \stackrel{?}{\leq} q^3 \f$
 *  <li> \f$ w \stackrel{?}{=} h_1^{s_1}h_2^{s_2}z^{-e} \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ u \stackrel{?}{=} g^{s_1}s^{N}c^{-e} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Private BC modulus of the verifier
 *  @param CT          Encrypted Message to prove knowledge and range
 *  @param E           Generated challenge
 *  @param c           Received commitment
 *  @param p           Received proof
 *  @return            MTA_OK if the proof is valid, MTA_FAIL otherwise
 */
extern int MTA_RP_verify(PAILLIER_public_key *key, COMMITMENTS_BC_priv_modulus *mod, octet *CT, octet *E, MTA_RP_commitment *c, MTA_RP_proof *p);

/** \brief Dump the commitment to octets
 *
 *  @param Z           Destination Octet for the z component of the commitment. FS_2048 long
 *  @param U           Destination Octet for the u component of the commitment. FS_4096 long
 *  @param W           Destination Octet for the w component of the commitment. FS_2048 long
 *  @param c           Commitment to export
 */
extern void MTA_RP_commitment_toOctets(octet *Z, octet *U, octet *W, MTA_RP_commitment *c);

/** \brief Read the commitments from octets
 *
 *  @param c           Destination commitment
 *  @param Z           Octet with the z component of the proof
 *  @param U           Octet with the u component of the proof
 *  @param W           Octet with the w component of the proof
 */
extern void MTA_RP_commitment_fromOctets(MTA_RP_commitment *c, octet *Z, octet *U, octet *W);

/** \brief Dump the proof to octets
 *
 *  @param S           Destination Octet for the s component of the proof. FS_2048 long
 *  @param S1          Destination Octet for the s1 component of the proof. HFS_2048 long
 *  @param S2          Destination Octet for the s2 component of the proof. FS_2048 + HFS_2048 long
 *  @param p           Proof to export
 */
extern void MTA_RP_proof_toOctets(octet *S, octet *S1, octet *S2, MTA_RP_proof *p);

/** \brief Read the proof from octets
 *
 *  @param p           Destination proof
 *  @param S           Octet with the s component of the proof
 *  @param S1          Octet with the s1 component of the proof
 *  @param S2          Octet with the s2 component of the proof
 */
extern void MTA_RP_proof_fromOctets(MTA_RP_proof *p, octet *S, octet *S1, octet *S2);

/** \brief Clean the memory containing the random values
 *
 *   @param rv         Random values to clean
 */
extern void MTA_RP_commitment_rv_kill(MTA_RP_commitment_rv *rv);

/* Receiver Zero Knowledge Proof API */

/** \brief Secret random values for the receiver ZKP commitment */
typedef struct
{
    BIG_1024_58 alpha[FFLEN_2048];              /**< Random value in \f$ [0, \ldots, q^3]          \f$ */
    BIG_1024_58 beta[FFLEN_2048];               /**< Random value in \f$ [0, \ldots, N]            \f$ */
    BIG_1024_58 gamma[FFLEN_2048];              /**< Random value in \f$ [0, \ldots, N]            \f$ */
    BIG_1024_58 rho[FFLEN_2048 + HFLEN_2048];   /**< Random value in \f$ [0, \ldots, \tilde{N}q]   \f$ */
    BIG_1024_58 rho1[FFLEN_2048 + HFLEN_2048];  /**< Random value in \f$ [0, \ldots, \tilde{N}q^3] \f$ */
    BIG_1024_58 sigma[FFLEN_2048 + HFLEN_2048]; /**< Random value in \f$ [0, \ldots, \tilde{N}q]   \f$ */
    BIG_1024_58 tau[FFLEN_2048 + HFLEN_2048];   /**< Random value in \f$ [0, \ldots, \tilde{N}q]   \f$ */
} MTA_ZK_commitment_rv;

/** \brief Public commitment for the Receiver ZKP */
typedef struct
{
    BIG_1024_58 z[FFLEN_2048];      /**< Commitment to h1, h2, x using rho */
    BIG_1024_58 z1[FFLEN_2048];     /**< Auxiliary Commitment to h1, h2, binding alpha and rho1 */
    BIG_1024_58 t[FFLEN_2048];      /**< Commitment to h1, h2, y using sigma */
    BIG_1024_58 v[2 * FFLEN_2048];  /**< Commitment to paillier PK and c1 using alpha and gamma */
    BIG_1024_58 w[FFLEN_2048];      /**< Auxiliary Commitment to h1, h2, binding gamma and tau */
} MTA_ZK_commitment;

/** \brief Range Proof for the Receiver ZKP */
typedef struct
{
    BIG_1024_58 s[FFLEN_2048];                /**< Proof of knowledge of the Paillier r value */
    BIG_1024_58 s1[FFLEN_2048];               /**< Proof of knowledge of x. It must be less than q^3 */
    BIG_1024_58 s2[FFLEN_2048 + HFLEN_2048];  /**< Auxiliary proof of knowledge for x */
    BIG_1024_58 t1[FFLEN_2048];               /**< Proof of knowledge of y */
    BIG_1024_58 t2[FFLEN_2048 + HFLEN_2048];  /**< Auxiliary proof of knowledge for y */
} MTA_ZK_proof;

/** \brief Commitment Generation for Receiver ZKP
 *
 *  Generate a commitment for the values x, y and c1
 *
 *  <ol>
 *  <li> \f$ \alpha \in_R [0, \ldots, q^3]\f$
 *  <li> \f$ \beta  \in_R [0, \ldots, N]\f$
 *  <li> \f$ \gamma \in_R [0, \ldots, N]\f$
 *  <li> \f$ \rho   \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ \rho_1 \in_R [0, \ldots, q^{3}\tilde{N}]\f$
 *  <li> \f$ \sigma \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ \tau   \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ z  = h_1^{x}h_2^{\rho}              \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ z_1 = h_1^{\alpha}h_2^{\rho_1}       \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ t  = h_1^{y}h_2^{\sigma}            \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ w  = h_1^{\gamma}h_2^{\tau}         \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ v  = c1^{\alpha}g^{\gamma}\beta^{N} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  </ol>
 *
 *  @param RNG         csprng for random generation
 *  @param key         Paillier key used to encrypt C1
 *  @param mod         Public BC modulus of the verifier
 *  @param X           Message to prove knowledge and range
 *  @param Y           Message to prove knowledge
 *  @param C1          Base Paillier Ciphertext
 *  @param c           Destinaton commitment
 *  @param rv          Random values associated to the commitment. If RNG is NULL this is read
 */
extern void MTA_ZK_commit(csprng *RNG, PAILLIER_public_key *key, COMMITMENTS_BC_pub_modulus *mod,  octet *X, octet *Y, octet *C1, MTA_ZK_commitment *c, MTA_ZK_commitment_rv *rv);

/** \brief Deterministic Challenge generations for Receiver ZKP
 *
 *  Generate a challenge binding together public parameters and commitment
 *
 *  <ol>
 *  <li> \f$ e = H( g | \tilde{N} | h_1 | h_2 | q | c_1 | c_2 | z | z1 | t | v | w ) \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Public BC modulus of the verifier
 *  @param C1          Base Paillier Ciphertext
 *  @param C2          New Paillier Ciphertext to prove knowledge and range
 *  @param c           Commitment of the prover
 *  @param E           Destination challenge
 */
extern void MTA_ZK_challenge(PAILLIER_public_key *key, COMMITMENTS_BC_pub_modulus *mod, octet *C1, octet *C2, MTA_ZK_commitment *c, octet *E);

/** \brief Proof generation for Receiver ZKP
 *
 *  Generate a proof of knowledge of x, y and a range proof for x
 *
 *  <ol>
 *  <li> \f$ s  = \beta r^e \text{ }\mathrm{mod}\text{ }N \f$
 *  <li> \f$ s_1 = ex + \alpha \f$
 *  <li> \f$ s_2 = e\rho + \rho_1 \f$
 *  <li> \f$ t_1 = ey + \gamma \f$
 *  <li> \f$ t_2 = e\sigma + \tau \f$
 *  </ol>
 *
 *  @param key         Private Paillier key of the prover
 *  @param rv          Random values associated to the commitment
 *  @param X           Message to prove knowledge and range
 *  @param Y           Message to prove knowledge
 *  @param R           Random value used in the Paillier addition
 *  @param E           Generated challenge
 *  @param p           Destination proof
 */
extern void MTA_ZK_prove(PAILLIER_public_key *key, MTA_ZK_commitment_rv *rv, octet *X, octet *Y, octet *R, octet *E, MTA_ZK_proof *p);

/** \brief Verify a Proof for Receiver ZKP
 *
 *  Verify the proof of knowledge of x, y associated to c1, c2 and of x range
 *
 *  <ol>
 *  <li> \f$ s_1 \stackrel{?}{\leq} q^3 \f$
 *  <li> \f$ z_1 \stackrel{?}{=} h_1^{s_1}h_2^{s_2}z^{-e}    \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ w  \stackrel{?}{=} h_1^{t_1}h_2^{t_2}t^{-e}    \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ v  \stackrel{?}{=} c1^{s_1}s^{N}g^{t_1}c2^{-e} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Private BC modulus of the verifier
 *  @param C1          Base Paillier Ciphertext
 *  @param C2          New Paillier Ciphertext to prove knowledge and range
 *  @param E           Generated challenge
 *  @param c           Received commitment
 *  @param p           Received proof
 *  @return            MTA_OK if the proof is valid, MTA_FAIL otherwise
 */
extern int MTA_ZK_verify(PAILLIER_private_key *key, COMMITMENTS_BC_priv_modulus *mod, octet *C1, octet *C2, octet *E, MTA_ZK_commitment *c, MTA_ZK_proof *p);

/** \brief Dump the commitment to octets
 *
 *  @param Z           Destination Octet for the z component of the commitment. FS_2048 long
 *  @param Z1          Destination Octet for the z1 component of the commitment. FS_2048 long
 *  @param T           Destination Octet for the t component of the commitment. FS_2048 long
 *  @param V           Destination Octet for the v component of the commitment. FS_4096 long
 *  @param W           Destination Octet for the w component of the commitment. FS_2048 long
 *  @param c           Commitment to export
 */
extern void MTA_ZK_commitment_toOctets(octet *Z, octet *Z1, octet *T, octet *V, octet *W, MTA_ZK_commitment *c);

/** \brief Read the commitments from octets
 *
 *  @param c           Destination commitment
 *  @param Z           Destination Octet for the z component of the commitment. FS_2048 long
 *  @param Z1          Destination Octet for the z1 component of the commitment. FS_2048 long
 *  @param T           Destination Octet for the t component of the commitment. FS_2048 long
 *  @param V           Destination Octet for the v component of the commitment. FS_4096 long
 *  @param W           Destination Octet for the w component of the commitment. FS_2048 long
 */
extern void MTA_ZK_commitment_fromOctets(MTA_ZK_commitment *c, octet *Z, octet *Z1, octet *T, octet *V, octet *W);

/** \brief Dump the proof to octets
 *
 *  @param S           Destination Octet for the s component of the proof. FS_2048 long
 *  @param S1          Destination Octet for the s1 component of the proof. HFS_2048 long
 *  @param S2          Destination Octet for the s2 component of the proof. FS_2048 + HFS_2048 long
 *  @param T1          Destination Octet for the t1 component of the proof. FS_2048 long
 *  @param T2          Destination Octet for the t2 component of the proof. FS_2048 + HFS_2048 long
 *  @param p           Proof to export
 */
extern void MTA_ZK_proof_toOctets(octet *S, octet *S1, octet *S2, octet *T1, octet *T2, MTA_ZK_proof *p);

/** \brief Read the proof from octets
 *
 *  @param p           Destination proof
 *  @param S           Octet with the s component of the proof
 *  @param S1          Octet with the s1 component of the proof
 *  @param S2          Octet with the s2 component of the proof
 *  @param T1          Octet with the t1 component of the proof
 *  @param T2          Octet with the t2 component of the proof
 */
extern void MTA_ZK_proof_fromOctets(MTA_ZK_proof *p, octet *S, octet *S1, octet *S2, octet *T1, octet *T2);

/** \brief Clean the memory containing the random values
 *
 *   @param rv         Random values to clean
 */
extern void MTA_ZK_commitment_rv_kill(MTA_ZK_commitment_rv *rv);

/* Receiver Zero Knowledge Proof with Check API */

/** \brief Secret random values for the receiver ZKP with check commitment */
typedef MTA_ZK_commitment_rv MTA_ZKWC_commitment_rv;

/** \brief Public commitment for the Receiver ZKP with check */
typedef struct
{
    MTA_ZK_commitment zkc;    /**< Commitment for the base Recevier ZKP */
    ECP_SECP256K1 U;          /**< Commitment for the DLOG knowledge proof */
} MTA_ZKWC_commitment;

/** \brief Range Proof for the Receiver ZKP with check */
typedef MTA_ZK_proof MTA_ZKWC_proof;

/** \brief Commitment Generation for Receiver ZKP with check
 *
 *  Generate a commitment for the values x, y and c1
 *
 *  <ol>
 *  <li> \f$ \alpha \in_R [0, \ldots, q^3]\f$
 *  <li> \f$ \beta  \in_R [0, \ldots, N]\f$
 *  <li> \f$ \gamma \in_R [0, \ldots, N]\f$
 *  <li> \f$ \rho   \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ \rho_1 \in_R [0, \ldots, q^{3}\tilde{N}]\f$
 *  <li> \f$ \sigma \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ \tau   \in_R [0, \ldots, q\tilde{N}]\f$
 *  <li> \f$ z  = h_1^{x}h_2^{\rho}              \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ z_1 = h_1^{\alpha}h_2^{\rho_1}       \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ t  = h_1^{y}h_2^{\sigma}            \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ w  = h_1^{\gamma}h_2^{\tau}         \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ v  = c1^{\alpha}g^{\gamma}\beta^{N} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  <li> \f$ U  = \alpha.G \f$
 *  </ol>
 *
 *  @param RNG         csprng for random generation
 *  @param key         Paillier key used to encrypt C1
 *  @param mod         Public BC modulus of the verifier
 *  @param X           Message to prove knowledge and range
 *  @param Y           Message to prove knowledge
 *  @param C1          Base Paillier Ciphertext
 *  @param c           Destinaton commitment
 *  @param rv          Random values associated to the commitment. If RNG is NULL this is read
 */
extern void MTA_ZKWC_commit(csprng *RNG, PAILLIER_public_key *key, COMMITMENTS_BC_pub_modulus *mod,  octet *X, octet *Y, octet *C1, MTA_ZKWC_commitment *c, MTA_ZKWC_commitment_rv *rv);

/** \brief Deterministic Challenge generations for Receiver ZKP with check
 *
 *  Generate a challenge binding together public parameters and commitment
 *
 *  <ol>
 *  <li> \f$ e = H( g | \tilde{N} | h_1 | h_2 | q | c_1 | c_2 | U | z | z1 | t | v | w ) \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Public BC modulus of the verifier
 *  @param C1          Base Paillier Ciphertext
 *  @param C2          New Paillier Ciphertext to prove knowledge and range
 *  @param X           Public exponent of the associated DLOG to prove knowledge
 *  @param c           Commitment of the prover
 *  @param E           Destination challenge
 */
extern void MTA_ZKWC_challenge(PAILLIER_public_key *key, COMMITMENTS_BC_pub_modulus *mod, octet *C1, octet *C2, octet *X, MTA_ZKWC_commitment *c, octet *E);

/** \brief Proof generation for Receiver ZKP with check
 *
 *  Generate a proof of knowledge of x, y and a range proof for x.
 *  These values are the same as for the ZKP without check. The
 *  knwoledge of the DLOG can be verified using the value U in the
 *  commitment
 *
 *  <ol>
 *  <li> \f$ s  = \beta r^e \text{ }\mathrm{mod}\text{ }N \f$
 *  <li> \f$ s_1 = ex + \alpha \f$
 *  <li> \f$ s_2 = e\rho + \rho_1 \f$
 *  <li> \f$ t_1 = ey + \gamma \f$
 *  <li> \f$ t_2 = e\sigma + \tau \f$
 *  </ol>
 *
 *  @param key         Private Paillier key of the prover
 *  @param rv          Random values associated to the commitment
 *  @param X           Message to prove knowledge and range
 *  @param Y           Message to prove knowledge
 *  @param R           Random value used in the Paillier addition
 *  @param E           Generated challenge
 *  @param p           Destination proof
 */
extern void MTA_ZKWC_prove(PAILLIER_public_key *key, MTA_ZKWC_commitment_rv *rv, octet *X, octet *Y, octet *R, octet *E, MTA_ZKWC_proof *p);

/** \brief Verify a Proof for Receiver ZKP with check
 *
 *  Verify the proof of knowledge of x, y associated to c1, c2 and of x range.
 *  Additionally verify the knowledge of X = x.G
 *
 *  <ol>
 *  <li> \f$ s_1 \stackrel{?}{\leq} q^3 \f$
 *  <li> \f$ z_1 \stackrel{?}{=} h_1^{s_1}h_2^{s_2}z^{-e}   \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ w  \stackrel{?}{=} h_1^{t_1}h_2^{t_2}t^{-e}    \text{ }\mathrm{mod}\text{ }\tilde{N} \f$
 *  <li> \f$ v  \stackrel{?}{=} c1^{s_1}s^{N}g^{t_1}c2^{-e} \text{ }\mathrm{mod}\text{ }N^2 \f$
 *  <li> \f$ U  \stackrel{?}{=} s_1.G - e.X \f$
 *  </ol>
 *
 *  @param key         Public Paillier key of the prover
 *  @param mod         Private BC modulus of the verifier
 *  @param C1          Base Paillier Ciphertext
 *  @param C2          New Paillier Ciphertext to prove knowledge and range
 *  @param X           Public ECP of the DLOG x.G
 *  @param E           Generated challenge
 *  @param c           Received commitment
 *  @param p           Received proof
 *  @return            MTA_OK if the proof is valid, MTA_FAIL otherwise
 */
extern int MTA_ZKWC_verify(PAILLIER_private_key *key, COMMITMENTS_BC_priv_modulus *mod, octet *C1, octet *C2, octet *X, octet *E, MTA_ZKWC_commitment *c, MTA_ZKWC_proof *p);

/** \brief Dump the commitment to octets
 *
 *  @param U           Octet with the commitment for the DLOG ZKP. EGS_SECP256K1 + 1 long
 *  @param Z           Destination Octet for the z component of the commitment. FS_2048 long
 *  @param Z1          Destination Octet for the z1 component of the commitment. FS_2048 long
 *  @param T           Destination Octet for the t component of the commitment. FS_2048 long
 *  @param V           Destination Octet for the v component of the commitment. FS_4096 long
 *  @param W           Destination Octet for the w component of the commitment. FS_2048 long
 *  @param c           Commitment to export
 */
extern void MTA_ZKWC_commitment_toOctets(octet *U, octet *Z, octet *Z1, octet *T, octet *V, octet *W, MTA_ZKWC_commitment *c);

/** \brief Read the commitments from octets
 *
 *  @param c           Destination commitment
 *  @param U           Octet with the commitment for the DLOG ZKP
 *  @param Z           Octet with the z component of the commitment
 *  @param Z1          Octet with the z1 component of the commitment
 *  @param T           Octet with the t component of the commitment
 *  @param V           Octet with the v component of the commitment
 *  @param W           Octet with the w component of the commitment
 *  @return            MTA_INVALID_ECP if U is not a valid ECP, MTA_OK otherwise
 */
extern int MTA_ZKWC_commitment_fromOctets(MTA_ZKWC_commitment *c, octet *U, octet *Z, octet *Z1, octet *T, octet *V, octet *W);

/** \brief Dump the proof to octets
 *
 *  @param S           Destination Octet for the s component of the proof. FS_2048 long
 *  @param S1          Destination Octet for the s1 component of the proof. HFS_2048 long
 *  @param S2          Destination Octet for the s2 component of the proof. FS_2048 + HFS_2048 long
 *  @param T1          Destination Octet for the t1 component of the proof. FS_2048 long
 *  @param T2          Destination Octet for the t2 component of the proof. FS_2048 + HFS_2048 long
 *  @param p           Proof to export
 */
extern void MTA_ZKWC_proof_toOctets(octet *S, octet *S1, octet *S2, octet *T1, octet *T2, MTA_ZKWC_proof *p);

/** \brief Read the proof from octets
 *
 *  @param p           Destination proof
 *  @param S           Octet with the s component of the proof
 *  @param S1          Octet with the s1 component of the proof
 *  @param S2          Octet with the s2 component of the proof
 *  @param T1          Octet with the t1 component of the proof
 *  @param T2          Octet with the t2 component of the proof
 */
extern void MTA_ZKWC_proof_fromOctets(MTA_ZKWC_proof *p, octet *S, octet *S1, octet *S2, octet *T1, octet *T2);

/** \brief Clean the memory containing the random values
 *
 *   @param rv         Random values to clean
 */
extern void MTA_ZKWC_commitment_rv_kill(MTA_ZKWC_commitment_rv *rv);

#ifdef __cplusplus
}
#endif

#endif
