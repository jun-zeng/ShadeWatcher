#include "file.h"

std::vector<std::string> TraverseDir(std::string data_dir) {
	dirent *entry;
	DIR *dp = opendir(data_dir.c_str());
	if (dp == NULL) {
		std::cerr << data_dir + " does not exist or could not be read" << std::endl;
		exit(1);
	}

	std::vector<std::string> dirs_path;
	// whether log files are under current dir
	// if not, we need to traverse sub dir
	while ((entry = readdir(dp))) {
		std::string dir_path(entry->d_name);
		auto pos = dir_path.find("json");
		if (pos != std::string::npos) {
			dirs_path.clear();
			dirs_path.push_back(data_dir + "/");
			break;
		}
		else if (dir_path.front() != '.' and dir_path.find("metadata") == std::string::npos) {
			dirs_path.push_back(data_dir + "/" + dir_path + "/");
		}
	}
	closedir (dp);
	return dirs_path;
}

std::vector<std::string> TraverseFile(std::string dir_path) {
    std::vector<std::string> files_path;
    dirent *entry;
	DIR *dp = opendir(dir_path.c_str());
	if (dp == NULL) {
		std::cerr << dir_path << " does not exist or could not be read" << std::endl;
		exit(1);
	}

    while ((entry = readdir(dp))) {
		std::string log_path(entry->d_name);
        files_path.push_back(log_path);
	}
	closedir (dp);
    return files_path;
}

std::vector<std::string> TraverseJsonFile(std::string dir_path) {
	// collect and sort log file names in data_dir
	// must process events Chronologically due to event dependency
	dirent *entry;
	DIR *dp = opendir(dir_path.c_str());
	if (dp == NULL) {
		std::cerr << dir_path << " does not exist or could not be read" << std::endl;
		exit(1);
	}
	std::vector<int> data_file_idx;
	std::string prefix;
	while ((entry = readdir(dp))) {
		std::string log_path(entry->d_name);
		auto pos = log_path.find("json.");
		if (pos != std::string::npos) {
			std::string file_idx = log_path.substr(pos + 5);
			data_file_idx.push_back(std::stoi(file_idx));
		}
		else if (log_path.find("json") != std::string::npos) {
			prefix = log_path;
		}
	}
	closedir (dp);

	// collect log file paths in chronological order
	std::sort(data_file_idx.begin(), data_file_idx.end());
	std::vector<std::string> files_path;

	files_path.push_back(dir_path + prefix);
	for (auto idx: data_file_idx) {
		files_path.push_back(dir_path + prefix + "." + std::to_string(idx));
	}
	return files_path;
}

std::vector<std::string> TraverseBeatDir(std::string beat_dir) {
	// traverse auditbeat parent directory & collect basic processing unit dir
	struct dirent *entry;
	DIR *dp = opendir(beat_dir.c_str());
	if (dp == NULL) {
		std::cerr << beat_dir << " does not exist or could not be read " << __FILE__ << " " <<  __LINE__ << std::endl;
		exit(EXIT_FAILURE);
	}

	std::vector<std::string> auditbeat_dirs_path;
	// whether auditbeat log files are under current dir
	// if not, we need to traverse sub dir
	while ((entry = readdir(dp))) {
		std::string dir_path(entry->d_name);
		auto pos = dir_path.find("auditbeat");
		if (pos != std::string::npos) {
			auditbeat_dirs_path.clear();
			auditbeat_dirs_path.push_back(beat_dir + "/");
			break;
		}
		else if (dir_path.front() != '.') {
			auditbeat_dirs_path.push_back(beat_dir + "/" + dir_path + "/");
		}
	}
	closedir(dp);
	return auditbeat_dirs_path;
}

std::vector<std::string> CollectBeatFile(std::string data_dir) {
	// collect and sort log file names in auditbeat_data_dir
	// must process events Chronologically due to event dependency
	struct dirent *entry;
	DIR *dp = opendir(data_dir.c_str());
	if (dp == NULL) {
		std::cerr << data_dir << " does not exist or could not be read " << __FILE__ << " " <<  __LINE__ << std::endl;
		exit(EXIT_FAILURE);
	}
	std::vector<int> data_file_idx;
	while ((entry = readdir(dp))) {
		std::string log_path(entry->d_name);
		auto pos = log_path.find("auditbeat.");
		if (pos != std::string::npos) {
			std::string file_idx = log_path.substr(pos + 10);
			data_file_idx.push_back(std::stoi(file_idx));
		} 
	}
	std::sort(data_file_idx.rbegin(), data_file_idx.rend());

	// collect log file paths in chronological order
	std::vector<std::string> auditbeat_files_path;
	for (auto idx: data_file_idx) {
		auditbeat_files_path.push_back(data_dir + "auditbeat." + std::to_string(idx));
	}
	auditbeat_files_path.push_back(data_dir + "auditbeat");

	closedir (dp);
	return auditbeat_files_path;
}

std::vector<std::string> CollectJsonFile(std::string data_dir) {
	std::vector<std::string> files;
	
	if (data_dir.find(".json") != std::string::npos) {
		// specify a json file in dataset
		files.push_back(data_dir);
	}
	else {
		// specify dir in dataset
		std::vector<std::string> dirs = TraverseDir(data_dir);
		for (auto dir: dirs) {
			auto _files = TraverseJsonFile(dir);
			files.insert(files.end(), _files.begin(), _files.end());
		}
	}
    
	return files;
}
