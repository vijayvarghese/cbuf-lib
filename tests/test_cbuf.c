/**
 * @file test_cbuf.c
 * @brief Unity test suite for cbuf library.
 */


#include "unity.h"
#include "cbuf.h"



static cbuf_s buf;

void setUp(void)
{
    (void)cbuf_init(&buf);
}


void tearDown(void)
{}



void test_cbuf_init_withValidPointer_returnOK(void)
{
    cbuf_s local;
    TEST_ASSERT_EQUAL(CBUF_OK, cbuf_init(&local));
}

void test_cbuf_init_withNULLPointer_CBUF_ERR_NULL(void)
{
    TEST_ASSERT_EQUAL(CBUF_ERR_NULL, cbuf_init(NULL));
}

void test_cbuf_is_empty_freshbuffer_true(void)
{
    TEST_ASSERT_TRUE(cbuf_is_empty(&buf));
    TEST_ASSERT_EQUAL(0U,cbuf_count(&buf));
}



/* ============================================================
 * cbuf_push tests
 * ========================================================== */

void test_cbuf_push_singleByte_countIsOne(void)
{
    TEST_ASSERT_EQUAL(CBUF_OK, cbuf_push(&buf, 0xABU));
    TEST_ASSERT_EQUAL(1U, cbuf_count(&buf));
}

void test_cbuf_push_whenFull_returnsErrFull(void)
{
    uint16_t i;
    for (i = 0U; i < (uint16_t)CBUF_MAX_SIZE; i++)
    {
        (void)cbuf_push(&buf, (uint8_t)i);
    }

    TEST_ASSERT_TRUE(cbuf_is_full(&buf));
    TEST_ASSERT_EQUAL(CBUF_ERR_FULL, cbuf_push(&buf, 0xFFU));
}

void test_cbuf_push_withNull_returnsErrNull(void)
{
    TEST_ASSERT_EQUAL(CBUF_ERR_NULL, cbuf_push(NULL, 0x00U));
}

/* ============================================================
 * cbuf_pop tests
 * ========================================================== */

void test_cbuf_pop_afterPush_returnsSameByte(void)
{
    uint8_t out = 0U;
    (void)cbuf_push(&buf, 0x42U);
    TEST_ASSERT_EQUAL(CBUF_OK, cbuf_pop(&buf, &out));
    TEST_ASSERT_EQUAL_HEX8(0x42U, out);
}

void test_cbuf_pop_emptyBuffer_returnsErrEmpty(void)
{
    uint8_t out = 0U;
    TEST_ASSERT_EQUAL(CBUF_ERR_EMPTY, cbuf_pop(&buf, &out));
}

void test_cbuf_pop_withNullBuffer_returnsErrNull(void)
{
    uint8_t out = 0U;
    TEST_ASSERT_EQUAL(CBUF_ERR_NULL, cbuf_pop(NULL, &out));
}

void test_cbuf_pop_withNullOutput_returnsErrNull(void)
{
    (void)cbuf_push(&buf, 0x01U);
    TEST_ASSERT_EQUAL(CBUF_ERR_NULL, cbuf_pop(&buf, NULL));
}

/* ============================================================
 * FIFO order test — critical correctness check
 * ========================================================== */

void test_cbuf_pushPop_multipleBytes_maintainsFifoOrder(void)
{
    uint8_t out = 0U;
    uint8_t i;

    for (i = 0U; i < 5U; i++)
    {
        (void)cbuf_push(&buf, i);
    }

    for (i = 0U; i < 5U; i++)
    {
        (void)cbuf_pop(&buf, &out);
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
}

/* ============================================================
 * Wrap-around test — the one that catches broken implementations
 * ========================================================== */

void test_cbuf_wrapAround_indicesWrapCorrectly(void)
{
    uint8_t out   = 0U;
    uint16_t i    = 0U;

    /* Fill the buffer completely */
    for (i = 0U; i < (uint16_t)CBUF_MAX_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(CBUF_OK, cbuf_push(&buf, (uint8_t)i));//Edited
    }

    /* Drain half */
    for (i = 0U; i < ((uint16_t)CBUF_MAX_SIZE / 2U); i++)
    {
        TEST_ASSERT_EQUAL(CBUF_OK, cbuf_pop(&buf, &out));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, out);
    }

    /* Refill the now-free half — this forces index wrap-around */
    for (i = 0U; i < ((uint16_t)CBUF_MAX_SIZE / 2U); i++)
    {
        TEST_ASSERT_EQUAL(CBUF_OK, cbuf_push(&buf, (uint8_t)(i + 0x80U)));
    }

    /* Drain the second original half */
    for (i = (uint16_t)CBUF_MAX_SIZE / 2U; i < (uint16_t)CBUF_MAX_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(CBUF_OK, cbuf_pop(&buf, &out));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, out);
    }

    /* Drain the wrapped-around new data */
    for (i = 0U; i < ((uint16_t)CBUF_MAX_SIZE / 2U); i++)
    {
        TEST_ASSERT_EQUAL(CBUF_OK, cbuf_pop(&buf, &out));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i + 0x80U), out);
    }

    TEST_ASSERT_TRUE(cbuf_is_empty(&buf));
}

/* ============================================================
 * cbuf_peek tests
 * ========================================================== */

void test_cbuf_peek_doesNotConsumeData(void)
{
    uint8_t out = 0U;
    (void)cbuf_push(&buf, 0x55U);

    TEST_ASSERT_EQUAL(CBUF_OK, cbuf_peek(&buf, &out));
    TEST_ASSERT_EQUAL_HEX8(0x55U, out);
    TEST_ASSERT_EQUAL(1U, cbuf_count(&buf));  /* count unchanged */
}

/* ============================================================
 * cbuf_flush tests
 * ========================================================== */

void test_cbuf_flush_clearsBuffer(void)
{
    (void)cbuf_push(&buf, 0x01U);
    (void)cbuf_push(&buf, 0x02U);

    TEST_ASSERT_EQUAL(CBUF_OK, cbuf_flush(&buf));
    TEST_ASSERT_TRUE(cbuf_is_empty(&buf));
    TEST_ASSERT_EQUAL(0U, cbuf_count(&buf));
}

/* ============================================================
 * Unity main — required entry point
 * ========================================================== */


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_cbuf_init_withNULLPointer_CBUF_ERR_NULL);
    RUN_TEST(test_cbuf_init_withValidPointer_returnOK);
    RUN_TEST(test_cbuf_is_empty_freshbuffer_true);


    RUN_TEST(test_cbuf_push_singleByte_countIsOne);
    RUN_TEST(test_cbuf_push_whenFull_returnsErrFull);
    RUN_TEST(test_cbuf_push_withNull_returnsErrNull);

    RUN_TEST(test_cbuf_pop_afterPush_returnsSameByte);
    RUN_TEST(test_cbuf_pop_emptyBuffer_returnsErrEmpty);
    RUN_TEST(test_cbuf_pop_withNullBuffer_returnsErrNull);
    RUN_TEST(test_cbuf_pop_withNullOutput_returnsErrNull);

    RUN_TEST(test_cbuf_pushPop_multipleBytes_maintainsFifoOrder);
    RUN_TEST(test_cbuf_wrapAround_indicesWrapCorrectly);

    RUN_TEST(test_cbuf_peek_doesNotConsumeData);
    RUN_TEST(test_cbuf_flush_clearsBuffer);

    return UNITY_END();
}