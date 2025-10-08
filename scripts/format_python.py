#!/usr/bin/env python3
"""Format Python files using ruff."""

import subprocess
import sys
from pathlib import Path


def main():
    """Run ruff format on Python files."""
    root = Path(__file__).parent.parent

    # Format scripts/ and src/test_*.py
    paths = [
        root / "scripts",
        root / "src" / "test_instructions.py",
        root / "src" / "test_output.py",
    ]

    paths = [p for p in paths if p.exists()]

    if not paths:
        print("No Python files found to format")
        return 0

    print("Formatting Python files with ruff...")

    try:
        subprocess.run(["ruff", "format"] + [str(p) for p in paths], check=True)
        print("✓ Python files formatted successfully")
        return 0
    except subprocess.CalledProcessError as e:
        print("✗ Formatting failed", file=sys.stderr)
        return e.returncode


if __name__ == "__main__":
    sys.exit(main())
