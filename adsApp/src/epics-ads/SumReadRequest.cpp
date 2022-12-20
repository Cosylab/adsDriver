// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <stdexcept>
#include <mutex>

#include "SumReadRequest.h"
#include "Connection.h"
#include "err.h"

#ifndef ADSIGRP_SUMUP_READ
#define ADSIGRP_SUMUP_READ 0xF080
#endif

/* Struct containing data needed to perform a single sum-read ADS operation */
struct ReadRequestChunk {
    /* ADS port (e.g. AMSPORT_R0_PLC_TC3) common to all variables in a chunk */
    uint16_t ads_port;

    /* Variables that are read in a sum-read operation */
    std::vector<std::shared_ptr<ADSVariable>> variables;

    /* Buffer containing data needed for a sum-read operation. For each variable
     * in variables vector, it contains ADS index group, ADS index offset and
     * data length. */
    std::vector<AdsSymbolInfoByName> sum_read_request_buffer;

    /* Sum read request buffer, containing the results of a sum-read operation
     */
    std::shared_ptr<SumReadBuffer> sum_read_data_buffer;

    ReadRequestChunk(uint16_t ads_port, uint16_t max_variables)
        : ads_port(ads_port), variables(), sum_read_request_buffer(),
          sum_read_data_buffer(std::make_shared<SumReadBuffer>(max_variables)) {
    }

    void add_variable(std::shared_ptr<ADSVariable> variable) {
        int rc = this->sum_read_data_buffer->add_variable(variable);
        if (rc != 0) {
            throw std::length_error(
                "could not add variable to sum-read buffer");
        }

        this->sum_read_request_buffer.push_back({0, 0, 0});

        this->variables.push_back(std::shared_ptr<ADSVariable>(variable));
    }
};

int SumReadRequest::get_num_chunks() { return this->get_chunks()->size(); }

bool SumReadRequest::is_allocated() { return this->allocated; }

bool SumReadRequest::is_initialized() { return this->initialized; }

SumReadRequest::SumReadRequest(const uint16_t max_variables_per_buffer,
                               std::shared_ptr<Connection> connection)
    : conn(connection) {
    if (max_variables_per_buffer == 0) {
        throw std::invalid_argument(
            "max_variables_per_buffer must be larger than zero");
    }

    this->max_vars_per_buffer = max_variables_per_buffer;

    if (connection == nullptr) {
        throw std::invalid_argument("connection must be set");
    }
}

SumReadRequest::~SumReadRequest() {
    this->deinitialize();
    this->deallocate();
}

std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
SumReadRequest::get_chunks() {
    auto v = std::make_shared<std::vector<std::shared_ptr<ReadRequestChunk>>>();

    for (auto chunk_set_itr = this->chunks_by_ads_port.begin();
         chunk_set_itr != this->chunks_by_ads_port.end(); chunk_set_itr++) {
        std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
            chunk_set = chunk_set_itr->second;
        for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
             chunk_itr++) {
            std::shared_ptr<ReadRequestChunk> chunk = *chunk_itr;
            v->push_back(std::shared_ptr<ReadRequestChunk>(chunk));
        }
    }

    return v;
}

int SumReadRequest::allocate(
    std::vector<std::shared_ptr<ADSVariable>> &variables) {
    if (this->is_allocated() == true) {
        return EPICSADS_INV_CALL;
    }

    for (auto var_itr = variables.begin(); var_itr != variables.end();
         var_itr++) {
        /* Get variable and ADS port which it uses */
        std::shared_ptr<ADSVariable> var = *var_itr;
        uint32_t ads_port = var->addr->get_ads_port();

        /* Create chunk set for the ADS port if necessary */
        if (this->chunks_by_ads_port.find(ads_port) ==
            this->chunks_by_ads_port.end()) {
            this->chunks_by_ads_port[ads_port] = std::make_shared<
                std::vector<std::shared_ptr<struct ReadRequestChunk>>>();
        }

        /* Get chunk set for ADS port */
        std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
            chunk_set = this->chunks_by_ads_port[ads_port];

        /* Create read request chunk if it doesn't exist or if its read buffer
         * is at full capacity  */
        if (chunk_set->size() == 0 ||
            chunk_set->back()->sum_read_data_buffer->at_capacity() == true) {
            chunk_set->push_back(std::make_shared<struct ReadRequestChunk>(
                ads_port, this->max_vars_per_buffer));
        }

        /* Add variable to the last chunk, which should have capacity */
        std::shared_ptr<ReadRequestChunk> chunk = chunk_set->back();
        try {
            chunk->add_variable(var);
        } catch (const std::exception &ex) {
            LOG_ERR("could not add ADS variable to read-request-chunk");
            goto ALLOC_ERROR;
        }
    }

    /* Initialize all sum-read data buffers */
    for (auto chunk_set_itr = this->chunks_by_ads_port.begin();
         chunk_set_itr != this->chunks_by_ads_port.end(); chunk_set_itr++) {
        std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
            chunk_set = chunk_set_itr->second;
        for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
             chunk_itr++) {
            std::shared_ptr<ReadRequestChunk> chunk = *chunk_itr;
            int rc = chunk->sum_read_data_buffer->initialize_buffer();
            if (rc != 0) {
                LOG_ERR("failed to initialize sum-read data buffer (%i): %s",
                        rc, ads_errors[rc].c_str());
                goto ALLOC_ERROR;
            }
        }
    }

    this->allocated = true;

    return 0;

ALLOC_ERROR:
    this->deallocate();
    return EPICSADS_ERROR;
}

int SumReadRequest::deallocate() {
    this->deinitialize();

    for (auto chunk_set_itr = this->chunks_by_ads_port.begin();
         chunk_set_itr != this->chunks_by_ads_port.end(); chunk_set_itr++) {
        std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
            chunk_set = chunk_set_itr->second;
        for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
             chunk_itr++) {
            std::shared_ptr<ReadRequestChunk> chunk = *chunk_itr;
            for (auto var_itr = chunk->variables.begin();
                 var_itr != chunk->variables.end(); var_itr++) {
                (*var_itr)->set_buffer_reader(EMPTY_BUFFER_DATA_POSITION);
            }
        }

        chunk_set->clear();
    }

    this->chunks_by_ads_port.clear();

    this->allocated = false;

    return 0;
}

int SumReadRequest::initialize() {
    /* Can't initialize before allocation */
    if (this->is_allocated() == false) {
        return EPICSADS_NOT_ALLOCATED;
    }

    /* There's no need for double initialization */
    if (this->initialized == true) {
        return EPICSADS_INV_CALL;
    }

    /* Configure sum-read request buffers with required data (ADS index group,
     * ADS index offset and variable size). For index group/offset, the variable
     * _must_ be resolved. */
    auto chunk_set = this->get_chunks();
    for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
         chunk_itr++) {
        std::shared_ptr<ReadRequestChunk> chunk = *chunk_itr;

        for (size_t i_var = 0; i_var < chunk->variables.size(); i_var++) {
            std::shared_ptr<ADSVariable> var = chunk->variables[i_var];
            if (var->addr->is_resolved() == false) {
                LOG_ERR("variable name is not resolved: '%s'",
                        var->addr->get_var_name().c_str());
                return EPICSADS_NOT_RESOLVED;
            }

            chunk->sum_read_request_buffer[i_var] = {
                var->addr->get_index_group(), var->addr->get_index_offset(),
                var->size()};
        }
    }

    this->initialized = true;

    return 0;
}

int SumReadRequest::deinitialize() {
    auto chunk_set = this->get_chunks();
    for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
         chunk_itr++) {
        std::shared_ptr<ReadRequestChunk> chunk = *chunk_itr;
        for (size_t i_var = 0; i_var < chunk->sum_read_request_buffer.size();
             i_var++) {
            chunk->sum_read_request_buffer[i_var] = {0, 0, 0};
        }
    }

    this->initialized = false;

    return 0;
}

void SumReadRequest::set_buffers_state(
    SumReadBuffer::SumReadBufferState state) {
    auto chunk_set = this->get_chunks();
    for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
         chunk_itr++) {
        (*chunk_itr)->sum_read_data_buffer->buffer_state = state;
    }
}


int SumReadRequest::read() {
    if (this->initialized == false) {
        return EPICSADS_INV_CALL;
    }

    auto chunk_set = this->get_chunks();
    for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
         chunk_itr++) {
        std::shared_ptr<SumReadBuffer> sum_read_data_buffer =
            (*chunk_itr)->sum_read_data_buffer;

        sum_read_data_buffer->save_buffer();

        uint32_t nelem = sum_read_data_buffer->get_num_variables();
        uint32_t read_buffer_size = sum_read_data_buffer->get_size();
        uint8_t *read_buffer = sum_read_data_buffer->get_buffer();
        uint32_t write_buffer_size =
            (*chunk_itr)->sum_read_request_buffer.size() *
            sizeof(AdsSymbolInfoByName);
        uint8_t *write_buffer =
            (uint8_t *)(*chunk_itr)->sum_read_request_buffer.data();
#ifdef USE_TC_ADS
        ads_ui32 bytes_read = 0;
#else
        uint32_t bytes_read = 0;
#endif

        if (sum_read_data_buffer->is_initialized() == false) {
            return EPICSADS_NOT_INITIALIZED;
        }

        std::lock_guard<epicsMutex> lock(this->conn->mtx);

        AmsAddr remote_ams_addr = {this->conn->get_remote_ams_netid(),
                                   (*chunk_itr)->ads_port};

        long rc = AdsSyncReadWriteReqEx2(
            this->conn->get_ads_port(), // ADS port
            &remote_ams_addr,           // AMS address
            ADSIGRP_SUMUP_READ,         // index group
            nelem, // offset; for SUMUP_READ it is the number of read commands
            read_buffer_size, // read buffer size in bytes
            read_buffer, // read data buffer (written by PLC), where the read
                         // operation results and data is stored
            write_buffer_size, // write buffer size in bytes
            write_buffer, // write buffer (data sent to PLC), containing read
                          // requests for PLC variables
            &bytes_read); // number of bytes read
        if (rc != 0) {
            this->set_buffers_state(SumReadBuffer::SumReadBufferState::Invalid);
            return ads_rc_to_epicsads_error(rc);
        }

        sum_read_data_buffer->buffer_state =
            SumReadBuffer::SumReadBufferState::Valid;
    }

    return 0;
}

std::vector<std::shared_ptr<ADSVariable>>
SumReadRequest::get_updated_variables() {
    std::vector<std::shared_ptr<ADSVariable>> vars;

    if (this->initialized == false) {
        return vars;
    }

    auto chunk_set = this->get_chunks();
    for (auto chunk_itr = chunk_set->begin(); chunk_itr != chunk_set->end();
         chunk_itr++) {
        auto updated_vars =
            (*chunk_itr)->sum_read_data_buffer->get_updated_variables();
        for (auto var = updated_vars.begin(); var != updated_vars.end();
             var++) {
            vars.push_back(*var);
        }
    }

    return vars;
}

void SumReadRequest::print_info(FILE *fd, int details) {
    if (details >= 1) {
        fprintf(fd, "Sum-read request report:\n");
        fprintf(fd, " Summary:\n");
        fprintf(fd,
                "   - Max number of variables per sum-request and sum-read "
                "buffer: %u\n",
                this->max_vars_per_buffer);
        fprintf(fd,
                "   - Number of sum-request and sum-read buffers (chunks) "
                "allocated: %i\n",
                this->get_num_chunks());
        fprintf(fd, "   - Buffers allocated: %s\n",
                (this->is_allocated() == true ? "yes" : "no"));
        fprintf(fd, "   - Buffers initialized: %s\n",
                (this->is_initialized() == true ? "yes" : "no"));
    }

    if (details >= 2) {
        fprintf(fd, " Details:\n");
        auto chunk_set = this->get_chunks();
        for (size_t i_chunk = 0; i_chunk < chunk_set->size(); i_chunk++) {
            auto chunk = chunk_set->at(i_chunk);
            fprintf(fd, "  Buffers chunk #%zu/%i:\n", (i_chunk + 1),
                    this->get_num_chunks());
            fprintf(fd, "    - ADS port: %u\n", chunk->ads_port);
            fprintf(fd, "    - Number of variables: %zu\n",
                    chunk->variables.size());
            fprintf(fd, "    - Sum-read buffer size: %zu bytes\n",
                    chunk->sum_read_data_buffer->get_size());

            if (details >= 3) {
                fprintf(fd, "    - Sum-request buffer elements:\n");
                for (size_t i_var = 0; i_var < chunk->variables.size();
                     i_var++) {
                    auto var = chunk->variables[i_var];
                    auto req_info = chunk->sum_read_request_buffer[i_var];
                    fprintf(fd, "    - Variable %zu/%zu:\n", (i_var + 1),
                            chunk->variables.size());
                    fprintf(fd, "       - Name: '%s'\n",
                            var->addr->info().c_str());
                    fprintf(fd,
                            "       - Port: %i; IGrp: %#09x; Ioff: %#09x; "
                            "Length: %u\n",
                            var->addr->get_ads_port(), req_info.indexGroup,
                            req_info.indexOffset, req_info.cbLength);
                }
            }
        }
    }
}
