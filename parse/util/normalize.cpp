#include "normalize.h"

std::string Jval2str(const Json::Value jval) {
	Json::FastWriter fastWriter;
	std::string tmp = fastWriter.write(jval);
	if (tmp[0] == '\"') {
		return tmp.substr(1, tmp.rfind("\"") - 1);
	}
	else {
		return tmp.substr(0, tmp.rfind("\n"));
	}
}

std::string Jval2strid(const Json::Value jval) {
	Json::FastWriter fastWriter;
	std::string tmp = fastWriter.write(jval);
	return tmp.substr(0, tmp.rfind("\n"));
}

std::string Jval2strArgs(const Json::Value jval) {
	Json::FastWriter fastWriter;
	std::string tmp = fastWriter.write(jval);
	return tmp.substr(1, tmp.rfind("]") - 1);
}

biguint_t Jval2int(const Json::Value jval) { 
	Json::FastWriter fastWriter;
	std::string tmp = fastWriter.write(jval);
	// std::cout << "String: " << tmp << std::endl;
	return (biguint_t)stoint128_t(tmp);
}

biguint_t stoint128_t(std::string const & in) {
    biguint_t res = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        const char c = in[i];
        if (not std::isdigit(c)) {
			// std::cerr << "Non-numeric character: " << in << " "<< __FILE__ << " "<<  __LINE__ << std::endl << std::endl;
            // throw std::runtime_error(std::string("Non-numeric character: ") +
            // c);
			continue;
		}
        res *= 10;
        res += c - '0';
    }
    return res;
}
