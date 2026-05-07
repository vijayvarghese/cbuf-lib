/**
 * @file cbuf.h
 * @brief Circular buffer (ring buffer) — MISRA C:2012 compliant
 *
 * Single-producer, single-consumer circular buffer.
 * No dynamic memory allocation. No global state.
 * All state lives in the cbuf_t struct owned by the caller.
 */

#ifndef CBUF_H
#define CBUF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CBUF_MAX_SIZE (256U)

/**
 * @brief Circular Buffer control structure 
 */
typedef struct
{
    uint8_t data[CBUF_MAX_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
}cbuf_s;

//Return status Codes
typedef enum
{
    CBUF_OK = 0,
    CBUF_ERR_FULL = 2,
    CBUF_ERR_EMPTY = 3,
    CBUF_ERR_NULL = 4
}cbuf_status_e;


//APIs

/**
 * @brief Initialise a circular buffer. Must be called before any function.
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 * @return CBUF_OK or CBUF_NULL
 */
cbuf_status_e cbuf_init(cbuf_s *cb);

/**
 * @brief Push one byte into the buffer.
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 * @param byte Data to be inserted into the buffer.
 * @return CBUF_OK, CBUF_ERR_FULL, or CBUF_ERR_NULL
 */
cbuf_status_e cbuf_push(cbuf_s *cb, uint8_t byte);

/**
 * @brief Pop one byte out of the buffer
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 * @param byte Out-Pointer where read data to be written. 
 * @return CBUF_OK, CBUF_ERR_EMPTY, or CBUF_ERR_NULL
 */
cbuf_status_e cbuf_pop(cbuf_s *cb, uint8_t *byte);

/**
 * @brief Read the tail without consuming it.
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 * @param byte Out-Pointer where read data to be written.
 * @return CBUF_ERR_EMPTY, 
 */
cbuf_status_e cbuf_peek(const cbuf_s *cb, uint8_t *byte);

/**
 * @brief Return the count if data present in the buffer.
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 */
uint16_t cbuf_count(const cbuf_s *cb);

/**
 * @brief Return true if buffer is empty 
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
 */
bool cbuf_is_empty(const cbuf_s *cb);

/** 
 * @brief Returns true if the buffer is full.
 * @param cb Pointer to caller-owned instance of cbuf_s struct.
*/
bool cbuf_is_full(const cbuf_s *cb);

/**
 * @brief Clear out all the contents.
 */
cbuf_status_e cbuf_flush(cbuf_s *cb);


#endif