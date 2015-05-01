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

#include"actions.h"
#include"main.h"

/**
 * parse range definition at which a warning or critical state should
 * be generated. This is a quite complex funktion which should be tested
 * by test_action_numeric.
 * For example pdef might be 10:20 if a argument -w 10:20 is specified at
 * invocation of the binary.
 * Attention: This method just changes state of the action_numeric object
 * and its return code does NOT say that the scanned data match this range.
 * This functionality is handled by the match_range function.
 **/
bool action_numeric::parse_range(const string pdef){
	string def(pdef); // make it mutable!
	infmin=false;infmax=false;
	lmin=0;lmax=0;
	inside=(def[0]=='@'); // alert if inside range
	if(inside) def.erase(0,1);

	const size_t sep=def.find_first_of(':');

	if(sep!=string::npos){ // delimiter found: lower boundary !=0
		if(sep==0) lmin=0; // start undefined -> 0
		else if(def[0]=='~') infmin=true;
		else try{ // FIXME: parse negative?
			lmin=stoi(def.substr(0,sep));
		}catch(void* e){ return false;};

		def=def.substr(sep+1);
	}// else lmin=0 -> default

	if(def.length()==0||def[0]=='~') infmax=true;
	else try{ // FIXME: parse negative?
		lmax=stoi(def);
	}catch(void* e){ return false;};
	return true;
}

/**
 * data read match this range?
 * @returns true if a alert should be generated
 **/
bool action_numeric::match_range(const string pdef){ // alert -> true
	if(!parse_range(pdef)){
		cerr << "Parsing range failed" << endl;
		return true;
	}
	return (inside!=( (!infmin&&(num<lmin)) || (!infmax&&(num>lmax)) ));
}
/**
 * match_range(string) if no range is given
 **/
bool action_numeric::match_range(){
	return false;
}

/**
 * see above
 **/
bool action_boolean::match_range(const string pdef){
	if(pdef=="@") return state;
	else return !state;
}
bool action_boolean::match_range(){
	return !state;
}

#ifdef RUN_TEST
/**
 * Test match_range and parse_range functionality. These values are taken
 * from the nagios plugin development guidelines.
 **/
bool action_numeric::test_action_numeric(){
	auto matcher=[&](signed int pnum, const string range)-> bool{
		num=pnum; return match_range(range);
	};
	return ((matcher(-1,"10")==true)&&
		(matcher(0,"10")==false)&&
		(matcher(10,"10")==false)&&
		(matcher(11,"10")==true)&& // FIXME :10
		(matcher(9,"10:")==true)&&
		(matcher(10,"10:")==false)&&
		(matcher(11,"10:")==false)&&
		(matcher(11,"~:10")==true)&&
		(matcher(10,"~:10")==false)&&
		(matcher(0,"~:10")==false)&&
		(matcher(-1,"~:10")==false)&&
		(matcher(9,"10:20")==true)&&
		(matcher(10,"10:20")==false)&&
		(matcher(20,"10:20")==false)&&
		(matcher(21,"10:20")==true)&&
		(matcher(9,"@10:20")==false)&&
		(matcher(10,"@10:20")==true)&&
		(matcher(20,"@10:20")==true)&&
		(matcher(21,"@10:20")==false));
}
#endif


/**
 * read value off JSON data
 **/
void action_numguest::scan(const Json::Value &v){
	if(v.isMember("guest-num_sta")&&v["guest-num_sta"].isInt()){
		vmsg+="\tGuest:"+v["guest-num_sta"].asString()+"\n";
		if(guest_add) num+=v["guest-num_sta"].asInt();
		else num-=v["guest-num_sta"].asInt();
	}
}

/**
 * read value off JSON data
 **/
void action_numall::scan(const Json::Value &v){
	if(v.isMember("num_sta")&&v["num_sta"].isInt()){
		vmsg+="\tStations:"+v["num_sta"].asString()+"\n";
		num+=v["num_sta"].asInt();
	}
}

/**
 * read value off JSON data
 **/
void action_numuser::scan(const Json::Value &v){
	action_numall::scan(v);
	action_numguest::scan(v);
}

/**
 * read value off JSON data
 **/
void action_online::scan(const Json::Value &v){
	if(v.isMember("state")){
		vmsg+="\tstate is: "+v["state"].asString()+"\n";
		if(v["state"].isInt()&&v["state"].asInt()==1)
			state=true;
		else state=false;
	}else{
		vmsg+="\tHas no state.\n";
		state=false;
	}
}
