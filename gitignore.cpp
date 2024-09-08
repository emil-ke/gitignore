#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

void printError(const std::string &msg) {
  std::cerr << "\033[1;31merror\033[0m\t" << msg << std::endl;
}

bool isGitFolderInDir(const std::string &path) {
  return std::filesystem::is_directory(path + "/.git");
}

void writeToFile(const std::string &path, std::string &msg) {
    // modify 'msg' directly if it's a directory
    if (std::filesystem::is_directory(msg) && msg.back() != '/') {
        msg += "/";
    }
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::app);
    if (!file) {
        printError("could not open " + path);
        return;
    }
    // read file to an unordered_set for fast lookup
    std::unordered_set<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.insert(line);
    }
    for (auto &line : lines) {
      if (line == msg) {
        std::cout << msg << " is already in " << path << std::endl;
        return;
    }
  }
    file.clear(); // clear error flags
    file.seekp(0, std::ios::end); // move the write pointer to the end
    file << msg << std::endl;
    std::cout << "Added " << msg << " to " << path << std::endl;
}

std::string getRelativePath(const std::string &from, const std::string &to) {
  return std::filesystem::relative(to, from).string();
}

std::string askUserForChoice(const std::vector<std::string> &choices,
                             const std::string &message) {
  std::cout << message << std::endl;
  for (size_t i = 0; i < choices.size(); ++i) {
    std::cout << i + 1 << ": " << choices[i] << std::endl;
  }
  std::cout << "Enter number: ";
  int choice;
  while (!(std::cin >> choice) || choice < 1 ||
         choice > static_cast<int>(choices.size())) {
    std::cin.clear(); // clear the error flag
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                    '\n'); // discard invalid input
    std::cout << "Invalid choice. Please enter a number between 1 and "
              << choices.size() << ": ";
  }
  return choices[choice - 1];
}

std::string findGitDirectory(std::string &current_directory) {
  // TODO (?) maybe add flag to change to some other depth.
  // Highly unlikely that user would be more than 60 directories deep though.
  int parent_traversal_limit = 60;

  std::vector<std::string> git_dirs;
  while (parent_traversal_limit--) {
    if (isGitFolderInDir(current_directory)) {
      git_dirs.push_back(current_directory);
    }
    if (current_directory == "/" || parent_traversal_limit == 0) {
      break;
    }
    current_directory =
        std::filesystem::path(current_directory).parent_path().string();
  }
  if (git_dirs.empty()) {
    printError("no .git directory found in any parent directory");
    return "";
  }
  if (git_dirs.size() == 1) {
    return git_dirs[0];
  }
  return askUserForChoice(git_dirs, "Select the .git directory to add to: ");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printError("usage: $ gitignore path/to/thing/to/ignore");
    return -1;
  }
  std::string current_directory = std::filesystem::current_path().string();
  if (current_directory.empty()) {
    return -1;
  }
  std::string arg_abs_path = std::filesystem::path(current_directory) / argv[1];
  if (!std::filesystem::exists(arg_abs_path)) {
    printError(arg_abs_path + " does not exist");
    return -1;
  }
  if (isGitFolderInDir(current_directory)) {
    std::string gitignore_file_path = current_directory + "/.gitignore";
    std::string relative_path =
        getRelativePath(current_directory, arg_abs_path);
    writeToFile(gitignore_file_path, relative_path);
    return 0;
  }
  std::string chosen_git_dir = findGitDirectory(current_directory);
  if (chosen_git_dir.empty()) {
    return -1; // Error message already printed in findGitDirectory
  }
  std::string gitignore_file_path = chosen_git_dir + "/.gitignore";
  std::string relative_path = getRelativePath(chosen_git_dir, arg_abs_path);
  writeToFile(gitignore_file_path, relative_path);
  return 0;
}
