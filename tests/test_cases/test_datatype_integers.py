"""Test integer data types (SINT, INT, DINT, UINT, UDINT, LINT).

This test module verifies reading and writing of various integer data types
supported by the ADS EPICS module.
"""

import time
import pytest


class TestSINTType:
    """Test SINT (8-bit signed integer) data type."""

    def test_sint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('sint_out', 42)
        time.sleep(0.1)
        assert read_pv('sint_in') == 42
        assert get_plc_var('TestPlan.sint_out') == 42

    def test_sint_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('sint_out', -50)
        time.sleep(0.1)
        assert read_pv('sint_in') == -50
        assert get_plc_var('TestPlan.sint_out') == -50

    def test_sint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('sint_out', 0)
        time.sleep(0.1)
        assert read_pv('sint_in') == 0
        assert get_plc_var('TestPlan.sint_out') == 0

    def test_sint_boundary_min(self, write_pv, read_pv, get_plc_var):
        """Test minimum representable value (-128)."""
        write_pv('sint_out', -128)
        time.sleep(0.1)
        assert read_pv('sint_in') == -128
        assert get_plc_var('TestPlan.sint_out') == -128

    def test_sint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (127)."""
        write_pv('sint_out', 127)
        time.sleep(0.1)
        assert read_pv('sint_in') == 127
        assert get_plc_var('TestPlan.sint_out') == 127


class TestINTType:
    """Test INT (16-bit signed integer) data type."""

    def test_int_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('int_out', 1234)
        time.sleep(0.1)
        assert read_pv('int_in') == 1234
        assert get_plc_var('TestPlan.int_out') == 1234

    def test_int_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('int_out', -5678)
        time.sleep(0.1)
        assert read_pv('int_in') == -5678
        assert get_plc_var('TestPlan.int_out') == -5678

    def test_int_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('int_out', 0)
        time.sleep(0.1)
        assert read_pv('int_in') == 0
        assert get_plc_var('TestPlan.int_out') == 0

    def test_int_boundary_min(self, write_pv, read_pv, get_plc_var):
        """Test minimum representable value (-32768)."""
        write_pv('int_out', -32768)
        time.sleep(0.1)
        assert read_pv('int_in') == -32768
        assert get_plc_var('TestPlan.int_out') == -32768

    def test_int_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (32767)."""
        write_pv('int_out', 32767)
        time.sleep(0.1)
        assert read_pv('int_in') == 32767
        assert get_plc_var('TestPlan.int_out') == 32767


class TestDINTType:
    """Test DINT (32-bit signed integer) data type."""

    def test_dint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('dint_out', 123456789)
        time.sleep(0.1)
        assert read_pv('dint_in') == 123456789
        assert get_plc_var('TestPlan.dint_out') == 123456789

    def test_dint_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('dint_out', -987654321)
        time.sleep(0.1)
        assert read_pv('dint_in') == -987654321
        assert get_plc_var('TestPlan.dint_out') == -987654321

    def test_dint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('dint_out', 0)
        time.sleep(0.1)
        assert read_pv('dint_in') == 0
        assert get_plc_var('TestPlan.dint_out') == 0

    def test_dint_boundary_min(self, write_pv, read_pv, get_plc_var):
        """Test minimum representable value (-2147483648)."""
        write_pv('dint_out', -2147483648)
        time.sleep(0.1)
        assert read_pv('dint_in') == -2147483648
        assert get_plc_var('TestPlan.dint_out') == -2147483648

    def test_dint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (2147483647)."""
        write_pv('dint_out', 2147483647)
        time.sleep(0.1)
        assert read_pv('dint_in') == 2147483647
        assert get_plc_var('TestPlan.dint_out') == 2147483647


class TestUINTType:
    """Test UINT (16-bit unsigned integer) data type."""

    def test_uint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('uint_out', 12345)
        time.sleep(0.1)
        assert read_pv('uint_in') == 12345
        assert get_plc_var('TestPlan.uint_out') == 12345

    def test_uint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('uint_out', 0)
        time.sleep(0.1)
        assert read_pv('uint_in') == 0
        assert get_plc_var('TestPlan.uint_out') == 0

    def test_uint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (65535)."""
        write_pv('uint_out', 65535)
        time.sleep(0.1)
        assert read_pv('uint_in') == 65535
        assert get_plc_var('TestPlan.uint_out') == 65535


class TestUDINTType:
    """Test UDINT (32-bit unsigned integer) data type."""

    def test_udint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('udint_out', 3000000000)
        time.sleep(0.1)
        assert read_pv('udint_in') == 3000000000
        assert get_plc_var('TestPlan.udint_out') == 3000000000

    def test_udint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('udint_out', 0)
        time.sleep(0.1)
        assert read_pv('udint_in') == 0
        assert get_plc_var('TestPlan.udint_out') == 0

    def test_udint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (4294967295)."""
        write_pv('udint_out', 4294967295)
        time.sleep(0.1)
        assert read_pv('udint_in') == 4294967295
        assert get_plc_var('TestPlan.udint_out') == 4294967295


class TestLINTType:
    """Test LINT (64-bit signed integer) data type."""

    def test_lint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('lint_out', 9000000000000000)
        time.sleep(0.1)
        assert read_pv('lint_in') == 9000000000000000
        assert get_plc_var('TestPlan.lint_out') == 9000000000000000

    def test_lint_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('lint_out', -8000000000000000)
        time.sleep(0.1)
        assert read_pv('lint_in') == -8000000000000000
        assert get_plc_var('TestPlan.lint_out') == -8000000000000000

    def test_lint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('lint_out', 0)
        time.sleep(0.1)
        assert read_pv('lint_in') == 0
        assert get_plc_var('TestPlan.lint_out') == 0

    def test_lint_boundary_min(self, write_pv, read_pv, get_plc_var):
        """Test minimum representable value (-9223372036854775808)."""
        write_pv('lint_out', -9223372036854775808)
        time.sleep(0.1)
        assert read_pv('lint_in') == -9223372036854775808
        assert get_plc_var('TestPlan.lint_out') == -9223372036854775808

    def test_lint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (9223372036854775807)."""
        write_pv('lint_out', 9223372036854775807)
        time.sleep(0.1)
        assert read_pv('lint_in') == 9223372036854775807
        assert get_plc_var('TestPlan.lint_out') == 9223372036854775807


class TestBYTEType:
    """Test BYTE (8-bit unsigned integer) data type."""

    def test_byte_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('byte_out', 100)
        time.sleep(0.1)
        assert read_pv('byte_in') == 100
        assert get_plc_var('TestPlan.byte_out') == 100

    def test_byte_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('byte_out', 0)
        time.sleep(0.1)
        assert read_pv('byte_in') == 0
        assert get_plc_var('TestPlan.byte_out') == 0

    def test_byte_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (255)."""
        write_pv('byte_out', 255)
        time.sleep(0.1)
        assert read_pv('byte_in') == 255
        assert get_plc_var('TestPlan.byte_out') == 255


class TestUSINTType:
    """Test USINT (8-bit unsigned integer) data type."""

    def test_usint_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('usint_out', 255)
        time.sleep(0.1)
        assert read_pv('usint_in') == 255
        assert get_plc_var('TestPlan.usint_out') == 255

    def test_usint_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('usint_out', 0)
        time.sleep(0.1)
        assert read_pv('usint_in') == 0
        assert get_plc_var('TestPlan.usint_out') == 0

    def test_usint_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (255)."""
        write_pv('usint_out', 255)
        time.sleep(0.1)
        assert read_pv('usint_in') == 255
        assert get_plc_var('TestPlan.usint_out') == 255


class TestWORDType:
    """Test WORD (16-bit unsigned integer) data type."""

    def test_word_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('word_out', 50000)
        time.sleep(0.1)
        assert read_pv('word_in') == 50000
        assert get_plc_var('TestPlan.word_out') == 50000

    def test_word_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('word_out', 0)
        time.sleep(0.1)
        assert read_pv('word_in') == 0
        assert get_plc_var('TestPlan.word_out') == 0

    def test_word_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (65535)."""
        write_pv('word_out', 65535)
        time.sleep(0.1)
        assert read_pv('word_in') == 65535
        assert get_plc_var('TestPlan.word_out') == 65535


class TestDWORDType:
    """Test DWORD (32-bit unsigned integer) data type."""

    def test_dword_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('dword_out', 3000000000)
        time.sleep(0.1)
        assert read_pv('dword_in') == 3000000000
        assert get_plc_var('TestPlan.dword_out') == 3000000000

    def test_dword_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('dword_out', 0)
        time.sleep(0.1)
        assert read_pv('dword_in') == 0
        assert get_plc_var('TestPlan.dword_out') == 0

    def test_dword_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value (4294967295)."""
        write_pv('dword_out', 4294967295)
        time.sleep(0.1)
        assert read_pv('dword_in') == 4294967295
        assert get_plc_var('TestPlan.dword_out') == 4294967295
