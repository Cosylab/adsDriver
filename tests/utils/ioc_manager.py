"""IOC subprocess management for testing."""

import subprocess
import threading
import time
import signal
import os
from typing import Optional, List


class IOCManager:
    """Manages an IOC subprocess for testing.

    This class handles starting an IOC as a subprocess, capturing its output,
    waiting for initialization to complete, and cleanly shutting it down.
    """

    def __init__(self, ioc_executable: str, st_cmd: str):
        """Initialize the IOC manager.

        Args:
            ioc_executable: Path to the IOC executable
            st_cmd: Path to the st.cmd startup script
        """
        self.ioc_executable = os.path.abspath(ioc_executable)
        self.st_cmd = os.path.abspath(st_cmd)
        self.process: Optional[subprocess.Popen] = None
        self.stdout_lines: List[str] = []
        self.stderr_lines: List[str] = []
        self._stdout_thread: Optional[threading.Thread] = None
        self._stderr_thread: Optional[threading.Thread] = None
        self._initialized = threading.Event()

    def _read_stdout(self):
        """Read stdout from the IOC process in a background thread."""
        if self.process and self.process.stdout:
            for line in iter(self.process.stdout.readline, ''):
                line_str = line.rstrip()
                self.stdout_lines.append(line_str)
                print(f"IOC stdout: {line_str}")

                # Check for initialization complete message
                if "iocRun: All initialization complete" in line_str:
                    self._initialized.set()

    def _read_stderr(self):
        """Read stderr from the IOC process in a background thread."""
        if self.process and self.process.stderr:
            for line in iter(self.process.stderr.readline, ''):
                line_str = line.rstrip()
                self.stderr_lines.append(line_str)
                print(f"IOC stderr: {line_str}")

                # Also check for initialization complete message on stderr
                if "iocRun: All initialization complete" in line_str:
                    self._initialized.set()

    def start(self, timeout: float = 30.0) -> bool:
        """Start the IOC process and wait for initialization.

        Args:
            timeout: Maximum time to wait for IOC to start (seconds)

        Returns:
            True if IOC started successfully, False otherwise
        """
        if self.is_running():
            print("IOC is already running")
            return True

        # Verify files exist
        if not os.path.exists(self.ioc_executable):
            raise FileNotFoundError(f"IOC executable not found: {self.ioc_executable}")
        if not os.path.exists(self.st_cmd):
            raise FileNotFoundError(f"st.cmd not found: {self.st_cmd}")

        # Get the directory containing st.cmd
        st_cmd_dir = os.path.dirname(self.st_cmd)

        print(f"Starting IOC: {self.ioc_executable}")
        print(f"Working directory: {st_cmd_dir}")
        print(f"Startup script: {self.st_cmd}")

        # Start the IOC process
        self.process = subprocess.Popen(
            [self.ioc_executable, self.st_cmd],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=st_cmd_dir,
            text=True,           # Enable text mode
            encoding='utf-8',    # Explicit encoding
            errors='replace',    # Handle decode errors gracefully
            bufsize=1           # Line buffered (now valid in text mode)
        )

        # Start threads to capture output
        self._stdout_thread = threading.Thread(target=self._read_stdout, daemon=True)
        self._stderr_thread = threading.Thread(target=self._read_stderr, daemon=True)
        self._stdout_thread.start()
        self._stderr_thread.start()

        # Wait for initialization to complete
        print(f"Waiting up to {timeout} seconds for IOC to initialize...")
        if self._initialized.wait(timeout=timeout):
            print("IOC initialization complete")
            return True
        else:
            print("IOC initialization timed out")
            self.stop()
            return False

    def stop(self):
        """Stop the IOC process cleanly."""
        if not self.is_running():
            return

        print("Stopping IOC...")

        try:
            # First try sending "exit" command
            if self.process and self.process.stdin:
                self.process.stdin.write("exit\n")
                self.process.stdin.flush()

            # Wait a bit for graceful shutdown
            if self.process:
                try:
                    self.process.wait(timeout=5.0)
                    print("IOC stopped gracefully")
                except subprocess.TimeoutExpired:
                    print("IOC did not stop gracefully, sending SIGINT...")
                    self.process.send_signal(signal.SIGINT)
                    try:
                        self.process.wait(timeout=5.0)
                        print("IOC stopped after SIGINT")
                    except subprocess.TimeoutExpired:
                        print("IOC did not respond to SIGINT, terminating...")
                        self.process.terminate()
                        self.process.wait(timeout=5.0)
        except Exception as e:
            print(f"Error stopping IOC: {e}")
            if self.process:
                self.process.kill()
                self.process.wait()
        finally:
            self.process = None
            self._initialized.clear()

    def is_running(self) -> bool:
        """Check if the IOC process is running.

        Returns:
            True if running, False otherwise
        """
        return self.process is not None and self.process.poll() is None

    def get_stdout(self) -> List[str]:
        """Get all stdout lines captured so far.

        Returns:
            List of stdout lines
        """
        return self.stdout_lines.copy()

    def get_stderr(self) -> List[str]:
        """Get all stderr lines captured so far.

        Returns:
            List of stderr lines
        """
        return self.stderr_lines.copy()
