#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <unistd.h>
#include <vector>

std::string GetWorkingDirectory() {
  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf)) == nullptr) {
    perror("getcwd");
    return "";
  }
  return std::string(buf);
}

bool IsGitInDir(const std::string &path) {
  std::filesystem::path gitDirPath = path + "/.git";
  return std::filesystem::is_directory(gitDirPath);
}

void WriteToFile(const std::string &path, const std::string &msg) {
  std::set<std::string> lines;
  std::ifstream infile(path);
  std::string line;
  while (std::getline(infile, line)) {
    lines.insert(line);
  }
  if (lines.find(msg) != lines.end()) {
    std::cerr << msg << " is already in " << path << std::endl;
    return;
  }
  std::ofstream outfile(path, std::ios_base::app);
  if (!outfile) {
    std::cerr << "ERROR--could not open or create " << path << std::endl;
    return;
  }
  outfile << msg << std::endl;
  std::cout << "Added " << msg << " to " << path << std::endl;
}

std::string GetRelativePath(const std::string &from, const std::string &to) {
  return std::filesystem::relative(to, from).string();
}

std::string AskUserForChoice(const std::vector<std::string> &choices,
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
    std::cerr << "ERROR--usage: gitignore path/to/file/to/ignore" << std::endl;
    return -1;
  }
  std::string currentdir = GetWorkingDirectory();
  if (currentdir.empty()) {
    return -1;
  }
  const std::string argFileAbsolutePos = currentdir + '/' + argv[1];
  if (IsGitInDir(currentdir)) {
    std::string gitignorePath = currentdir + "/.gitignore";
    std::string relativePath = GetRelativePath(currentdir, argFileAbsolutePos);
    WriteToFile(gitignorePath, relativePath);
    return 0;
  }
  int parentDirLimit = 50;
  std::vector<std::string> gitDirs;
  while (parentDirLimit--) {
    if (IsGitInDir(currentdir)) {
      gitDirs.push_back(currentdir);
    }
    if (currentdir == "/" || parentDirLimit == 0) {
      break;
    }
    currentdir = std::filesystem::path(currentdir).parent_path().string();
  }
  if (gitDirs.empty()) {
    std::cerr << "ERROR--no .git directory found in any parent directory"
              << std::endl;
    return -1;
  }
  std::string chosenGitDir = AskUserForChoice(
      gitDirs, "Select the .git directory to add the file to:");
  if (chosenGitDir.empty()) {
    std::cerr << "ERROR--no directory selected" << std::endl;
    return -1;
  }
  std::string gitignorePath = chosenGitDir + "/.gitignore";
  std::string relativePath = GetRelativePath(chosenGitDir, argFileAbsolutePos);
  WriteToFile(gitignorePath, relativePath);
  return 0;
}
