#!/usr/bin/env python3
"""(LIACS specific command) Create submission tarballs for Part A or Part B."""

import argparse
import shutil
import subprocess
import sys
import tarfile
from pathlib import Path


def main():
    """Interactive submission tarball creator."""
    parser = argparse.ArgumentParser(description="Create assignment submission tarball")
    parser.add_argument(
        "part", choices=["2a", "2b", "a", "b"], help="Part A (2a) or Part B (2b)"
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify contents without creating tarball",
    )

    args = parser.parse_args()

    # Normalize part name
    part = "2a" if args.part in ["2a", "a"] else "2b"
    part_upper = "A" if part == "2a" else "B"

    print(f"=== Creating Assignment Part {part_upper} Submission ===\n")

    # Get student IDs interactively
    print("Enter student ID(s):")
    student1 = input("Student 1 ID (sXXXXXXX): ").strip()
    if not student1:
        print("Error: At least one student ID required", file=sys.stderr)
        return 1

    student2 = input("Student 2 ID (leave blank if working alone): ").strip()

    # Validate student IDs
    if not student1.startswith("s"):
        student1 = "s" + student1
    if student2 and not student2.startswith("s"):
        student2 = "s" + student2

    # Generate filename
    if student2:
        filename = f"assignment2{part_upper}-{student1}-{student2}.tar.gz"
    else:
        filename = f"assignment2{part_upper}-{student1}.tar.gz"

    print(f"\nSubmission filename: {filename}")

    # Prepare paths
    root = Path(__file__).parent.parent
    src_dir = root / "src"
    deliverables_dir = root / "deliverables" / part
    deliverables_dir.mkdir(parents=True, exist_ok=True)

    staging_dir = deliverables_dir / f"assignment2{part_upper}"
    tarball_path = deliverables_dir / filename

    # Clean build artifacts first
    print("\nCleaning build artifacts...")
    try:
        subprocess.run(
            ["/usr/bin/make", "clean"],
            cwd=src_dir,
            check=True,
            capture_output=True,
        )
        print("✓ Build cleaned")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("⚠ Warning: make clean failed")

    # Define what to include
    include_patterns = [
        "*.cc",
        "*.h",
        "*.cpp",
        "*.hpp",
        "Makefile",
        "README.md",
        "test_instructions.py",
        "test_output.py",
        "overview.pdf",
    ]

    include_dirs = [
        "tests",
        "testdata",
    ]

    # Part B: include report
    if part == "2b":
        print("\nPart B requires a report (report.pdf or report.md)")
        report_path = input("Path to report file (or leave blank to skip): ").strip()
        if report_path:
            include_patterns.append(report_path)

    print(f"\nFiles to include in {filename}:")
    print("  Source files:")
    for pattern in include_patterns:
        matches = list(src_dir.glob(pattern))
        for match in matches:
            print(f"    - {match.relative_to(src_dir)}")

    print("  Directories:")
    for dirname in include_dirs:
        dir_path = src_dir / dirname
        if dir_path.exists():
            print(f"    - {dirname}/")

    if args.check:
        print("\n✓ Check complete (no tarball created)")
        return 0

    # Confirm before creating
    confirm = input("\nCreate tarball? [Y/n]: ").strip().lower()
    if confirm and confirm != "y":
        print("Cancelled")
        return 0

    # Create staging directory
    if staging_dir.exists():
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True)

    # Copy files
    print("\nCopying files...")
    for pattern in include_patterns:
        for src_file in src_dir.glob(pattern):
            if src_file.is_file():
                dest = staging_dir / src_file.name
                shutil.copy2(src_file, dest)
                print(f"  ✓ {src_file.name}")

    for dirname in include_dirs:
        src_subdir = src_dir / dirname
        if src_subdir.exists():
            dest_subdir = staging_dir / dirname
            shutil.copytree(src_subdir, dest_subdir)
            print(f"  ✓ {dirname}/")

    # Create tarball
    print(f"\nCreating {filename}...")
    with tarfile.open(tarball_path, "w:gz") as tar:
        tar.add(staging_dir, arcname=staging_dir.name)

    # Cleanup staging
    shutil.rmtree(staging_dir)

    # Verify tarball
    file_size = tarball_path.stat().st_size / (1024 * 1024)  # MB
    print(f"\n✓ Submission created successfully!")
    print(f"  Path: {tarball_path}")
    print(f"  Size: {file_size:.2f} MB")

    print(f"\nNext steps:")
    print(f"  1. Verify contents: tar -tzf {tarball_path}")
    print(f"  2. Submit to Brightspace")
    print(
        f"  3. Include student IDs ({student1}"
        + (f", {student2}" if student2 else "")
        + ") in submission text box"
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())
