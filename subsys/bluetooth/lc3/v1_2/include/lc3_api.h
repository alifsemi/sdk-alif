/*
 * Copyright (c) 2024 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _LC3_API_H
#define _LC3_API_H

#include <stdint.h>
#include <stddef.h>
#include "compiler.h"

/* Sizes fixed by internal implementation of LC3 codec */
#define LC3_CFG_STRUCT_SIZE     2012
#define LC3_ENCODER_STRUCT_SIZE 1576
#define LC3_DECODER_STRUCT_SIZE 132

typedef struct {
	uint8_t bytes[LC3_CFG_STRUCT_SIZE];
} lc3_cfg_t __ALIGN(4);

typedef struct {
	uint8_t bytes[LC3_ENCODER_STRUCT_SIZE];
} lc3_encoder_t __ALIGN(4);

typedef struct {
	uint8_t bytes[LC3_DECODER_STRUCT_SIZE];
} lc3_decoder_t __ALIGN(4);

typedef enum {
	FRAME_DURATION_7_5_MS = 750,
	FRAME_DURATION_10_MS = 1000
} lc3_frame_duration_t;

typedef enum lc3_init_err_code {
	LC3_INIT_ERR_NONE,
	LC3_INIT_ERR_PATCH,
} lc3_init_err_code_t;

/**
 * @brief Initialise the LC3 codec in ROM. This function must be called before any other LC3 codec
 *	  functions from ROM are used.
 *
 *	  Note: When there is an error in patching the ROM the API returns LC3_INIT_ERR_PATCH.
 *		In this specific error scenario the API still initializes the codec without
 *		applying the ROM patch. This means that an application can still continue using
 *		the underlying LC3 codec APIs even if the ROM patching failed.
 *
 * @return LC3_INIT_ERR_NONE  On success
 *	   LC3_INIT_ERR_PATCH Pathing LC3 ROM failed but stack is initialized and APIs still usable
 */
lc3_init_err_code_t lc3_api_rom_init(void const *patch);

/**
 * @brief Configure an instance of the LC3 codec
 *
 * @param cfg	   Instance of the lc3_cfg_t struct to be populated
 * @param fs	   Sampling rate in Hz, must be one of supported values:
 *		   (8000, 16000, 24000, 32000, 44100, 48000)
 * @param duration Frame duration
 *
 * @return 0 on success, or negative error code
 */
int lc3_api_configure(lc3_cfg_t *cfg, int32_t fs, lc3_frame_duration_t duration);

/**
 * @brief Intitialise an LC3 codec encoder instance using the provided configuration
 *
 * @param cfg	  The codec configuration previously configured with a call to lc3_api_configure
 * @param encoder The encoder instance to be populated
 *
 * @return 0 on success, or negative error code
 */
int lc3_api_initialise_encoder(lc3_cfg_t *cfg, lc3_encoder_t *encoder);

/**
 * @brief Get the required size of the encoder output
 *
 * @param bitrate  Bit rate in Hz
 * @param fs	   Sampling frequency in Hz
 * @param duration Frame duration
 */
uint16_t lc3_api_get_byte_count(uint32_t bitrate, int fs, lc3_frame_duration_t duration);

/**
 * @brief Get required scratch memory size for encoder
 *
 * @param cfg The codec configuration previously configured with a call to lc3_api_configure
 *
 * @return The required size in bytes of the scratch memory
 */
size_t lc3_api_encoder_scratch_size(lc3_cfg_t *cfg);

/**
 * @brief Encode a single frame using the LC3 encoder
 *
 * @param cfg	      Configuration of the codec
 * @param encoder     Encoder instance to use
 * @param input	      Pointer to the input audio PCM samples (note this is overwritten by routine)
 * @param output      Pointer to location to store the output compressed bit stream
 * @param output_len  Length of the output compressed bit stream (ie. length of output memory).
 *		      This depends on the bitrate, sample rate and frame size and can be calculated
 *		      using lc3_api_get_byte_count
 * @param scratch_mem Pointer to a memory area used to store temporary data. The size of
 *		      scratch_mem can be determined using lc3_api_encoder_scratch_size.
 *		      The scratch memory can be shared by multiple encoders provided that they are
 *		      not run concurrently.
 */
int lc3_api_encode_frame(lc3_cfg_t *cfg, lc3_encoder_t *encoder, int16_t *input, uint8_t *output,
			 uint16_t output_len, int32_t *scratch_mem);

/**
 * @brief Get the required size of the decoder status memory area
 *
 * @param cfg The codec configuration previously configured with a call to lc3_api_configure
 *
 * @return The required size in bytes of the status memory
 */
size_t lc3_api_decoder_status_size(lc3_cfg_t *cfg);

/**
 * @brief Intitialise an LC3 codec decoder instance using the provided configuration
 *
 * @param cfg	     The codec configuration previously configured with a call to lc3_api_configure
 * @param encoder    The decoder instance to be populated
 * @param status_mem Pointer to an area of memory that the decoder can use to store static data.
 *		     The size of this memory area can be determined using
 *		     lc3_api_decoder_status_size and it must be 4-byte aligned.
 *
 * @return 0 on success, or negative error code
 */
int lc3_api_initialise_decoder(lc3_cfg_t *cfg, lc3_decoder_t *decoder, int32_t *status_mem);

/**
 * @brief Get required scratch memory size for decoder
 *
 * @param cfg The codec configuration previously configured with a call to lc3_api_configure
 *
 * @return The required size in bytes of the scratch memory
 */
size_t lc3_api_decoder_scratch_size(lc3_cfg_t *cfg);

/**
 * @brief Decode a single frame using the LC3 decoder
 *
 * @param cfg		Configuration of the codec
 * @param decoder	Decoder instance to use
 * @param input		Pointer to input compressed bit stream
 * @param input_len	Length of the input compressed bit stream.
 *			This value can be calculated using lc3_api_get_byte_count
 * @param bad_frame	Usually set to zero, or if input frame is lost/corrupted set to non-zero
 *			value
 * @param bec_detect	Populated by the routine with 1 if a corrupted input frame is detected
 * @param output	Pointer to location to store the output PCM audio data
 * @param scratch_mem	Pointer to a memory are used to store temporary data. The size of
 *			scratch_mem can be determined using lc3_api_decoder_scratch_size. The
 *			scratch memory can be shared by multiple decoders provided that they are not
 *			run concurrently.
 */
int lc3_api_decode_frame(lc3_cfg_t *cfg, lc3_decoder_t *decoder, const uint8_t *input,
			 int input_len, uint8_t bad_frame, uint8_t *bec_detect, int16_t *output,
			 int32_t *scratch_mem);

#endif /* _LC3_API_H */
