"""
Mock ADS Handler for EPICS ADS Module Testing

This module provides a mock ADS server implementation extending pyads.testserver.AdvancedHandler
to support testing of the EPICS ADS module. It pre-registers all required test variables with
appropriate TwinCAT data types.
"""

import struct
import logging
from pyads.testserver import AdvancedHandler, PLCVariable, AmsResponseData
from pyads import constants

logger = logging.getLogger(__name__)


class ADSTestHandler(AdvancedHandler):
    """
    Mock ADS handler that pre-registers test variables for all supported TwinCAT data types.

    Variables follow the naming convention: TestPlan.<type>_<direction>
    where direction is either 'in' (for input to IOC) or 'out' (for output from IOC).

    Supported data types:
    - Boolean: BOOL
    - Signed integers: SINT, INT, DINT, LINT
    - Unsigned integers: UINT, UDINT
    - Floating point: REAL, LREAL
    - String: STRING
    - Arrays: All numeric types with 10 elements each
    """

    def __init__(self):
        """Initialize the handler and pre-register all test variables."""
        super().__init__()

        # Register all test variables
        self._register_boolean_variables()
        self._register_integer_variables()
        self._register_float_variables()
        self._register_string_variables()
        self._register_array_variables()

    def get_variable_by_indices(self, index_group, index_offset):
        """
        Override to support handle-based access via ADSIGRP_SYM_VALBYHND.

        When index_group is ADSIGRP_SYM_VALBYHND (0xF005), the index_offset is
        actually a handle number, not a direct index offset.
        """
        if index_group == constants.ADSIGRP_SYM_VALBYHND:
            return self.get_variable_by_handle(index_offset)
        return super().get_variable_by_indices(index_group, index_offset)

    def handle_request(self, request):
        """Override handle_request to patch READWRITE, READ, and WRITE command handling."""
        # Get the command ID
        command_id = struct.unpack("<H", request.ams_header.command_id)[0]

        # Check if this is a READWRITE command (0x0009)
        if command_id == 0x0009:
            return self._patched_handle_read_write(request)

        # Check if this is a READ command (0x0002)
        if command_id == 0x0002:
            return self._patched_handle_read(request)

        # Check if this is a WRITE command (0x0003)
        if command_id == 0x0003:
            return self._patched_handle_write(request)

        # For all other commands, use the original handler
        return super().handle_request(request)

    def _patched_handle_read_write(self, request):
        """Patched READWRITE handler that supports SUMUP_READ with size limits."""
        data = request.ams_header.data
        state = request.ams_header.state_flags

        index_group = struct.unpack("<I", data[:4])[0]
        index_offset = struct.unpack("<I", data[4:8])[0]
        read_length = struct.unpack("<I", data[8:12])[0]
        write_length = struct.unpack("<I", data[12:16])[0]
        write_data = data[16 : (16 + write_length)]

        logger.debug(
            f"READWRITE: index_group={hex(index_group)} ({index_group}), "
            f"index_offset={hex(index_offset)} ({index_offset}), "
            f"read_len={read_length}, write_len={write_length}"
        )

        # Handle SUMUP_READ - respecting size limits
        if index_group == constants.ADSIGRP_SUMUP_READ:
            try:
                num_requests = index_offset
                rq_list = [
                    (
                        struct.unpack("<I", write_data[i : i + 4])[0],  # index_group
                        struct.unpack("<I", write_data[i + 4 : i + 8])[0],  # index_offset
                        struct.unpack("<I", write_data[i + 8 : i + 12])[0],  # size
                    )
                    for i in range(0, num_requests * 12, 12)
                ]

                logger.debug(f"READWRITE SUMUP_READ: {num_requests} variables requested")

                # Pack error codes (0 = success for all variables)
                read_data = struct.pack("<" + num_requests * "I", *(num_requests * [0]))
                logger.debug(f"Error codes: {len(read_data)} bytes")

                # Append variable data, respecting the requested size
                for idx_grp, idx_off, size in rq_list:
                    var = self.get_variable_by_indices(idx_grp, idx_off)
                    var_data = var.value[:size]
                    read_data += var_data
                    logger.debug(f"  Added {len(var_data)} bytes (requested {size})")

                logger.debug(f"READWRITE SUMUP_READ response: {len(read_data)} bytes total")
                content = struct.pack("<I", len(read_data)) + read_data
                logger.debug(f"Content: {len(content)} bytes total (4-byte length header + {len(read_data)} data)")

                # Wrap in AmsResponseData like the original handler does
                error_code = b'\x00\x00\x00\x00'
                response_data = error_code + content
                logger.debug(f"Returning AmsResponseData with {len(response_data)} bytes")
                return AmsResponseData(state, request.ams_header.error_code, response_data)
            except Exception as e:
                logger.error(f"Error in SUMUP_READ handler: {e}", exc_info=True)
                raise

        # For all other index_groups, use the original logic
        logger.debug("Not SUMUP_READ, calling original handler")
        return self._original_handle_read_write(request)

    def _original_handle_read_write(self, request):
        """Call the original AdvancedHandler's read_write logic."""
        return AdvancedHandler.handle_request(self, request)

    def _patched_handle_read(self, request):
        """Patched READ handler that properly handles SUMUP_READ with size limits."""
        data = request.ams_header.data
        state = request.ams_header.state_flags

        index_group = struct.unpack("<I", data[:4])[0]
        index_offset = struct.unpack("<I", data[4:8])[0]
        read_length = struct.unpack("<I", data[8:12])[0]

        logger.debug(
            f"READ: index_group={hex(index_group)} ({index_group}), "
            f"index_offset={hex(index_offset)} ({index_offset}), "
            f"read_len={read_length}"
        )

        # Handle SUMUP_READ specially to respect size limits
        if index_group == constants.ADSIGRP_SUMUP_READ:
            num_requests = index_offset
            write_data = data[12:]  # Get the request list from the data

            rq_list = [
                (
                    struct.unpack("<I", write_data[i : i + 4])[0],  # index_group
                    struct.unpack("<I", write_data[i + 4 : i + 8])[0],  # index_offset
                    struct.unpack("<I", write_data[i + 8 : i + 12])[0],  # size
                )
                for i in range(0, num_requests * 12, 12)
            ]

            logger.debug(f"READ SUMUP_READ: {num_requests} variables requested")

            # Pack error codes (0 = success for all variables)
            read_data = struct.pack("<" + num_requests * "I", *(num_requests * [0]))

            # Append variable data, respecting the requested size
            for idx_grp, idx_off, size in rq_list:
                var = self.get_variable_by_indices(idx_grp, idx_off)
                # IMPORTANT: Only send the requested number of bytes, not the entire value
                read_data += var.value[:size]

            logger.debug(f"READ SUMUP_READ response: {len(read_data)} bytes total")
            content = struct.pack("<I", len(read_data)) + read_data

            # Wrap in AmsResponseData
            error_code = b'\x00\x00\x00\x00'
            response_data = error_code + content
            return AmsResponseData(state, request.ams_header.error_code, response_data)

        # For all other READ operations, use the original handler
        return AdvancedHandler.handle_request(self, request)

    def _patched_handle_write(self, request):
        """Patched WRITE handler that links _out_array variables to _in_array variables."""
        # First, let the original handler process the write
        response = AdvancedHandler.handle_request(self, request)

        # Extract the write parameters to determine which variable was written
        data = request.ams_header.data
        index_group = struct.unpack("<I", data[:4])[0]
        index_offset = struct.unpack("<I", data[4:8])[0]
        write_length = struct.unpack("<I", data[8:12])[0]
        write_data = data[12 : (12 + write_length)]

        logger.debug(
            f"WRITE: index_group={hex(index_group)} ({index_group}), "
            f"index_offset={hex(index_offset)} ({index_offset}), "
            f"write_len={write_length}"
        )

        # Get the variable that was written
        var = self.get_variable_by_indices(index_group, index_offset)
        if var and hasattr(var, 'name'):
            var_name = var.name
            logger.debug(f"WRITE to variable: {var_name}, data length: {len(write_data)}")

            # Check if this is an _out_array or output variable and link to corresponding _in_array or input variable
            in_var_name = None
            if '_out_array' in var_name:
                in_var_name = var_name.replace('_out_array', '_in_array')
            elif 'stringout' in var_name:
                # Handle string output variables
                in_var_name = var_name.replace('stringout', 'stringin')
            elif var_name.endswith('_out') or '.bo' in var_name:
                # Handle scalar output variables (e.g., sint_out, bo, udint_out, dword_out, etc.)
                in_var_name = var_name.replace('_out', '_in').replace('.bo', '.bi')

            if in_var_name:
                try:
                    # Find the corresponding _in variable by name
                    in_var = self.get_variable_by_name(in_var_name)

                    if in_var:
                        # Copy the written data to the _in variable
                        # Ensure we match the size of the target variable's value buffer
                        if len(write_data) <= len(in_var.value):
                            in_var.value = write_data + in_var.value[len(write_data):]
                        else:
                            in_var.value = write_data[:len(in_var.value)]
                        logger.debug(f"Linked {var_name} -> {in_var_name}, copied {len(write_data)} bytes")
                    else:
                        logger.warning(f"Could not find corresponding variable: {in_var_name}")
                except Exception as e:
                    logger.error(f"Error linking variables: {e}", exc_info=True)

        return response

    def _register_boolean_variables(self):
        """Register boolean test variables (bi, bo)."""
        # Boolean input (bi) - reads from PLC
        self.add_variable(PLCVariable(
            name="TestPlan.bi",
            value=struct.pack('<?', False),
            ads_type=constants.ADST_BIT,
            symbol_type='BOOL'
        ))

        # Boolean output (bo) - writes to PLC
        self.add_variable(PLCVariable(
            name="TestPlan.bo",
            value=struct.pack('<?', False),
            ads_type=constants.ADST_BIT,
            symbol_type='BOOL'
        ))

    def _register_integer_variables(self):
        """Register signed and unsigned integer test variables."""
        # Signed integers - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.sint_in",
            value=struct.pack('<b', 0),
            ads_type=constants.ADST_INT8,
            symbol_type='SINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.int_in",
            value=struct.pack('<h', 0),
            ads_type=constants.ADST_INT16,
            symbol_type='INT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.dint_in",
            value=struct.pack('<i', 0),
            ads_type=constants.ADST_INT32,
            symbol_type='DINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.lint_in",
            value=struct.pack('<q', 0),
            ads_type=constants.ADST_INT64,
            symbol_type='LINT'
        ))

        # Signed integers - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.sint_out",
            value=struct.pack('<b', 0),
            ads_type=constants.ADST_INT8,
            symbol_type='SINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.int_out",
            value=struct.pack('<h', 0),
            ads_type=constants.ADST_INT16,
            symbol_type='INT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.dint_out",
            value=struct.pack('<i', 0),
            ads_type=constants.ADST_INT32,
            symbol_type='DINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.lint_out",
            value=struct.pack('<q', 0),
            ads_type=constants.ADST_INT64,
            symbol_type='LINT'
        ))

        # Unsigned integers - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.uint_in",
            value=struct.pack('<H', 0),
            ads_type=constants.ADST_UINT16,
            symbol_type='UINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.udint_in",
            value=struct.pack('<I', 0),
            ads_type=constants.ADST_UINT32,
            symbol_type='UDINT'
        ))

        # Unsigned integers - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.uint_out",
            value=struct.pack('<H', 0),
            ads_type=constants.ADST_UINT16,
            symbol_type='UINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.udint_out",
            value=struct.pack('<I', 0),
            ads_type=constants.ADST_UINT32,
            symbol_type='UDINT'
        ))

        # BYTE - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.byte_in",
            value=struct.pack('<B', 0),
            ads_type=constants.ADST_UINT8,
            symbol_type='BYTE'
        ))

        # BYTE - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.byte_out",
            value=struct.pack('<B', 0),
            ads_type=constants.ADST_UINT8,
            symbol_type='BYTE'
        ))

        # USINT - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.usint_in",
            value=struct.pack('<B', 0),
            ads_type=constants.ADST_UINT8,
            symbol_type='USINT'
        ))

        # USINT - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.usint_out",
            value=struct.pack('<B', 0),
            ads_type=constants.ADST_UINT8,
            symbol_type='USINT'
        ))

        # WORD - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.word_in",
            value=struct.pack('<H', 0),
            ads_type=constants.ADST_UINT16,
            symbol_type='WORD'
        ))

        # WORD - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.word_out",
            value=struct.pack('<H', 0),
            ads_type=constants.ADST_UINT16,
            symbol_type='WORD'
        ))

        # DWORD - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.dword_in",
            value=struct.pack('<I', 0),
            ads_type=constants.ADST_UINT32,
            symbol_type='DWORD'
        ))

        # DWORD - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.dword_out",
            value=struct.pack('<I', 0),
            ads_type=constants.ADST_UINT32,
            symbol_type='DWORD'
        ))

    def _register_float_variables(self):
        """Register floating point test variables."""
        # Single precision float - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.real_in",
            value=struct.pack('<f', 0.0),
            ads_type=constants.ADST_REAL32,
            symbol_type='REAL'
        ))

        # Single precision float - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.real_out",
            value=struct.pack('<f', 0.0),
            ads_type=constants.ADST_REAL32,
            symbol_type='REAL'
        ))

        # Double precision float - input direction (reads from PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.lreal_in",
            value=struct.pack('<d', 0.0),
            ads_type=constants.ADST_REAL64,
            symbol_type='LREAL'
        ))

        # Double precision float - output direction (writes to PLC)
        self.add_variable(PLCVariable(
            name="TestPlan.lreal_out",
            value=struct.pack('<d', 0.0),
            ads_type=constants.ADST_REAL64,
            symbol_type='LREAL'
        ))

    def _register_string_variables(self):
        """Register string test variables.

        Provides a 2x2 matrix of string variables for comprehensive testing:
        - N=40: PLC buffer matches EPICS MAX_STRING_SIZE limit
        - N=60: PLC buffer exceeds EPICS limit (tests truncation)
        """
        # N=40 variables (40 chars + null = 41 bytes)
        self.add_variable(PLCVariable(
            name="TestPlan.stringin_n40",
            value=b'\x00' * 41,
            ads_type=constants.ADST_STRING,
            symbol_type='STRING(40)'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.stringout_n40",
            value=b'\x00' * 41,
            ads_type=constants.ADST_STRING,
            symbol_type='STRING(40)'
        ))

        # N=60 variables (60 chars + null = 61 bytes)
        self.add_variable(PLCVariable(
            name="TestPlan.stringin_n60",
            value=b'\x00' * 61,
            ads_type=constants.ADST_STRING,
            symbol_type='STRING(60)'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.stringout_n60",
            value=b'\x00' * 61,
            ads_type=constants.ADST_STRING,
            symbol_type='STRING(60)'
        ))

    def _register_array_variables(self):
        """Register array test variables for all numeric types."""
        array_size = 10

        # SINT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.sint_in_array",
            value=struct.pack(f'<{array_size}b', *([0] * array_size)),
            ads_type=constants.ADST_INT8,
            symbol_type=f'ARRAY [0..{array_size-1}] OF SINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.sint_out_array",
            value=struct.pack(f'<{array_size}b', *([0] * array_size)),
            ads_type=constants.ADST_INT8,
            symbol_type=f'ARRAY [0..{array_size-1}] OF SINT'
        ))

        # INT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.int_in_array",
            value=struct.pack(f'<{array_size}h', *([0] * array_size)),
            ads_type=constants.ADST_INT16,
            symbol_type=f'ARRAY [0..{array_size-1}] OF INT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.int_out_array",
            value=struct.pack(f'<{array_size}h', *([0] * array_size)),
            ads_type=constants.ADST_INT16,
            symbol_type=f'ARRAY [0..{array_size-1}] OF INT'
        ))

        # DINT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.dint_in_array",
            value=struct.pack(f'<{array_size}i', *([0] * array_size)),
            ads_type=constants.ADST_INT32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF DINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.dint_out_array",
            value=struct.pack(f'<{array_size}i', *([0] * array_size)),
            ads_type=constants.ADST_INT32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF DINT'
        ))

        # LINT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.lint_in_array",
            value=struct.pack(f'<{array_size}q', *([0] * array_size)),
            ads_type=constants.ADST_INT64,
            symbol_type=f'ARRAY [0..{array_size-1}] OF LINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.lint_out_array",
            value=struct.pack(f'<{array_size}q', *([0] * array_size)),
            ads_type=constants.ADST_INT64,
            symbol_type=f'ARRAY [0..{array_size-1}] OF LINT'
        ))

        # UINT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.uint_in_array",
            value=struct.pack(f'<{array_size}H', *([0] * array_size)),
            ads_type=constants.ADST_UINT16,
            symbol_type=f'ARRAY [0..{array_size-1}] OF UINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.uint_out_array",
            value=struct.pack(f'<{array_size}H', *([0] * array_size)),
            ads_type=constants.ADST_UINT16,
            symbol_type=f'ARRAY [0..{array_size-1}] OF UINT'
        ))

        # UDINT arrays
        self.add_variable(PLCVariable(
            name="TestPlan.udint_in_array",
            value=struct.pack(f'<{array_size}I', *([0] * array_size)),
            ads_type=constants.ADST_UINT32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF UDINT'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.udint_out_array",
            value=struct.pack(f'<{array_size}I', *([0] * array_size)),
            ads_type=constants.ADST_UINT32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF UDINT'
        ))

        # REAL arrays
        self.add_variable(PLCVariable(
            name="TestPlan.real_in_array",
            value=struct.pack(f'<{array_size}f', *([0.0] * array_size)),
            ads_type=constants.ADST_REAL32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF REAL'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.real_out_array",
            value=struct.pack(f'<{array_size}f', *([0.0] * array_size)),
            ads_type=constants.ADST_REAL32,
            symbol_type=f'ARRAY [0..{array_size-1}] OF REAL'
        ))

        # LREAL arrays
        self.add_variable(PLCVariable(
            name="TestPlan.lreal_in_array",
            value=struct.pack(f'<{array_size}d', *([0.0] * array_size)),
            ads_type=constants.ADST_REAL64,
            symbol_type=f'ARRAY [0..{array_size-1}] OF LREAL'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.lreal_out_array",
            value=struct.pack(f'<{array_size}d', *([0.0] * array_size)),
            ads_type=constants.ADST_REAL64,
            symbol_type=f'ARRAY [0..{array_size-1}] OF LREAL'
        ))

        # BYTE arrays
        self.add_variable(PLCVariable(
            name="TestPlan.byte_in_array",
            value=struct.pack(f'<{array_size}B', *([0] * array_size)),
            ads_type=constants.ADST_UINT8,
            symbol_type=f'ARRAY [0..{array_size-1}] OF BYTE'
        ))

        self.add_variable(PLCVariable(
            name="TestPlan.byte_out_array",
            value=struct.pack(f'<{array_size}B', *([0] * array_size)),
            ads_type=constants.ADST_UINT8,
            symbol_type=f'ARRAY [0..{array_size-1}] OF BYTE'
        ))

    def clear(self):
        """Clear all variables from the handler."""
        # Note: AdvancedHandler stores variables internally
        # This method is here for compatibility with the fixture
        # but the handler will be recreated each session anyway
        pass
