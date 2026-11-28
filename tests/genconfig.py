"""Run with python3 tests/genconfig.py /path/to/fastfetch (requires pexpect)."""

import json
import os
import sys
import tempfile
from pathlib import Path

import pexpect


def test_small_logo(binary, directory):
    path = directory / "config.jsonc"
    path.write_text("{}")
    env = dict(os.environ, TERM="xterm-256color", XDG_CONFIG_HOME=str(directory))
    env.pop("NO_COLOR", None)

    with pexpect.spawn(binary, ["-c", "none", "--gen-config", str(path)], env=env, timeout=10) as child:
        child.expect_exact("interactive config generator")
        child.send("l")
        child.expect_exact("interactive config generator")
        child.send("s")
        child.expect_exact("File exists. Overwrite?")
        child.send("y")
        child.expect(pexpect.EOF)

    assert json.loads(path.read_text())["logo"] == {"type": "small"}


if __name__ == "__main__":
    with tempfile.TemporaryDirectory() as directory:
        test_small_logo(sys.argv[1], Path(directory))
