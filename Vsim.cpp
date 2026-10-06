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

std::string printSimulation(const int& cycle, const int& pc, const std::string& instruction){
    std::string output = "--------------------\n";
    output+= "Cycle " + std::to_string(cycle) + ":\t" + std::to_string(pc) + '\t' + instruction + '\n';
    output+= "Registers\n";
    output+= "x00:";
    for (int i = 0; i < 32; i++) {
        output += "\t" + std::to_string(registers[i]);

        if ((i + 1) % 8 == 0) {
            output += "\n";

            if (i != 31) {
                if (i < 9) {
                    output += "x0" + std::to_string(i + 1) + ":";
                } else {
                    output += "x" + std::to_string(i + 1) + ":";
                }
            }
        }
    }
    
    output += "Data\n";
    int count = 0;

    for (auto it = data.begin(); it != data.end(); ++it) {
        if (count % 8 == 0) {
            if (count != 0) {
                output += "\n";
            }

            output += std::to_string(it->first) + ":\t";
        }

        output += std::to_string(it->second) + "\t";
        count++;
    }

    output += "\n";

    return output;
}

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
    if(i.type == "sw"){i.decodedLine = "sw x" + std::to_string(i.rs1) + ", " + std::to_string(i.imm) + "(x" + std::to_string(i.rs2) + ")";}
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


void Execute(const Instruction& i, int& pc){


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

    std::ofstream simulation_file("simulation.txt");
    if (!simulation_file.is_open()) {
        std::cout << "Error opening write file!" << std::endl;
        return -1;
    }

    pc=256;
    Instruction cur = instructions[pc];
    int cycle=1;

    while (cur.type!="break"){
        switch (cur.category)
        {
        case 1:
            if (cur.type == "beq" && registers[cur.rs1] == registers[cur.rs2]){
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc+=cur.imm<<1;
            }
            else if (cur.type == "bne" && registers[cur.rs1] != registers[cur.rs2]){
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc+=cur.imm<<1;
            }
            else if (cur.type == "blt" && registers[cur.rs1] < registers[cur.rs2]){
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc+=cur.imm<<1;
            }
            else if (cur.type == "sw"){
                data[registers[cur.rs2]+cur.imm] = registers[cur.rs1];
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc+=4;
            }
            else{
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc+=4;
            }
            break;
        case 2:
            if (cur.type == "add"){registers[cur.rd] = registers[cur.rs1] + registers[cur.rs2];}
            else if (cur.type == "sub"){registers[cur.rd] = registers[cur.rs1] - registers[cur.rs2];}
            else if (cur.type == "and"){registers[cur.rd] = registers[cur.rs1] & registers[cur.rs2];}
            else if (cur.type == "or"){registers[cur.rd] = registers[cur.rs1] | registers[cur.rs2];}
            simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
            pc+=4;
            break;
        case 3:
            if (cur.type == "addi"){registers[cur.rd] = registers[cur.rs1] + cur.imm;}
            else if (cur.type == "andi"){registers[cur.rd] = registers[cur.rs1] & cur.imm;}
            else if (cur.type == "ori"){registers[cur.rd] = registers[cur.rs1] | cur.imm;}
            else if (cur.type == "slli"){registers[cur.rd] = registers[cur.rs1] << cur.imm;}
            else if (cur.type == "srai"){registers[cur.rd] = registers[cur.rs1] >> cur.imm;}
            else if (cur.type == "lw"){registers[cur.rd]=data[registers[cur.rs1]+cur.imm];}
            simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
            pc+=4;
            break;
        case 4:
            if (cur.type == "jal"){
                registers[cur.rd]=pc+4;
                simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
                pc += (cur.imm << 1);
            }
            break;
        }
        cycle++;
        cur = instructions[pc];
    }
    simulation_file<<printSimulation(cycle, pc, cur.decodedLine);
    simulation_file.close();
    return 0;
}