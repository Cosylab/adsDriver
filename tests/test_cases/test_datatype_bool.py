"""Test boolean data type (BOOL).

This test module verifies reading and writing of boolean data type
supported by the ADS EPICS module.
"""

import time
import pytest


class TestBOOLType:
    """Test BOOL (boolean) data type."""

    def test_bool_write_read_false(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading False (0)."""
        write_pv('bool_out', 0)
        time.sleep(0.1)
        assert read_pv('bool_in') == 0
        assert get_plc_var('TestPlan.bo') == 0

    def test_bool_write_read_true(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading True (1)."""
        write_pv('bool_out', 1)
        time.sleep(0.1)
        assert read_pv('bool_in') == 1
        assert get_plc_var('TestPlan.bo') == 1

    def test_bool_toggle_false_to_true(self, write_pv, read_pv, get_plc_var):
        """Test toggling from False to True."""
        # Set to False
        write_pv('bool_out', 0)
        time.sleep(0.1)
        assert read_pv('bool_in') == 0
        assert get_plc_var('TestPlan.bo') == 0

        # Toggle to True
        write_pv('bool_out', 1)
        time.sleep(0.1)
        assert read_pv('bool_in') == 1
        assert get_plc_var('TestPlan.bo') == 1

    def test_bool_toggle_true_to_false(self, write_pv, read_pv, get_plc_var):
        """Test toggling from True to False."""
        # Set to True
        write_pv('bool_out', 1)
        time.sleep(0.1)
        assert read_pv('bool_in') == 1
        assert get_plc_var('TestPlan.bo') == 1

        # Toggle to False
        write_pv('bool_out', 0)
        time.sleep(0.1)
        assert read_pv('bool_in') == 0
        assert get_plc_var('TestPlan.bo') == 0

    def test_bool_multiple_toggles(self, write_pv, read_pv, get_plc_var):
        """Test multiple toggle operations."""
        for expected_value in [1, 0, 1, 0, 1]:
            write_pv('bool_out', expected_value)
            time.sleep(0.1)
            assert read_pv('bool_in') == expected_value
            assert get_plc_var('TestPlan.bo') == expected_value
