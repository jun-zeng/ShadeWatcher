#include "readlog.h"

LogLoader::LogLoader(std::string log_path){
	path = log_path;
	f_input = new std::fstream(log_path, std::ios::in);
	event_num = 0;
}

LogLoader::~LogLoader(){
	std::vector<std::pair<event_t, uint64_t>>().swap(offset);
	f_input->close();
	delete(f_input);
}

void LogLoader::LoadBEATLog() {
	seq_t seq = 0;
	uint64_t offset_ = 0;
	
	// std::cout << "loading and parsing log" << std::endl;
	std::string line;
	while(std::getline(*f_input, line)) {
		// read sequence number 
		auto found_seq = line.find("\"sequence\"");
		if (found_seq == std::string::npos) {
			// seq does not matter for non syscall events
			offset_ = f_input->tellg(); 
			continue;
		}
	
		auto seq_start = found_seq + 11;
		auto seq_end = line.find(",", seq_start + 1);
		if (seq_end == std::string::npos) {
			seq_end = line.find("}", seq_start) + 1;
		}

		seq = std::stoi(line.substr(seq_start, seq_end - seq_start));
		offset.push_back(std::make_pair(seq, offset_));
		offset_ = f_input->tellg();
		event_num += 1;
	}
	std::sort(offset.begin(), offset.end());
	f_input->clear();
}

void LogLoader::LoadDARPALog() {
	uint64_t offset_ = 0;
	std::string line;
	while(std::getline(*f_input, line)) {
		offset.push_back(std::make_pair(event_num, offset_));
		offset_ = f_input->tellg();
		event_num += 1;
	}
	f_input->clear();
}

bool LogLoader::GetEvent(event_t id, Json::Value &event){
	f_input->seekg(offset.at(id).second, std::ios::beg);

	std::string line;
	getline(*f_input, line);

	Json::CharReaderBuilder builder {};
	auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

	JSONCPP_STRING errs;
	const auto parseSuccessful = reader->parse(line.c_str(),
											   line.c_str() + line.length(),
											   &event,
											   &errs);

	if (!parseSuccessful){
		std::cerr << "Fail to parse file:" << path << std::endl;
		return false;
	}

	return true;
}

void LogLoader::PrintBEATEvent(const Json::Value event){
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string pid = Jval2str(event["process"]["pid"]);
	std::string exe = Jval2str(event["process"]["exe"]);
	std::string syscall = Jval2str(event["auditd"]["data"]["syscall"]);
	std::cout << "\nseq " << uint128tostring(seq);
	std::cout << "\tproc: pid " << pid;
	std::cout << "\texe" << exe;
	std::cout << "\tsyscall: " << syscall << std::endl;
}
