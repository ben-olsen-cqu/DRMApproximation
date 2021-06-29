#pragma once
#include <string>
#include <vector>
#include <variant>
#include <iostream>

enum class ParamType {Text, Real, Integer, Bool};

struct Parameter
{
private:
	std::string name; //name for -- usage in command line arguments
	std::string abbrev; //abbreviation for name for use with - command line arguments, typically taken as first letter of name unless already taken
	ParamType type; //The type of value being stored

	//use std::variant to hold multiple data types
	std::variant<std::string, float, int, bool> value;
	float fvalue;
	int ivalue;
	bool bvalue;

public:
	//Initialise the parameter without storing a value
	Parameter(std::string _name, std::string _abbrev, ParamType _type)
	{
		name = _name;
		type = _type;
        abbrev = _abbrev;
	}

	//Store a value in an already initialised parameter
	void Store(std::string _value)
	{
		if (type == ParamType::Text)
		{
			value = _value;
		}
		else if (type == ParamType::Integer)
		{
			value = std::stoi(_value);
		}
		else if (type == ParamType::Real)
		{
			value = std::stof(_value);
		}
		else if (type == ParamType::Bool)
		{
			if (_value == "true")
			{
				value = true;
			}
			else
			{
				value = false;
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
			return std::get<int>(value);
		else
			std::cout << "This is not an Integer Parameter!";
        return -1;
	}

	//Retreive a text value
	std::string RetrieveS()
	{
		if (type == ParamType::Integer)
			return std::get<std::string>(value);
		else
			std::cout << "This is not an Text Parameter!";
        return "";
	}

	//Retreive a bool value
	bool RetrieveB()
	{
		if (type == ParamType::Integer)
			return std::get<bool>(value);
		else
			std::cout << "This is not an Bool Parameter!";
        return false;
	}

	//Retreive a float value
	float RetrieveF()
	{
		if (type == ParamType::Integer)
			return std::get<float>(value);
		else
			std::cout << "This is not an Real Parameter!";
        return -1.0f;
	}
};

class ParamManager
{
	//Container for all parameters
	std::vector<Parameter> params;

	void Store(Parameter p)
	{
		params.push_back(p);
	}

	void Set(int argc, char** argv)
	{
		if (argc <= 1) //Only the executable name so no need to proceed further
			return;

		if (params.size() < 1) //Check if any parameters are stored to check against
		{
			std::cout << "Store a parameter before assigning a value!";
		}
		else
		{
            //parameter assigning
		}
	}

	Parameter Get(std::string abbrev)
	{
		for(Parameter p : params)
		{
			if (p.Abbreviation() == abbrev)
				return p;
		}
	}

	std::string ValueS(std::string name)
	{
		for(Parameter p : params)
		{
			if (p.Name() == name)
				if (p.Type() == ParamType::Text)
				{
					return p.RetrieveS();
				}
				else
                {
                    std::cout << "This is not an Text Parameter!";
                    
                }
		}
	}

	float ValueF(std::string name)
	{
		for(Parameter p : params)
		{
			if (p.Name() == name)
            {
				if (p.Type() == ParamType::Real)
				{
					return p.RetrieveF();
				}
				else
                {
                    std::cout << "This is not an Real Parameter!";
                }
            }
		}
        return -1.0f;
	}

	int ValueI(std::string name)
	{
		for (Parameter p : params)
		{
			if (p.Name() == name)
				if (p.Type() == ParamType::Integer)
				{
					return p.RetrieveI();
				}
				else
                {
                    std::cout << "This is not an Integer Parameter!";
                    
                }
		}
	}

	bool ValueB(std::string name)
	{
		for(Parameter p : params)
		{
			if (p.Name() == name)
				if (p.Type() == ParamType::Bool)
				{
					return p.RetrieveB();
				}
				else
                {
                    std::cout << "This is not an Bool Parameter!";
                    
                }
		}
	}

};
