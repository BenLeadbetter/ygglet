"""CLI entry point for ygglet-build."""

import os
import shutil
import subprocess
import sys
from pathlib import Path

import click

# Files that must exist at the project root
ROOT_MARKERS = ["Cargo.toml", "CMakeLists.txt", "conanfile.py"]


def validate_project_root(path: Path) -> Path:
    """Validate that path is the ygglet project root."""
    path = path.resolve()
    if not path.is_dir():
        click.echo(f"Error: {path} is not a directory", err=True)
        sys.exit(1)

    missing = [m for m in ROOT_MARKERS if not (path / m).exists()]
    if missing:
        click.echo(f"Error: {path} does not appear to be the project root", err=True)
        click.echo(f"Missing: {', '.join(missing)}", err=True)
        sys.exit(1)

    return path


def run_command(args: list[str], cwd: Path, env: dict | None = None) -> None:
    """Run a command, merging env with current environment."""
    full_env = os.environ.copy()
    if env:
        full_env.update(env)
    subprocess.run(args, cwd=cwd, env=full_env, check=True)


def get_build_type(release: bool) -> str:
    """Get CMake build type string."""
    return "Release" if release else "Debug"


# =============================================================================
# Main CLI group
# =============================================================================


@click.group()
@click.option(
    "--root",
    type=click.Path(exists=True, file_okay=False, path_type=Path),
    default=Path.cwd(),
    help="Project root directory (default: current directory)",
)
@click.option(
    "--build-dir",
    type=click.Path(path_type=Path),
    default=None,
    help="Build directory (default: <root>/build)",
)
@click.pass_context
def main(ctx: click.Context, root: Path, build_dir: Path | None) -> None:
    """Build tooling for the ygglet project."""
    ctx.ensure_object(dict)
    root = validate_project_root(root)
    ctx.obj["root"] = root
    ctx.obj["build_dir"] = (build_dir or root / "build").resolve()


# =============================================================================
# Conan subcommand group
# =============================================================================


@main.group()
def conan() -> None:
    """Conan dependency management commands."""
    pass


def do_conan_install(root: Path, build_dir: Path, build_type: str) -> None:
    """Run conan install to generate toolchain and dependency files."""
    click.echo(f"[conan] Installing dependencies ({build_type})...")
    build_dir.mkdir(parents=True, exist_ok=True)
    run_command(
        [
            "conan",
            "install",
            str(root),
            f"--output-folder={build_dir}",
            f"--settings=build_type={build_type}",
            "--build=missing",
            # Disable CMakeUserPresets.json (we use toolchain file directly)
            "-c:h",
            "tools.cmake.cmaketoolchain:user_presets=",
        ],
        cwd=root,
    )


@conan.command("install")
@click.option("--release", is_flag=True, help="Install for release build")
@click.pass_context
def conan_install(ctx: click.Context, release: bool) -> None:
    """Install conan dependencies."""
    do_conan_install(ctx.obj["root"], ctx.obj["build_dir"], get_build_type(release))


# =============================================================================
# CMake subcommand group
# =============================================================================


@main.group()
def cmake() -> None:
    """CMake build commands."""
    pass


def do_cmake_configure(root: Path, build_dir: Path, build_type: str) -> None:
    """Configure the CMake project."""
    cmake_dir = build_dir / "cmake"
    toolchain_file = build_dir / "conan_toolchain.cmake"

    if not toolchain_file.exists():
        click.echo(
            f"Error: {toolchain_file} not found. Run 'conan install' first.", err=True
        )
        sys.exit(1)

    click.echo("[cmake] Configuring...")
    cmake_dir.mkdir(parents=True, exist_ok=True)
    run_command(
        [
            "cmake",
            "-S",
            str(root),
            "-B",
            str(cmake_dir),
            f"-DCMAKE_BUILD_TYPE={build_type}",
            f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
        ],
        cwd=root,
    )


def do_cmake_build(build_dir: Path, build_type: str) -> None:
    """Build the CMake project."""
    cmake_dir = build_dir / "cmake"

    if not cmake_dir.exists():
        click.echo(
            "Error: CMake build directory not found. Run 'cmake configure' first.",
            err=True,
        )
        sys.exit(1)

    click.echo("[cmake] Building...")
    run_command(
        ["cmake", "--build", str(cmake_dir), "--config", build_type],
        cwd=cmake_dir,
    )


def do_cmake_test(build_dir: Path) -> None:
    """Run CMake tests."""
    cmake_dir = build_dir / "cmake"

    if not cmake_dir.exists():
        click.echo(
            "Error: CMake build directory not found. Run 'cmake build' first.",
            err=True,
        )
        sys.exit(1)

    click.echo("[cmake] Running tests...")
    run_command(
        ["ctest", "--test-dir", str(cmake_dir), "--output-on-failure"],
        cwd=cmake_dir,
    )


@cmake.command("configure")
@click.option("--release", is_flag=True, help="Configure for release build")
@click.pass_context
def cmake_configure(ctx: click.Context, release: bool) -> None:
    """Configure the CMake project."""
    do_cmake_configure(ctx.obj["root"], ctx.obj["build_dir"], get_build_type(release))


@cmake.command("build")
@click.option("--release", is_flag=True, help="Build in release mode")
@click.pass_context
def cmake_build(ctx: click.Context, release: bool) -> None:
    """Build the CMake project."""
    do_cmake_build(ctx.obj["build_dir"], get_build_type(release))


@cmake.command("test")
@click.pass_context
def cmake_test(ctx: click.Context) -> None:
    """Run CMake tests."""
    do_cmake_test(ctx.obj["build_dir"])


# =============================================================================
# Cargo subcommand group
# =============================================================================


@main.group()
def cargo() -> None:
    """Cargo build commands."""
    pass


def cargo_env(build_dir: Path) -> dict:
    """Get environment variables for cargo commands."""
    return {"CARGO_TARGET_DIR": str(build_dir / "cargo")}


def do_cargo_install(root: Path, build_dir: Path) -> None:
    """Install cargo dependencies."""
    click.echo("[cargo] Fetching dependencies...")
    run_command(["cargo", "fetch"], cwd=root, env=cargo_env(build_dir))


def do_cargo_build(root: Path, build_dir: Path, release: bool) -> None:
    """Build the Cargo workspace."""
    args = ["cargo", "build"]
    if release:
        args.append("--release")

    click.echo("[cargo] Building...")
    run_command(args, cwd=root, env=cargo_env(build_dir))


def do_cargo_test(root: Path, build_dir: Path) -> None:
    """Run Cargo tests."""
    click.echo("[cargo] Running tests...")
    run_command(["cargo", "test"], cwd=root, env=cargo_env(build_dir))


def do_cargo_clippy(root: Path, build_dir: Path) -> None:
    """Run Cargo clippy."""
    click.echo("[cargo] Running clippy...")
    run_command(
        ["cargo", "clippy", "--", "-D", "warnings"],
        cwd=root,
        env=cargo_env(build_dir),
    )


@cargo.command("install")
@click.pass_context
def cargo_install(ctx: click.Context) -> None:
    """Fetch cargo dependencies."""
    do_cargo_install(ctx.obj["root"], ctx.obj["build_dir"])


@cargo.command("build")
@click.option("--release", is_flag=True, help="Build in release mode")
@click.pass_context
def cargo_build(ctx: click.Context, release: bool) -> None:
    """Build the Cargo workspace."""
    do_cargo_build(ctx.obj["root"], ctx.obj["build_dir"], release)


@cargo.command("test")
@click.pass_context
def cargo_test(ctx: click.Context) -> None:
    """Run Cargo tests."""
    do_cargo_test(ctx.obj["root"], ctx.obj["build_dir"])


@cargo.command("clippy")
@click.pass_context
def cargo_clippy(ctx: click.Context) -> None:
    """Run Cargo clippy linter."""
    do_cargo_clippy(ctx.obj["root"], ctx.obj["build_dir"])


# =============================================================================
# Top-level orchestration commands
# =============================================================================


@main.command("install")
@click.option("--release", is_flag=True, help="Install for release build")
@click.pass_context
def install(ctx: click.Context, release: bool) -> None:
    """Install all dependencies (conan install + cargo fetch)."""
    root = ctx.obj["root"]
    build_dir = ctx.obj["build_dir"]
    build_type = get_build_type(release)

    do_conan_install(root, build_dir, build_type)
    do_cargo_install(root, build_dir)


@main.command("configure")
@click.option("--release", is_flag=True, help="Configure for release build")
@click.pass_context
def configure(ctx: click.Context, release: bool) -> None:
    """Configure the project (conan install + cmake configure)."""
    root = ctx.obj["root"]
    build_dir = ctx.obj["build_dir"]
    build_type = get_build_type(release)

    do_conan_install(root, build_dir, build_type)
    do_cmake_configure(root, build_dir, build_type)


def ensure_configured(root: Path, build_dir: Path, build_type: str) -> None:
    """Ensure conan and cmake are configured, running them if needed."""
    toolchain_file = build_dir / "conan_toolchain.cmake"
    cmake_dir = build_dir / "cmake"

    if not toolchain_file.exists():
        do_conan_install(root, build_dir, build_type)

    if not cmake_dir.exists():
        do_cmake_configure(root, build_dir, build_type)


@main.command("build")
@click.option("--release", is_flag=True, help="Build in release mode")
@click.pass_context
def build(ctx: click.Context, release: bool) -> None:
    """Build the project (cmake build + cargo build)."""
    root = ctx.obj["root"]
    build_dir = ctx.obj["build_dir"]
    build_type = get_build_type(release)

    ensure_configured(root, build_dir, build_type)
    do_cmake_build(build_dir, build_type)
    do_cargo_build(root, build_dir, release)


@main.command("test")
@click.option("--release", is_flag=True, help="Build and test in release mode")
@click.pass_context
def test(ctx: click.Context, release: bool) -> None:
    """Run all tests (cmake test + cargo test)."""
    root = ctx.obj["root"]
    build_dir = ctx.obj["build_dir"]
    build_type = get_build_type(release)

    ensure_configured(root, build_dir, build_type)
    do_cmake_build(build_dir, build_type)
    do_cmake_test(build_dir)
    do_cargo_test(root, build_dir)


@main.command("clean")
@click.pass_context
def clean(ctx: click.Context) -> None:
    """Clean build artifacts."""
    build_dir = ctx.obj["build_dir"]

    if build_dir.exists():
        click.echo(f"Removing {build_dir}...")
        shutil.rmtree(build_dir)
        click.echo("Clean complete.")
    else:
        click.echo("Nothing to clean.")


if __name__ == "__main__":
    main()
