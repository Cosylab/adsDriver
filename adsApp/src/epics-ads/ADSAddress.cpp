// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <sstream>
#include <limits>
#include <boost/tokenizer.hpp>

#include "Types.h"
#include "err.h"
#include "ADSAddress.h"

/* Helper functions */
static ADSDataType parse_data_type(const std::string s);
static ADSDataType parse_data_type_autoparam(const std::string s);
static Operation parse_operation(const std::string s);
static uint32_t parse_nelem_autoparam(const std::string s);
static uint32_t parse_nelem(const std::string s);
static uint16_t parse_ads_port(const std::string s);
static uint32_t parse_index_group(const std::string s);
static uint32_t parse_index_offset(const std::string s);
static uint32_t parse_notification_delay(const std::string s);
static std::string parse_variable_name(const std::string s);
static std::vector<std::string> tokenize(const std::string s);
static std::string parse_param_value(const std::string s);
static uint32_t parse_dec_or_hex_int(const std::string s);

ADSDataType ADSAddress::get_data_type() const { return this->data_type; }

std::string ADSAddress::get_var_name() const { return this->variable_name; }

Operation ADSAddress::get_operation() const { return this->operation; }

uint16_t ADSAddress::get_ads_port() const { return this->ads_port; }

uint32_t ADSAddress::get_index_group() const { return this->index_group; }

uint32_t ADSAddress::get_index_offset() const { return this->index_offset; }

uint32_t ADSAddress::get_nelem() const { return this->nelem; }

uint32_t ADSAddress::get_notification_delay() const {
    return this->ads_notification_delay;
}

bool ADSAddress::is_resolved() const { return this->name_is_resolved; }

const std::string ADSAddress::info() {
    std::ostringstream o;

    o << ads_datatypes_str[this->get_data_type()];
    o << "[" << this->get_nelem() << " elem/"
      << (this->get_nelem() * ads_datatype_sizes[this->get_data_type()])
      << " bytes] ";
    o << "P=" << this->get_ads_port() << " ";

    if (this->get_var_name() != "") {
        o << "V=" << std::hex << this->get_var_name() << " (handle G=0x"
          << this->get_index_group() << " O=0x" << this->get_index_offset()
          << ")";
    } else {
        o << "G=0x" << std::hex << this->get_index_group() << " O=0x"
          << this->get_index_offset();
    }
    o << std::dec;

    return o.str();
}

/* Construct instance from address specifier. */
ADSAddress::ADSAddress(const std::string address) {
    /*Expected vector should look like this:
     * [0] variable type + optional nelem, e.g. "LREAL" or "LREAL[3]".
     * [1] operation, e.g. "R" or "W".
     * [2] ADS port, e.g. "P=PLC_TC3" or "P=800".
     * [3] Either ADS index group (e.g. "G=0x100") which implies register
     *     access or variable name (e.g. "V=Variable"), which implies variable
     *     access.
     *
     * Register access elements:
     * [4] ADS index offset, e.g. "O=0x200".
     * [5] (optional) ADS notification delay, e.g. "D=1000".
     *
     * Variable access elements:
     * [4] (optional) ADS notification delay, e.g. "D=1000".
     * */
    auto tokens = tokenize(address);
    if (tokens.size() < 4) {
        throw std::invalid_argument("Invalid address specifier: '" + address +
                                    "'");
    }

    this->data_type = parse_data_type(tokens[0]);
    this->nelem = parse_nelem(tokens[0]);
    this->operation = parse_operation(tokens[1]);
    this->ads_port = parse_ads_port(tokens[2]);

    /* [3] specifies either ADS index group or ADS variable name */
    if (tokens[3][0] == 'G') {
        this->parse_register_specifier(tokens);
    } else {
        this->parse_variable_specifier(tokens);
    }

    /* String type requires number of element parameter. Otherwise nelem is set
     * to 1 by default. */
    if (this->data_type == ADSDataType::STRING && this->nelem == 0) {
        throw std::invalid_argument(
            "Missing NELEM specifier for STRING datatype");
    }

    if (this->nelem == 0) {
        this->nelem = 1;
    }

    if (this->index_group != 0 || this->index_offset != 0) {
        this->name_is_resolved = true;
    } else {
        this->name_is_resolved = false;
    }
}

// for autoparamdriver
ADSAddress::ADSAddress(std::string const &function,
                       std::vector<std::string> const &arguments) {

    this->data_type = parse_data_type_autoparam(function);
    if (function.find("[]") != std::string::npos ||
        function.find("STRING") != std::string::npos) {

        this->nelem = parse_nelem_autoparam(arguments[0]);
        this->operation = parse_operation(arguments[1]);
        this->ads_port = parse_ads_port(arguments[2]);

        // for now we support only variable specifier
        this->variable_name = parse_variable_name(arguments[3]);

    } else if (function.find("_digi") != std::string::npos) {
        this->nelem = 0;
        this->operation = parse_operation(arguments[0]);
        this->ads_port = parse_ads_port(arguments[1]);

        // for now we support only variable specifier
        this->variable_name = parse_variable_name(arguments[2]);

    } else {
        this->nelem = 0;
        this->operation = parse_operation(arguments[0]);
        this->ads_port = parse_ads_port(arguments[1]);

        // for now we support only variable specifier
        this->variable_name = parse_variable_name(arguments[2]);
    }

    /* String type requires number of element parameter. Otherwise nelem is set
     * to 1 by default. */
    if (this->data_type == ADSDataType::STRING && this->nelem == 0) {
        throw std::invalid_argument(
            "Missing NELEM specifier for STRING datatype");
    }

    if (this->nelem == 0) {
        this->nelem = 1;
    }

    // since we only support variable names, we don't need to
    // look at index groups and offsets like the other constructor does
    this->name_is_resolved = false;
}

int ADSAddress::resolve(const uint32_t ads_index_group,
                        const uint32_t ads_index_offset) {
    if (this->name_is_resolved == true) {
        return EPICSADS_INV_CALL;
    }

    this->index_group = ads_index_group;
    this->index_offset = ads_index_offset;
    this->name_is_resolved = true;

    return 0;
}

int ADSAddress::unresolve() {
    /* Variables addressed by symbolic name can be resolved and unresolved
     * (their handle acquired and released). Variables addressed by index/offset
     * address cannot be unresolved. */
    if (this->variable_name == "") {
        return EPICSADS_INV_CALL;
    }

    this->index_group = 0;
    this->index_offset = 0;
    this->name_is_resolved = false;

    return 0;
}

void ADSAddress::parse_register_specifier(
    std::vector<std::string> &address_tokens) {
    if (address_tokens.size() < 4) {
        throw std::invalid_argument("Missing index group specifier");
    }
    this->index_group = parse_index_group(address_tokens[3]);

    if (address_tokens.size() < 5) {
        throw std::invalid_argument("Missing index offset specifier");
    }
    this->index_offset = parse_index_offset(address_tokens[4]);

    /* Notification delay is an optional parameter */
    if (address_tokens.size() >= 6) {
        this->ads_notification_delay =
            parse_notification_delay(address_tokens[5]);
    }
}

void ADSAddress::parse_variable_specifier(
    std::vector<std::string> &address_tokens) {
    if (address_tokens.size() < 4) {
        throw std::invalid_argument("Missing variable name specifier");
    }
    this->variable_name = parse_variable_name(address_tokens[3]);

    /* Notification delay is an optional parameter */
    if (address_tokens.size() >= 5) {
        this->ads_notification_delay =
            parse_notification_delay(address_tokens[4]);
    }
}

// autoparam version
static ADSDataType parse_data_type_autoparam(const std::string s) {
    ADSDataType data_type;

    if (s.find("[]") != std::string::npos) {
        data_type = ads_datatypes[s.substr(0, s.find("[]"))];
    } else if (s.find("_digi") != std::string::npos) {
        data_type = ads_datatypes[s.substr(0, s.find("_digi"))];
    } else {
        data_type = ads_datatypes[s];
    }

    if (data_type == ADSDataType::UNKNOWN) {
        throw std::invalid_argument("Invalid datatype '" + s + "'");
    }

    return data_type;
}

/* Parse PLC variable data type. Expected format: "DATATYPE[NELEM]", with
 * "[NELEM]" being an optional parameter. DATATYPE must be one of the string
 * keys from ads_datatypes.
 *
 * Throws std::invalid_argument if DATATYPE is not specified or invalid. */
static ADSDataType parse_data_type(const std::string s) {
    ADSDataType data_type;

    if (s.find("[") != std::string::npos) {
        data_type = ads_datatypes[s.substr(0, s.find("["))];
    } else {
        data_type = ads_datatypes[s];
    }

    if (data_type == ADSDataType::UNKNOWN) {
        throw std::invalid_argument("Invalid datatype '" + s + "'");
    }

    return data_type;
}

/* Parse operation (read or write). Expected format: "R|W", i.e. "R" for read
 * and "W" for write.
 *
 * Throws std::invalid_argument if operation is not specified or invalid. */
static Operation parse_operation(const std::string s) {
    if (s == "R") {
        return Operation::Read;
    } else if (s == "W") {
        return Operation::Write;
    } else {
        throw std::invalid_argument("Invalid operation '" + s + "'");
    }
}

// for autoparam
static uint32_t parse_nelem_autoparam(const std::string s) {
    std::string value;

    try {
        value = s.substr(s.find("N=") + 2, std::string::npos);
    } catch (std::out_of_range &) {
        return 0;
    }

    if (value == "") {
        return 0;
    }

    /* stol throws std::invalid_argument if value can't be converted to integer
     */
    int64_t nelem = std::stol(value);

    /* NELEM that is negative or larger than uin32_t NELE */
    if (nelem < 1 || nelem > std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("Invalid number of elements: " + value);
    }

    return static_cast<uint32_t>(nelem);
}

/* Parse the optional number of elements specifier. Expected format:
 * "DATATYPE[NELEM]", e.g. "LREAL[13]".
 *
 * Throws std::invalid_argument if NELEM cannot be converted to integer. */
static uint32_t parse_nelem(const std::string s) {
    std::string value;

    try {
        value = s.substr(s.find("N="), std::string::npos);
    } catch (std::out_of_range &) {
        return 0;
    }

    if (value == "") {
        return 0;
    }

    /* Start with index 2 to skip the "N=" part */
    value = value.substr(2, std::string::npos);

    /* stol throws std::invalid_argument if value can't be converted to integer
     */
    int64_t nelem = std::stol(value);

    /* NELEM that is negative or larger than uin32_t NELE */
    if (nelem < 1 || nelem > std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("Invalid number of elements: " + value);
    }

    return static_cast<uint32_t>(nelem);
}

/* Parse ADS port specifier. Expected format: "P=PORT", where PORT is an
 * unsigned integer or a string key from ads_port_map (e.g. "PLC_TC3").
 *
 * Throws std::invalid_argument if port is not specified or invalid. */
static uint16_t parse_ads_port(const std::string s) {
    const std::string value = parse_param_value(s);

    /* ADS port can be specified as a string enum (e.g. PLC_TC3) */
    const uint16_t as_int = ads_port_map[value];
    if (as_int != 0) {
        return as_int;
    }

    /* Alternatively, ADS port can be specified as a base 10 or base 16 integer
     */
    try {
        return parse_dec_or_hex_int(value);
    } catch (...) {
        throw std::invalid_argument("Invalid ADS port '" + s + "'");
    }
}

/* Parse ADS index group specifier. Expected format: "G=GROUP", where GROUP
 * is an unsigned integer in dec or hex format. E.g. "G=1234" or "G=0xABCD".
 *
 * Throws std::invalid_argument if index group is not specified or cannot be
 * converted to integer. */
static uint32_t parse_index_group(const std::string s) {
    const std::string value = parse_param_value(s);

    /* Attempt to parse numeric group specifier */
    try {
        return parse_dec_or_hex_int(value);
    } catch (...) {
        throw std::invalid_argument("Invalid ADS index group '" + s + "'");
    }
}

/* Parse ADS index offset specifier. Expected format: "O=OFFSET", where OFFSET
 * is an unsigned integer in dec or hex format. E.g. "O=1234" or "O=0xABCD".
 *
 * Throws std::invalid_argument if index offset is not specified or cannot be
 * converted to integer. */
static uint32_t parse_index_offset(const std::string s) {
    const std::string value = parse_param_value(s);

    /* Attempt to parse numeric port specifier */
    try {
        return parse_dec_or_hex_int(value);
    } catch (...) {
        throw std::invalid_argument("Invalid ADS index offset '" + s + "'");
    }
}

/* Parse variable name specifier. Expected format: "V=VARIABLE_NAME", e.g.
 * "V=Main.Temperature".
 *
 * Throws std::invalid_argument if variable name is not specified. */
static std::string parse_variable_name(const std::string s) {
    const std::string value = parse_param_value(s);
    if (value == "") {
        throw std::invalid_argument("Variable name not specified");
    }

    return value;
}

/* Parse ADS notification delay optional specifier in microseconds. Expected
 * format: "D=DELAY" [us], e.g. "D=1000".
 *
 * Throws std::invalid_argument if notification delay is not specified or cannot
 * be converted to integer. */
static uint32_t parse_notification_delay(const std::string s) {
    const std::string value = parse_param_value(s);
    if (value == "") {
        throw std::invalid_argument("Notification delay not specified");
    }

    return std::stol(value);
}

/* Split address specifier by ' ' into vector elements. */
static std::vector<std::string> tokenize(const std::string s) {
    std::vector<std::string> tokens;

    boost::char_separator<char> separator(" ");
    boost::tokenizer<boost::char_separator<char>> tok(s, separator);
    for (auto itr = tok.begin(); itr != tok.end(); itr++) {
        tokens.push_back(*itr);
    }

    return tokens;
}

/* Parse value from '=' delimited parameters, e.g. "V=VariableName". */
static std::string parse_param_value(const std::string s) {
    if (s.find("=") == std::string::npos) {
        return "";
    }
    return s.substr(s.find("=") + 1, std::string::npos);
}

/* Convert decimal or hexadecimal number from string to unsigned integer.
 *
 * Throws std::stol() exceptions. */
static uint32_t parse_dec_or_hex_int(const std::string s) {
    if (s.substr(0, 2) == "0x") {
        return std::stol(s, nullptr, 16);
    } else {
        return std::stol(s, nullptr, 10);
    }
}
