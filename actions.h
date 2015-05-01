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

#ifndef _ACTIONS_H_
#define _ACTIONS_H_
#include<jsoncpp/json/json.h>
#include<string>
using namespace std;

class action_generic{
	public:
		action_generic(){};
		~action_generic(){};
		virtual void scan(const Json::Value&)=0;
		virtual bool match_range(const string def)=0;
		virtual bool match_range()=0;
		virtual string toString(){ return "generic"; }
		virtual string perfdata()=0;
		string vmsg="";
};

class action_boolean: public action_generic{
	protected:
		bool state;
	public:
		action_boolean():state(false){};
		bool match_range(const string pdef);
		bool match_range();

		virtual string toString(){
			return "action_boolean: "+string(state?"true":"false"); }
		virtual string perfdata()=0;
		#ifdef RUN_TEST
//		bool test_action_boolean();
		#endif
};


// TODO: Match_range(null) implementieren!

class action_numeric: public action_generic{
	protected:
		bool parse_range(const string pdef);
		int num;
		bool infmin=false,infmax=false,inside=false;
		int lmin=0;int lmax=0;
	public:
		action_numeric():num(0){};
		bool match_range(const string pdef);
		bool match_range();
		virtual string toString(){
			return "action_numeric: "+to_string(num); }
		virtual string perfdata()=0;
		#ifdef RUN_TEST
		bool test_action_numeric();
		#endif
};

class action_numguest: public virtual action_numeric{
	protected:
		bool guest_add=true; // add or subtract value: numuser
	public:
		void scan(const Json::Value &v);
		virtual string toString(){
			return "action_numguest: "+to_string(num); }
		virtual string perfdata(){ return "numguest="+to_string(num); };
};

class action_numall: public virtual action_numeric{
	public:
		void scan(const Json::Value &v);
		virtual string toString(){
			return "action_numall: "+to_string(num); }
		virtual string perfdata(){ return "numall="+to_string(num); };
};

class action_numuser: public virtual action_numeric, public action_numall,
		public action_numguest {
	public:
		action_numuser():action_numeric(),action_numall(),action_numguest(){
			guest_add=false;
		};
		void scan(const Json::Value &v);
		virtual string toString(){
			return "action_numuser: "+to_string(num); }
		virtual string perfdata(){ return "numuser="+to_string(num); };
};

class action_online: public virtual action_boolean {
	public:
		action_online():action_boolean(){};
		void scan(const Json::Value &v);
		virtual string toString(){
			return "action_online: "+string(state?"true":"false"); }
		virtual string perfdata(){
			return "online="+string(state?"true":"false"); };
};

#endif //_ACTIONS_H_
