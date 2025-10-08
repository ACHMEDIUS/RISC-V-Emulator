#!/usr/bin/env python3
"""Lint Python files using ruff (check only, no format)."""

import subprocess
import sys
from pathlib import Path


def main():
    """Run ruff check on Python files."""
    root = Path(__file__).parent.parent

    paths = [
        root / "scripts",
        root / "src" / "test_instructions.py",
        root / "src" / "test_output.py",
    ]

    paths = [p for p in paths if p.exists()]

    if not paths:
        print("No Python files found to lint")
        return 0

    print("Linting Python files with ruff...")

    try:
        subprocess.run(["ruff", "check"] + [str(p) for p in paths], check=True)
        print("✓ No linting issues found")
        return 0
    except subprocess.CalledProcessError as e:
        return e.returncode


if __name__ == "__main__":
    sys.exit(main())
