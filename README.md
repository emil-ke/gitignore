# gitignore

![Screenshot of terminal using the CLI tool](/assets/gitignore_screenshot.webp "CLI usage example")

A small CLI tool for quickly adding files or folders to the root `.gitignore` of a Git repo no matter how deep in the project tree you are. It finds the nearest `.git` folder, computes the correct relative path, and appends it to `.gitignore` if it's not already listed. This makes for a nicer repo that isn't populated with a bunch of `.gitignore` files in every subdirectory, and it's much faster than manually editing a `.gitignore` file.

### Usage

```bash
gitignore path/to/file_or_dir
```

It adds the given path (relative to repo root) to `.gitignore`, skipping if it's already in there.

### Build

Requires a C++17-compatible compiler. E.g.,

```bash
clang++ gitignore.cpp -o gitignore -std=c++17 -O2
```

Then move it somewhere in your PATH:

```bash
mv gitignore ~/.local/bin/
```

That's it. Now you may call `gitignore` from anywhere inside a Git project.

```bash
gitignore super_duper_secret_information.md
```
