#include "tripletdar.h"

Triplet::Triplet(const Json::Value &_event, KG* _infotbl, int _dataset_type): event(_event) {
	infotbl = _infotbl;
	dataset_type = _dataset_type;
	event_analyzed = 0;

	// Todo: Unknown event: EVENT_ACCEPT EVENT_CHANGE_PRINCIPAL EVENT_EXIT
	// EVENT_LINK EVENT_MODIFY_FILE_ATTRIBUTES EVENT_MPROTECT EVENT_TRUNCATE
	// EVENT_UNIT EVENT_BOOT EVENT_MMAP EVENT_OTHER

	syscallMap["EVENT_EXECUTE"] = SyscallType_t::Execve;
	syscallMap["EVENT_CLONE"] = SyscallType_t::Clone;
	syscallMap["EVENT_FORK"] = SyscallType_t::Clone;
	syscallMap["EVENT_OPEN"] = SyscallType_t::Open;
	syscallMap["EVENT_CLOSE"] = SyscallType_t::Close;
	syscallMap["EVENT_CONNECT"] = SyscallType_t::Connect;
	syscallMap["EVENT_UNLINK"] = SyscallType_t::Delete;
	syscallMap["EVENT_READ"] = SyscallType_t::Read;
	syscallMap["EVENT_WRITE"] = SyscallType_t::Write;
	syscallMap["EVENT_RECVFROM"] = SyscallType_t::Recvfrom;
	syscallMap["EVENT_SENDTO"] = SyscallType_t::Sendto;
	syscallMap["EVENT_RECVMSG"] = SyscallType_t::Recvmsg;
	syscallMap["EVENT_SENDMSG"] = SyscallType_t::Sendmsg;
	syscallMap["EVENT_RENAME"] = SyscallType_t::Rename;
	syscallMap["EVENT_READ_SOCKET_PARAMS"] = SyscallType_t::Recv;
	syscallMap["EVENT_WRITE_SOCKET_PARAMS"] = SyscallType_t::Send;
	syscallMap["EVENT_LOADLIBRARY"] = SyscallType_t::Load;
	syscallMap["EVENT_CREATE_OBJECT"] = SyscallType_t::Create;
	syscallMap["EVENT_UPDATE"] = SyscallType_t::Update;
}

Triplet::~Triplet() {
	syscallMap.clear();
}

void Triplet::Event2triplet() {
	std::string syscall_str = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["type"]);

	// Temporarily deal with the EVENT_BOOT
	if (syscall_str == "EVENT_BOOT" || syscall_str == "EVENT_MMAP" || syscall_str == "EVENT_OTHER" || syscall_str == "EVENT_MPROTECT")
		return;

	std::string uuid = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["uuid"]);
	std::string sub = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["subject"]["com.bbn.tc.schema.avro.cdm18.UUID"]);
	std::string obj = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["predicateObject"]["com.bbn.tc.schema.avro.cdm18.UUID"]);
	seq_t seq = Jval2int(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["sequence"]["long"]);
	std::string timestamp = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["timestampNanos"]);	
	// Currently, we treat each graph as a session. That is, there is no session concept in darpa dataset
	// Todo: {"datum":{"com.bbn.tc.schema.avro.cdm18.StartMarker":{"sessionNumber":3}},"CDMVersion":"18","source":"SOURCE_LINUX_SYSCALL_TRACE"}
	sess_t sess = 0;

	std::hash<std::string> hasher;
	hash_t sub_id = (hash_t)hasher(sub);
	hash_t obj_id = (hash_t)hasher(obj);
	hash_t e_id = (hash_t)hasher(uuid);

	switch(syscallMap[syscall_str]) {
		case SyscallType_t::Execve: {
			SyscallExecve(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Open: {
			event_analyzed++;
			break;
		}
		case SyscallType_t::Close: {
			event_analyzed++;
			break;
		}
		case SyscallType_t::Clone: {
			SyscallClone(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Connect: {
			SyscallConnect(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Delete: {
			SyscallDelete(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Read: {
			SyscallRead(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Write: {
			SyscallWrite(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Recvfrom: {
			SyscallRecvfrom(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Sendto: {
			SyscallSendto(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Sendmsg: {
			SyscallSendmsg(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Recvmsg: {
			SyscallRecvmsg(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Send: {
			SyscallSend(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Recv: {
			SyscallRecv(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Rename: {
			std::string obj2 = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["predicateObject2"]["com.bbn.tc.schema.avro.cdm18.UUID"]);
			SyscallRename(seq, sess, sub_id, obj_id, obj, obj2, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Load: {
			SyscallLoad(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Create: {
			SyscallCreate(seq, sess, sub_id, obj_id, e_id, timestamp);
			event_analyzed++;
			break;
		}
		case SyscallType_t::Update: {
			std::string obj2 = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Event"]["predicateObject2"]["com.bbn.tc.schema.avro.cdm18.UUID"]);
			SyscallUpdate(seq, sess, sub_id, obj_id, obj, obj2, timestamp);
			event_analyzed++;
			break;
		}
		default: {
			break;
		}
	}
}

void Triplet::SyscallClone(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id , std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Clone, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
}

void Triplet::SyscallExecve(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e;
	if (dataset_type == 0) {
		e = new KGEdge (sub_id, obj_id, EdgeType_t::Execve, seq, sess, e_id, timestamp);
	}
	else if (dataset_type == 1) {
		e = new KGEdge (obj_id, sub_id, EdgeType_t::Execve, seq, sess, e_id, timestamp);
	}
	else {
		return;
	}

	infotbl->InsertEdge(e_id, e);
	// file => proc 
	// cannot do it in this way, do we need a search here?
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallOpen(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Open, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);	
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallConnect(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Connect, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallDelete(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Delete, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallRead(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (obj_id, sub_id, EdgeType_t::Read, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallWrite(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Write, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallRecvfrom(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (obj_id, sub_id, EdgeType_t::Recv, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallSendto(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Send, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallSendmsg(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Send, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallRecvmsg(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (obj_id, sub_id, EdgeType_t::Recv, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallSend(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Send, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallRecv(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (obj_id, sub_id, EdgeType_t::Recv, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallRename(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, std::string obj, std::string obj2, std::string timestamp) {
	std::hash<std::string> hasher;
	hash_t obj2_id = (hash_t)hasher(obj2);

	std::string sess_str = std::to_string(sess);

	// sess string with obj to make up a new edge id
	hash_t e1_id = (hash_t)hasher(sess_str + obj);
	hash_t e2_id = (hash_t)hasher(sess_str + obj2);

	KGEdge *e1 = new KGEdge (obj_id, sub_id, EdgeType_t::Read, seq, sess, e1_id, timestamp);
	infotbl->InsertEdge(e1_id, e1);
	infotbl->InsertFileInteraction(obj_id, e1);
	infotbl->InsertProcInteraction(sub_id, e1);

	KGEdge *e2 = new KGEdge (sub_id, obj2_id, EdgeType_t::Write, seq, sess, e2_id, timestamp);
	infotbl->InsertEdge(e2_id, e2);
	infotbl->InsertFileInteraction(obj2_id, e2);
	infotbl->InsertProcInteraction(sub_id, e2);
}

void Triplet::SyscallLoad(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (obj_id, sub_id, EdgeType_t::Load, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallCreate(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, hash_t e_id, std::string timestamp) {
	KGEdge *e = new KGEdge (sub_id, obj_id, EdgeType_t::Create, seq, sess, e_id, timestamp);
	infotbl->InsertEdge(e_id, e);
	infotbl->InsertFileInteraction(obj_id, e);
	infotbl->InsertProcInteraction(sub_id, e);
}

void Triplet::SyscallUpdate(seq_t seq, sess_t sess, hash_t sub_id, hash_t obj_id, std::string obj, std::string obj2, std::string timestamp) {
	std::hash<std::string> hasher;
	hash_t obj2_id = (hash_t)hasher(obj2);

	std::string sess_str = std::to_string(sess);

	// sess string with obj to make up a new edge id
	hash_t e1_id = (hash_t)hasher(sess_str + obj);
	hash_t e2_id = (hash_t)hasher(sess_str + obj2);

	// The information flow is: old obj => sub => new obj
	// KGEdge *e1 = new KGEdge (obj_id, sub_id, EdgeType_t::Delete, seq, sess, e1_id);
	KGEdge *e1 = new KGEdge (obj_id, sub_id, EdgeType_t::Delete, seq, sess, e1_id, timestamp);
	infotbl->InsertEdge(e1_id, e1);
	infotbl->InsertFileInteraction(obj_id, e1);
	infotbl->InsertProcInteraction(sub_id, e1);

	KGEdge *e2 = new KGEdge (sub_id, obj2_id, EdgeType_t::Create, seq, sess, e2_id, timestamp);
	infotbl->InsertEdge(e2_id, e2);
	infotbl->InsertFileInteraction(obj2_id, e2);
	infotbl->InsertProcInteraction(sub_id, e2);
}

void Triplet::LoadProc() {
	std::string pid = Jval2strid(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["cid"]);
	std::string exe;
	if (dataset_type == 0) {
		exe = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["properties"]["map"]["name"]);
	}
	else if (dataset_type == 1) {
		exe = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["cmdLine"]["string"]);
	}
	else {
		return;
	}

	std::string args = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["properties"]["map"]["path"]);
	std::string ppid = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["properties"]["map"]["ppid"]);
	std::string uuid = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.Subject"]["uuid"]);

	NodeProc *p_temp = new NodeProc (pid, exe, args, ppid, uuid);
	infotbl->InsertProc(p_temp);
}

void Triplet::LoadFile() {
	std::string name;
	if (dataset_type == 0) {
		name = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.FileObject"]["baseObject"]["properties"]["map"]["path"]);
	}
	else if (dataset_type == 1) {
		name = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.FileObject"]["baseObject"]["properties"]["map"]["filename"]);
	}
	else {
		return;
	}

	std::string uuid = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.FileObject"]["uuid"]);
	std::string version = " ";

	NodeFile *f_temp = new NodeFile (name, version, uuid);
	infotbl->InsertFile(f_temp);
}

void Triplet::LoadSock() {
	std::string ip = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.NetFlowObject"]["remoteAddress"]);
	std::string port = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.NetFlowObject"]["remotePort"]);
	std::string sname = Jval2str(ip + ":" + port);
	std::string uuid = Jval2str(event["datum"]["com.bbn.tc.schema.avro.cdm18.NetFlowObject"]["uuid"]);

	NodeSocket *s_temp = new NodeSocket (sname, uuid);
	infotbl->InsertSocket(s_temp);
}
