/*
 *  ConfigEntry.h
 *  Crag
 *
 *  Created by john on 6/26/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once


//#define ENABLE_CONFIG


#if defined(ENABLE_CONFIG)


class ConfigEntry
{
	friend class ConfigManager;

	ConfigEntry & operator=(ConfigEntry const &);	// undefined
	ConfigEntry(const ConfigEntry &);	// undefined
public:
	ConfigEntry(char const * init_name);
	virtual ~ConfigEntry();
	
	virtual int ValueToString(char * string) const = 0;
	virtual int StringToValue(char const * string) = 0;

private:
	ConfigEntry * next;
	char const * name;
};


template<typename S> class Config : protected ConfigEntry
{
public:
	Config(S & init_var, char const * init_name) 
	: ConfigEntry(init_name)
	, var(init_var)
	{ 
	}
	
	virtual int ValueToString(char * string) const;
	virtual int StringToValue(char const * string);
	
protected:
	
	S & var;
};


template<typename S> class ConfigAngle : protected Config<S>
{
public:
	ConfigAngle(S & init_var, char const * init_name) 
	: Config<S>(init_var, init_name)
	{ 
	}
	
	virtual int ValueToString(char * string) const;
	virtual int StringToValue(char const * string);
};


#define CONFIG_DEFINE(name, type, default) type name = default; Config<type> name##config (name, #name)
#define CONFIG_DEFINE_MEMBER(class, name, type, default) type class::name = default; Config<type> name##config (class::name, #name)

#define CONFIG_DEFINE_ANGLE(name, type, default) type name = DegToRad(default); ConfigAngle<type> name##config (name, #name)
#define CONFIG_DEFINE_ANGLE_MEMBER(class, name, type, default) type class::name = DegToRad(default); ConfigAngle<type> name##config (class::name, #name)

#else

#define CONFIG_DEFINE(name, type, default) type name = default
#define CONFIG_DEFINE_MEMBER(class, name, type, default) type class::name = default

#define CONFIG_DEFINE_ANGLE(name, type, default) type name = DegToRad(default)
#define CONFIG_DEFINE_ANGLE_MEMBER(class, name, type, default) type class::name = DegToRad(default)

#endif

#define CONFIG_DECLARE(name, type) extern type name
#define CONFIG_DECLARE_MEMBER(name, type) static type name

