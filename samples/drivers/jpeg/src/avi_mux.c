/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file avi_mux.c
 * @brief AVI v1 muxer for MJPEG (single video stream, no audio).
 *
 * Layout produced:
 *
 *   RIFF (size) "AVI "
 *     LIST (size) "hdrl"
 *       avih (56)              MainAVIHeader
 *       LIST (size) "strl"
 *         strh (56)            AVIStreamHeader (vids / MJPG)
 *         strf (40)            BITMAPINFOHEADER (MJPG, 24bpp)
 *     LIST (size) "movi"
 *       00dc (size) <jpeg> [pad]   ... per frame
 *     idx1 (size)
 *       16 B per frame: ckid("00dc"), flags(KEYFRAME), offset, size
 *
 * idx1 offsets are relative to the "movi" FOURCC position — the de-facto
 * convention used by ffmpeg/VLC/MediaPlayer.
 */

#include "avi_mux.h"
#include <string.h>
#include <errno.h>

#define AVIF_HASINDEX     0x00000010U
#define AVIIF_KEYFRAME    0x00000010U

/* ---- little-endian writers --------------------------------------------- */

static inline void w_u16(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
}

static inline void w_u32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

static inline void w_fourcc(uint8_t *p, const char *s)
{
	p[0] = (uint8_t)s[0];
	p[1] = (uint8_t)s[1];
	p[2] = (uint8_t)s[2];
	p[3] = (uint8_t)s[3];
}

/* ---- bounds-checked appenders ------------------------------------------ */

static int put_u16(struct avi_mux *m, uint16_t v)
{
	if (m->pos + 2U > m->cap) {
		return -ENOSPC;
	}
	w_u16(m->buf + m->pos, v);
	m->pos += 2U;
	return 0;
}

static int put_u32(struct avi_mux *m, uint32_t v)
{
	if (m->pos + 4U > m->cap) {
		return -ENOSPC;
	}
	w_u32(m->buf + m->pos, v);
	m->pos += 4U;
	return 0;
}

static int put_fourcc(struct avi_mux *m, const char *s)
{
	if (m->pos + 4U > m->cap) {
		return -ENOSPC;
	}
	w_fourcc(m->buf + m->pos, s);
	m->pos += 4U;
	return 0;
}

static int put_bytes(struct avi_mux *m, const void *data, uint32_t n)
{
	if (m->pos + n > m->cap) {
		return -ENOSPC;
	}
	memcpy(m->buf + m->pos, data, n);
	m->pos += n;
	return 0;
}

static int put_zeros(struct avi_mux *m, uint32_t n)
{
	if (m->pos + n > m->cap) {
		return -ENOSPC;
	}
	memset(m->buf + m->pos, 0, n);
	m->pos += n;
	return 0;
}

/* ---- public API -------------------------------------------------------- */

int avi_mux_init(struct avi_mux *m, uint8_t *buf, uint32_t cap,
		 uint32_t width, uint32_t height, uint32_t fps)
{
	uint32_t hdrl_size_pos;
	uint32_t hdrl_data_pos;
	uint32_t strl_size_pos;
	uint32_t strl_data_pos;
	int ret;

	if (m == NULL || buf == NULL) {
		return -EINVAL;
	}
	if (width == 0U || height == 0U || fps == 0U) {
		return -EINVAL;
	}
	/* Header alone needs ~ 12 + 8 + 4 + 8 + 56 + 8 + 4 + 8 + 56 + 8 + 40 + 12 = 224 B.
	 * Require some breathing room so finalize() can also write idx1.
	 */
	if (cap < 4096U) {
		return -ENOSPC;
	}

	memset(m, 0, sizeof(*m));
	m->buf = buf;
	m->cap = cap;
	m->width = width;
	m->height = height;
	m->fps = fps;

	/* RIFF (size) "AVI " */
	if ((ret = put_fourcc(m, "RIFF"))) {
		return ret;
	}
	m->riff_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {            /* backpatched in finalize */
		return ret;
	}
	if ((ret = put_fourcc(m, "AVI "))) {
		return ret;
	}

	/* LIST (size) "hdrl" */
	if ((ret = put_fourcc(m, "LIST"))) {
		return ret;
	}
	hdrl_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {            /* backpatched at end of init */
		return ret;
	}
	hdrl_data_pos = m->pos;
	if ((ret = put_fourcc(m, "hdrl"))) {
		return ret;
	}

	/* avih chunk: id + size(56) + 56 bytes of MainAVIHeader */
	if ((ret = put_fourcc(m, "avih"))) {
		return ret;
	}
	if ((ret = put_u32(m, 56U))) {
		return ret;
	}
	if ((ret = put_u32(m, 1000000U / fps))) {       /* dwMicroSecPerFrame */
		return ret;
	}
	m->avih_max_bps_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {                   /* dwMaxBytesPerSec   */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwPaddingGranularity */
		return ret;
	}
	if ((ret = put_u32(m, AVIF_HASINDEX))) {        /* dwFlags            */
		return ret;
	}
	m->avih_frames_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {                   /* dwTotalFrames      */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwInitialFrames    */
		return ret;
	}
	if ((ret = put_u32(m, 1U))) {                   /* dwStreams          */
		return ret;
	}
	m->avih_buf_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {                   /* dwSuggestedBufferSize */
		return ret;
	}
	if ((ret = put_u32(m, width))) {                /* dwWidth            */
		return ret;
	}
	if ((ret = put_u32(m, height))) {               /* dwHeight           */
		return ret;
	}
	if ((ret = put_zeros(m, 16U))) {                /* dwReserved[4]      */
		return ret;
	}

	/* LIST (size) "strl" */
	if ((ret = put_fourcc(m, "LIST"))) {
		return ret;
	}
	strl_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {
		return ret;
	}
	strl_data_pos = m->pos;
	if ((ret = put_fourcc(m, "strl"))) {
		return ret;
	}

	/* strh chunk: id + size(56) + 56 bytes of AVIStreamHeader */
	if ((ret = put_fourcc(m, "strh"))) {
		return ret;
	}
	if ((ret = put_u32(m, 56U))) {
		return ret;
	}
	if ((ret = put_fourcc(m, "vids"))) {            /* fccType            */
		return ret;
	}
	if ((ret = put_fourcc(m, "MJPG"))) {            /* fccHandler         */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwFlags            */
		return ret;
	}
	if ((ret = put_u16(m, 0U))) {                   /* wPriority          */
		return ret;
	}
	if ((ret = put_u16(m, 0U))) {                   /* wLanguage          */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwInitialFrames    */
		return ret;
	}
	if ((ret = put_u32(m, 1U))) {                   /* dwScale            */
		return ret;
	}
	if ((ret = put_u32(m, fps))) {                  /* dwRate (= fps)     */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwStart            */
		return ret;
	}
	m->strh_length_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {                   /* dwLength           */
		return ret;
	}
	m->strh_buf_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {                   /* dwSuggestedBufferSize */
		return ret;
	}
	if ((ret = put_u32(m, 0xFFFFFFFFU))) {          /* dwQuality (-1)     */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* dwSampleSize       */
		return ret;
	}
	if ((ret = put_u16(m, 0U))) {                   /* rcFrame.left       */
		return ret;
	}
	if ((ret = put_u16(m, 0U))) {                   /* rcFrame.top        */
		return ret;
	}
	if ((ret = put_u16(m, (uint16_t)width))) {      /* rcFrame.right      */
		return ret;
	}
	if ((ret = put_u16(m, (uint16_t)height))) {     /* rcFrame.bottom     */
		return ret;
	}

	/* strf chunk: id + size(40) + BITMAPINFOHEADER */
	if ((ret = put_fourcc(m, "strf"))) {
		return ret;
	}
	if ((ret = put_u32(m, 40U))) {
		return ret;
	}
	if ((ret = put_u32(m, 40U))) {                  /* biSize             */
		return ret;
	}
	if ((ret = put_u32(m, width))) {                /* biWidth            */
		return ret;
	}
	if ((ret = put_u32(m, height))) {               /* biHeight (positive)*/
		return ret;
	}
	if ((ret = put_u16(m, 1U))) {                   /* biPlanes           */
		return ret;
	}
	if ((ret = put_u16(m, 24U))) {                  /* biBitCount         */
		return ret;
	}
	if ((ret = put_fourcc(m, "MJPG"))) {            /* biCompression      */
		return ret;
	}
	if ((ret = put_u32(m, width * height * 3U))) {  /* biSizeImage        */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* biXPelsPerMeter    */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* biYPelsPerMeter    */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* biClrUsed          */
		return ret;
	}
	if ((ret = put_u32(m, 0U))) {                   /* biClrImportant     */
		return ret;
	}

	/* Backpatch strl & hdrl LIST sizes (size = bytes after the size field). */
	w_u32(m->buf + strl_size_pos, m->pos - strl_data_pos);
	w_u32(m->buf + hdrl_size_pos, m->pos - hdrl_data_pos);

	/* LIST (size) "movi" — frames go inside, finalize patches the size. */
	if ((ret = put_fourcc(m, "LIST"))) {
		return ret;
	}
	m->movi_size_pos = m->pos;
	if ((ret = put_u32(m, 0U))) {
		return ret;
	}
	m->movi_data_pos = m->pos;
	if ((ret = put_fourcc(m, "movi"))) {
		return ret;
	}

	return 0;
}

int avi_mux_write_frame(struct avi_mux *m, const void *jpeg, uint32_t size)
{
	uint32_t pad;
	int ret;

	if (m == NULL || jpeg == NULL || size == 0U) {
		return -EINVAL;
	}
	if (m->frames >= AVI_MUX_MAX_FRAMES) {
		return -ENOSPC;
	}

	pad = size & 1U;                                /* AVI chunks pad to even */

	/* Reserve room for chunk header + payload + pad + future idx1 entry +
	 * idx1 chunk header. The +24 covers (16 B for this entry + 8 B idx1 hdr
	 * incremental) so finalize() cannot fail later for *this* frame.
	 */
	if (m->pos + 8U + size + pad + 16U > m->cap) {
		return -ENOSPC;
	}

	/* Index entry: offset relative to "movi" FOURCC (de-facto convention). */
	m->idx[m->frames].offset = m->pos - m->movi_data_pos;
	m->idx[m->frames].size = size;

	if ((ret = put_fourcc(m, "00dc"))) {
		return ret;
	}
	if ((ret = put_u32(m, size))) {
		return ret;
	}
	if ((ret = put_bytes(m, jpeg, size))) {
		return ret;
	}
	if (pad && (ret = put_zeros(m, pad))) {
		return ret;
	}

	if (size > m->max_chunk) {
		m->max_chunk = size;
	}
	m->frames++;
	return 0;
}

int avi_mux_finalize(struct avi_mux *m)
{
	uint32_t idx_bytes;
	uint32_t total_bytes = 0U;
	uint32_t bps;
	uint32_t suggest;
	int ret;

	if (m == NULL) {
		return -EINVAL;
	}
	if (m->frames == 0U) {
		return -EINVAL;
	}

	/* Patch movi LIST size = bytes after its 4-byte size field. */
	w_u32(m->buf + m->movi_size_pos,
	      m->pos - m->movi_size_pos - 4U);

	/* idx1 chunk: 8-byte header + 16 bytes per entry. */
	idx_bytes = m->frames * 16U;
	if (m->pos + 8U + idx_bytes > m->cap) {
		return -ENOSPC;
	}

	if ((ret = put_fourcc(m, "idx1"))) {
		return ret;
	}
	if ((ret = put_u32(m, idx_bytes))) {
		return ret;
	}
	for (uint32_t i = 0U; i < m->frames; i++) {
		if ((ret = put_fourcc(m, "00dc"))) {
			return ret;
		}
		if ((ret = put_u32(m, AVIIF_KEYFRAME))) {
			return ret;
		}
		if ((ret = put_u32(m, m->idx[i].offset))) {
			return ret;
		}
		if ((ret = put_u32(m, m->idx[i].size))) {
			return ret;
		}
		total_bytes += m->idx[i].size;
	}

	/* RIFF size = total file size minus the 8-byte ("RIFF" + size) prefix. */
	w_u32(m->buf + m->riff_size_pos, m->pos - 8U);

	/* Frame counts in avih and strh. */
	w_u32(m->buf + m->avih_frames_pos, m->frames);
	w_u32(m->buf + m->strh_length_pos, m->frames);

	/* Hints (max bytes/sec, suggested buffer size). */
	bps = (total_bytes / m->frames) * m->fps;
	suggest = m->max_chunk + 1024U;
	w_u32(m->buf + m->avih_max_bps_pos, bps);
	w_u32(m->buf + m->avih_buf_size_pos, suggest);
	w_u32(m->buf + m->strh_buf_size_pos, suggest);

	return 0;
}
