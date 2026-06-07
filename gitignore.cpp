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
  // modify 'msg' directly if it's a directory -- this is why msg is mutable (could instead be a copy, but whatever)
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
  // This limit is a bit arbitrary and cute.
	// It's incredibly unlikely that a user would be more than 1000 directories deep, though.
	// A directory tree is finite and therefore a loop that terminates at root
	// is an invariant; but still, I think this defensive limits is safer, in line
	// with a kind of NASA style.
  int parent_traversal_limit = 1000;

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

int processPath(const std::string &path, const std::string &git_dir) {
    std::string abs_path = std::filesystem::absolute(path).string();
    if (!std::filesystem::exists(abs_path)) {
        printError(abs_path + " does not exist");
        return -1;
    }

    std::string gitignore_file_path = git_dir + "/.gitignore";
    std::string relative_path = getRelativePath(git_dir, abs_path);
    writeToFile(gitignore_file_path, relative_path);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printError("usage (assuming gitignore is in $PATH): $ gitignore path1 [path2 ...]");
        return -1;
    }
    std::string current_directory = std::filesystem::current_path().string();
    if (current_directory.empty()) {
        return -1;
    }
    std::string git_dir;
    // if already inside a git repo, no need to search up.
    if (isGitFolderInDir(current_directory)) {
        git_dir = current_directory;
    } else {
        git_dir = findGitDirectory(current_directory);
        if (git_dir.empty()) {
            return -1; // error already printed
        }
    }
    // process each arg
    int overall_status = 0;
    for (int i = 1; i < argc; ++i) {
        int status = processPath(argv[i], git_dir);
        if (status != 0) {
            overall_status = status; // record failure but continue processing others
        }
    }
    return overall_status;
}
