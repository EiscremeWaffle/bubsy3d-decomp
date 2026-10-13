import contextvars
import os
import signal
import subprocess
import time


_deadline = contextvars.ContextVar("command_deadline", default=None)


class CommandDeadline:
    def __init__(self, seconds):
        if not 0 < seconds <= 900:
            raise ValueError("Command deadline must be between 1 and 900 seconds")
        self._seconds = seconds

    def __enter__(self):
        self._token = _deadline.set(time.monotonic() + self._seconds)
        return self

    def __exit__(self, exception_type, exception, traceback):
        _deadline.reset(self._token)


def _kill_process_tree(process):
    if os.name == "nt":
        try:
            subprocess.run(
                ["taskkill", "/PID", str(process.pid), "/T", "/F"],
                capture_output=True,
                timeout=5,
                check=False,
            )
        except (OSError, subprocess.TimeoutExpired):
            pass
        if process.poll() is None:
            try:
                process.kill()
            except OSError:
                pass
        return

    try:
        os.killpg(process.pid, signal.SIGTERM)
    except ProcessLookupError:
        return
    try:
        process.wait(timeout=2)
    except subprocess.TimeoutExpired:
        pass
    try:
        os.killpg(process.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass


def run_command(command, cwd=None, input=None, capture_output=False, text=False, timeout=120, check=False):
    deadline = _deadline.get()
    if deadline is not None:
        remaining = deadline - time.monotonic()
        timeout = remaining if timeout is None else min(timeout, remaining)
    if timeout is not None and timeout <= 0:
        raise subprocess.TimeoutExpired(command, 0)

    process = subprocess.Popen(
        command,
        cwd=cwd,
        stdin=subprocess.PIPE if input is not None else None,
        stdout=subprocess.PIPE if capture_output else None,
        stderr=subprocess.PIPE if capture_output else None,
        text=text,
        creationflags=subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0,
        start_new_session=os.name != "nt",
    )
    try:
        stdout, stderr = process.communicate(input=input, timeout=timeout)
    except subprocess.TimeoutExpired as error:
        _kill_process_tree(process)
        try:
            stdout, stderr = process.communicate(timeout=2)
        except subprocess.TimeoutExpired as cleanup_error:
            if process.poll() is None:
                process.kill()
            for stream in (process.stdout, process.stderr):
                if stream is not None:
                    stream.close()
            try:
                process.wait(timeout=1)
            except subprocess.TimeoutExpired:
                pass
            stdout = cleanup_error.output or error.output
            stderr = cleanup_error.stderr or error.stderr
        raise subprocess.TimeoutExpired(
            command,
            timeout,
            output=stdout if stdout is not None else error.output,
            stderr=stderr if stderr is not None else error.stderr,
        ) from error

    result = subprocess.CompletedProcess(command, process.returncode, stdout, stderr)
    if check and result.returncode:
        raise subprocess.CalledProcessError(result.returncode, command, stdout, stderr)
    return result