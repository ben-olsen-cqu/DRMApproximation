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
	//Store a parameter in the manager
	void Store(Parameter p)
	{
		params.push_back(p);
	}

	//Set the values of stored parameters if passed in by the command line or set manually
	void Set(int argc, char* argv[])
	{
		if (argc <= 1) //Only the executable name so no need to proceed further
			return;

		if (params.size() < 1) //Check if any parameters are stored to check against
		{
			std::cout << "Store a parameter before assigning a value!";
		}
		else
		{
            //parameter assigning from input argc and argv
			std::vector<std::string> args;
			for (int i = 1; i < argc; i++)
			{
				//std::cout << argv[i] << "\n";
				std::string s = argv[i];
				args.push_back(s);
			}

			//"\t value:" << args[i + 1] <<
			try
			{
			while (args.size() > 1)
			{
				
					SetParam(&args);
				
			}
			}
			catch (std::exception ex)
			{
				std::cout << "Error Parsing Command Line Arguments: ";
				std::cout << ex.what() << "\n";
			}
			for (Parameter p : params)
			{
				if (p.Name() == "Path")
				{
					p.Store(args[0]);
				}
			}
		}
	}
	
	//Set parameters - function used by Set()
	int SetParam(std::vector<std::string>* args)
	{
		for (int i = 0; i < args->size(); i++)
		{
			if ((*args)[i][0] == '-')
			{
				if ((*args)[i][1] == '-') //Long Arg
				{
					for (Parameter p : params)
					{
						if (("--" + p.Name()) == (*args)[i])
						{
							if (p.Type() != ParamType::Bool)
							{
								if ((*args)[i + 1][0] != '-')
								{
									p.Store((*args)[i + 1]);
									args->erase(args->begin() + i + 1);
									args->erase(args->begin() + i);
									return 1;
								}
								else
								{
									throw std::runtime_error("No Value for Argument: " + (*args)[i]);
								}
							}
							else //Bool param type so set the flag to true
							{
								p.Store("true");
								args->erase(args->begin() + i);
								return 1;
							}
						}
					}
				}
				else 
				{
					if ((*args)[i].size() >= 3) //Multiple short args grouped together
					{
						int loops = (*args)[i].size() + 1;
						for (int j = 1; j < loops; j++)
						{
							for (Parameter p : params)
							{
								if (p.Abbreviation()[0] == (*args)[i][j])
								{
									if (p.Type() == ParamType::Bool)
									{
										p.Store("true");
									}
									else //Bool param type so set the flag to true
									{
										throw std::runtime_error("Grouped parameters cannot have values: " + (*args)[i]);
									}
								}
							}
						}
						args->erase(args->begin() + i);
						return 1;
					}
					else //Single short arg
					{
						for (Parameter p : params)
						{
							if (p.Abbreviation()[0] == (*args)[i][1])
							{
								if (p.Type() != ParamType::Bool)
								{
									if ((*args)[i + 1][0] != '-')
									{
										p.Store((*args)[i + 1]);
										args->erase(args->begin() + i + 1);
										args->erase(args->begin() + i);
										return 1;
									}
									else
									{
										throw std::runtime_error("No Value for Argument: " + (*args)[i]);
									}
								}
								else //Bool param type so set the flag to true
								{
									p.Store("true");
									args->erase(args->begin() + i);
									return 1;
								}
							}
						}

					}
				}
			}
		}
	}

	void Output()
	{
		for (Parameter p : params)
		{
			switch (p.Type())
			{
			case ParamType::Bool:
			{
				std::cout << p.Name() << "(" << p.Abbreviation() << ") - BOOL : " << std::boolalpha << p.RetrieveB() << std::noboolalpha << "\n";
				break;
			}
			case ParamType::Integer:
			{
				std::cout << p.Name() << "(" << p.Abbreviation() << ") - INT : " << p.RetrieveI() << "\n";
				break;
			}
			case ParamType::Real:
			{
				std::cout << p.Name() << "(" << p.Abbreviation() << ") - FLOAT : " << p.RetrieveF() << "\n";
				break;
			}
			case ParamType::Text:
			{
				std::cout << p.Name() << "(" << p.Abbreviation() << ") - TEXT : " << p.RetrieveS() << "\n";
				break;
			}
			}
		}
	}

	//Get a parameter by searching by name
	Parameter GetbyAbbrev(std::string abbrev)
	{
		for(Parameter p : params)
		{
			if (p.Abbreviation() == abbrev)
				return p;
		}
	}
	
	//Get a parameter by searching by abbreviation
	Parameter GetbyName(std::string name)
	{
		for (Parameter p : params)
		{
			if (p.Name() == name)
				return p;
		}
	}
	
	//Get a string value from a parameter by searching for the parameter by name
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
	
	//Get a float value from a parameter by searching for the parameter by name
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
	
	//Get a integer value from a parameter by searching for the parameter by name
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
	
	//Get a bool value from a parameter by searching for the parameter by name
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
