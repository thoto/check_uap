// check_uap checks on access point controllers
// Copyright (C) 2015 thoto
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include<jsoncpp/json/json.h>
#include<fstream>
#include<iostream>
#include<string>
#include<list>

#define UNUSED(expr) (void)(expr)

using namespace std;

#include"main.h"
#include"actions.h"

string vmsg=""; // messages to output in verbose debug mode


enum nagret{RET_UNKNOWN, RET_CRITICAL, RET_WARNING, RET_OK};

int nagret_return=3;

/**
 * generate nagios valid output and return return code to be returned
 **/
int nagret_gen(enum nagret nr, string msg, string perf=""){
	switch(nr){
		case RET_CRITICAL:
			cout<<"UAP CRITICAL - "+msg+"|"+perf<<endl;
			return nagret_return=2;
			break;
		case RET_WARNING:
			cout<<"UAP WARNING - "+msg+"|"+perf<<endl;
			return nagret_return=1;
			break;
		case RET_OK:
			cout<<"UAP OK - "+msg+"|"+perf<<endl;
			return nagret_return=0;
			break;
		case RET_UNKNOWN:
		default:
			cout<<"UAP UNKNOWN - "+msg+"|"+perf<<endl;
			return nagret_return=3;
	}
}

/**
 * die on error
 **/
void die(string msg){
	exit(nagret_gen(RET_UNKNOWN,"EHH: "+msg));
}

int main(int argc, char** argv){
	UNUSED(argc); UNUSED(argv);
	Json::Reader json_reader;
	Json::Value json_root;

	// every argument needs one state at the parser stage
	string arg;
	enum argparsemode{APM_NONE, APM_WARN, APM_CRIT, APM_HOST, APM_VERSION,
		APM_HELP, APM_VERBOSE, APM_TIMEOUT, APM_FILE, APM_SUM, APM_ACTION}
			apm=APM_NONE;

	// simple structure describing an argument to pass to this application
	class nextval_t{
		public:
			string st; size_t sl; string lt; argparsemode state; string desc;
			nextval_t(string st,string lt,argparsemode state,string desc=""):
				st(st),sl(st.length()),lt(lt),state(state),desc(desc){};
	};

	// list of all possible arguments
	list<nextval_t> nvl={ {"-w","--warning",APM_WARN,"warning level"},
		{"-c","--critical",APM_CRIT,"critical level"},
		{"-H","--hostname",APM_HOST,"hostname (no function)"},
		{"-V","--version",APM_VERSION,"print version"},
		{"-h","--help",APM_HELP,"print this help"},
		{"-v","--verbose",APM_VERBOSE,"verbosity of output"},
		{"-t","--timeout",APM_TIMEOUT,"timeout"},
		{"-f","--file",APM_FILE},
		{"-s","--sum",APM_SUM},
		{"-a","--action",APM_ACTION} };

	auto print_args=[nvl](){
		for(auto nv: nvl){
			cerr <<"\t"<< nv.lt << "("<<nv.st<<"): "<<nv.desc<<endl;
		}
	};

	// basic variable initialization
	string warn=""; string crit="";
	vmsg=""; // messages to output in verbose debug mode
	int vlvl=0; // verbosity
	string file="foo.json"; // file to read data from
	string mac=""; // mac to look for
	enum cu_action{ ACTION_NUMALL,ACTION_NUMGUEST,ACTION_NUMUSER,ACTION_ONLINE}
		action=ACTION_NUMALL;

	auto opt_sum=[&](){
		cerr << "warning level: " << warn << endl;
		cerr << "critical level: " << crit << endl;
		cerr << "verbosity level: " << vlvl << endl;
		cerr << "file to read: " << file << endl;
		cerr << "AP to look for: " << ((mac=="")?"all":mac) << endl;
		cerr << "action: ";
		if(action==ACTION_NUMALL) cerr << "Number of all clients";
		else if(action==ACTION_NUMGUEST) cerr << "Number of all guests";
		else if(action==ACTION_NUMUSER) cerr << "Number of all users";
		else if(action==ACTION_ONLINE) cerr << "AP online";
		else cerr << "UNKNOWN";
		cerr << endl;
	};


#ifdef RUN_TEST	
	// testing stuff
	auto testdummy=new action_numuser();
	vmsg+="Test result: ";
	vmsg+=(testdummy->test_action_numeric())?"ok":"FAIL!!!";
	vmsg+="\n";
#endif



	bool skip=false;
	int i=1;
	// parse arguments
	// if you want to know what it does: read help text.
	while(i<argc||apm!=APM_NONE){
		if(!skip&&i<argc) arg=string(argv[i]); // don't take next argument
		else skip=false;

		switch(apm){
			case APM_NONE:
				for(auto nv: nvl){
					if(arg.compare(0,nv.sl,nv.st)==0){ // short syntax
						apm=nv.state;
						if(arg.length()>nv.sl){ // combined parameter eg. -c123
							arg=arg.substr(nv.sl); skip=true;
						}
					}else if(arg.compare(nv.lt)==0) apm=nv.state; // long
				}
				if(apm==APM_NONE) {
					if(mac.empty()){ //take mac
						mac=arg;
					}else die("invalid argument: "+arg);
				}
				break;
			case APM_WARN:
				if(i>=argc) die("invalid definition.");
				warn=arg;
				apm=APM_NONE; break;
			case APM_CRIT:
				if(i>=argc) die("invalid definition.");
				crit=arg;
				apm=APM_NONE; break;
			case APM_HOST:
				if(i>=argc) die("invalid definition.");
				vmsg+="ignoring host argument";
				apm=APM_NONE; break;
			case APM_VERSION:
				cerr << "0.0.1" << endl;
				exit(nagret_gen(RET_UNKNOWN,"")); break;
			case APM_HELP:
				cerr << "possible arguments:" << endl;
				print_args();
				exit(nagret_gen(RET_UNKNOWN,"")); break;
			case APM_VERBOSE:
				if(i>=argc) die("invalid definition.");
				if(arg=="0") vlvl=0;
				else if(arg=="1") vlvl=1;
				else if(arg=="2") vlvl=2;
				else if(arg=="3") vlvl=3;
				else die("invalid argument: "+arg);
				apm=APM_NONE; break;
			case APM_TIMEOUT:
				if(i>=argc) die("invalid definition.");
				vmsg+="ignoring timeout argument";
				apm=APM_NONE; break;
			case APM_FILE:
				if(i>=argc) die("invalid definition.");
				file=arg;
				apm=APM_NONE; break;
			case APM_SUM:
				if(i>=argc) die("invalid definition.");
				mac=""; skip=true;
				apm=APM_NONE; break;
			case APM_ACTION:
				if(i>=argc) die("invalid definition.");

				if(arg=="numall") action=ACTION_NUMALL;
				else if (arg=="numguest") action=ACTION_NUMGUEST;
				else if (arg=="numuser") action=ACTION_NUMUSER;
				else if (arg=="online") action=ACTION_ONLINE;
				else die("invalid argument: "+arg);
				apm=APM_NONE; break;
		}
		if(!skip) i++; // don't take next argument
	}

	if(vlvl==3){
		cerr << vmsg << endl; vmsg="";
		opt_sum();
	}

	// file handling: open JSON parser
	ifstream data(file);
	if(!data.good()) die("Can't open file");
	json_reader.parse(data,json_root);
	Json::Value json_data=json_root["data"];
	if(json_data.isNull()) die("data null");
	if(!json_data.isArray()) die("data invalid");

	// evaluate action
	action_generic* action_handler=nullptr;

	switch(action){
		case ACTION_NUMALL:
			action_handler=new action_numall(); break;
		case ACTION_NUMUSER:
			action_handler=new action_numuser(); break;
		case ACTION_NUMGUEST:
			action_handler=new action_numguest(); break;
		case ACTION_ONLINE:
			action_handler=new action_online(); break;
	}

	// iterate over every access point and check if it should be handled
	// by action_handler and do so if wanted.
	for(Json::Value a:json_data){
		if(!a.isMember("mac")) vmsg+="invalid element\n";
		else{
			if(vlvl>=1) vmsg+="AP has MAC "+a["mac"].asString()+"\n";
			if(mac==""||mac.compare(a["mac"].asString())==0){
				if(vlvl>=2) vmsg+="\tMatch.\n";
				if(action_handler!=nullptr){
					action_handler->scan(a);
					if(vlvl>=2){ vmsg+=action_handler->toString()+"\n"
							+action_handler->vmsg; action_handler->vmsg="";}
				}
			}
		}
	}

	// generates performance data ... that's not the nice way, I know
	auto perfdata_gen=[action_handler,warn,crit]()->string{
		return action_handler->perfdata()+";"+string(warn)+";"+
			string(crit)+";0";
	};

	// Match range and handle return code
	int ret_code=3;
	if(action_handler!=nullptr){
		if((crit==""&&warn==""&&action_handler->match_range())||
				(crit!=""&&action_handler->match_range(crit))){
			ret_code=nagret_gen(RET_CRITICAL," . ",perfdata_gen());
		}
		else if(warn!="" && action_handler->match_range(warn)){
			ret_code=nagret_gen(RET_WARNING," . ",perfdata_gen());
		}else {
			ret_code=nagret_gen(RET_OK," . ",perfdata_gen());
		}
	}

	// output debugging data if verbose output is wanted
	if(vlvl>0){
		if(!vmsg.empty()) cerr << vmsg << endl;
	}
	return ret_code;

}


