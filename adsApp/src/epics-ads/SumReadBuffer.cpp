// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <stdexcept>
#include <cstdio>
#include <cstring>

#include "err.h"
#include "Variable.h"
#include "SumReadBuffer.h"

unsigned int SumReadBuffer::get_num_variables() {
    return this->variables.size();
}

unsigned int SumReadBuffer::get_max_variables() { return this->max_nvariables; }

size_t SumReadBuffer::get_max_data_size_soft_limit() {
    return this->max_data_size_soft_limit;
}

size_t SumReadBuffer::get_size() { return this->buffer_size; }

bool SumReadBuffer::is_initialized() { return this->buffer_initialized; }

bool SumReadBuffer::at_capacity() {
    return (this->data_size() >= this->get_max_data_size_soft_limit()) ||
           (this->next_result_offset >= this->max_nvariables);
}

uint8_t *SumReadBuffer::get_buffer() { return this->buffer; }

uint8_t *SumReadBuffer::get_buffer_data_section() {
    return this->buffer + this->start_of_data_offset();
}

SumReadBuffer::SumReadBuffer(const uint16_t max_nvariables)
    : rwlock(RWLock::prefer_writer) {
    if (max_nvariables == 0) {
        throw std::invalid_argument("max_nvariables must be larger that zero");
    }

    this->max_nvariables = max_nvariables;
}

SumReadBuffer::SumReadBuffer(const uint16_t max_nvariables,
                             const size_t data_size_soft_limit)
    : SumReadBuffer(max_nvariables) {
    if (data_size_soft_limit == 0) {
        throw std::invalid_argument(
            "data_size_soft_limit must be larger that zero");
    }

    this->max_data_size_soft_limit = data_size_soft_limit;
}

SumReadBuffer::~SumReadBuffer() {
    if (this->prev_data_buffer != nullptr) {
        free(this->prev_data_buffer);
        this->prev_data_buffer = nullptr;
    }

    if (this->buffer != nullptr) {
        free(this->buffer);
        this->buffer = nullptr;
        this->buffer_initialized = false;
    }
}

int SumReadBuffer::add_variable(std::shared_ptr<ADSVariable> variable) {
    if (this->is_initialized() == true) {
        return EPICSADS_NOT_INITIALIZED;
    }

    if (this->get_num_variables() == this->max_nvariables) {
        return EPICSADS_LIMIT;
    }

    if (this->at_capacity() == true) {
        return EPICSADS_LIMIT;
    }

    /* Check if adding the variable would make the buffer too large */
    size_t new_size = this->buffer_size + variable->size() + this->result_size;
    if (new_size < this->buffer_size) {
        return EPICSADS_OVERFLOW;
    }

    /* Store variable in a vector */
    try {
        this->variables.push_back(variable);
    } catch (const std::exception &ex) {
        LOG_ERR("vector.push_back() failed: %s", ex.what());
        return EPICSADS_ERROR;
    }

    /* Store information about where the variables' corresponding data can be
     * found in the buffer */
    BufferDataPosition bdp = {this, this->next_result_offset,
                              this->next_data_offset};
    variable->set_buffer_reader(bdp);

    this->next_result_offset++;
    this->next_data_offset += variable->size();
    this->buffer_size = new_size;

    return 0;
}

int SumReadBuffer::read_data(uint16_t offset_result, size_t offset_data,
                             const size_t data_size, uint32_t *result,
                             char *data) {
    if (this->buffer == nullptr) {
        return EPICSADS_NOT_INITIALIZED;
    } else if (result == nullptr || data == nullptr) {
        return EPICSADS_INV_PARAM;
    } else if (offset_result >= this->next_result_offset) {
        return EPICSADS_OUT_OF_RANGE;
    } else if ((offset_data + data_size) > this->next_data_offset) {
        return EPICSADS_OUT_OF_RANGE;
    }

    this->rwlock.lock_read();

    /* Return codes block is stored at the beginning of the buffer. */
    uint32_t *buffer_rc = ((uint32_t *)this->buffer) + offset_result;
    *result = *buffer_rc;

    /* Data is stored in the buffer after the return codes block. */
    uint8_t *buffer_data =
        this->buffer + this->start_of_data_offset() + offset_data;
    memcpy(data, buffer_data, data_size);

    this->rwlock.unlock_read();

    return 0;
}

int SumReadBuffer::initialize_buffer() {
    if (this->get_num_variables() == 0) {
        return EPICSADS_NO_DATA;
    }

    if (this->buffer_initialized == true) {
        return EPICSADS_INV_CALL;
    }

    this->buffer = (uint8_t *)calloc(this->buffer_size, sizeof(uint8_t));
    if (this->buffer == nullptr) {
        return EPICSADS_ERROR;
    }

    this->prev_data_buffer =
        (uint8_t *)calloc(this->buffer_size, sizeof(uint8_t));
    // TODO: cheap optimization is to not allocate it when there aren't any
    // notify-on-update variables
    if (this->prev_data_buffer == nullptr) {
        free(this->buffer);
        this->buffer = nullptr;
        return EPICSADS_ERROR;
    }

    this->buffer_initialized = true;

    return 0;
}

void SumReadBuffer::save_buffer() {
    if (this->prev_data_buffer == nullptr) {
        return;
    }

    this->rwlock.lock_write();
    memcpy(this->prev_data_buffer, this->buffer, this->buffer_size);
    this->rwlock.unlock_write();
}

size_t SumReadBuffer::results_size() {
    return this->get_num_variables() * this->result_size;
}

size_t SumReadBuffer::data_size() {
    return this->buffer_size - this->results_size();
}

size_t SumReadBuffer::start_of_data_offset() { return this->results_size(); }
