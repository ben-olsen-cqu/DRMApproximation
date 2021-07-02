#pragma once
#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <exception>

enum class ParamType {Text, Real, Integer, Bool};

struct Parameter
{
private:
	std::string name; //name for -- usage in command line arguments
	std::string abbrev; //abbreviation for name for use with - command line arguments, typically taken as first letter of name unless already taken
	ParamType type; //The type of value being stored

	//use multiple data types
	std::string svalue;
	int ivalue;
	float fvalue;
	bool bvalue;

public:
	//Initialise the parameter without storing a value
	Parameter(std::string _name, std::string _abbrev, ParamType _type)
	{
		name = _name;
		type = _type;
        abbrev = _abbrev;
		bvalue = false;
	}

	//Store a value in an already initialised parameter
	void Store(std::string _value)
	{
		if (type == ParamType::Text)
		{
			svalue = _value;
		}
		else if (type == ParamType::Integer)
		{
			ivalue = std::stoi(_value);
		}
		else if (type == ParamType::Real)
		{
			fvalue = std::stof(_value);
		}
		else if (type == ParamType::Bool)
		{
			if (_value == "true")
			{
				bvalue = true;
			}
			else
			{
				bvalue = false;
			}
		}
		else
		{
			std::cout << "Error Unknown Parameter Type Passed to Parameter Parser";
		}
	}

	//Initialise a parameter and store the value
	Parameter(std::string _name, std::string _abbrev, ParamType _type, std::string _value)
	{
		name = _name;
		type = _type;
        abbrev = _abbrev;
		Store(_value);
		bvalue = false;
	}

	//Get the name of a parameter
	std::string Name()
	{
		return name;
	}
	
	//Get the abbreviation of a property
	std::string Abbreviation()
	{
		return abbrev;
	}
	
	//Get the type of parameter
	ParamType Type()
	{
		return type;
	}

	//Retreive an integer value
	int RetrieveI()
	{
		if (type == ParamType::Integer)
			return ivalue;
		else
			std::cout << "This is not an Integer Parameter!";
        return -1;
	}

	//Retreive a text value
	std::string RetrieveS()
	{
		if (type == ParamType::Text)
			return svalue;
		else
			std::cout << "This is not an Text Parameter!";
        return "";
	}

	//Retreive a bool value
	bool RetrieveB()
	{
		if (type == ParamType::Bool)
			return bvalue;
		else
			std::cout << "This is not an Bool Parameter!";
        return false;
	}

	//Retreive a float value
	float RetrieveF()
	{
		if (type == ParamType::Real)
			return fvalue;
		else
			std::cout << "This is not an Real Parameter!";
        return -1.0f;
	}
};

class ParamManager
{
private:
	//Container for all parameters
	std::vector<Parameter> params;
public:
	//Constructor

	ParamManager() :params() {}

	//Functions

	void Store(Parameter p);						//Store a parameter in the manager
	int SetParam(std::vector<std::string>* args);	//Set parameters - function used by Set()
	void Set(int argc, char* argv[]);				//Set the values of stored parameters if passed in by the command line or set manually
	void Output();									//cout the names and values of the params
	int MaxParamCount();
	
	Parameter GetbyAbbrev(std::string abbrev);	//Get a parameter by searching by name
	Parameter GetbyName(std::string name);		//Get a parameter by searching by abbreviation
	
	std::string ValueS(std::string name);	//Get a string value from a parameter by searching for the parameter by name
	float ValueF(std::string name);			//Get a float value from a parameter by searching for the parameter by name
	int ValueI(std::string name);			//Get a integer value from a parameter by searching for the parameter by name
	bool ValueB(std::string name);			//Get a bool value from a parameter by searching for the parameter by name

};
