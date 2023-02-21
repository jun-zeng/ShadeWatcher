#include "cmdargument.h"

CmdArgument::CmdArgument(int argc, char **argv) {
    if (argc > 1) {
        tokens.assign(argv + 1, argv + argc);
    }
}

std::string CmdArgument::GetComOption(std::string option) {
    auto it = std::find(tokens.begin(), tokens.end(), option);
    if (it != tokens.end()) {
        it++;
        if (it != tokens.end()) {
            return *it;
        }
    }
    std::string empty("");
    return empty;
}

std::string CmdArgument::GetNextComOption(std::string option) {
    auto it = std::find(tokens.begin(), tokens.end(), option);
    if (it != tokens.end()) {
        it++;
        if (it != tokens.end()) {
            it++;
            if (it != tokens.end()) {
                return *it;
            }
        }
    }
    std::string empty("");
    return empty;
}

std::string CmdArgument::GetSecComOption(std::string option) {
    auto it = std::find(tokens.begin(), tokens.end(), option);
    std::string empty("");
    if (it != tokens.end()) {
        it++;
        if (it != tokens.end()) {
            it++;
            if (it != tokens.end()) {
                return *(it++);
            }
        }
    }
    return empty;
}

bool CmdArgument::ComOptionExist(std::string option) {
    auto it = std::find(tokens.begin(), tokens.end(), option);
    if (it != tokens.end()) {
        return true;
    }
    else {
        return false;
    }
}

void CmdArgument::ComDarHelp() {
    std::cout << "Usage: ./driverdar [options] " << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -trace (-t) <str>" << "\t\t\t" << "Specify log data path as <str>" << std::endl;
    std::cout << "  -dataset (-da) <str>" << "\t\t\t" << "Specify darpa dataset as <str>: engagement + team (e.g., e3/trace)" << std::endl;
    std::cout << "  -graph (-g)" << "\t\t\t\t" << "Visualize knowledge graph" << std::endl;
    std::cout << "  -multithread (-m) <int>" << "\t\t" << "Pass <int> as the number of threads to construct KG (default: single thread)" << std::endl;
    std::cout << "  -threadconfig (-t) <str>" << "\t\t" << "Define configure file for multithread" << std::endl;
    std::cout << "  -loadfromdb (-ldb) <str1> <str2>" << "\t\t" << "Load Darpa graphs <str1> from database in <str2>"<< std::endl;
    std::cout << "  -loadfile (-lf)" << "\t\t\t" << "Load KG from local file"<< std::endl;
    std::cout << "  -storetodb (-sdb) <str>" << "\t\t\t" << "Store KG to database in <str>" << std::endl;
    std::cout << "  -storefile (-sf)" << "\t\t\t" << "Store KG to local file" << std::endl;
    std::cout << "  -storeentity (-se)" << "\t\t\t" << "Store system entities to local file" << std::endl;
    std::cout << "  -loadentity (-le)" << "\t\t\t" << "Load system entities from local file" << std::endl;
    std::cout << "  -delschema (-dsch) <str1> <str2>" << "\t" << "Delete a schema <str1> in a database <str2>" << std::endl;

    std::cout << "\nExample:" << std::endl;
    std::cout << "\t1. store KG to files: ./driverdar -dataset e3_trace -trace ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json -multithread -storefile" << std::endl;
    std::cout << "\t2. Load KG from files: ./driverdar -dataset e3_trace -loadfile" << std::endl;
    std::cout << "\t3. Store KG to database: ./driverdar -dataset e3_trace -trace ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json -multithread -storetodb dbtest.cfg" << std::endl;

    std::cout << "\t3. Load all KGs from database: ./driverdar -dataset e3_trace -loadfromdb all dbtest.cfg" << std::endl;
    std::cout << "\t3. Load partial KGs from database: ./driverdar -dataset e3_trace -loadfromdb 1,2,3 dbtest.cfg" << std::endl;
    std::cout << "\t4. When doing eval for darpa attack, e.g., attack 3, with loading entity and multithread: ./driverdar -dataset e3_trace -trace ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json.4 -multithread -le" <<std::endl;
}

void CmdArgument::ComKafkaHelp() {
    std::cout << "Usage: ./driverbeat [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -trace (-t) <str>" << "\t\t\t" << "Specify log data path as <str>" << std::endl;
    std::cout << "  -kafkaconfig (-k) <str>" << "\t\t" << "Define configure file for multithread" << std::endl;
    std::cout << "  -graph (-g)" << "\t\t\t\t" << "Visualize knowledge graph" << std::endl;
    std::cout << "  -loadfromdb (-ldb) <str>" << "\t\t" << "Load KG from database in <str>"<< std::endl;
    std::cout << "  -storetodb (-sdb) <str>" << "\t\t\t" << "Store KG to database in <str>" << std::endl;
}

void CmdArgument::ComBeatHelp() {
    std::cout << "Usage: ./driverbeat [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -trace (-t) <str>" << "\t\t\t" << "Specify log data path as <str>" << std::endl;
    std::cout << "  -dataset (-da) <str>" << "\t\t\t" << "Specify encoding dir as <str>: Default as log" << std::endl;
    std::cout << "  -graph (-g)" << "\t\t\t\t" << "Visualize knowledge graph" << std::endl;
    std::cout << "  -loadfromdb (-ldb) <str>" << "\t\t" << "Load KG from database in <str>"<< std::endl;
    std::cout << "  -loadfile (-lf)" << "\t\t\t" << "Load KG from local file"<< std::endl;
    std::cout << "  -storetodb (-sdb) <str>" << "\t\t" << "Store KG to database in <str>" << std::endl;
    std::cout << "  -storeentity (-se)" << "\t\t\t" << "Store system entities to local file" << std::endl;
    std::cout << "  -storefile (-sf)" << "\t\t\t" << "Store KG to local file" << std::endl;
    std::cout << "  -delschema (-dsch) <str1> <str2>" << "\t" << "Delete a schema <str1> in a database <str2>" << std::endl;

    std::cout << "\nExample:" << std::endl;
    std::cout << "\tvisualize KG: ./driverbeat -trace ../data/examples/nano_scp_1 -graph" << std::endl;
    std::cout << "\tstore system entities: ./driverbeat -trace ../data/examples/nano_scp_1 -storeentity" << std::endl;
    std::cout << "\tload system entities: ./driverbeat -loadentity -dataset log" << std::endl;
}
