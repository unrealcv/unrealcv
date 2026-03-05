import subprocess
from pathlib import Path


def run(command: list[str]) -> None:
    subprocess.run(command, check=True)


def install_if_exists(requirement_path: Path) -> None:
    if requirement_path.exists():
        run(["python3", "-m", "pip", "install", "-r", str(requirement_path)])


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]

    run(["python3", "-m", "pip", "install", "--upgrade", "pip"])

    pyproject = repo_root / "client/python/pyproject.toml"
    if pyproject.exists():
        run(["python3", "-m", "pip", "install", "-e", str(repo_root / "client/python")])

    install_if_exists(repo_root / "docs/requirements.txt")
    install_if_exists(repo_root / "test/requirements.txt")

    print("Development bootstrap completed successfully")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
