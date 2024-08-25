# gitignore

A simple CLI tool for automatically adding a file to `.gitignore` in the root of a Git repo. It scans up the directory tree to find a `.git` folder, updates the `.gitignore` with the file path if it's not already listed, uses relative paths.

Uses C++17 or later.