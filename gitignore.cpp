#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

std::string GetWorkingDirectory() {
  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf)) == nullptr) {
    perror("getcwd");
    return "";
  }
  return std::string(buf);
}

bool IsGitInDir(const std::string &path) {
  DIR *dir = opendir(path.c_str());
  if (!dir) {
    perror("opendir");
    return false;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (((std::string)entry->d_name == ".git") && (entry->d_type == DT_DIR)) {
      closedir(dir);
      return true;
    }
  }
  closedir(dir);
  return false;
}

void WriteToFile(const std::string &path, const std::string &msg) {
  std::ifstream infile(path.c_str());
  std::string line;
  bool alreadyExists = false;
  while (std::getline(infile, line)) {
    if (line == msg) {
      alreadyExists = true;
      break;
    }
  }
  infile.close();
  if (!alreadyExists) {
    std::ofstream outfile(path.c_str(), std::ios_base::app);
    if (!outfile) {
      std::cerr << "ERROR--could not open or create " << path << std::endl;
      return;
    }
    outfile << msg << std::endl;
    std::cout << "Added " << msg << " to " << path << std::endl;
  } else {
    std::cout << msg << " is already in " << path << std::endl;
  }
}

std::string GetRelativePath(const std::string &from, const std::string &to) {
  return std::filesystem::relative(to, from).string();
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
  while (true) {
    if (IsGitInDir(currentdir)) {
      std::string gitignorePath = currentdir + "/.gitignore";
      std::string relativePath =
          GetRelativePath(currentdir, argFileAbsolutePos);
      WriteToFile(gitignorePath, relativePath);
      break;
    } else {
      std::size_t pos = currentdir.find_last_of('/');
      if (pos == std::string::npos) {
        std::cerr << "ERROR--no .git directory found in any parent directories"
                  << std::endl;
        return -1;
      }
      currentdir = currentdir.substr(0, pos);
    }
  }
  return 0;
}

// TODO: Usage isn't exactly how I want it to be. If there's no .git in the
// current directory, ask if the user want to add it to a parent's .git
// directory (not just automatically do it) and if there's more than one .git,
// going up the tree ask which to add to (if any).
// TODO: Think about and handle edge cases. Give meaningful error messages.
