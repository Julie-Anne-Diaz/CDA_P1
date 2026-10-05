#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

struct Instruction{
    std::string line;
    std::string decodedLine;
    std::string type;
    int category;
    int rd = -1;
    int rs1 = -1;
    int rs2 = -1;
    int imm = 0;
};

std::vector<int> registers(32,0);
std::map<int,int> data;
std::map<int,Instruction> instructions;

int binaryToInt(const std::string& bits){
    unsigned long long value = 0;

    for(char bit : bits){
        value = value * 2 + (bit - '0');
    }

    return static_cast<int>(value);
}
int binaryToSigned(const std::string& bits){
    long long value = 0;

    for(char bit : bits){
        value = value * 2 + (bit - '0');
    }

    if(bits[0] == '1'){
        value -= (1LL << bits.length());
    }

    return static_cast<int>(value);
}

int parseCategory(std::string line){
    line=line.substr(line.length()-2);
    if (line=="00")return 4;
    else if (line=="01")return 2;
    else if (line=="10")return 3;
    else if (line=="11")return 1;
    else return -1;
}

void parseType1(Instruction& i,std::string line){
    i.rs2 = binaryToInt(line.substr(7, 5));
    i.rs1 = binaryToInt(line.substr(12, 5));
    i.imm = binaryToSigned(line.substr(0, 7) + line.substr(20, 5));
    if(i.type == "sw"){i.decodedLine = "sw x" + std::to_string(i.rs2) + ", " + std::to_string(i.imm) + "(x" + std::to_string(i.rs1) + ")";}
    else{i.decodedLine = i.type + " x" + std::to_string(i.rs1) + ", x" + std::to_string(i.rs2) + ", #" + std::to_string(i.imm);}
}
void parseType2(Instruction& i,std::string line){
    i.rs2 = binaryToInt(line.substr(7, 5));
    i.rs1 = binaryToInt(line.substr(12, 5));
    i.rd = binaryToInt(line.substr(20, 5));
    i.decodedLine = i.type + " x" + std::to_string(i.rd) + ", x" + std::to_string(i.rs1) + ", x" + std::to_string(i.rs2);
}
void parseType3(Instruction& i,std::string line){
    i.rs1 = binaryToInt(line.substr(12, 5));
    i.rd = binaryToInt(line.substr(20, 5));
    if(i.type == "slli" || i.type == "srai"){i.imm = binaryToInt(line.substr(7, 5));}
    else{i.imm = binaryToSigned(line.substr(0, 12));}
    if(i.type == "lw"){i.decodedLine ="lw x" + std::to_string(i.rd) + ", " + std::to_string(i.imm) + "(x" + std::to_string(i.rs1) + ")";}
    else{i.decodedLine = i.type + " x" + std::to_string(i.rd) + ", x" + std::to_string(i.rs1) + ", #" + std::to_string(i.imm);}
}
void parseType4(Instruction& i,std::string line){
    if (i.type == "jal"){
        i.rd = binaryToInt(line.substr(20, 5));
        i.imm = binaryToSigned(line.substr(0, 20));
        i.decodedLine = i.type + " x" + std::to_string(i.rd) + ", #" + std::to_string(i.imm);
        return;
    }
    i.decodedLine = i.type;
}

void parseInstruction(Instruction& i,std::string line){
    line=line.substr(line.length()-7,5);
    i.category=parseCategory(i.line);
    switch (i.category)
    {
    case 1:
        if (line=="00000")i.type= "beq";
        else if (line=="00001")i.type= "bne";
        else if (line=="00010")i.type= "blt";
        else if (line=="00011")i.type= "sw";
        parseType1(i,i.line);
        break;
    case 2:
        if (line=="00000")i.type= "add";
        else if (line=="00001")i.type= "sub";
        else if (line=="00010")i.type= "and";
        else if (line=="00011")i.type= "or";
        parseType2(i,i.line);
        break;
    case 3:
        if (line=="00000")i.type= "addi";
        else if (line=="00001")i.type= "andi";
        else if (line=="00010")i.type= "ori";
        else if (line=="00011")i.type= "slli";
        else if (line=="00100")i.type= "srai";
        else if (line=="00101")i.type= "lw";
        parseType3(i,i.line);
        break;
    case 4:
        if (line=="00000")i.type= "jal";
        else if (line=="11111")i.type= "break";
        parseType4(i,i.line);
        break;
    }
}

int main(){
    std::ifstream my_file("sample.txt");
    std::ofstream disassembly_file("disassembly.txt");

    if (!my_file.is_open()) {
        std::cout << "Error opening read file!" << std::endl;
        return -1;
    }
    if (!disassembly_file.is_open()) {
        std::cout << "Error opening write file!" << std::endl;
        return -1;
    }

    std::string line;
    int pc= 256;
    while (std::getline(my_file, line)) {
        Instruction i;
        i.line = line;
        parseInstruction(i,line);
        instructions[pc] = i;
        disassembly_file << line <<'\t'<< pc << ' ' << i.decodedLine << '\n';
        pc+=4;
        if (i.type == "break") {
            break;
        }
    }
    while (std::getline(my_file, line)) {
        data[pc] = binaryToSigned(line);
        disassembly_file << line <<'\t'<< pc << ' ' << data[pc] << '\n';
        pc+=4;
    }

    my_file.close();
    disassembly_file.close();

    return 0;
}