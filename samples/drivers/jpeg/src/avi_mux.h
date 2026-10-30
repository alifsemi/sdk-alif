/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file avi_mux.h
 * @brief Tiny in-RAM AVI v1 muxer for MJPEG streams.
 *
 * Builds a Microsoft AVI file (RIFF / hdrl / strl(vids,MJPG) / movi / idx1)
 * directly into a caller-provided buffer. Single video stream, no audio.
 *
 * Usage:
 *   uint8_t *buf = ...; uint32_t cap = ...;
 *   struct avi_mux m;
 *   avi_mux_init(&m, buf, cap, 1280, 720, 30);
 *   for each JPEG frame:
 *       avi_mux_write_frame(&m, jpeg_bytes, jpeg_size);
 *   avi_mux_finalize(&m);    // file is now buf[0..m.pos)
 *
 * Notes:
 * - JPEG bytes passed to write_frame must be a complete Baseline JPEG
 *   (FF D8 ... FF D9), trimmed of trailing padding.
 * - File grows monotonically; nothing is ever moved or rewritten except
 *   four 32-bit size fields backpatched in finalize().
 */

#ifndef AVI_MUX_H_
#define AVI_MUX_H_

#include <stdint.h>
#include <stddef.h>

/** Hard cap on indexed frames; bumps the idx[] table footprint by 8 B each. */
#define AVI_MUX_MAX_FRAMES   32

struct avi_mux {
	uint8_t *buf;                 /**< output buffer base                  */
	uint32_t cap;                 /**< output buffer capacity in bytes     */
	uint32_t pos;                 /**< current write offset                */

	/* Backpatch positions captured during init/write. */
	uint32_t riff_size_pos;       /**< RIFF chunk size field offset        */
	uint32_t movi_size_pos;       /**< 'movi' LIST size field offset       */
	uint32_t movi_data_pos;       /**< offset of "movi" FOURCC (idx anchor)*/
	uint32_t avih_frames_pos;     /**< avih.dwTotalFrames offset           */
	uint32_t avih_max_bps_pos;    /**< avih.dwMaxBytesPerSec offset        */
	uint32_t avih_buf_size_pos;   /**< avih.dwSuggestedBufferSize offset   */
	uint32_t strh_length_pos;     /**< strh.dwLength offset                */
	uint32_t strh_buf_size_pos;   /**< strh.dwSuggestedBufferSize offset   */

	uint32_t frames;              /**< frames written so far               */
	uint32_t width, height, fps;
	uint32_t max_chunk;           /**< largest frame seen (for hints)      */

	struct {
		uint32_t offset;      /**< offset relative to "movi" FOURCC    */
		uint32_t size;        /**< frame payload size (no chunk hdr)   */
	} idx[AVI_MUX_MAX_FRAMES];
};

/**
 * @brief Initialise the muxer and write the AVI header (RIFF + hdrl + movi).
 * @return 0 on success, -EINVAL on bad args, -ENOSPC if @p cap is too small.
 */
int avi_mux_init(struct avi_mux *m, uint8_t *buf, uint32_t cap,
		 uint32_t width, uint32_t height, uint32_t fps);

/**
 * @brief Append one JPEG frame as a `00dc` chunk and record its index entry.
 * @return 0 on success, -ENOSPC if buffer or idx table is full, -EINVAL on bad args.
 */
int avi_mux_write_frame(struct avi_mux *m, const void *jpeg, uint32_t size);

/**
 * @brief Append the `idx1` chunk and backpatch RIFF/movi/frame-count fields.
 *        After this returns, @p m->buf [0 .. m->pos) is the complete AVI file.
 */
int avi_mux_finalize(struct avi_mux *m);

#endif /* AVI_MUX_H_ */
