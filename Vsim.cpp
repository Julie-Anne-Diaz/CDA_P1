//On my honor, I have neither given nor received any unauthorized aid on this assignment.

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

struct Instruction{
    std::string line;
    std::string decodedLine;
    std::string type;
    int category;
    int rd = -1;
    int rs1 = -1;
    int rs2 = -1;
    int imm = 0;

    int address = 0;
    int value1 = 0;
    int value2 = 0;
    int result = 0;
};

std::vector<int> registers(32,0);
std::map<int,int> data;
std::map<int,Instruction> instructions;
std::vector<Instruction> preIssue; //4 max
std::vector<Instruction> preALU1; //2 max
std::vector<Instruction> preALU2; //2 max
std::vector<Instruction> preMEM; //1 max
std::vector<Instruction> postALU2; //1 max
std::vector<Instruction> postMEM; //1 max
std::vector<Instruction> executed; //1 max
std::vector<Instruction> waiting; //1 max   
std::vector<Instruction> writeback; //1 max
std::map<int,int> inUse;



std::string printSimulation(const int& cycle, const int& pc, const std::string& instruction){
    std::string output = "--------------------\n";
    output+= "Cycle " + std::to_string(cycle) + ":\t" + std::to_string(pc) + '\t' + instruction + '\n';
    output+="\n\nIF Unit:\n\t";
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
bool isMemory(const Instruction& i){
    return i.type == "lw" || i.type == "sw";
}

bool isBranch(const Instruction& i){
    return i.type == "beq" || i.type == "bne" || i.type == "blt";
}

bool writesRegister(const Instruction& i){
    if (i.rd <= 0){
        return false;
    }
    if (i.type == "sw" || i.type == "beq" || i.type == "bne" || i.type == "blt" || i.type == "break"){
        return false;
    }
    return true;
}

bool readsRegister(const Instruction& i, int reg){
    if (reg == 0){
        return false;
    }
    if (i.type == "add" || i.type == "sub" || i.type == "and" || i.type == "or"){
        return i.rs1 == reg || i.rs2 == reg;
    }
    if (i.type == "addi" || i.type == "andi" || i.type == "ori" || i.type == "slli" || i.type == "srai" || i.type == "lw"){
        return i.rs1 == reg;
    }
    if (i.type == "sw"){
        return i.rs1 == reg || i.rs2 == reg;
    }
    if (isBranch(i)){
        return i.rs1 == reg || i.rs2 == reg;
    }
    return false;
}

void Execute(const Instruction& cur){

    if (writesRegister(cur)){
        registers[cur.rd] = cur.result;
    }

    registers[0] = 0;
}
void rebuildInUse(){
    inUse.clear();
    for (const Instruction& i : preALU1){
        if (writesRegister(i)){
            inUse[i.rd]++;
        }
    }
    for (const Instruction& i : preALU2){
        if (writesRegister(i)){
            inUse[i.rd]++;
        }
    }
    for (const Instruction& i : preMEM){
        if (writesRegister(i)){
            inUse[i.rd]++;
        }
    }
    for (const Instruction& i : postMEM){
        if (writesRegister(i)){
            inUse[i.rd]++;
        }
    }
    for (const Instruction& i : postALU2){
        if (writesRegister(i)){
            inUse[i.rd]++;
        }
    }
}
int executeBranch(const Instruction& i, const std::vector<int>& oldRegisters){
    if (i.type == "beq"){

        if (oldRegisters[i.rs1] == oldRegisters[i.rs2]){
            return i.address + (i.imm << 1);
        }
    }
    else if (i.type == "bne"){

        if (oldRegisters[i.rs1] != oldRegisters[i.rs2]){
            return i.address + (i.imm << 1);
        }
    }
    else if (i.type == "blt"){

        if (oldRegisters[i.rs1] < oldRegisters[i.rs2]){
            return i.address + (i.imm << 1);
        }
    }
    return i.address + 4;
}
bool branchReady(const Instruction& branch, const std::vector<Instruction>& oldPreIssue){
    //check instructions already active in the pipeline
    for (int reg = 1; reg < 32; reg++){
        if (inUse.count(reg) &&
            inUse[reg] > 0 &&
            readsRegister(branch, reg)){
            return false;
        }
    }
    // Check older instructions still in Pre-Issue
    for (const Instruction& older : oldPreIssue){
        if (writesRegister(older) &&
            readsRegister(branch, older.rd)){
            return false;
        }
    }
    return true;
}
//save values at issue to prevent WAR
void captureValues(Instruction& i, const std::vector<int>& oldRegisters){
    if (i.type == "add" || i.type == "sub" || i.type == "and" || i.type == "or"){
        i.value1 = oldRegisters[i.rs1];
        i.value2 = oldRegisters[i.rs2];
    }
    else if (i.type == "addi" || i.type == "andi" || i.type == "ori" || i.type == "slli" || i.type == "srai"){
        i.value1 = oldRegisters[i.rs1];
    }
    else if (i.type == "lw"){
        i.value1 = oldRegisters[i.rs1];
    }
    else if (i.type == "sw"){
        i.value1 = oldRegisters[i.rs2];
        i.value2 = oldRegisters[i.rs1];
    }
}
void executeALU(Instruction& i){
    if (i.type == "lw" || i.type == "sw"){
        i.result = i.value1 + i.imm;
    }
    else if (i.type == "add"){
        i.result = i.value1 + i.value2;
    }
    else if (i.type == "sub"){
        i.result = i.value1 - i.value2;
    }
    else if (i.type == "and"){
        i.result = i.value1 & i.value2;
    }
    else if (i.type == "or"){
        i.result = i.value1 | i.value2;
    }
    else if (i.type == "addi"){
        i.result = i.value1 + i.imm;
    }
    else if (i.type == "andi"){
        i.result = i.value1 & i.imm;
    }
    else if (i.type == "ori"){
        i.result = i.value1 | i.imm;
    }
    else if (i.type == "slli"){
        i.result = i.value1 << i.imm;
    }
    else if (i.type == "srai"){
        i.result = i.value1 >> i.imm;
    }
}
bool RAW(const Instruction& younger, const Instruction& older){
    if (!writesRegister(older)){
        return false;
    }
    return readsRegister(younger, older.rd);
}

bool WAW(const Instruction& younger, const Instruction& older){
    return writesRegister(younger) && writesRegister(older) && younger.rd == older.rd;
}

bool WAR(const Instruction& younger, const Instruction& older){
    if (!writesRegister(younger)){
        return false;
    }
    return readsRegister(older, younger.rd);
}

void Pipeline(){
    std::ofstream simulation_file("simulation.txt");
    if (!simulation_file.is_open()){
        return;
    }

    int pc = 256;
    int cycle = 1;
    bool finished = false;

    while (!finished){
        // Save the state from the end of the previous cycle
        std::vector<int> oldRegisters = registers;

        std::vector<Instruction> oldPreIssue = preIssue;
        std::vector<Instruction> oldPreALU1 = preALU1;
        std::vector<Instruction> oldPreALU2 = preALU2;
        std::vector<Instruction> oldPreMEM = preMEM;
        std::vector<Instruction> oldPostMEM = postMEM;
        std::vector<Instruction> oldPostALU2 = postALU2;

        rebuildInUse();
        executed.clear();

        //wb
        postMEM = oldPostMEM;
        postALU2 = oldPostALU2;

        if (!oldPostMEM.empty()){
            Execute(oldPostMEM[0]);
            postMEM.erase(postMEM.begin());
        }

        if (!oldPostALU2.empty()){
            Execute(oldPostALU2[0]);
            postALU2.erase(postALU2.begin());
        }

        //mem

        preMEM = oldPreMEM;

        if (!oldPreMEM.empty()){
            Instruction i = oldPreMEM[0];
            preMEM.erase(preMEM.begin());
            if (i.type == "lw"){
                if (data.count(i.result)){
                    i.result = data[i.result];
                }
                else{
                    i.result = 0;
                }
                postMEM.push_back(i);
            }
            else if (i.type == "sw"){
                data[i.result] = i.value2;
            }
        }

        //alu1

        preALU1 = oldPreALU1;

        if (!oldPreALU1.empty()){
            Instruction i = oldPreALU1[0];
            preALU1.erase(preALU1.begin());
            executeALU(i);
            preMEM.push_back(i);
        }

        //alu2

        preALU2 = oldPreALU2;
        if (!oldPreALU2.empty()){
            Instruction i = oldPreALU2[0];
            preALU2.erase(preALU2.begin());
            executeALU(i);
            postALU2.push_back(i);
        }

        //issue

        bool issuedMemory = false;
        bool issuedALU = false;
        std::set<int> issuedIndexes;

        for (int i = 0; i < (int)oldPreIssue.size(); i++){
            Instruction candidate = oldPreIssue[i];
            bool memory = isMemory(candidate);
            bool hazard = false;

            // Check destination queue space and use old q sizes
            if (memory){
                if (issuedMemory){
                    continue;
                }

                if (oldPreALU1.size() >= 2){
                    continue;
                }
            }

            else{
                if (issuedALU){
                    continue;
                }

                if (oldPreALU2.size() >= 2){
                    continue;
                }
            }

            //RAW

            for (int reg = 1; reg < 32; reg++){

                if (inUse.count(reg) &&
                    inUse[reg] > 0 &&
                    readsRegister(candidate, reg)){
                    hazard = true;
                    break;
                }
            }

            if (hazard){
                continue;
            }

            //WAW
            if (writesRegister(candidate) &&
                inUse.count(candidate.rd) &&
                inUse[candidate.rd] > 0){
                continue;
            }


            //check earlier isntructions in pre issue
            for (int j = 0; j < i; j++){
                const Instruction& older = oldPreIssue[j];
                // RAW 
                if (RAW(candidate, older)){

                    hazard = true;
                    break;
                }
                // WAW and WAR if instruc didnt issue this cycle
                if (issuedIndexes.count(j) == 0){
                    if (WAW(candidate, older) ||
                        WAR(candidate, older)){
                        hazard = true;
                        break;
                    }
                }
            }

            if (hazard){
                continue;
            }

            //mem ordering

            if (candidate.type == "lw"){
                // A lw cant pass sw
                for (int j = 0; j < i; j++){
                    if (oldPreIssue[j].type == "sw"){
                        hazard = true;
                        break;
                    }
                }
            }

            else if (candidate.type == "sw"){
                // Sw ordering
                for (int j = 0; j < i; j++){

                    if (oldPreIssue[j].type == "sw"){
                        hazard = true;
                        break;
                    }
                }
            }

            if (hazard){
                continue;
            }

            //instruc issues and capture vals
            captureValues(candidate, oldRegisters);


            if (memory){
                preALU1.push_back(candidate);
                issuedMemory = true;
            }

            else{
                preALU2.push_back(candidate);
                issuedALU = true;
            }

            issuedIndexes.insert(i);


            //only one mem and alu allowed /cycle
            if (issuedMemory && issuedALU){
                break;
            }
        }

        //remove issued instrucs
        preIssue.clear();

        for (int i = 0; i < (int)oldPreIssue.size(); i++){
            if (issuedIndexes.count(i) == 0){
                preIssue.push_back(oldPreIssue[i]);
            }
        }

        //branch from previous cycle
        bool hadWaitingBranch = !waiting.empty();

        if (hadWaitingBranch){
            Instruction branch = waiting[0];
            if (branchReady(branch, oldPreIssue)){
                waiting.clear();
                executed.push_back(branch);
                pc = executeBranch(branch, oldRegisters);
            }
        }

        //fetch

        if (!hadWaitingBranch){
            int freeSlots = 4 - (int)oldPreIssue.size();

            if (freeSlots > 2){
                freeSlots = 2;
            }

            for (int fetch = 0; fetch < freeSlots && !finished; fetch++){
                if (instructions.find(pc) == instructions.end()){
                    finished = true;
                    break;
                }

                Instruction next = instructions[pc];

                //break
                if (next.type == "break"){
                    executed.push_back(next);
                    finished = true;
                    break;
                }

                //jal
                if (next.type == "jal"){
                    executed.push_back(next);

                    if (next.rd != 0){
                        registers[next.rd] = next.address + 4;
                    }

                    registers[0] = 0;
                    pc = next.address + (next.imm << 1);
                    break;
                }


                //branch
                if (isBranch(next)){
                    bool ready = branchReady(next, oldPreIssue);

                    for (const Instruction& older : preIssue){
                        if (writesRegister(older) &&
                            readsRegister(next, older.rd)){
                            ready = false;
                            break;
                        }
                    }

                    if (!ready){
                        waiting.clear();
                        waiting.push_back(next);
                        //keep pc pointing at branch
                        pc = next.address;
                        break;
                    }

                    //branch executes immediatelky

                    executed.push_back(next);
                    pc = executeBranch(next, oldRegisters);
                    break;
                }


                //normal instruction
                preIssue.push_back(next);
                pc += 4;
            }
        }
        registers[0] = 0;
        cycle++;
    }

    simulation_file.close();
}


int main(int argc, char* argv[]){
    if (argc < 2) {
        return -1;
    }

    std::ifstream my_file(argv[1]);
    std::ofstream disassembly_file("disassembly.txt");

    if (!my_file.is_open()) {
        return -1;
    }
    if (!disassembly_file.is_open()) {
        return -1;
    }

    std::string line;
    int pc= 256;
    while (std::getline(my_file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Instruction i;
        i.line = line;
        i.address = pc;
        parseInstruction(i,line);
        instructions[pc] = i;
        disassembly_file << line <<'\t'<< pc << '\t' << i.decodedLine << '\n';
        pc+=4;
        if (i.type == "break") {
            break;
        }
    }
    while (std::getline(my_file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        data[pc] = binaryToSigned(line);
        disassembly_file << line <<'\t'<< pc << '\t' << data[pc] << '\n';
        pc+=4;
    }

    my_file.close();
    disassembly_file.close();


    Pipeline();
    return 0;
}