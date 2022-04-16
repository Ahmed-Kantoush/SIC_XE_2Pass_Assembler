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

vector<string> loc_vec;
vector<string> instr_vec;

queue<pair<string, int>> literal;
unordered_map<string, pair<int, int>> lit_table;

bool End = false;
int loc_counter;

unordered_map<string, pair<char, int>> sym_table;

unordered_map<string, int> inst_set;
unordered_map<char, int> registers;
unordered_set<string> res_words;

void define();
bool pass1(string);

void LTORG();

void print_out(ofstream&);
void print_sym(ofstream&);
void print_lit(ofstream&);

bool EQU(string, int&, char&);

bool is_number(const std::string&);

int main()
{
    define();

    fstream file("code.txt");
    ofstream out("out.txt", ofstream::out | ofstream::trunc);
    ofstream sym("sym_table.txt", ofstream::out | ofstream::trunc);
    ofstream lit("lit_table.txt", ofstream::out | ofstream::trunc);

    if(file.is_open() && out.is_open() && sym.is_open() && lit.is_open())
    {
        string line;
        getline(file, line);
        string name, start;
        stringstream str(line);
        str >> name >> start >> hex >> loc_counter;

        if (start != "Start" || name.length() > 6)
        {
            cout << "Could not find start!" << endl;
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

        LTORG();

        print_out(out);
        print_sym(sym);
        print_lit(lit);

        file.close();
        out.close();
        sym.close();
        lit.close();

        cout << "Assembled successfully" << endl;
    }

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

    instr_vec.push_back(line);

    //Literals
    if(operand[0] == '=')
    {
        string op = operand;
        int value = 0;
        if (operand[1] == 'C' || operand[1] == 'c')
        {
            operand.erase(0, 3);
            string s;
            char x = operand[0];
            int i = 0;
            while (x != '\'')
            {
                string temp;
                int y = x;
                stringstream ss;
                ss << hex << y;
                ss >> temp;
                s = s+temp;
                i++;
                x = operand[i];
            }
            stringstream ss;
            ss << hex << s;
            ss >> value;
        }
        else if (operand[1] == 'X' || operand[1] == 'x')
        {
            operand.erase(0, 3);
            operand.erase(operand.end()-1);
            stringstream ss;
            ss << hex << operand;
            ss >> value;
        }
        else
        {
            operand.erase(0, 1);
            value = stoi(operand);
        }
        literal.push({op, value});
    }


    //Handle literal table
    if (instr == "LTORG")
    {
        loc_vec.push_back("----");
        LTORG();
        return 1;
    }

    if (instr == "BASE" || instr == "NOBASE")
    {
        loc_vec.push_back("----");
        return 1;
    }

    //Check for invalid instructions
    if (inst_set.find(instr) == inst_set.end() && res_words.find(instr) == res_words.end())
    {
        cout << "Error: Could not resolve " + instr + "!" << endl;
        return 0;
    }

    //Handle symbols
    if(!label.empty())
    {
        if (instr != "EQU")
        {
            if (sym_table.find(label) == sym_table.end())
                sym_table[label] = {'R', loc_counter};
            else
            {
                cout << "Error: " + label + " is defined more than once!" << endl;
                return 0;
            }
        }
        else
        {
            int value;
            char type;
            if(EQU(operand, value, type))
            {
                loc_vec.push_back("----");
                sym_table[label] = {type, value};
                return 1;
            }
            else
                return 0;
        }
    }


    //Handle location counter
    int format = 3;

    if (instr[0] == '$')
        format = 5;
    else if (instr[0] == '+')
        format = 4;
    else if (operand == "")
        format = 1;
    else if ((registers.find(operand[0]) != registers.end()) && ((operand.size() == 1) || (operand[1] == ',')))
        format = 2;

    if (instr == "End")
    {
        End = true;
        loc_vec.push_back("----");
    }
    else if (instr == "RESW")
    {
        int x = stoi(operand);
        loc_vec.push_back(to_string(loc_counter));
        loc_counter += 3*x;
    }
    else if (instr == "RESB")
    {
        loc_vec.push_back(to_string(loc_counter));
        int x = stoi(operand);
        loc_counter += x;
    }
    else if (instr == "BYTE")
    {
        loc_vec.push_back(to_string(loc_counter));
        loc_counter++;
    }
    else if (instr == "WORD")
    {
        loc_vec.push_back(to_string(loc_counter));
        loc_counter += 3;
    }
    else if (instr == "RESDW")
    {
        loc_vec.push_back(to_string(loc_counter));
        loc_counter += 6;
    }
    else
    {
        loc_vec.push_back(to_string(loc_counter));
        loc_counter += format;
    }
    return 1;
}

void LTORG()
{
    map<int, string> temp;
    while(!literal.empty())
    {
        if(lit_table.find(literal.front().first) == lit_table.end())
        {
            string s = "         " + literal.front().first;
            temp[loc_counter] = s;
            lit_table[literal.front().first] = {literal.front().second, loc_counter};
            literal.pop();
            loc_counter += 3;
        }
    }
    for(auto i : temp)
    {
        loc_vec.push_back(to_string(i.first));
        instr_vec.push_back(i.second);
    }
}

void print_out(ofstream& out)
{
    for (int i=0; i < instr_vec.size(); i++)
    {
        if (loc_vec[i] != "----")
            out << hex << uppercase << setw(4) << setfill('0') << right << stoi(loc_vec[i]) << " ";
        else
            out << "---- ";
        out << instr_vec[i] << endl;
    }
}

void print_sym(ofstream& sym)
{
    sym << setw(8) << left << "Symbol" << "Type   Value" << endl;
    map<int,pair<string,char>> sym_table_o;
    for(auto it : sym_table){
        sym_table_o[it.second.second]=make_pair(it.first, it.second.first);
    }
    for (auto it : sym_table_o)
    {
        sym << setw(10) << setfill(' ') << left << it.second.first << setw(5) << it.second.second;
        sym << hex << uppercase << setw(4) << setfill('0') << right << it.first << endl;
    }
}

void print_lit(ofstream& lit)
{
    lit << setw(8) << left << "Name" << "Value     Address" << endl;
    map<int,pair<string,int>> lit_table_o;
    for(auto it : lit_table){
        lit_table_o[it.second.second]=make_pair(it.first, it.second.first);
    }
    for (auto it : lit_table_o)
    {
        lit << setw(8) << setfill(' ') << left << it.second.first;
        lit << setw(6) << hex << uppercase << setfill('0') << right << it.second.second << "    ";
        lit << setw(4) << hex << uppercase << setfill('0') << right << it.first << endl;
    }
}

bool EQU(string line, int& value, char& type)
{
    string s1;
    string s2;
    char op = '0';
    bool f = false;


    for (char x : line)
    {
        if (x == '+' || x == '-' || x == '*' || x == '/')
        {
            op = x;
            f = true;
        }
        else if(f)
            s2 += x;
        else
            s1 += x;
    }
    int x, y;

    if (op == '0' || line[0] == '*')
    {
        if (line[0] == '*')
        {
            x = loc_counter;
            type = 'R';
            value = x;
            return true;
        }
        else if (sym_table.find(s1) == sym_table.end())
        {
            cout << "Error: Could not find symbol at EQU!" << endl;
            return false;
        }
        else
        {
            value = sym_table[s1].second;
            type = sym_table[s1].first;
            return 1;
        }
    }
    else
    {
        if ((sym_table.find(s1) == sym_table.end()) && (!is_number(s1)))
        {
            cout << "Error: Could not find symbol " + s1 << endl;
            return false;
        }
        else if ((sym_table.find(s2) == sym_table.end()) && (!is_number(s2)))
        {
            cout << "Error: Could not find symbol " + s2 << endl;
            return false;
        }

        if (is_number(s1) && is_number(s2))
        {
            x = stoi(s1);
            y = stoi(s2);
            type = 'A';
        }
        else if (is_number(s1))
        {
            x = stoi(s1);
            y = sym_table[s2].second;
            type = sym_table[s2].first;
        }
        else if (is_number(s2))
        {
            y = stoi(s2);
            x = sym_table[s1].second;
            type = sym_table[s1].first;
        }
        else
        {
            if (sym_table[s1].first == 'A' && sym_table[s2].first == 'A')
                type = 'A';
            else if ((sym_table[s1].first == 'R' && sym_table[s2].first == 'R'))
                type = 'u';
            else
                type = 'R';

            x = sym_table[s1].second;
            y = sym_table[s2].second;
        }
    }

    switch(op)
    {
        case '+' :
        {
            if(type == 'A' || type == 'R')
            {
                value = x+y;
                return true;
            }
            else
            {
                cout << "Cannot add two un-absolute values!" << endl;
                return false;
            }
            break;
        }

        case '-' :
        {
            if(type == 'A' || type == 'R')
            {
                value = x-y;
                type = 'A';
                return true;
            }
            else
            {
                value = x-y;
                type = 'R';
                return true;
            }
            break;
        }

        case '*' :
        {
            if(type == 'A')
            {
                value = x*y;
                return true;
            }
            else
            {
                cout << "Cannot multiply un-absolute values!" << endl;
                return false;
            }
            break;
        }

        case '/' :
        {
            if(type == 'A')
            {
                value = x/y;
                return true;
            }
            else
            {
                cout << "Cannot divide un-absolute values!" << endl;
                return false;
            }
            break;
        }

        default :
        {
            return true;
            break;
        }
    }
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
