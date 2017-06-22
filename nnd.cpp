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


//most functions not including literals that
//push themselves onto the stack(such as ints)
class Word : public NNDObject{
public:
  Word(){}
  virtual ~Word(){}
};

class DsWord : public Word{
public:
  DsWord(){}
  ~DsWord(){}
  virtual void eval(std::list<NNDObject*>&) = 0;
};

class PsWord : public Word{
public:
  std::string name;
  std::string end;
  std::function<void(std::string,
		     std::unordered_map<std::string, Word*>&,
		     std::list<NNDObject*> &)> parseWord;
  PsWord(std::string n,
	 std::string e,
	 std::function<void(std::string,
			    std::unordered_map<std::string, Word*>&,
			    std::list<NNDObject*> &)> w){
    parseWord = w;
    name = n;
    end = e;
  }
  ~PsWord(){}
  std::string toString() {return "PsWord: " + name;}
  void parse(std::string input,
	     std::unordered_map<std::string, Word*> &env,
	     std::list<NNDObject*> &programStack){
    parseWord(input,env,programStack);
  }
};

class Function : public DsWord{
public:
  std::string name;
  std::function<void(std::list<NNDObject*> &)> word;
  Function(std::string n,
	   std::function<void(std::list<NNDObject*> &)> w){word = w; name = n;}
  Function(const Function& fn) :
    name( fn.name ), word( fn.word )
  {}
  ~Function(){}
  std::string toString(){return name;}
  void eval(std::list<NNDObject*> &st){
    word(st);
  }
};

////
//basic int with deep copy constructor
class Int : public DsWord{
public:
  int num;
  Int(int n){num = n;}
  Int(const Int& i) :
    num( i.num )
  {}
  ~Int(){}
  std::string toString() {return std::to_string(num);};
  void eval(std::list<NNDObject*> &st){
    st.push_front(this);
  }
};

class Bool : public DsWord{
public:
  bool boolean;
  Bool(bool b){boolean = b;}
  Bool(const Bool& i) :
    boolean( i.boolean )
  {}
  ~Bool(){}
  std::string toString(){
      if(boolean){
	return "t";
	  }
      else{
	return "f";
      }
  }
  void eval(std::list<NNDObject*> &st){
    st.push_front(this);
  }
};

class String : public DsWord{
public:
  std::string str;
  String(std::string s){str = s;}
  String(const String& s) :
    str( s.str )
  {}
  ~String(){}
  std::string toString() {return '"'+str+'"';};
  void eval(std::list<NNDObject*> &st){
    st.push_front(this);
  }
};

class NNDList : public DsWord{
public:
  std::list<NNDObject*> nndList;
  NNDList(std::list<NNDObject*> l){nndList = l;}
  NNDList(const NNDList& l) :
    nndList( l.nndList )
  {}
  ~NNDList(){}
  std::string toString(){
    std::string temp = "[";
    for(auto const& i : nndList){
      temp += " " + i->toString();
    }
    return temp += " ]";
  }
  void eval(std::list<NNDObject*> &st){
    st.push_front(this);
  }
};

//total state of the running program
struct Program{
  std::list<NNDObject*> dataStack;
  std::list<NNDObject*> programStack;
  std::unordered_map<std::string, Word*> &env;

  Program(std::list<NNDObject*> ds,
	  std::list<NNDObject*> ps,
	  std::unordered_map<std::string, Word*> &e) : 
    dataStack( ds ), programStack( ps ), env( e )
  {}
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

void subtract(std::list<NNDObject*> &st){
  if(st.size() >= 2){
    NNDObject* xDPtr = st.front();
    st.pop_front();
    NNDObject* yDPtr = st.front();
    st.pop_front();

    Int* xPtr = dynamic_cast<Int*>(xDPtr);
    Int* yPtr = dynamic_cast<Int*>(yDPtr);
    if(xPtr && yPtr){
      st.push_front(new Int(yPtr->num - xPtr->num));
    }
  }
}

void add(std::list<NNDObject*> &st){
  if(st.size() >= 2){
    NNDObject* xDPtr = st.front();
    st.pop_front();
    NNDObject* yDPtr = st.front();
    st.pop_front();

    Int* xPtr = dynamic_cast<Int*>(xDPtr);
    Int* yPtr = dynamic_cast<Int*>(yDPtr);
    if(xPtr && yPtr){
      st.push_front(new Int(xPtr->num + yPtr->num));
    }
  }
}

void eval(std::list<NNDObject*> &st){
  NNDObject* objectPtr = st.front();
  st.pop_front();
  NNDList* listPtr = dynamic_cast<NNDList*>(objectPtr);
  if(listPtr){
    for(auto const& i : listPtr->nndList){
      DsWord* dsPtr = dynamic_cast<DsWord*>(i);
      if(dsPtr){
	dsPtr->eval(st);
      }
    }
  }
}

void lte(std::list<NNDObject*> &st){
  if(st.size() >= 2){
    NNDObject* secondPtr = st.front();
    st.pop_front();
    NNDObject* firstPtr = st.front();
    st.pop_front();

    Int* sPtr = dynamic_cast<Int*>(secondPtr);
    Int* fPtr = dynamic_cast<Int*>(firstPtr);

    if(sPtr && fPtr){
      if(fPtr->num <= sPtr->num){
	st.push_front(new Bool(true));
      }
      else
	st.push_front(new Bool(false));
    }
  }
}

void lt(std::list<NNDObject*> &st){
  if(st.size() >= 2){
    NNDObject* secondPtr = st.front();
    st.pop_front();
    NNDObject* firstPtr = st.front();
    st.pop_front();

    Int* sPtr = dynamic_cast<Int*>(secondPtr);
    Int* fPtr = dynamic_cast<Int*>(firstPtr);

    if(sPtr && fPtr){
      if(fPtr->num < sPtr->num){
	st.push_front(new Bool(true));
      }
      else
	st.push_front(new Bool(false));
    }
  }
}

void drop(std::list<NNDObject*> &st){
  if(!st.empty()){
    st.pop_front();
  }
}

void nndIf(std::list<NNDObject*> &st){
  if(st.size() >= 3){
    NNDObject* yesPtr = st.front();
    st.pop_front();
    NNDObject* noPtr = st.front();
    st.pop_front();
    NNDObject* boolPtr = st.front();
    st.pop_front();

    Bool* bPtr = dynamic_cast<Bool*>(boolPtr);
    NNDList* nPtr = dynamic_cast<NNDList*>(noPtr);
    NNDList* yPtr = dynamic_cast<NNDList*>(yesPtr);

    if(bPtr && nPtr && yPtr){
      if(bPtr->boolean){
	st.push_front(yPtr);
	eval(st);
      }
      else{
	st.push_front(nPtr);
	eval(st);
      }
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
std::list<NNDObject*> parse(std::string input, //fix this, maybe do this one at a time
			    std::unordered_map<std::string, Word*> &env){
  std::list<NNDObject*> parsed;
  std::istringstream iss(input);
  std::string i;
  while(iss >> i){
    //literal int check
    if(allDigit(i)){
      int iToInt = std::stoi(i);
      parsed.push_back(new Int(iToInt));
    }
    //predifined function check
    else{
      std::unordered_map<std::string,Word*>::iterator lookup = env.find(i);
      if(lookup != env.end()){
	Function* dsPtr = dynamic_cast<Function*>(&*lookup->second);
	if(dsPtr){
	  parsed.push_back(dsPtr);
	}
	else{
	  PsWord* psPtr = dynamic_cast<PsWord*>(&*lookup->second);
	  if(psPtr){  
	    //hack for now
	    //fix this
	    bool foundEnd = false;
	    std::list<std::string> delimStack;
	    std::string input;
	    std::string temp;
	    while(!foundEnd){
	      iss>>temp;
	      if (psPtr->name == temp) {
		delimStack.push_front(psPtr->name);
	      }
	      if(temp == psPtr->end){
		if(delimStack.empty()){
		  foundEnd = true;
		}
		else{
		  delimStack.pop_front();
		}
	      }
	      if (!foundEnd){
		//an even bigger hack
		//fix this
		input += " " + temp;
	      }
	    }
	    psPtr->parse(input, env, parsed);
	  }
	}
      }
    }
  } 
  return parsed;
}

std::string trim(std::string input){
  return input.substr(input.find_first_not_of(" \t"),input.find_last_not_of(" \t"));
}

std::string getFirstWord(std::string input){
  return input.substr(0,input.find_first_of(" \t"));
}

std::string getRest(std::string input){
  return input.substr(input.find_first_of(" \t"));
}

std::string sliceFirstWord(std::string input){
  std::string temp = input.substr(input.find_first_of(" \t")); 
  return trim(temp);
}

//testing function
void everythingIsAList(std::list<NNDObject*> &st){
  std::list<NNDObject*> newList;
  while(!st.empty()){
    newList.push_front(st.front());
    st.pop_front();
  }
  st.push_front(new NNDList(newList));
}

//parsing word that creates parsing words
void defineSyntax(std::string input,
		  std::unordered_map<std::string,Word*> &env,
		  std::list<NNDObject*> &ps){

  input = trim(input);
  std::string name = getFirstWord(input);
  input = sliceFirstWord(input);

  std::string end = getFirstWord(input);
  std::string rest = sliceFirstWord(input);

  std::list<NNDObject*> parsed = parse(rest,env);

  std::function<void(std::string,
		     std::unordered_map<std::string,Word*>&,
		     std::list<NNDObject*> &)> fn;
  
  fn = [=](std::string input,
	   std::unordered_map<std::string,Word*> &env,
	   std::list<NNDObject*> &ps){
    std::list<NNDObject*> tempStack;
    tempStack.push_front(new String(input));
    for(auto const& i : parsed){
      DsWord* wordPtr = dynamic_cast<DsWord*>(i);
      if(wordPtr){
	wordPtr->eval(tempStack);
      }
    }
    while(!tempStack.empty()){
      ps.push_back(tempStack.front());
      tempStack.pop_front();
    }
  };
  env.insert({name,new PsWord(name,end,fn)});
}

//parsing word that creates words
void defineWord(std::string input,
		std::unordered_map<std::string,Word*> &env,
		std::list<NNDObject*> &ps){
  
  input = trim(input);
  std::string name = getFirstWord(input);
  std::string rest = sliceFirstWord(input);
  //we temp update the env to hold the name->fn that does nothing
  //we do this so we can reuse the standard parse function
  //again this is a total hack and probably should be changed.
  //fix this.
  std::function<void(std::list<NNDObject*> &)> fn;
  Function* updatedWord = new Function(name,fn);

  env.insert({name,updatedWord});

  std::list<NNDObject*> parsed = parse(rest,env);

  updatedWord->word = [=](std::list<NNDObject*> &st){
    for(auto const& i : parsed){
      DsWord* wordPtr = dynamic_cast<DsWord*>(i);
      if(wordPtr){
	if(wordPtr->toString() != name){
	  wordPtr->eval(st);
	}
      }
    }
  };
}

Program* initProgram(){
  //start with just the dup function
  std::unordered_map<std::string, Word*>* env = new std::unordered_map<std::string, Word*>();
  std::list<NNDObject*> dataStack;
  std::list<NNDObject*> programStack;
  Program* p = new Program(dataStack,programStack,*env);

  p->env.insert({"dup",new Function("dup",&dup)});
  p->env.insert({"*",new Function("*",&mul)});
  p->env.insert({"clear",new Function("clear",&clear)});
  p->env.insert({"eval", new Function("eval", &eval)});
  p->env.insert({"if", new Function("if", &nndIf)});
  p->env.insert({"<", new Function("<", &lt)});
  p->env.insert({"<=", new Function("<=", &lte)});
  p->env.insert({"drop", new Function("drop", &drop)});
  p->env.insert({"-", new Function("-", &subtract)});
  p->env.insert({"+", new Function("+", &add)});

  std::function<void(std::list<NNDObject*> &)> pfn =
    [=](std::list<NNDObject*> &st){
    std::list<NNDObject*> temp;
    if(!st.empty()){
      String* strPtr = dynamic_cast<String*>(st.front());
      if(strPtr){
	temp = parse(strPtr->str,*env);
      }
    }
    st.pop_front();
    st.push_front(new NNDList(temp));
  };
  p->env.insert({"parse",new Function("parse", pfn)});

  std::function<void(std::string,
		     std::unordered_map<std::string,Word*>&,
		     std::list<NNDObject*> &)> listFn;

  listFn = [](std::string input,
	  std::unordered_map<std::string,Word*> &env,
	  std::list<NNDObject*> &ps){
    std::list<NNDObject*> parsed = parse(input,env);
    ps.push_back(new NNDList(parsed));
  };

  p->env.insert({"[",new PsWord("[","]",listFn)});
  p->env.insert({":",new PsWord(":",";",&defineWord)});
  p->env.insert({":s",new PsWord(":s",";",&defineSyntax)});

  //testing if we use this function it should
  //only be for parsing words
  p->env.insert({"elist",new Function("elist", &everythingIsAList)});
  
  return p;
}

void run(Program* p){
  while(!p->programStack.empty()){
    DsWord* wordPtr = dynamic_cast<DsWord*>(p->programStack.front());
    if(wordPtr){
      wordPtr->eval(p->dataStack);
    }
    p->programStack.pop_front();
  }
}

void updateProgramStack(Program* p, std::string str){
  p->programStack = parse(str, p->env);
}

int main(){
  //lest start by evaluating
  //"5 dup"
  std::cout<<"\n";
  std::cout<<"NND repl: 'quit' to quit"<<std::endl;

  std::string commands;
  Program* p = initProgram();
  while(commands != "quit"){
    std::cout<<">> ";
    //a plain cin is cool here to get a
    //quick "step-by-step"
    getline(std::cin,commands);

    updateProgramStack(p, commands);
    run(p);

    for(auto const& i : p->dataStack){
      std::cout<<i->toString()<<std::endl;
    }
    
    std::cout<<"\n";
  }
  return 0;
}
