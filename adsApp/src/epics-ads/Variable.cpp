// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <cstring>
#include <memory>
#include <mutex>
#include "err.h"
#include "Variable.h"
#include "Connection.h"
#include "Types.h"

uint32_t ADSVariable::size() {
    return this->elem_size * this->addr->get_nelem();
}

void ADSVariable::set_connection(std::shared_ptr<Connection> connection) {
    this->conn = connection;
}

std::shared_ptr<Connection> ADSVariable::get_connection() { return this->conn; }

void ADSVariable::set_buffer_reader(BufferDataPosition br) {
    this->buffer_reader = br;
}

BufferDataPosition ADSVariable::get_buffer_reader() {
    return this->buffer_reader;
}

bool ADSVariable::uses_write_readback() { return this->write_readback; }

void ADSVariable::set_write_readback(const bool readback) {
    this->write_readback = readback;
}

bool ADSVariable::updateDataHash(int new_hash) {
    bool changed = new_hash != array_data_hash;
    array_data_hash = new_hash;
    return changed;
}

ADSVariable::ADSVariable(std::shared_ptr<ADSAddress> address) : addr(address) {
    this->elem_size = ads_datatype_sizes.at(this->addr->get_data_type());
}

int ADSVariable::read_from_buffer(const uint32_t size, char *buffer) {
    if (this->buffer_reader == EMPTY_BUFFER_DATA_POSITION) {
        return EPICSADS_NO_DATA;
    }

    if (this->buffer_reader.buffer == nullptr) {
        // TODO: This shouldn't happen
        return EPICSADS_ERROR;
    }

    if (this->buffer_reader.buffer->buffer_state ==
        SumReadBuffer::SumReadBufferState::Invalid) {
        return EPICSADS_DISCONNECTED;
    }

    /* Number of bytes to read is set to whichever is smaller (bytewise). If it
     * is the PLC variable, the driver shouldn't read past it. If data buffer is
     * smaller, the driver shouldn't write past it. */
    uint32_t bytes_to_read = std::min(size, this->size());
    uint32_t read_result = 0;

    /* Read latest data and read result from buffer */
    int rc = buffer_reader.buffer->read_data(
        buffer_reader.off_result, buffer_reader.off_data, bytes_to_read,
        &read_result, buffer);

    /* Return error if sum-read operation wasn't successul */
    if (rc != 0) {
        return ads_rc_to_epicsads_error(rc);
    }

    /* Return error if the specific variable could not be retrieved (e.g. due
     * to wrong index group), although the sum-read operation was successful*/
    if (read_result != 0) {
        LOG_WARN("variable '%s' could not be read (%u): %s",
                 this->addr->info().c_str(), read_result,
                 errorMap[read_result].c_str());
        return EPICSADS_NO_DATA;
    }

    /* Return the written value for the first read after a write to avoid
     * jumping between "new value" -> "old value" -> "new value" */
    if (write_readback && !last_written.empty()) {
        memcpy(buffer, last_written.data(), last_written.size());
        last_written.clear();
    }

    return 0;
}

std::pair<uint8_t *, RWLock *> ADSVariable::get_read_data() {
    if (this->buffer_reader == EMPTY_BUFFER_DATA_POSITION) {
        return {nullptr, nullptr};
    }

    if (this->buffer_reader.buffer == nullptr) {
        // TODO: shouldn't happen
        return {nullptr, nullptr};
    }

    if (this->buffer_reader.buffer->buffer_state ==
        SumReadBuffer::SumReadBufferState::Invalid) {
        return {nullptr, nullptr};
    }

    SumReadBuffer *buffer = this->buffer_reader.buffer;
    RWLock *rwlock = &buffer->rwlock;
    rwlock->lock_read();
    uint8_t *data = buffer->get_buffer_data_section();
    data += this->buffer_reader.off_data;

    return {data, rwlock};
}

int ADSVariable::write(const char *data, const uint32_t size) {
    if (data == nullptr) {
        return EPICSADS_INV_PARAM;
    }

    if (this->conn == nullptr) {
        return EPICSADS_NOT_INITIALIZED;
    }

    if (this->addr->is_resolved() == false) {
        return EPICSADS_NOT_RESOLVED;
    }

    if (size > this->size()) {
        return EPICSADS_OVERFLOW;
    }

    std::vector<uint8_t> buffer(this->size(), 0);
    memcpy(buffer.data(), data, size);

    uint32_t bytes_to_write = this->size();

    std::lock_guard<epicsMutex> lock(this->conn->mtx);

    AmsAddr remote_ams_addr = {this->conn->get_remote_ams_netid(),
                                      this->addr->get_ads_port()};

    long rc = AdsSyncWriteReqEx(this->conn->get_ads_port(),     // ADS port
                                &remote_ams_addr,               // AMS address
                                this->addr->get_index_group(),  // index group
                                this->addr->get_index_offset(), // index offset
                                bytes_to_write,                 // data length
                                buffer.data());                 // data buffer
    if (rc != 0) {
        return ads_rc_to_epicsads_error(rc);
    }

    if (write_readback) {
        last_written = buffer;
    }

    return 0;
}

int ADSVariable::read(uint8_t *data, const uint32_t size,
                      uint32_t *bytes_read) {
    if (data == nullptr) {
        return EPICSADS_INV_PARAM;
    }

    if (this->conn == nullptr) {
        return EPICSADS_NOT_INITIALIZED;
    }

    if (this->addr->is_resolved() == false) {
        return EPICSADS_NOT_RESOLVED;
    }

    /* Number of bytes to read is set to whichever is smaller (bytewise). If it
     * is the PLC variable, the driver shouldn't read past it. If data buffer is
     * smaller, the driver shouldn't write past it. */
    uint32_t bytes_to_read = std::min(size, this->size());

    std::lock_guard<epicsMutex> lock(this->conn->mtx);

    AmsAddr remote_ams_addr = {this->conn->get_remote_ams_netid(),
                                      this->addr->get_ads_port()};


    long rc = AdsSyncReadReqEx2(this->conn->get_ads_port(),     // ADS port
                                &remote_ams_addr,               // AMS address
                                this->addr->get_index_group(),  // index group
                                this->addr->get_index_offset(), // index offset
                                bytes_to_read,                  // data length
                                data,        // read (result data) buffer
#ifdef USE_TC_ADS
                     (ads_ui32*)
#endif
                                bytes_read); // bytes written into read buffer

    if (rc != 0) {
        return ads_rc_to_epicsads_error(rc);
    }

    return 0;
}
