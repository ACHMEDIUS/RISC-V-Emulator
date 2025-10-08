#!/usr/bin/env python3
"""Format C++ source files using clang-format."""

import os
import subprocess
import sys
from pathlib import Path
from typing import Iterable

CPP_SUFFIXES = (".cc", ".cpp", ".h", ".hpp")
EXCLUDE_DIR_NAMES = {"Windows", "build"}


def is_excluded(path: Path, root: Path) -> bool:
    """Return True when ``path`` lives inside a directory we skip."""
    relative_parts = path.relative_to(root).parts[:-1]
    return any(part in EXCLUDE_DIR_NAMES for part in relative_parts)


def iter_cpp_files(root: Path) -> Iterable[Path]:
    """Yield C++ sources under ``root`` in a stable order."""
    return sorted(
        path
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix in CPP_SUFFIXES
        and not is_excluded(path, root)
    )


def run_clang_format(clang_format: str, path: Path) -> bytes:
    """Return the formatted contents for ``path``."""
    result = subprocess.run(
        [clang_format, "--style=file", str(path)],
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout.encode("utf-8")


def main():
    """Run clang-format on all C++ files in src/ without touching unchanged files."""
    root = Path(__file__).parent.parent
    src_dir = root / "src"

    if not src_dir.exists():
        print("src/ directory not found", file=sys.stderr)
        return 1

    clang_format = os.environ.get("CLANG_FORMAT", "clang-format")
    cpp_files = list(iter_cpp_files(src_dir))

    if not cpp_files:
        print("No C++ files found to format")
        return 0

    formatted = 0
    unchanged = 0

    try:
        for path in cpp_files:
            formatted_bytes = run_clang_format(clang_format, path)
            current_bytes = path.read_bytes()
            if current_bytes != formatted_bytes:
                path.write_bytes(formatted_bytes)
                formatted += 1
            else:
                unchanged += 1
    except FileNotFoundError:
        print(
            f"✗ '{clang_format}' not found. Install clang-format or set CLANG_FORMAT.",
            file=sys.stderr,
        )
        return 1
    except subprocess.CalledProcessError as e:
        print(f"✗ Formatting failed for {path}", file=sys.stderr)
        return e.returncode

    total = formatted + unchanged
    print(
        f"Checked {total} C++ files: {formatted} reformatted, "
        f"{unchanged} already compliant."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
