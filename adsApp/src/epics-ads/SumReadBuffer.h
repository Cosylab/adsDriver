// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef SUMREADBUFFER_H
#define SUMREADBUFFER_H

#include <cstdint>
#include <memory>
// #include "ADSPortDriver.h"
#include "RWLock.h"
#include "autoparamHandler.h"

/* Defined in Variable.h, which includes this header file (prevent cyclic
 * include). */
class ADSVariable;

class SumReadBuffer {
  protected:
    /* Variables added to this buffer */
    std::vector<std::shared_ptr<ADSVariable>> variables;

    uint16_t max_nvariables = 0; /* Maximum allowed number of variables */
    size_t max_data_size_soft_limit =
        1000000; /* Maximum soft limit for data segment size */
    uint16_t next_result_offset =
        0; /* Buffer index for the next added result */
    size_t next_data_offset =
        0; /* Buffer byte offset from the beginning of the data block */
    size_t buffer_size = 0; /* Buffer size in bytes (results and data blocks) */

    /* Buffer contains result code and data elements. Result codes are stored
     * at the beginning of the buffer and are all of the same size. Data
     * elements are stored after the results block and are of varying size. */
    uint8_t *buffer = nullptr;
    bool buffer_initialized = false;

    uint8_t *prev_data_buffer = nullptr;

    size_t results_size();         /* Size of results in bytes */
    size_t data_size();            /* Size of data in bytes */
    size_t start_of_data_offset(); /* Byte offset where data elements are stored
                                      in buffer */

  public:
    enum class SumReadBufferState {
        Valid,
        Invalid
    } buffer_state = SumReadBufferState::Invalid;
    SumReadBuffer(const uint16_t max_nvariables);
    SumReadBuffer(const uint16_t max_nvariables,
                  const size_t data_size_soft_limit);
    ~SumReadBuffer();

    /* R/W lock should be used when accessing the underlying data buffer through
     * get_buffer() and get_data_buffer_data_section(). */
    RWLock rwlock;

    static const size_t result_size = sizeof(uint32_t);

    unsigned int get_num_variables(); /* Number of variables added */
    unsigned int get_max_variables(); /* Maximum number of variables allowd */
    size_t get_max_data_size_soft_limit(); /* Soft limit for maximum data
                                              segment size in bytes */
    size_t get_size();                     /* Buffer size in bytes */
    bool is_initialized();                 /* True if buffer is initialized */
    bool at_capacity(); /* True if no more variables can be added to buffer */

    /* Pointer to underlying buffer storage. Code performing ADS sum read
     * writes to this buffer.
     *
     * The caller should use the `rwlock` property to acquire a read/write lock
     * before attempting to manipulate the buffer and release the lock when
     * done. */
    uint8_t *get_buffer();

    /* Pointer to underlaying buffer storage segment containing data, skipping
     * over the results section.
     *
     * The caller should use the `rwlock` property to acquire a read/write lock
     * before attempting to manipulate the buffer and release the lock when
     * done. */
    uint8_t *get_buffer_data_section();

    /* Reserve storage in the buffer for a variable of DATA_SIZE bytes. On
     * success, OFFSET_RESULT points to ADS read operation result value for
     * the variable and OFFSET_DATA points to the read data.
     * Variables can only be added while the buffer is _not_ initialized. */
    int add_variable(std::shared_ptr<ADSVariable> variable);

    /* Read (PLC variable) data from buffer into DATA. OFFSET_RESULT,
     * OFFSET_DATA and DATA_SIZE must match the values obtained/passed to the
     * previous call to add_variable().
     *
     * This method implicitly acquires read lock before reading data from the
     * underlying buffer. */
    int read_data(uint16_t offset_result, size_t offset_data,
                  const size_t data_size, uint32_t *result, char *data);

    /* Allocate storage for the buffer. After initialization, no more variables
     * can be added to the buffer. */
    int initialize_buffer();

    /* Saves a copy of the current buffer content into `prev_data_buffer`. The
     * copy is used by get_updated_variables() to find variables whose values
     * have changed after an ADS sum-read.
     *
     * This method implicitly acquires write lock before copying the buffer. */
    void save_buffer();

    // for autoparam use
    // std::vector<std::shared_ptr<Variable>> get_updated_variables(
    //     std::vector<Autoparam::DeviceVariable *> &interruptVars);
};
#endif /* SUMREADBUFFER_H */
