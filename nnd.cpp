#include <iostream>
//#include <stack>
#include <string>
#include <functional>
#include <list>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <ctype.h>
#include <unordered_map>

//everything that lives on the stack
class Data{
public:
  Data(){}
  virtual ~Data(){}
  virtual std::string toString() = 0;
};

//basic int with deep copy constructor
class Int : public Data{
public:
  int num;
  Int(int n){num = n;}
  Int(Int* i){num = i->num;}
  ~Int(){}
  std::string toString(){return std::to_string(num);};
};

//everything that lives in the program state
//i.e 'dup' or '5'(a function that pushes 5 to the
//stack)
class Eval{
public:
  Eval(){};
  virtual ~Eval(){};
  virtual void eval(std::list<Data*> &) = 0;
};

//should probably get replaced
//can be collapsed into Word
class IntWord : public Eval{
public:
  int num;
  IntWord(int n){num = n;}
  ~IntWord(){}
  void eval(std::list<Data*> &st){
    st.push_front(new Int(num));
  }
  
  int getInt(){return num;}
};

//most functions not including literals that
//push themselves onto the stack(such as ints)
class Word : public Eval{
public:
  std::function<void(std::list<Data*> &)> word;
  Word(std::function<void(std::list<Data*> &)> w){word = w;}
  ~Word(){}
  void eval(std::list<Data*> &st){
    word(st);
  }
};

//total state of the running program
struct Program{
  std::list<Data*> dataStack;
  std::list<Eval*> programStack;
  std::unordered_map<std::string,std::function<void(std::list<Data*> &)>> env;
};

//fix to do hard copies?
void dup(std::list<Data*> &st){
  if(!st.empty()){
    st.push_front(st.front());
  }
}

void mul(std::list<Data*> &st){
  if(st.size() >= 2){
    Data* xDPtr = st.front();
    st.pop_front();
    Data* yDPtr = st.front();
    st.pop_front();

    Int* xPtr = dynamic_cast<Int*>(xDPtr);
    Int* yPtr = dynamic_cast<Int*>(yDPtr);
    if(xDPtr && yPtr){
      st.push_front(new Int(xPtr->num * yPtr->num));
    }
  }
}

//simply splits the string by its spaces
std::list<std::string> tokenize(std::string str){
  std::list<std::string> tokens;
  std::istringstream iss(str);
  copy(std::istream_iterator<std::string>(iss),
       std::istream_iterator<std::string>(),
       back_inserter(tokens));
  return tokens;
}

//little debugging function
void printTokens(std::list<std::string> tokens){
  for(auto const& i : tokens){
    std::cout<<i<<std::endl;
  }
}

//if every char is a digit
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

//changes a list of tokens to a list of there literal
//representations ["5" "dup"] -> [Eval that pushes 5, Eval that dup's]
std::list<Eval*> identify(std::list<std::string> tokens,
			  std::unordered_map<std::string,std::function<void(std::list<Data*> &)>> env){
  std::list<Eval*> identified;
  for(auto const& i : tokens){
    std::unordered_map<std::string,std::function<void(std::list<Data*> &)>>::const_iterator lookup = env.find(i);
    //literal int check
    if(allDigit(i)){
      identified.push_back(new IntWord(std::stoi(i)));
    }
    //predifined function check
    else if(lookup != env.end()){
      identified.push_back(new Word(lookup->second));
    }
  }
  return identified;
}

Program initProgram(){
  Program p;
  //start with just the dup function
  std::unordered_map<std::string,std::function<void(std::list<Data*> &)>> hmap;
  hmap["dup"] = &dup;
  hmap["*"] = &mul;

  std::list<Data*> dataStack;
  p.dataStack = dataStack;

  std::list<Eval*> programStack;
  p.programStack = programStack;

  p.env = hmap;
  
  return p;
}

void run(Program &p){
  while(!p.programStack.empty()){
    p.programStack.front()->eval(p.dataStack);
    p.programStack.pop_front();
  }
}

void updateProgramStack(Program &p, std::string str){
  p.programStack = identify(tokenize(str), p.env);
}

int main(){
  //lest start by evaluating
  //"5 dup"
  std::cout<<"\n";
  std::cout<<"NNG repl: 'quit' to quit"<<std::endl;

  std::string commands;
  Program p = initProgram();
  while(commands != "quit"){
    std::cout<<">> ";
    //a plain cin is cool here to get a
    //quick "step-by-step"
    getline(std::cin,commands);

    updateProgramStack(p, commands);
    run(p);

    for(auto const& i : p.dataStack){
      std::cout<<i->toString()<<std::endl;
    }
    
    std::cout<<"\n";
  }
  return 0;
}
