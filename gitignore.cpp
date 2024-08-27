#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <unistd.h>
#include <vector>

void printError(const std::string &msg) {
  std::cerr << "\033[1;31merror\033[0m\t" << msg << std::endl;
}

std::string getWorkingDirectory() {
  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf)) == nullptr) {
    perror("getcwd");
    return "";
  }
  return std::string(buf);
}

bool isGitFolderInDir(const std::string &path) {
  return std::filesystem::is_directory(path + "/.git");
}

void writeToFile(const std::string &path, const std::string &msg) {
  std::set<std::string> lines;
  std::ifstream infile(path);
  std::string line;
  while (std::getline(infile, line)) {
    lines.insert(line);
  }
  if (lines.find(msg) != lines.end()) {
    std::cout << msg << " is already in " << path << std::endl;
    return;
  }
  std::ofstream outfile(path, std::ios_base::app);
  if (!outfile) {
    printError("could not open or create " + path);
    return;
  }
  outfile << msg << std::endl;
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printError("usage: gitignore path/to/file/to/ignore");
    return -1;
  }
  std::string current_directory = getWorkingDirectory();
  if (current_directory.empty()) {
    return -1;
  }
  const std::string arg_abs_path = current_directory + '/' + argv[1];
  if (std::filesystem::exists(arg_abs_path) == false) {
    printError(arg_abs_path + " does not exist");
    return -1;
  }
  if (isGitFolderInDir(current_directory)) {
    std::string gitignore_file_path = current_directory + "/.gitignore";
    std::string relativePath = getRelativePath(current_directory, arg_abs_path);
    writeToFile(gitignore_file_path, relativePath);
    return 0; // return instantly if .git in current directory
  }
  int parent_traversal_limit = 50;
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
    return -1;
  }
  std::string chosen_git_dir = askUserForChoice(
      git_dirs, "Select the .git directory to add the file to:");
  if (chosen_git_dir.empty()) {
    printError("no directory selected");
    return -1;
  }
  std::string gitignore_file_path = chosen_git_dir + "/.gitignore";
  std::string relativePath = getRelativePath(chosen_git_dir, arg_abs_path);
  writeToFile(gitignore_file_path, relativePath);
  return 0;
}
