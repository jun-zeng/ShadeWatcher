#include "parserbeat.h"

void Loadmetainfo(std::string auditbeat_dir_path, KG *infotbl) {
	std::string proc_path = auditbeat_dir_path + "/procinfo";
	std::string fd_path = auditbeat_dir_path + "/fdinfo";
	std::string socket_path = auditbeat_dir_path + "/socketinfo";

	infotbl->LoadProc(proc_path);
	infotbl->LoadFd(fd_path);
	infotbl->LoadSocket(socket_path);
}

void KGBeatParse(std::string auditbeat_file_path, KG *infotbl) {
	auto start = OverheadStart();

	// Read Log
	std::cout << "beat file: " << auditbeat_file_path << std::endl;
	LogLoader *logload = new LogLoader(auditbeat_file_path);
	logload->LoadBEATLog();
	event_t event_num = logload->event_num;

	Json::Value event;
	Triplet t(event, infotbl);

	// progress bar parameters
	#ifdef BAR
		float progress = 0.0;
		float addup = 1.0 / event_num;
		int width = 70;
	#endif

	for (event_t id = 0; id < event_num; id++) {
		// Progress bar: high overhead (0.71s -> 0.93s)
		#ifdef BAR
			ProgressBar(width, progress, addup);
		#endif

		// read event in json format from audit logs
		logload->GetEvent(id, event);
		infotbl->event_num += 1;

		// ignore failed syscall (exception: EINPROGRESS in connect syscall)
		std::string status = Jval2str(event["auditd"]["result"]);
		if (status.compare("fail") == 0) {
			std::string ret_value = Jval2str(event["auditd"]["data"]["exit"]);
			if (ret_value.compare("EINPROGRESS") != 0) {
				continue;
			}
		}

		// translate events into triplets
		if (event["auditd"]["data"].isMember("syscall")){
			t.Event2triplet();
		}
		// add login process
		else if (event["auditd"]["data"].isMember("acct")) {
			std::string op = Jval2str(event["auditd"]["data"]["op"]);
			if (op.compare("PAM:accounting") == 0 || op.compare("PAM:session_open") == 0) {
				infotbl->InsertLoginProc(event, logload, id + 1);
			}
		}
	}
	delete(logload);

	// recove names for processes without being executed
	infotbl->ProcInfoRecover();

	// compute e_id for each edges in KGEdgeList and create KGEdgeTable
	// we didn't compute e_id when creating edges becasue n_id may change
	// and using unordered map plus extract incur high overhead
	infotbl->GenKGEdgeTable();

	// reduce noise:
	ReduceNoise noise_reduction(infotbl);
	noise_reduction.DeleteNoise();

	// cleanup
	// 1. translate pipe nodes into pipe edges
	// 2. record external sockets -> delete local sockets
	infotbl->DeletePipeEdge();
	infotbl->DeleteLocalSocket();

	infotbl->edge_num += infotbl->KGEdgeTable.size();

	OverheadEnd(start, "KG construction");
}
