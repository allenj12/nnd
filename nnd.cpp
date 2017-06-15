#include <iostream>
#include <stack>
#include <string>
#include <functional>
#include <list>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <ctype.h>
#include <unordered_map>

std::string program = "5 dup";

class Data{
public:
  Data(){}
  virtual ~Data(){}
};

class Int : public Data{
public:
  int num;
  Int(int n){num = n;}
  Int(Int* i){num = i->num;}
  ~Int(){}
};

class Eval{
public:
  Eval(){};
  virtual ~Eval(){};
  virtual void eval(std::stack<Data*> &) = 0;
};

//should probably get replaced
//can be collapsed int Word
class IntWord : public Eval{
public:
  int num;
  IntWord(int n){num = n;}
  ~IntWord(){}
  void eval(std::stack<Data*> &st){
    Data *i = new Int(num);
    st.push(i);
  }
  
  int getInt(){return num;}
};

class Word : public Eval{
public:
  std::function<void(std::stack<Data*> &)> word;
  Word(std::function<void(std::stack<Data*> &)> w){word = w;}
  ~Word(){}
  void eval(std::stack<Data*> &st){
    word(st);
  }
};

struct Program{
  std::stack<Data*> dataStack;
  std::list<Eval*> programStack;
  std::unordered_map<std::string,std::function<void(std::stack<Data*> &)>> env;
};

//fix to do hard copies?
void dup(std::stack<Data*> &st){
  st.push(st.top());
}

std::list<std::string> tokenize(std::string str){
  std::list<std::string> tokens;
  std::istringstream iss(str);
  copy(std::istream_iterator<std::string>(iss),
       std::istream_iterator<std::string>(),
       back_inserter(tokens));
  return tokens;
}

void printTokens(std::list<std::string> tokens){
  for(auto const& i : tokens){
    std::cout<<i<<std::endl;
  }
}

bool allDigit(std::string str){
  bool all = true;
  for(auto const& i : str){
    if(!isdigit(i)){
      all = false;
      break;
    }
  }
  return all;
}

std::list<Eval*> identify(std::list<std::string> tokens,
			  std::unordered_map<std::string,std::function<void(std::stack<Data*> &)>> env){
  std::list<Eval*> identified;
  for(auto const& i : tokens){
    std::unordered_map<std::string,std::function<void(std::stack<Data*> &)>>::const_iterator lookup = env.find(i);
    if(allDigit(i)){
      Eval* digit = new IntWord(std::stoi(i));
      identified.push_back(digit);
    }
    else if(lookup != env.end()){
      Eval* function = new Word(lookup->second);
      identified.push_back(function);
    }
  }
  return identified;
}

Program initProgram(){
  Program p;
  //start with just the dup function
  std::unordered_map<std::string,std::function<void(std::stack<Data*> &)>> hmap;
  void (*d)(std::stack<Data*> &) = &dup;
  hmap["dup"] = d;

  std::stack<Data*> dataStack;
  p.dataStack = dataStack;

  std::list<std::string> tokens = tokenize(program);
  std::list<Eval*> programStack = identify(tokens,hmap);
  p.programStack = programStack;

  p.env = hmap;
  
  return p;
}

void run(Program &p){
  for(auto const& i : p.programStack){
    i->eval(p.dataStack);
  }
}

int main(){
  //lest start by evaluating
  //"5 dup"
  Program p = initProgram();
  run(p);

  while(!p.dataStack.empty()){
    Data* dataPtr = p.dataStack.top();
    std::cout<<dataPtr<<std::endl;
    p.dataStack.pop();

    Int* intPtr = dynamic_cast<Int*>(dataPtr);
    if(intPtr){
      std::cout<<intPtr->num<<std::endl;
    }
  }
  
  return 0;
}
