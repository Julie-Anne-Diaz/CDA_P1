#include <string>


struct Instruction{
    std::string line;
    int address;
    std::string type;
    int category;
    int rd = -1;
    int rs1 = -1;
    int rs2 = -1;
    int imm = 0; 
};

int parseCategory(std::string line){
    line=line.substr(line.length()-2);
    if (line=="00")return 0;
    else if (line=="01")return 1;
    else if (line=="10")return 2;
    else if (line=="11")return 3;
    else return -1;
}

std::string parseType(std::string line){
    line=line.substr(line.length()-7,5);
    if (line=="00000")return "ADD";
    else if (line=="00001")return "SUB";
    else if (line=="00010")return "AND";
    else if (line=="00011")return "OR";
    else return "";
}

int main(){

    return 0;
}