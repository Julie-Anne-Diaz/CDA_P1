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
    if (line=="00")return 4;
    else if (line=="01")return 2;
    else if (line=="10")return 3;
    else if (line=="11")return 1;
    else return -1;
}

std::string parseType(std::string line){
    line=line.substr(line.length()-7,5);
    int cat=parseCategory(line);
    switch (cat)
    {
    case 2:
        if (line=="00000")return "add";
        else if (line=="00001")return "sub";
        else if (line=="00010")return "and";
        else if (line=="00011")return "or";
        else return "";
        break;
    case 3:
        if (line=="00000")return "addi";
        else if (line=="00001")return "andi";
        else if (line=="00010")return "ori";
        else if (line=="00011")return "slli";
        else if (line=="00100")return "srai";
        else if (line=="00101")return "lw";
        else return "";
        break;
    case 1:
        if (line=="00000")return "beq";
        else if (line=="00001")return "bne";
        else if (line=="00010")return "blt";
        else if (line=="00011")return "sw";
        else return "";
        break;
    case 4:
        if (line=="00000")return "jal";
        else if (line=="11111")return "break";
        else return "";
        break;
    case -1:
        break;
    }

}

int main(){

    return 0;
}