#!/usr/bin/env python3
"""Clean build artifacts."""

import subprocess
import sys
from pathlib import Path


def main():
    """Run make clean in src/ directory."""
    src_dir = Path(__file__).parent.parent / "src"

    if not src_dir.exists():
        print("Error: src/ directory not found", file=sys.stderr)
        return 1

    try:
        result = subprocess.run(["make", "clean"], cwd=src_dir, check=True)
        print("Build artifacts cleaned successfully")
        return result.returncode
    except subprocess.CalledProcessError as e:
        return e.returncode


if __name__ == "__main__":
    sys.exit(main())
