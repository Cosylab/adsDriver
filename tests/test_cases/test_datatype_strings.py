"""Test string data type (STRING).

This test module verifies reading and writing of string data types
supported by the ADS EPICS module.

EPICS strings are 40 bytes long, which limits stringin and stringout records to
40 bytes (39 chars + null terminator).

Test Structure:
===============
1. Basic Functionality Tests - Use N=40 records for general testing
2. N=40 Edge Cases - PLC buffer matches EPICS buffer limit
3. N=60 Edge Cases - PLC buffer exceeds EPICS limit (tests truncation)

All write tests inspect the PLC buffer directly to verify:
- Correct data was written
- Proper null-termination
- Expected truncation behavior

"""

import time
import pytest


class TestSTRINGType:
    """Test STRING data type with comprehensive edge case coverage."""

    # ========================================================================
    # Section 1: Basic Functionality Tests (using N=40 records)
    # ========================================================================

    def test_short_string(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading short string."""
        write_pv('string_out_n40', 'Hello')
        time.sleep(0.1)
        assert read_pv('string_in_n40') == 'Hello'
        assert get_plc_var('TestPlan.stringout_n40') == 'Hello'

    def test_medium_string(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading medium-length string."""
        test_string = 'The quick brown fox jumps'
        write_pv('string_out_n40', test_string)
        time.sleep(0.1)
        assert read_pv('string_in_n40') == test_string
        assert get_plc_var('TestPlan.stringout_n40') == test_string

    def test_empty_string(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading empty string."""
        write_pv('string_out_n40', '')
        time.sleep(0.1)
        assert read_pv('string_in_n40') == ''
        assert get_plc_var('TestPlan.stringout_n40') == ''

    # ========================================================================
    # Section 2: N=40 Edge Cases (PLC buffer matches EPICS limit)
    # ========================================================================

    def test_n40_write_39_chars(self, write_pv, get_plc_var):
        """Test writing 39 chars with N=40 (one less than buffer size).

        Expected: No truncation, null at position 39.
        """
        test_string = 'A' * 39
        write_pv('string_out_n40', test_string)
        time.sleep(0.1)

        # Verify PLC has correct value
        assert get_plc_var('TestPlan.stringout_n40') == test_string

    def test_n40_write_40_chars(self, write_pv, get_plc_var):
        """Test writing 40 chars with N=40 (full EPICS buffer).

        Expected: Truncated to 39 chars + null (due to MAX_STRING_SIZE=40).
        This is correct behavior - EPICS can only hold 39 chars + null.
        """
        test_string = 'B' * 40
        write_pv('string_out_n40', test_string)
        time.sleep(0.1)

        # Verify PLC has truncated value (39 chars)
        assert get_plc_var('TestPlan.stringout_n40') == test_string[:39]

    def test_n40_read_39_chars(self, read_pv, set_plc_var):
        """Test reading 39 chars from N=40 PLC buffer.

        Expected: Read all 39 chars successfully.
        """
        test_string = 'C' * 39

        # Set PLC variable
        set_plc_var('TestPlan.stringin_n40', test_string)
        time.sleep(0.1)

        result = read_pv('string_in_n40')
        assert result == test_string
        assert len(result) == 39

    def test_n40_read_40_chars(self, read_pv, set_plc_var):
        """Test reading 40 chars from N=40 PLC buffer.

        Expected: Truncated to 39 chars (MAX_STRING_SIZE=40 limit).
        """
        test_string = 'D' * 40

        # Set PLC variable
        set_plc_var('TestPlan.stringin_n40', test_string)
        time.sleep(0.1)

        result = read_pv('string_in_n40')
        assert result == test_string[:39]  # Truncated
        assert len(result) == 39

    # ========================================================================
    # Section 3: N=60 Edge Cases (PLC buffer exceeds EPICS limit)
    # These tests verify proper truncation and null-termination when the
    # PLC buffer is larger than what EPICS can hold (MAX_STRING_SIZE=40).
    # ========================================================================

    def test_n60_write_39_chars(self, write_pv, get_plc_var):
        """Test writing 39 chars with N=60.

        Expected: No truncation, null at position 39.
        PLC buffer is larger but string fits in EPICS buffer.
        """
        test_string = 'E' * 39
        write_pv('string_out_n60', test_string)
        time.sleep(0.1)

        # Verify PLC has correct value
        assert get_plc_var('TestPlan.stringout_n60') == test_string

    def test_n60_write_50_chars(self, write_pv, get_plc_var):
        """Test writing 50 chars with N=60 (exceeds EPICS buffer).

        Expected: Truncated to 39 chars + null.
        This is NOT a bug - EPICS cannot hold more than 39 chars + null.
        """
        test_string = 'F' * 50
        write_pv('string_out_n60', test_string)
        time.sleep(0.1)

        # Verify PLC has truncated value (39 chars)
        assert get_plc_var('TestPlan.stringout_n60') == test_string[:39]

    def test_n60_write_60_chars(self, write_pv, get_plc_var):
        """Test writing 60 chars with N=60 (full PLC buffer capacity).

        Expected: Truncated to 39 chars + null.
        Tests maximum truncation case.
        """
        test_string = 'G' * 60
        write_pv('string_out_n60', test_string)
        time.sleep(0.1)

        # Verify PLC has truncated value (39 chars)
        assert get_plc_var('TestPlan.stringout_n60') == test_string[:39]

    def test_n60_read_39_chars(self, read_pv, set_plc_var):
        """Test reading 39 chars from N=60 PLC buffer.

        Expected: Read all 39 chars successfully.
        """
        test_string = 'H' * 39

        # Set PLC variable
        set_plc_var('TestPlan.stringin_n60', test_string)
        time.sleep(0.1)

        result = read_pv('string_in_n60')
        assert result == test_string
        assert len(result) == 39

    def test_n60_read_50_chars(self, read_pv, set_plc_var):
        """Test reading 50 chars from N=60 PLC buffer.

        Expected: Truncated to 39 chars (MAX_STRING_SIZE=40 limit).
        """
        test_string = 'I' * 50

        # Set PLC variable
        set_plc_var('TestPlan.stringin_n60', test_string)
        time.sleep(0.1)

        result = read_pv('string_in_n60')
        assert result == test_string[:39]  # Truncated
        assert len(result) == 39

    def test_n60_read_60_chars(self, read_pv, set_plc_var):
        """Test reading 60 chars from N=60 PLC buffer.

        This is the edge case that triggered the heap-buffer-overflow bug
        with the old Octet constructor. PLC has 60 chars but EPICS can
        only hold 39 + null (MAX_STRING_SIZE=40).

        Expected: Truncated to 39 chars with proper null-termination.
        This verifies the Octet constructor fix works correctly.
        """
        test_string = 'J' * 60

        # Set PLC variable with full 60 characters
        set_plc_var('TestPlan.stringin_n60', test_string)
        time.sleep(0.1)

        result = read_pv('string_in_n60')
        # Must truncate to 39 chars due to MAX_STRING_SIZE=40
        assert result == test_string[:39]
        assert len(result) == 39
