#!/usr/bin/env python3
"""Format all source files (C++ and Python)."""

import sys

from scripts import format_cpp, format_python


def main():
    """Run all formatters."""
    print("=== Formatting C++ ===")
    cpp_result = format_cpp.main()

    print("\n=== Formatting Python ===")
    py_result = format_python.main()

    if cpp_result == 0 and py_result == 0:
        print("\n✓ All files formatted successfully")
        return 0
    else:
        print("\n✗ Some formatting failed", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
