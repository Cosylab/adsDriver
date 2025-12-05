"""Helper functions for working with EPICS PVs using p4p."""

from typing import Any, Optional
import p4p.client.thread


def get_pv_with_prefix(pv_name: str, prefix: str = "TEST") -> str:
    """Prepend a prefix to a PV name.

    Args:
        pv_name: The base PV name (without prefix)
        prefix: The prefix to prepend (default: "TEST")

    Returns:
        The full PV name with prefix, in the form "PREFIX:PV_NAME"

    Examples:
        >>> get_pv_with_prefix("temperature")
        'TEST:temperature'
        >>> get_pv_with_prefix("pressure", "IOC")
        'IOC:pressure'
    """
    if ":" in pv_name:
        # PV already has a prefix, return as-is
        return pv_name
    return f"{prefix}:{pv_name}"


def wait_for_pv(
    context: p4p.client.thread.Context,
    pv_name: str,
    timeout: float = 5.0
) -> bool:
    """Wait for a PV to become available.

    Args:
        context: The p4p Context to use
        pv_name: The PV name to wait for
        timeout: Maximum time to wait in seconds

    Returns:
        True if PV is available, False if timeout
    """
    try:
        context.get(pv_name, timeout=timeout)
        return True
    except TimeoutError:
        return False
    except Exception:
        return False


def read_pv_with_retry(
    context: p4p.client.thread.Context,
    pv_name: str,
    retries: int = 3,
    timeout: float = 5.0
) -> Optional[Any]:
    """Read a PV with automatic retry on failure.

    Args:
        context: The p4p Context to use
        pv_name: The PV name to read
        retries: Number of retry attempts
        timeout: Timeout for each attempt in seconds

    Returns:
        The PV value, or None if all attempts failed
    """
    for attempt in range(retries):
        try:
            return context.get(pv_name, timeout=timeout)
        except Exception as e:
            if attempt < retries - 1:
                print(f"Retry {attempt + 1}/{retries} reading {pv_name}: {e}")
            else:
                print(f"Failed to read {pv_name} after {retries} attempts: {e}")
                return None
    return None


def write_pv_with_retry(
    context: p4p.client.thread.Context,
    pv_name: str,
    value: Any,
    retries: int = 3,
    timeout: float = 5.0
) -> bool:
    """Write a PV with automatic retry on failure.

    Args:
        context: The p4p Context to use
        pv_name: The PV name to write
        value: The value to write
        retries: Number of retry attempts
        timeout: Timeout for each attempt in seconds

    Returns:
        True if write succeeded, False if all attempts failed
    """
    for attempt in range(retries):
        try:
            context.put(pv_name, value, timeout=timeout)
            return True
        except Exception as e:
            if attempt < retries - 1:
                print(f"Retry {attempt + 1}/{retries} writing {pv_name}: {e}")
            else:
                print(f"Failed to write {pv_name} after {retries} attempts: {e}")
                return False
    return False
