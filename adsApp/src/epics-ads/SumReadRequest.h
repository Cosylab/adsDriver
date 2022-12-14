// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef SUMREADREQUEST_H
#define SUMREADREQUEST_H

#include <map>
#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

#ifdef USE_TC_ADS
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsApi.h>
#else
#include <AdsLib.h>
#endif

#include "Variable.h"
#include "SumReadBuffer.h"

/* Defined in Connection.h, which includes this header file (prevent cyclic
 * include). */
class Connection;

/* Used internally */
struct ReadRequestChunk;

class SumReadRequest {
  protected:
    std::shared_ptr<Connection> conn;
    uint16_t max_vars_per_buffer = 0;
    bool allocated = false;
    bool initialized = false;

    void set_buffers_state(SumReadBuffer::SumReadBufferState state);

    /* Read request chunks are grouped by ADS port (e.g. PLC_TC3), then split
     * according to max. number of variables per single sum-read buffer (e.g.
     * 500). */
    std::map< uint32_t,
              std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>> >
        chunks_by_ads_port;

    /* Helper method returns a vector of chunks from chunks_by_ads_port_map */
    std::shared_ptr<std::vector<std::shared_ptr<ReadRequestChunk>>>
    get_chunks();

  public:
    /* Number of chunks, i.e. the number of sub-requests needed to sum-read all
     * variables specified with reserve(variables). */
    int get_num_chunks();

    bool is_allocated();
    bool is_initialized();

    SumReadRequest(const uint16_t max_variables_per_buffer,
                   std::shared_ptr<Connection> connection);
    ~SumReadRequest();

    /* Allocate space for ADS sum-read buffers used for performing ADS sum-read
     * operations for the specified ADS variables. The specified variables do
     * not need to be resolved.
     * initialize() must be called before sum-read operations can be executed.
     */
    int allocate(std::vector<std::shared_ptr<ADSVariable>> &variables);

    /* Deallocate ADS sum-read buffers and decouple variables from sum-read
     * buffers, which were setup in call to allocate(). */
    int deallocate();

    /* Initializes sum-read buffers used for performing ADS sum-read operations.
     * The variables that were specified when calling allocate(variables) _must_
     * be resolved. */
    int initialize();

    /* Deinitialize sum-read buffers. */
    int deinitialize();

    /* Perform ADS sum-read operation. initialize() must be called before. */
    int read();

    /* Return ADS variables whose value has changed between two latest calls
     * to read(). */
    std::vector<std::shared_ptr<ADSVariable>> get_updated_variables();

    /* Print information about buffers used to FD. The DETAILS determines
     * verbosity:
     * 0 - no output
     * 1 - summary info
     * 2 - per-buffer info
     * 3 - variables info */
    void print_info(FILE *fd, int details);
};

#endif /* SUMREADREQUEST_H */
