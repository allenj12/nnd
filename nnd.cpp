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

//everything that lives on the data stack
//or the program stack
class NNDObject{
public:
  NNDObject(){}
  virtual ~NNDObject(){}
  virtual std::string toString() = 0;
};

//basic int with deep copy constructor
class Int : public NNDObject{
public:
  int num;
  Int(int n){num = n;}
  Int(Int* i){num = i->num;}
  ~Int(){}
  std::string toString(){return std::to_string(num);};
};
/*
class NNDList : public NNDObject{
public:
  std::list<NNDObject> nndList;
  NNDList(std::list<NNDObject*> l){nndList = l;}
  NNDList(NNDList* i){nndList = i->nndList;}//fix this
  ~NNDList(){}
  std::string toString(){return "a list";};//fix this
};
*/
//most functions not including literals that
//push themselves onto the stack(such as ints)
class Word : public NNDObject{
public:
  Word(){}
  virtual ~Word(){}
};

class DsWord : public Word{
public:
  std::string name;
  std::function<void(std::list<NNDObject*> &)> word;
  DsWord(std::string n, std::function<void(std::list<NNDObject*> &)> w){word = w; name = n;}
  ~DsWord(){}
  std::string toString() override {return name;}
  void eval(std::list<NNDObject*> &st){
    word(st);
  }
};

class PsWord : public Word{
public:
  std::string name;
  std::function<void( std::string, std::list<NNDObject*> &)> parseWord;
  PsWord(std::string n,std::function<void(std::string, std::list<NNDObject*> &)> w){
    parseWord = w;
    name = n;
  }
  ~PsWord(){}
  std::string toString() override {return name;}
  void parse(std::string input, std::list<NNDObject*> &programStack){
    parseWord(input,programStack);
  }
};

//total state of the running program
struct Program{
  std::list<NNDObject*> dataStack;
  std::list<NNDObject*> programStack;
  std::unordered_map<std::string, Word*> env;
};

//fix this to do deep copies?
void dup(std::list<NNDObject*> &st){
  if(!st.empty()){
    st.push_front(st.front());
  }
}

void clear(std::list<NNDObject*> &st){
  while(!st.empty()){
    st.pop_front();
  }
}

void mul(std::list<NNDObject*> &st){
  if(st.size() >= 2){
    NNDObject* xDPtr = st.front();
    st.pop_front();
    NNDObject* yDPtr = st.front();
    st.pop_front();

    Int* xPtr = dynamic_cast<Int*>(xDPtr);
    Int* yPtr = dynamic_cast<Int*>(yDPtr);
    if(xPtr && yPtr){
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
//representations ["5" "dup"] -> [NNDObject that pushes 5, NNDObject that dup's]
std::list<NNDObject*> parse(std::list<std::string> tokens, //fix this, maybe do this one at a time
			    std::unordered_map<std::string, Word*> env){
  std::list<NNDObject*> parsed;
  for(auto const& i : tokens){
    //literal int check
    if(allDigit(i)){
      int iToInt = std::stoi(i);
      parsed.push_back(new DsWord(i,
				  [=](std::list<NNDObject*> &st){
				    st.push_front(new Int(iToInt));
				  }));
    }
    //predifined function check
    else{
      std::unordered_map<std::string,Word*>::iterator lookup = env.find(i);
      if(lookup != env.end()){

	DsWord* dsPtr = dynamic_cast<DsWord*>(&*lookup->second);
	if(dsPtr){
	  parsed.push_back(dsPtr);
	}
	else{
	  PsWord* psPtr = dynamic_cast<PsWord*>(&*lookup->second);
	  if(psPtr){
	    //parse things
	  }
	}
      }
    }
  } 
  return parsed;
}

DsWord* fnToDsWord(std::string name, std::function<void(std::list<NNDObject*> &)> fn){
  return new DsWord(name, fn);
}

Program initProgram(){
  Program p;
  //start with just the dup function
  std::unordered_map<std::string, Word*> hmap;
  hmap["dup"] = fnToDsWord("dup",&dup);
  hmap["*"] = fnToDsWord("*", &mul);
  hmap["clear"] = fnToDsWord("clear", &clear);

  std::list<NNDObject*> dataStack;
  p.dataStack = dataStack;

  std::list<NNDObject*> programStack;
  p.programStack = programStack;

  p.env = hmap;
  
  return p;
}

void run(Program &p){
  while(!p.programStack.empty()){
    DsWord* wordPtr = dynamic_cast<DsWord*>(p.programStack.front());
    if(wordPtr){
      wordPtr->eval(p.dataStack);
    }
    p.programStack.pop_front();
  }
}

void updateProgramStack(Program &p, std::string str){
  p.programStack = parse(tokenize(str), p.env);
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
