#include "../Headers/Params.h"

void ParamManager::Store(Parameter p)
{
	params.push_back(p);	
}

int ParamManager::SetParam(std::vector<std::string>* args)
{
	for (int i = 0; i < args->size(); i++)
	{
		if ((*args)[i][0] == '-')
		{
			if ((*args)[i][1] == '-') //Long Arg
			{
				for (Parameter &p : params)
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
						for (Parameter &p : params)
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
					for (Parameter &p : params)
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

void ParamManager::Set(int argc, char* argv[])
{
	if (argc <= 1) //Only the executable name so no need to proceed further
		return;

	if (params.size() < 1) //Check if any parameters are stored to check against
	{
		std::cout << "Store a parameter before assigning a value!";
	}
	else
	{
		if ((argc-1) <= MaxParamCount())
		{
			//parameter assigning from input argc and argv
			std::vector<std::string> args;
			for (int i = 1; i < argc; i++)
			{
				std::string s = argv[i];
				args.push_back(s);
			}

			try
			{
				while (args.size() > 1 )
				{

					SetParam(&args);

				}
			}
			catch (std::exception ex)
			{
				std::cout << "Error Parsing Command Line Arguments: ";
				std::cout << ex.what() << "\n";
			}
			for (Parameter& p : params)
			{
				if (p.Name() == "Path")
				{
					std::string prev = " ";

					for (std::string a : args)
					{
						if (a[0] != '-')
						{

							p.Store(a);
						}
						prev = a;
					}
				}
			}
		}
		else
		{
			std::cout << "Unknown Command Line Parameters Specified";
		}
	}
}

void ParamManager::Output()
{
	for (Parameter p : params)
	{
		switch (p.Type())
		{
		case ParamType::Bool:
		{
			std::cout << p.Name() << "(" << p.Abbreviation() << ") : " << std::boolalpha << p.RetrieveB() << std::noboolalpha << "\n";
			break;
		}
		case ParamType::Integer:
		{
			std::cout << p.Name() << "(" << p.Abbreviation() << ") : " << p.RetrieveI() << "\n";
			break;
		}
		case ParamType::Real:
		{
			std::cout << p.Name() << "(" << p.Abbreviation() << ") : " << p.RetrieveF() << "\n";
			break;
		}
		case ParamType::Text:
		{
			std::cout << p.Name() << "(" << p.Abbreviation() << ") : " << p.RetrieveS() << "\n";
			break;
		}
		}
	}
}

int ParamManager::MaxParamCount()
{
	if (!(params.size() < 1)) //Check if any parameters are stored to check against
	{
		int count = 0;
		for (Parameter p : params)
		{
			if (p.Type() == ParamType::Bool || p.Name() == "Path")
			{
				count++;
			}
			else
			{
				count += 2;
			}
		}
		return count;
	}
	return 0;
}

Parameter ParamManager::GetbyAbbrev(std::string abbrev)
{
	for (Parameter p : params)
	{
		if (p.Abbreviation() == abbrev)
			return p;
	}
}

Parameter ParamManager::GetbyName(std::string name)
{
	for (Parameter p : params)
	{
		if (p.Name() == name)
			return p;
	}
}

std::string ParamManager::ValueS(std::string name)
{
	for (Parameter p : params)
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

float ParamManager::ValueF(std::string name)
{
	for (Parameter p : params)
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

int ParamManager::ValueI(std::string name)
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

bool ParamManager::ValueB(std::string name)
{
	for (Parameter p : params)
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
