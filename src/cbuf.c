/**
 * @file cbuf.c

 */

#include "cbuf.h"
#include <string.h>

#define CBUF_MAX_SIZE (256U)


cbuf_status_e cbuf_init(cbuf_s *cb)
{
    cbuf_status_e status;

    if (cb == NULL)
    {
        status = CBUF_ERR_NULL;
    }
    else
    {
        (void)memset(cb->data,0U,sizeof(cb->data));
        cb->count = 0U;
        cb->head = 0U;
        cb->tail = 0U;
        status = CBUF_OK;

    }
    return status;
}

cbuf_status_e cbuf_push(cbuf_s *cb, uint8_t byte)
{
    cbuf_status_e status;
    if (cb == NULL)
    {
        status = CBUF_ERR_NULL;
    }
    else if (cb->count >= (uint16_t)CBUF_MAX_SIZE)
    {
        status = CBUF_ERR_FULL;
    }
    else
    {
        cb->data[cb->head] = byte;
        cb->head = (uint16_t)((cb->head + 1U) % (uint16_t)CBUF_MAX_SIZE);
        cb->count++;
        status = CBUF_OK;
    }
    return status;
} 

cbuf_status_e cbuf_pop(cbuf_s *cb, uint8_t *byte)
{
    cbuf_status_e status;
    if((cb == NULL) || (byte == NULL))
    {
        status = CBUF_ERR_NULL;
    }
    else if(cb->count == 0U)
    {
        status = CBUF_ERR_EMPTY;
    }
    else
    {
        *byte = cb->data[cb->tail];
        cb->tail = (uint16_t)((cb->tail + 1U) % (uint16_t)CBUF_MAX_SIZE);
        cb->count--;
        status = CBUF_OK;
    }
    return status;
}

cbuf_status_e cbuf_peek(const cbuf_s *cb, uint8_t *byte)
{
    cbuf_status_e status;
    if((cb == NULL) || (byte == NULL))
    {
        status = CBUF_ERR_NULL;
    }
    else if(cb->count == 0U)
    {
        status = CBUF_ERR_EMPTY;
    }
    else
    {
        *byte = cb->data[cb->tail];
        status = CBUF_OK;
    }
    return status;
}

bool cbuf_is_full(const cbuf_s *cb)
{
    bool status = false;
    if (cb != NULL)
    {
        status = (cb->count == (uint16_t)CBUF_MAX_SIZE);
    }
    return status;
}

bool cbuf_is_empty(const cbuf_s *cb)
{
    bool status = true;
    if (cb != NULL)
    {
        status = (cb->count == 0U);
    }
    return status;
}

uint16_t cbuf_count(const cbuf_s *cb)
{
    uint16_t result = 0U;
    if (cb != NULL)
    {
        result = cb->count;
    }
    return result;
}

cbuf_status_e cbuf_flush(cbuf_s *cb)
{
    return cbuf_init(cb);
}