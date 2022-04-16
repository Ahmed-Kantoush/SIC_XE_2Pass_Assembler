#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <queue>
#include <stack>
#include <map>

using namespace std;

bool End = false;
int loc_counter, index_reg, base, st = 0;

queue<long long> inst;
queue<int> formatt;

unordered_map<string, int> sym_table;

unordered_map<string, int> inst_set;
unordered_map<char, int> registers;
unordered_set<string> res_words;

void define();
bool pass1(string);
bool pass2(string, ofstream&);
void print_t(ofstream&);

int main()
{
    cout << "Enter index register value: ";
    cin >> index_reg;

    cout << "Enter base register value: ";
    cin >> base;

    define();

    fstream file("code.txt");
    ofstream out("out.txt", ofstream::out | ofstream::trunc);

    if(file.is_open() && out.is_open())
    {
        int start_loc;
        string line;
        getline(file, line);
        string name, start;
        stringstream str(line);
        str >> name >> start >> hex >> loc_counter;
        start_loc = loc_counter;

        if (start != "Start" || name.length() > 6)
        {
            cout << "Error in first line!" << endl;
            return 0;
        }

        while(getline(file, line))
        {
            if (End)
                break;
            if (!pass1(line))
                return 0;
        }

        if (!End)
        {
            cout << "Error: Did not find end!" << endl;
            return 0;
        }

        file.clear();
        file.seekg(0, ios::beg);
        getline(file, line);

        out << "H." + name;
        for(int i = name.length(); i < 6; i++)
            out << "_";
        out << '.' << setfill('0') << setw(6) << uppercase << hex << start_loc;
        out << '.' << setfill('0') << setw(6) << uppercase << hex << loc_counter - start_loc << endl;

        End = false;
        loc_counter = start_loc;
        while(getline(file, line))
        {
            if (End)
                break;
            if (!pass2(line, out))
                return 0;
        }

        if (!inst.empty())
            print_t(out);

        out << "E." << setfill('0') << setw(6) << uppercase << hex << start_loc;

        file.close();
        out.close();

        while (!formatt.empty()){
            cout << formatt.front() << endl;formatt.pop();}


        cout << "Assembled successfully" << endl;
        for(auto it:sym_table)
            cout << hex << it.first << ": " << it.second << endl;
    }
    else
        cout << "Could not find 'code.txt'" << endl;
    return 0;
}

void define()
{
    res_words.insert("RESW");
    res_words.insert("RESB");
    res_words.insert("RESDW");
    res_words.insert("WORD");
    res_words.insert("BYTE");
    res_words.insert("LTORG");
    res_words.insert("EQU");
    res_words.insert("BASE");
    res_words.insert("NOBASE");
    res_words.insert("End");

    inst_set["ADD"] = 0x18;
    inst_set["AND"] = 0x40;
    inst_set["COMP"] = 0x28;
    inst_set["DIV"] = 0x24;
    inst_set["J"] = 0x3C;
    inst_set["JEQ"] = 0x30;
    inst_set["JGT"] = 0x34;
    inst_set["JLT"] = 0x38;
    inst_set["JSUB"] = 0x48;
    inst_set["LDA"] = 0x00;
    inst_set["LDCH"] = 0x50;
    inst_set["LDL"] = 0x08;
    inst_set["LDX"] = 0x04;
    inst_set["MUL"] = 0x20;
    inst_set["OR"] = 0x44;
    inst_set["RD"] = 0xD8;
    inst_set["RSUB"] = 0x4C;
    inst_set["STA"] = 0x0C;
    inst_set["STCH"] = 0x54;
    inst_set["STL"] = 0x14;
    inst_set["STSW"] = 0xE8;
    inst_set["STX"] = 0x10;
    inst_set["SUB"] = 0x1C;
    inst_set["TD"] = 0xE0;
    inst_set["TIX"] = 0x2C;
    inst_set["WD"] = 0xDC;
    inst_set["ADDF"] = 0x58;
    inst_set["ADDR"] = 0x90;
    inst_set["CLEAR"] = 0xB4;
    inst_set["COMPF"] = 0x88;
    inst_set["COMPR"] = 0xA0;
    inst_set["DIVF"] = 0x64;
    inst_set["DIVR"] = 0x9C;
    inst_set["FIX"] = 0xC4;
    inst_set["FLOAT"] = 0xC0;
    inst_set["HIO"] = 0xF4;
    inst_set["LDB"] = 0x68;
    inst_set["LDF"] = 0x70;
    inst_set["LDS"] = 0x6C;
    inst_set["LDT"] = 0x74;
    inst_set["LPS"] = 0xD0;
    inst_set["MULF"] = 0x60;
    inst_set["MULR"] = 0x98;
    inst_set["NORM"] = 0xC8;
    inst_set["RMO"] = 0xAC;
    inst_set["SHIFTL"] = 0xA4;
    inst_set["SHIFTR"] = 0xA8;
    inst_set["SIO"] = 0xF0;
    inst_set["SSK"] = 0xEC;
    inst_set["STB"] = 0x78;
    inst_set["STF"] = 0x80;
    inst_set["STI"] = 0xD4;
    inst_set["STS"] = 0x7C;
    inst_set["STD"] = 0x84;
    inst_set["SUBF"] = 0x5C;
    inst_set["SUBR"] = 0x94;
    inst_set["SVC"] = 0xB0;
    inst_set["TIO"] = 0xF8;
    inst_set["TIXR"] = 0xB8;

    registers['A'] = 0;
    registers['X'] = 1;
    registers['L'] = 2;
    registers['B'] = 3;
    registers['S'] = 4;
    registers['T'] = 5;
    registers['F'] = 6;
}

bool pass1(string line)
{
    // check for label
    string instr, operand;
    stringstream str(line);
    string label;
    while(getline(str, label, ' '))
    {
        if(label[0] == '.')
            return 1;
        break;
    }

    str >> instr >> operand;

    //Handle location counter
    int format = 3;

    if (instr[0] == '$')
    {
        format = 5;
        instr = instr.substr(1, instr.length()-1);
    }
    else if (instr[0] == '+')
    {
        format = 4;
        instr = instr.substr(1, instr.length()-1);
    }
    else if (operand == "")
        format = 1;
    else if ((registers.find(operand[0]) != registers.end()) && ((operand.size() == 1) || (operand[1] == ',')))
        format = 2;


    //Check for invalid instructions
    if (inst_set.find(instr) == inst_set.end() && res_words.find(instr) == res_words.end())
    {
        cout << "Error: Could not resolve " + instr + "!" << endl;
        return 0;
    }

    //Handle symbols
    if(!label.empty())
    {
        if (sym_table.find(label) == sym_table.end())
            sym_table[label] = loc_counter;
        else
        {
            cout << "Error: " + label + " is defined more than once!" << endl;
            return 0;
        }
    }

    if (instr == "End")
    {
        End = true;
    }
    else if (instr == "RESW")
    {
        int x = stoi(operand);
        loc_counter += 3*x;
    }
    else if (instr == "RESB")
    {
        int x = stoi(operand);
        loc_counter += x;
    }
    else if (instr == "BYTE")
    {
        loc_counter++;
    }
    else if (instr == "WORD")
    {
        loc_counter += 3;
    }
    else if (instr == "RESDW")
    {
        loc_counter += 6;
    }
    else
    {
        loc_counter += format;
    }
    return 1;
}


bool pass2(string line, ofstream& out)
{
    bool x = false, imm = false, ind = false;
    int loc = 0, op = 0;

    string func, operand;
    stringstream str(line);
    string label;
    while(getline(str, label, ' '))
    {
        if(label[0] == '.')
            return 1;
        break;
    }

    str >> func >> operand;

    if (res_words.find(func) != res_words.end())
        return 1;

    int format = 3;

    if (func[0] == '$')
    {
        op = inst_set[func.substr(1, func.length()-1)];
    }
    else if (func[0] == '+')
    {
        format = 4;
        op = inst_set[func.substr(1, func.length()-1)];
    }
    else if (operand == "")
    {
        inst.push(inst_set[func]);
        formatt.push(1);
        loc_counter += 1;
        return 1;
    }
    else if ((registers.find(operand[0]) != registers.end()) && ((operand.size() == 1) || (operand[1] == ',')))
    {
        long long instr = 0;
        instr |= inst_set[func];
        instr <<= 4;
        instr |= registers[operand[0]];
        instr <<= 4;
        char regb;
        str >> regb;
        instr |= registers[regb];
        inst.push(instr);
        formatt.push(2);
        loc_counter += 2;
        return 1;
    }

    if (!str.eof())
    {
        x = true;
        if(*operand.rbegin() == ',')
            operand.erase(operand.end()-1);
    }

    if (operand[0] == '#')
    {
        operand = operand.substr(1,operand.length()-1);
        loc = stoi(operand);
        imm = true;
    }
    else if (operand[0] == '@')
    {
        operand = operand.substr(1,operand.length()-1);
        loc = sym_table[operand];
        ind = true;
    }
    else
        loc = sym_table[operand];

    if (format == 4)
    {
        long long instr = op;
        if (imm)
            instr |= 1;
        else if (ind)
            instr |= 2;
        else
            instr |= 3;
        instr <<= 4;
        instr |= 1;
        instr <<= 20;
        instr |= loc;
        inst.push(instr);
        formatt.push(4);
        loc_counter += 4;
        return 1;
    }

    if (format == 5)
    {
        long long instr = inst_set[func];
        instr <<= 1;
        instr |= x;
        instr <<= 2;
        int disp;
        if (abs(loc - loc_counter) <= 2047)
        {
            instr |= 1;
            instr <<= 13;
            disp = loc - loc_counter;
            instr |= (disp & 0xFFF);
        }
        else
        {
            instr |= 2;
            instr <<= 13;
            disp = loc - base;
            instr |= (disp & 0xFFF);
        }
        if (disp % 2 == 0)
            instr |= 1 << 17;
        if (disp > 0)
            instr |= 1 << 16;
        inst.push(instr);
        formatt.push(3);
        loc_counter += 3;
    }


    int prev_loc = loc_counter;

    if (func == "End")
    {
        End = true;
        return 1;
    }
    else if (func == "RESW")
    {
        print_t(out);
        int x = stoi(operand);
        loc_counter += 3*x;
        st = loc_counter;
        return 1;
    }
    else if (func == "RESB")
    {
        print_t(out);
        int x = stoi(operand);
        loc_counter += x;
        st = loc_counter;
        return 1;
    }
    else if (func == "RESDW")
    {
        print_t(out);
        int x = stoi(operand);
        loc_counter += 6*x;
        st = loc_counter;
        return 1;
    }
    else if (func == "BYTE")
    {
        long long instr = 0;

        if (operand[0] == 'C' || operand[0] == 'c')
        {
            operand.erase(0, 2);
            char x = operand[0];
            int i = 0;
            while (x != '\'')
            {
                instr |= x;
                i++;
                x = operand[i];
                instr <<= 8;
            }
            instr >>= 8;
            inst.push(instr);
            formatt.push(i);
            loc_counter += i;
        }
        else if (operand[0] == 'X' || operand[0] == 'x')
        {
            operand.erase(0, 2);
            operand.erase(operand.end()-1);
            std::stringstream ss;
            ss << hex << operand;
            ss >> instr;
            loc_counter++;
            inst.push(instr);
            formatt.push(1);
        }
        else
        {
            instr = stoi(operand);
            loc_counter++;
            inst.push(instr);
            formatt.push(1);
        }
        return 1;
    }
    else if (func == "WORD")
    {
        long long instr = 0;
        if (operand[0] == 'X' || operand[0] == 'x')
        {
            operand.erase(0, 2);
            operand.erase(operand.end()-1);
            std::stringstream ss;
            ss << hex << operand;
            ss >> instr;
        }
        else
            instr = stoi(operand);

        inst.push(instr);
        formatt.push(3);
        return 1;
    }

    loc_counter += 3;

    long long instr = inst_set[func];
    if (imm)
            instr |= 1;
        else if (ind)
            instr |= 2;
        else
            instr |= 3;
    instr <<= 1;
    instr |= x;
    instr <<= 2;
    if (abs(loc - loc_counter) <= 2047)
    {
        instr |= 1;
        instr <<= 13;
        int disp = loc - loc_counter;
        instr |= (disp & 0xFFF);
    }
    else
    {
        instr |= 2;
        instr <<= 13;
        int disp = loc - base;
        instr |= (disp & 0xFFF);
    }
    inst.push(instr);
    formatt.push(3);
    return 1;
}

void print_t(ofstream& out)
{
    queue<int> help1;
    queue<long long> help2;

    if (st == loc_counter)
        return;
    out << "T." << setfill('0') << setw(6) << uppercase << hex << st;
    int bytes = 0;
    while(bytes+formatt.front() <= 30 && !formatt.empty())
    {
        help1.push(formatt.front());
        help2.push(inst.front());
        bytes += formatt.front();
        inst.pop();
        formatt.pop();
    }

    out << "." << setfill('0') << setw(2) << uppercase << hex << bytes;
    while(!help1.empty())
    {
        out << ".";
        out << setfill('0') << setw(help1.front()*2) << uppercase << hex << help2.front();
        help1.pop();
        help2.pop();
    }

    st += bytes;
    out << endl;
    if (!formatt.empty())
        print_t(out);
}
