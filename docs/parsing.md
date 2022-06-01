# Parsing Audit Records
Provenance graph is a common representation of audit records. The nodes in the
graph represent system entities (e.g., processes, files, sockets) and edges
represent system calls in the direction of information flow.

We currently support parsing audit records in two formats: Auditbeat's json format
and Common Data Model format (i.e., DARPA Transparent Computing dataset format):

Besides, we support parsing audit records in parallel with multiple threads.

## How to Use
Make Sure you have successfully complied driverbeat (for auditbeat) and
   driverdar (for DARPA) under `Shadewatcher/parse`. We run the following
   examples on a Ubuntu desktop with Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz.

1. Parse audit records from auditbeat
```bash
# Please first extract our audit data (e.g., nano_scp_1.tar.gz) in 'Shadewatcher/data/examples'
./driverbeat -trace ../data/examples/nano_scp_1
```
Expected Result:
```
Processing Dir: ../data/examples/nano_scp_1/
beat file: ../data/examples/nano_scp_1/auditbeat
recover process info
Reduce noisy events
	collecting temporary file
	collecting ShadowFileEdge
	collecting ShadowProcEdge
	collecting MissingEdge
	collecting Library
	deleting nosiy events
Reduce Noise Events runtime overhead: 0.0101685

KG construction runtime overhead: 1.5823

KG Statistics
#Events: 68783
#Edge: 5945
#Noisy events: 11507
#Proc: 605
#File: 953
#Socket: 3
#Node: 1561
#Node(1561) = #Proc(605) + #File(953) + #Socket(3)
```

2. Parse audit records from DARPA TC dataset (single thread)
```bash
./driverdar -dataset e3_trace -trace DARPA_AUDIT_DIR
```
Expected Result:
```
darpa file: ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json
Reduce noisy events
	collecting temporary file
	collecting ShadowFileEdge
	collecting ShadowProcEdge
	collecting MissingEdge
	collecting Library
	deleting nosiy events
Reduce Noise Events runtime overhead: 10.9453

KG construction runtime overhead: 102.565

KG Statistics
#Events: 2659722
#Edge: 128486
#Noisy events: 2240649
#Proc: 398659
#File: 55737
#Socket: 25124
#Node: 479520
#Node(479520) = #Proc(398659) + #File(55737) + #Socket(25124)
```
3. Parse audit records from DARPA TC dataset (multiple threads)
```bash
./driverdar -dataset e3_trace -trace ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json -multithread 8
```
Expected Result:
```
darpa file: ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json
Multi-thread Configure file: ../config/multithread.cfg
Reduce noisy events
	collecting temporary file
	collecting ShadowFileEdge
	collecting ShadowProcEdge
	collecting MissingEdge
	collecting Library
	deleting nosiy events
Reduce Noise Events runtime overhead: 11.2711

KG construction runtime overhead: 25.4445

KG Statistics
#Events: 2659722
#Edge: 128486
#Noisy events: 2240649
#Proc: 398659
#File: 55737
#Socket: 25124
#Node: 479520
#Node(479520) = #Proc(398659) + #File(55737) + #Socket(25124)
```
