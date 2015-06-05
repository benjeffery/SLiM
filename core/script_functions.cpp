//
//  script_functions.cpp
//  SLiM
//
//  Created by Ben Haller on 4/6/15.
//  Copyright (c) 2015 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
//

//	This file is part of SLiM.
//
//	SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with SLiM.  If not, see <http://www.gnu.org/licenses/>.


#include "script_functions.h"
#include "script_functionsignature.h"
#include "script_pathelement.h"
#include "script_interpreter.h"
#include "g_rng.h"

#include "math.h"

#include <ctime>


using std::string;
using std::vector;
using std::map;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::istream;
using std::ostream;


ScriptValue *Execute_rep(string p_function_name, vector<ScriptValue*> p_arguments);
ScriptValue *Execute_repEach(string p_function_name, vector<ScriptValue*> p_arguments);
ScriptValue *Execute_seq(string p_function_name, vector<ScriptValue*> p_arguments);


//
//	Construct our built-in function map
//

// We allocate all of our function signatures once and keep them forever, for faster ScriptInterpreter startup
vector<const FunctionSignature *> &ScriptInterpreter::BuiltInFunctions(void)
{
	static vector<const FunctionSignature *> *signatures = nullptr;
	
	if (!signatures)
	{
		signatures = new vector<const FunctionSignature *>;
		
		// ************************************************************************************
		//
		//	math functions
		//
		
		signatures->push_back((new FunctionSignature("abs",			FunctionIdentifier::absFunction,		kScriptValueMaskNumeric))->AddNumeric());
		signatures->push_back((new FunctionSignature("acos",		FunctionIdentifier::acosFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("asin",		FunctionIdentifier::asinFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("atan",		FunctionIdentifier::atanFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("atan2",		FunctionIdentifier::atan2Function,		kScriptValueMaskFloat))->AddNumeric()->AddNumeric());
		signatures->push_back((new FunctionSignature("ceil",		FunctionIdentifier::ceilFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("cos",			FunctionIdentifier::cosFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("exp",			FunctionIdentifier::expFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("floor",		FunctionIdentifier::floorFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("isFinite",	FunctionIdentifier::isFiniteFunction,	kScriptValueMaskLogical))->AddFloat());
		signatures->push_back((new FunctionSignature("isInfinite",	FunctionIdentifier::isInfiniteFunction,	kScriptValueMaskLogical))->AddFloat());
		signatures->push_back((new FunctionSignature("isNAN",		FunctionIdentifier::isNaNFunction,		kScriptValueMaskLogical))->AddFloat());
		signatures->push_back((new FunctionSignature("log",			FunctionIdentifier::logFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("log10",		FunctionIdentifier::log10Function,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("log2",		FunctionIdentifier::log2Function,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("product",		FunctionIdentifier::productFunction,	kScriptValueMaskNumeric | kScriptValueMaskSingleton))->AddNumeric());
		signatures->push_back((new FunctionSignature("round",		FunctionIdentifier::roundFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("sin",			FunctionIdentifier::sinFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("sqrt",		FunctionIdentifier::sqrtFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("sum",			FunctionIdentifier::sumFunction,		kScriptValueMaskNumeric | kScriptValueMaskSingleton))->AddNumeric());
		signatures->push_back((new FunctionSignature("tan",			FunctionIdentifier::tanFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("trunc",		FunctionIdentifier::truncFunction,		kScriptValueMaskFloat))->AddNumeric());
		
		
		// ************************************************************************************
		//
		//	summary statistics functions
		//
		
		signatures->push_back((new FunctionSignature("max",			FunctionIdentifier::maxFunction,		kScriptValueMaskAnyBase | kScriptValueMaskSingleton))->AddAnyBase());
		signatures->push_back((new FunctionSignature("mean",		FunctionIdentifier::meanFunction,		kScriptValueMaskFloat))->AddNumeric());
		signatures->push_back((new FunctionSignature("min",			FunctionIdentifier::minFunction,		kScriptValueMaskAnyBase | kScriptValueMaskSingleton))->AddAnyBase());
		signatures->push_back((new FunctionSignature("range",		FunctionIdentifier::rangeFunction,		kScriptValueMaskNumeric))->AddNumeric());
		signatures->push_back((new FunctionSignature("sd",			FunctionIdentifier::sdFunction,			kScriptValueMaskFloat | kScriptValueMaskSingleton))->AddNumeric());
		
		
		// ************************************************************************************
		//
		//	vector construction functions
		//
		
		signatures->push_back((new FunctionSignature("c",			FunctionIdentifier::cFunction,			kScriptValueMaskAny))->AddEllipsis());
		signatures->push_back((new FunctionSignature("float",		FunctionIdentifier::floatFunction,		kScriptValueMaskFloat))->AddInt_S());
		signatures->push_back((new FunctionSignature("integer",		FunctionIdentifier::integerFunction,	kScriptValueMaskInt))->AddInt_S());
		signatures->push_back((new FunctionSignature("logical",		FunctionIdentifier::logicalFunction,	kScriptValueMaskLogical))->AddInt_S());
		signatures->push_back((new FunctionSignature("object",		FunctionIdentifier::objectFunction,		kScriptValueMaskObject)));
		signatures->push_back((new FunctionSignature("rbinom",		FunctionIdentifier::rbinomFunction,		kScriptValueMaskInt))->AddInt_S()->AddInt()->AddFloat());
		signatures->push_back((new FunctionSignature("rep",			FunctionIdentifier::repFunction,		kScriptValueMaskAny))->AddAny()->AddInt_S());
		signatures->push_back((new FunctionSignature("repEach",		FunctionIdentifier::repEachFunction,	kScriptValueMaskAny))->AddAny()->AddInt());
		signatures->push_back((new FunctionSignature("rexp",		FunctionIdentifier::rexpFunction,		kScriptValueMaskFloat))->AddInt_S()->AddNumeric_O());
		signatures->push_back((new FunctionSignature("rnorm",		FunctionIdentifier::rnormFunction,		kScriptValueMaskFloat))->AddInt_S()->AddNumeric_O()->AddNumeric_O());
		signatures->push_back((new FunctionSignature("rpois",		FunctionIdentifier::rpoisFunction,		kScriptValueMaskInt))->AddInt_S()->AddNumeric());
		signatures->push_back((new FunctionSignature("runif",		FunctionIdentifier::runifFunction,		kScriptValueMaskFloat))->AddInt_S()->AddNumeric_O()->AddNumeric_O());
		signatures->push_back((new FunctionSignature("sample",		FunctionIdentifier::sampleFunction,		kScriptValueMaskAny))->AddAny()->AddInt()->AddLogical_OS()->AddNumeric_O());
		signatures->push_back((new FunctionSignature("seq",			FunctionIdentifier::seqFunction,		kScriptValueMaskNumeric))->AddNumeric_S()->AddNumeric_S()->AddNumeric_OS());
		signatures->push_back((new FunctionSignature("seqAlong",	FunctionIdentifier::seqAlongFunction,	kScriptValueMaskInt))->AddAny());
		signatures->push_back((new FunctionSignature("string",		FunctionIdentifier::stringFunction,		kScriptValueMaskString))->AddInt_S());
		
		
		// ************************************************************************************
		//
		//	value inspection/manipulation functions
		//
		
		signatures->push_back((new FunctionSignature("all",			FunctionIdentifier::allFunction,		kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddLogical());
		signatures->push_back((new FunctionSignature("any",			FunctionIdentifier::anyFunction,		kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddLogical());
		signatures->push_back((new FunctionSignature("cat",			FunctionIdentifier::catFunction,		kScriptValueMaskNULL))->AddAny()->AddString_OS());
		signatures->push_back((new FunctionSignature("ifelse",		FunctionIdentifier::ifelseFunction,		kScriptValueMaskAny))->AddLogical()->AddAny()->AddAny());
		signatures->push_back((new FunctionSignature("nchar",		FunctionIdentifier::ncharFunction,		kScriptValueMaskInt))->AddString());
		signatures->push_back((new FunctionSignature("paste",		FunctionIdentifier::pasteFunction,		kScriptValueMaskString | kScriptValueMaskSingleton))->AddAny()->AddString_OS());
		signatures->push_back((new FunctionSignature("print",		FunctionIdentifier::printFunction,		kScriptValueMaskNULL))->AddAny());
		signatures->push_back((new FunctionSignature("rev",			FunctionIdentifier::revFunction,		kScriptValueMaskAny))->AddAny());
		signatures->push_back((new FunctionSignature("size",		FunctionIdentifier::sizeFunction,		kScriptValueMaskInt | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("sort",		FunctionIdentifier::sortFunction,		kScriptValueMaskAnyBase))->AddAnyBase()->AddLogical_OS());
		signatures->push_back((new FunctionSignature("sortBy",		FunctionIdentifier::sortByFunction,		kScriptValueMaskObject))->AddObject()->AddString_S()->AddLogical_OS());
		signatures->push_back((new FunctionSignature("str",			FunctionIdentifier::strFunction,		kScriptValueMaskNULL))->AddAny());
		signatures->push_back((new FunctionSignature("strsplit",	FunctionIdentifier::strsplitFunction,	kScriptValueMaskString))->AddString_S()->AddString_OS());
		signatures->push_back((new FunctionSignature("substr",		FunctionIdentifier::substrFunction,		kScriptValueMaskString))->AddString()->AddInt()->AddInt_O());
		signatures->push_back((new FunctionSignature("unique",		FunctionIdentifier::uniqueFunction,		kScriptValueMaskAny))->AddAny());
		signatures->push_back((new FunctionSignature("which",		FunctionIdentifier::whichFunction,		kScriptValueMaskInt))->AddLogical());
		signatures->push_back((new FunctionSignature("whichMax",	FunctionIdentifier::whichMaxFunction,	kScriptValueMaskInt))->AddAnyBase());
		signatures->push_back((new FunctionSignature("whichMin",	FunctionIdentifier::whichMinFunction,	kScriptValueMaskInt))->AddAnyBase());
		
		
		// ************************************************************************************
		//
		//	value type testing/coercion functions
		//
		
		signatures->push_back((new FunctionSignature("asFloat",     FunctionIdentifier::asFloatFunction,	kScriptValueMaskFloat))->AddAny());
		signatures->push_back((new FunctionSignature("asInteger",	FunctionIdentifier::asIntegerFunction,	kScriptValueMaskInt))->AddAny());
		signatures->push_back((new FunctionSignature("asLogical",	FunctionIdentifier::asLogicalFunction,	kScriptValueMaskLogical))->AddAny());
		signatures->push_back((new FunctionSignature("asString",	FunctionIdentifier::asStringFunction,	kScriptValueMaskString))->AddAny());
		signatures->push_back((new FunctionSignature("element",		FunctionIdentifier::elementFunction,	kScriptValueMaskString | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isFloat",		FunctionIdentifier::isFloatFunction,	kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isInteger",	FunctionIdentifier::isIntegerFunction,	kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isLogical",	FunctionIdentifier::isLogicalFunction,	kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isNULL",		FunctionIdentifier::isNULLFunction,		kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isObject",	FunctionIdentifier::isObjectFunction,	kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("isString",	FunctionIdentifier::isStringFunction,	kScriptValueMaskLogical | kScriptValueMaskSingleton))->AddAny());
		signatures->push_back((new FunctionSignature("type",		FunctionIdentifier::typeFunction,		kScriptValueMaskString | kScriptValueMaskSingleton))->AddAny());
		
		
		// ************************************************************************************
		//
		//	bookkeeping functions
		//
		
		signatures->push_back((new FunctionSignature("date",		FunctionIdentifier::dateFunction,		kScriptValueMaskString | kScriptValueMaskSingleton)));
		signatures->push_back((new FunctionSignature("function",	FunctionIdentifier::functionFunction,	kScriptValueMaskNULL))->AddString_OS());
		signatures->push_back((new FunctionSignature("globals",		FunctionIdentifier::globalsFunction,	kScriptValueMaskNULL)));
		signatures->push_back((new FunctionSignature("help",		FunctionIdentifier::helpFunction,		kScriptValueMaskNULL))->AddString_OS());
		signatures->push_back((new FunctionSignature("license",		FunctionIdentifier::licenseFunction,	kScriptValueMaskNULL)));
		signatures->push_back((new FunctionSignature("rm",			FunctionIdentifier::rmFunction,			kScriptValueMaskNULL))->AddString_O());
		signatures->push_back((new FunctionSignature("setSeed",		FunctionIdentifier::setSeedFunction,	kScriptValueMaskNULL))->AddInt_S());
		signatures->push_back((new FunctionSignature("stop",		FunctionIdentifier::stopFunction,		kScriptValueMaskNULL))->AddString_OS());
		signatures->push_back((new FunctionSignature("time",		FunctionIdentifier::timeFunction,		kScriptValueMaskString | kScriptValueMaskSingleton)));
		signatures->push_back((new FunctionSignature("version",		FunctionIdentifier::versionFunction,	kScriptValueMaskString | kScriptValueMaskSingleton)));
		
		
		// ************************************************************************************
		//
		//	object instantiation
		//
		
		signatures->push_back((new FunctionSignature("Path",		FunctionIdentifier::PathFunction,		kScriptValueMaskObject | kScriptValueMaskSingleton))->AddString_OS());
		
		
		// alphabetize, mostly to be nice to the auto-completion feature
		std::sort(signatures->begin(), signatures->end(), CompareFunctionSignatures);
	}
	
	return *signatures;
}

void ScriptInterpreter::RegisterSignature(const FunctionSignature *p_signature)
{
	function_map_.insert(FunctionMapPair(p_signature->function_name_, p_signature));
}

void ScriptInterpreter::RegisterBuiltInFunctions(void)
{
	vector<const FunctionSignature *> &built_in_functions = ScriptInterpreter::BuiltInFunctions();
	
	for (auto sig : built_in_functions)
		function_map_.insert(FunctionMapPair(sig->function_name_, sig));
}


//
//	Executing function calls
//

ScriptValue *ConcatenateScriptValues(string p_function_name, vector<ScriptValue*> p_arguments)
{
#pragma unused(p_function_name)
	ScriptValueType highest_type = ScriptValueType::kValueNULL;
	bool has_object_type = false, has_nonobject_type = false, all_invisible = true;
	string element_type;
	
	// First figure out our return type, which is the highest-promotion type among all our arguments
	for (ScriptValue *arg_value : p_arguments)
	{
		ScriptValueType arg_type = arg_value->Type();
		
		if (arg_type > highest_type)
			highest_type = arg_type;
		
		if (!arg_value->Invisible())
			all_invisible = false;
		
		if (arg_type == ScriptValueType::kValueObject)
		{
			if (arg_value->Count() > 0)		// object(0) parameters do not conflict with other object types
			{
				string this_element_type = static_cast<ScriptValue_Object *>(arg_value)->ElementType();
				
				if (element_type.length() == 0)
				{
					// we haven't seen a (non-empty) object type yet, so remember what type we're dealing with
					element_type = this_element_type;
				}
				else
				{
					// we've already seen a object type, so check that this one is the same type
					if (element_type.compare(this_element_type) != 0)
						SLIM_TERMINATION << "ERROR (" << p_function_name << "): objects of different types cannot be mixed." << endl << slim_terminate();
				}
			}
			
			has_object_type = true;
		}
		else
			has_nonobject_type = true;
	}
	
	if (has_object_type && has_nonobject_type)
		SLIM_TERMINATION << "ERROR (" << p_function_name << "): object and non-object types cannot be mixed." << endl << slim_terminate();
	
	// If we've got nothing but NULL, then return NULL; preserve invisibility
	if (highest_type == ScriptValueType::kValueNULL)
		return (all_invisible ? ScriptValue_NULL::ScriptValue_NULL_Invisible() : new ScriptValue_NULL());
	
	// Create an object of the right return type, concatenate all the arguments together, and return it
	if (highest_type == ScriptValueType::kValueLogical)
	{
		ScriptValue_Logical *result = new ScriptValue_Logical();
		
		for (ScriptValue *arg_value : p_arguments)
			if (arg_value->Type() != ScriptValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushLogical(arg_value->LogicalAtIndex(value_index));
		
		return result;
	}
	else if (highest_type == ScriptValueType::kValueInt)
	{
		ScriptValue_Int *result = new ScriptValue_Int();
		
		for (ScriptValue *arg_value : p_arguments)
			if (arg_value->Type() != ScriptValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushInt(arg_value->IntAtIndex(value_index));
		
		return result;
	}
	else if (highest_type == ScriptValueType::kValueFloat)
	{
		ScriptValue_Float *result = new ScriptValue_Float();
		
		for (ScriptValue *arg_value : p_arguments)
			if (arg_value->Type() != ScriptValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushFloat(arg_value->FloatAtIndex(value_index));
		
		return result;
	}
	else if (highest_type == ScriptValueType::kValueString)
	{
		ScriptValue_String *result = new ScriptValue_String();
		
		for (ScriptValue *arg_value : p_arguments)
			if (arg_value->Type() != ScriptValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushString(arg_value->StringAtIndex(value_index));
		
		return result;
	}
	else if (has_object_type)
	{
		ScriptValue_Object *result = new ScriptValue_Object();
		
		for (ScriptValue *arg_value : p_arguments)
			for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
				result->PushElement(arg_value->ElementAtIndex(value_index));
		
		return result;
	}
	else
	{
		SLIM_TERMINATION << "ERROR (" << p_function_name << "): type '" << highest_type << "' is not supported by ConcatenateScriptValues()." << endl << slim_terminate();
	}
	
	return nullptr;
}

ScriptValue *Execute_rep(string p_function_name, vector<ScriptValue*> p_arguments)
{
#pragma unused(p_function_name)
	ScriptValue *arg1_value = p_arguments[0];
	int arg1_count = arg1_value->Count();
	ScriptValue *arg2_value = p_arguments[1];
	int arg2_count = arg2_value->Count();
	
	// the return type depends on the type of the first argument, which will get replicated
	ScriptValue *result = arg1_value->NewMatchingType();
	
	if (arg2_count == 1)
	{
		int64_t rep_count = arg2_value->IntAtIndex(0);
		
		for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
			for (int value_idx = 0; value_idx < arg1_count; value_idx++)
				result->PushValueFromIndexOfScriptValue(value_idx, arg1_value);
	}
	
	return result;
}

ScriptValue *Execute_repEach(string p_function_name, vector<ScriptValue*> p_arguments)
{
	ScriptValue *arg1_value = p_arguments[0];
	int arg1_count = arg1_value->Count();
	ScriptValue *arg2_value = p_arguments[1];
	int arg2_count = arg2_value->Count();
	
	// the return type depends on the type of the first argument, which will get replicated
	ScriptValue *result = arg1_value->NewMatchingType();
	
	if (arg2_count == 1)
	{
		int64_t rep_count = arg2_value->IntAtIndex(0);
		
		for (int value_idx = 0; value_idx < arg1_count; value_idx++)
			for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
				result->PushValueFromIndexOfScriptValue(value_idx, arg1_value);
	}
	else if (arg2_count == arg1_count)
	{
		for (int value_idx = 0; value_idx < arg1_count; value_idx++)
		{
			int64_t rep_count = arg2_value->IntAtIndex(value_idx);
			
			for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
				result->PushValueFromIndexOfScriptValue(value_idx, arg1_value);
		}
	}
	else
	{
		SLIM_TERMINATION << "ERROR (Execute_repEach): function " << p_function_name << "() requires that its second argument's size() either (1) be equal to 1, or (2) be equal to the size() of its first argument." << endl << slim_terminate();
	}
	
	return result;
}

ScriptValue *Execute_seq(string p_function_name, vector<ScriptValue*> p_arguments)
{
	ScriptValue *result = nullptr;
	ScriptValue *arg1_value = p_arguments[0];
	ScriptValueType arg1_type = arg1_value->Type();
	ScriptValue *arg2_value = p_arguments[1];
	ScriptValueType arg2_type = arg2_value->Type();
	ScriptValue *arg3_value = ((p_arguments.size() == 3) ? p_arguments[2] : nullptr);
	ScriptValueType arg3_type = (arg3_value ? arg3_value->Type() : ScriptValueType::kValueInt);
	
	if ((arg1_type == ScriptValueType::kValueFloat) || (arg2_type == ScriptValueType::kValueFloat) || (arg3_type == ScriptValueType::kValueFloat))
	{
		// float return case
		ScriptValue_Float *float_result = new ScriptValue_Float();
		result = float_result;
		
		double first_value = arg1_value->FloatAtIndex(0);
		double second_value = arg2_value->FloatAtIndex(0);
		double default_by = ((first_value < second_value) ? 1 : -1);
		double by_value = (arg3_value ? arg3_value->FloatAtIndex(0) : default_by);
		
		if (by_value == 0.0)
			SLIM_TERMINATION << "ERROR (Execute_seq): function " << p_function_name << " requires a by argument != 0." << endl << slim_terminate();
		if (((first_value < second_value) && (by_value < 0)) || ((first_value > second_value) && (by_value > 0)))
			SLIM_TERMINATION << "ERROR (Execute_seq): function " << p_function_name << " by argument has incorrect sign." << endl << slim_terminate();
		
		if (by_value > 0)
			for (double seq_value = first_value; seq_value <= second_value; seq_value += by_value)
				float_result->PushFloat(seq_value);
		else
			for (double seq_value = first_value; seq_value >= second_value; seq_value += by_value)
				float_result->PushFloat(seq_value);
	}
	else
	{
		// int return case
		ScriptValue_Int *int_result = new ScriptValue_Int();
		result = int_result;
		
		int64_t first_value = arg1_value->IntAtIndex(0);
		int64_t second_value = arg2_value->IntAtIndex(0);
		int64_t default_by = ((first_value < second_value) ? 1 : -1);
		int64_t by_value = (arg3_value ? arg3_value->IntAtIndex(0) : default_by);
		
		if (by_value == 0)
			SLIM_TERMINATION << "ERROR (Execute_seq): function " << p_function_name << " requires a by argument != 0." << endl << slim_terminate();
		if (((first_value < second_value) && (by_value < 0)) || ((first_value > second_value) && (by_value > 0)))
			SLIM_TERMINATION << "ERROR (Execute_seq): function " << p_function_name << " by argument has incorrect sign." << endl << slim_terminate();
		
		if (by_value > 0)
			for (int64_t seq_value = first_value; seq_value <= second_value; seq_value += by_value)
				int_result->PushInt(seq_value);
		else
			for (int64_t seq_value = first_value; seq_value >= second_value; seq_value += by_value)
				int_result->PushInt(seq_value);
	}
	
	return result;
}

ScriptValue *ScriptInterpreter::ExecuteFunctionCall(string const &p_function_name, vector<ScriptValue*> const &p_arguments, ostream &p_output_stream)
{
	ScriptValue *result = nullptr;
	
	// Get the function signature and check our arguments against it
	auto signature_iter = function_map_.find(p_function_name);
	
	if (signature_iter == function_map_.end())
		SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): unrecognized function name " << p_function_name << "." << endl << slim_terminate();
	
	const FunctionSignature *signature = signature_iter->second;
	bool class_method = signature->is_class_method;
	bool instance_method = signature->is_instance_method;
	
	if (class_method || instance_method)
		SLIM_TERMINATION << "ERROR (ScriptInterpreter::ExecuteFunctionCall): internal error: " << p_function_name << " is designated as a class method or instance method." << endl << slim_terminate();
	
	signature->CheckArguments("function", p_arguments);
	
	// We predefine variables for the return types, and preallocate them here if possible.  This is for code brevity below.
	ScriptValue_NULL *null_result = nullptr;
	ScriptValue_Logical *logical_result = nullptr;
	ScriptValue_Float *float_result = nullptr;
	ScriptValue_Int *int_result = nullptr;
	ScriptValue_String *string_result = nullptr;
	ScriptValueMask return_type_mask = signature->return_mask_ & kScriptValueMaskFlagStrip;
	
	if (return_type_mask == kScriptValueMaskNULL)
	{
		null_result = ScriptValue_NULL::ScriptValue_NULL_Invisible();	// assumed that invisible is correct when the return type is NULL
		result = null_result;
	}
	else if (return_type_mask == kScriptValueMaskLogical)
	{
		logical_result = new ScriptValue_Logical();
		result = logical_result;
	}
	else if (return_type_mask == kScriptValueMaskFloat)
	{
		float_result = new ScriptValue_Float();
		result = float_result;
	}
	else if (return_type_mask == kScriptValueMaskInt)
	{
		int_result = new ScriptValue_Int();
		result = int_result;
	}
	else if (return_type_mask == kScriptValueMaskString)
	{
		string_result = new ScriptValue_String();
		result = string_result;
	}
	// else the return type is not predictable and thus cannot be set up beforehand; the function implementation will have to do it
	
	// Prefetch arguments to allow greater brevity in the code below
	int n_args = (int)p_arguments.size();
	ScriptValue *arg1_value = (n_args >= 1) ? p_arguments[0] : nullptr;
	ScriptValueType arg1_type = (n_args >= 1) ? arg1_value->Type() : ScriptValueType::kValueNULL;
	int arg1_count = (n_args >= 1) ? arg1_value->Count() : 0;
	
	/*
	ScriptValue *arg2_value = (n_args >= 2) ? p_arguments[1] : nullptr;
	ScriptValueType arg2_type = (n_args >= 2) ? arg2_value->Type() : ScriptValueType::kValueNULL;
	int arg2_count = (n_args >= 2) ? arg2_value->Count() : 0;
	
	ScriptValue *arg3_value = (n_args >= 3) ? p_arguments[2] : nullptr;
	ScriptValueType arg3_type = (n_args >= 3) ? arg3_value->Type() : ScriptValueType::kValueNULL;
	int arg3_count = (n_args >= 3) ? arg3_value->Count() : 0;
	*/
	
	// Now we look up the function again and actually execute it
	switch (signature->function_id_)
	{
		case FunctionIdentifier::kNoFunction:
			SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): internal logic error." << endl << slim_terminate();
			break;
			
		case FunctionIdentifier::kDelegatedFunction:
			result = signature->delegate_function_(signature->delegate_object_, p_function_name, p_arguments, p_output_stream, *this);
			break;
			
			
		// ************************************************************************************
		//
		//	math functions
		//
#pragma mark *** Math functions
			
#pragma mark abs
		case FunctionIdentifier::absFunction:
			if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					int_result->PushInt(llabs(arg1_value->IntAtIndex(value_index)));
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					float_result->PushFloat(fabs(arg1_value->FloatAtIndex(value_index)));
			}
			break;
			
#pragma mark acos
		case FunctionIdentifier::acosFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(acos(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark asin
		case FunctionIdentifier::asinFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(asin(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark atan
		case FunctionIdentifier::atanFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(atan(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark atan2
		case FunctionIdentifier::atan2Function:
		{
			ScriptValue *arg2_value = p_arguments[1];
			int arg2_count = arg2_value->Count();
			
			if (arg1_count != arg2_count)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function atan2() requires arguments of equal length." << endl << slim_terminate();
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(atan2(arg1_value->FloatAtIndex(value_index), arg2_value->FloatAtIndex(value_index)));
			break;
		}
			
#pragma mark ceil
		case FunctionIdentifier::ceilFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(ceil(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark cos
		case FunctionIdentifier::cosFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(cos(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark exp
		case FunctionIdentifier::expFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(exp(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark floor
		case FunctionIdentifier::floorFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(floor(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark isFinite
		case FunctionIdentifier::isFiniteFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				logical_result->PushLogical(isfinite(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark isInfinite
		case FunctionIdentifier::isInfiniteFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				logical_result->PushLogical(isinf(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark isNAN
		case FunctionIdentifier::isNaNFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				logical_result->PushLogical(isnan(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark log
		case FunctionIdentifier::logFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(log(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark log10
		case FunctionIdentifier::log10Function:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(log10(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark log2
		case FunctionIdentifier::log2Function:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(log2(arg1_value->FloatAtIndex(value_index)));
			break;

#pragma mark product
		case FunctionIdentifier::productFunction:
			if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				int64_t product = 1;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					int64_t old_product = product;
					int64_t temp = arg1_value->IntAtIndex(value_index);
					
					product *= arg1_value->IntAtIndex(value_index);
					
					// raise on overflow; test after doing the multiplication
					if (product / temp != old_product)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): overflow in product() with integer argument; use asFloat() to convert the argument." << endl << slim_terminate();
				}
				
				int_result->PushInt(product);
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				double product = 1;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					product *= arg1_value->FloatAtIndex(value_index);
				
				float_result->PushFloat(product);
			}
			break;
			
#pragma mark sum
		case FunctionIdentifier::sumFunction:
			if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				int64_t sum = 0;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					int64_t temp = arg1_value->IntAtIndex(value_index);
					
					// raise on overflow; test prior to doing the addition
					if (((temp > 0) && (sum > INT64_MAX - temp)) || ((temp < 0) && (sum < INT64_MIN - temp)))
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): overflow in sum() with integer argument; use asFloat() to convert the argument." << endl << slim_terminate();
					
					sum += arg1_value->IntAtIndex(value_index);
				}
				
				int_result->PushInt(sum);
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				double sum = 0;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					sum += arg1_value->FloatAtIndex(value_index);
				
				float_result->PushFloat(sum);
			}
			break;

#pragma mark round
		case FunctionIdentifier::roundFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(round(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark sin
		case FunctionIdentifier::sinFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(sin(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark sqrt
		case FunctionIdentifier::sqrtFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(sqrt(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark tan
		case FunctionIdentifier::tanFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(tan(arg1_value->FloatAtIndex(value_index)));
			break;
			
#pragma mark trunc
		case FunctionIdentifier::truncFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				float_result->PushFloat(trunc(arg1_value->FloatAtIndex(value_index)));
			break;
			
			
		// ************************************************************************************
		//
		//	summary statistics functions
		//
#pragma mark *** Summary statistics functions
			
#pragma mark max
		case FunctionIdentifier::maxFunction:
			if (arg1_count == 0)
			{
				result = new ScriptValue_NULL();
			}
			else if (arg1_type == ScriptValueType::kValueLogical)
			{
				logical_result = new ScriptValue_Logical();
				result = logical_result;
				
				bool max = arg1_value->LogicalAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					bool temp = arg1_value->LogicalAtIndex(value_index);
					if (max < temp)
						max = temp;
				}
				logical_result->PushLogical(max);
			}
			else if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				int64_t max = arg1_value->IntAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					int64_t temp = arg1_value->IntAtIndex(value_index);
					if (max < temp)
						max = temp;
				}
				int_result->PushInt(max);
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				double max = arg1_value->FloatAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					double temp = arg1_value->FloatAtIndex(value_index);
					if (max < temp)
						max = temp;
				}
				float_result->PushFloat(max);
			}
			else if (arg1_type == ScriptValueType::kValueString)
			{
				string_result = new ScriptValue_String();
				result = string_result;
				
				string max = arg1_value->StringAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					string temp = arg1_value->StringAtIndex(value_index);
					if (max < temp)
						max = temp;
				}
				string_result->PushString(max);
			}
			break;
			
#pragma mark mean
		case FunctionIdentifier::meanFunction:
		{
			double sum = 0;
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				sum += arg1_value->FloatAtIndex(value_index);
			float_result->PushFloat(sum / arg1_count);
			break;
		}
			
#pragma mark min
		case FunctionIdentifier::minFunction:
			if (arg1_count == 0)
			{
				result = new ScriptValue_NULL();
			}
			else if (arg1_type == ScriptValueType::kValueLogical)
			{
				logical_result = new ScriptValue_Logical();
				result = logical_result;
				
				bool min = arg1_value->LogicalAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					bool temp = arg1_value->LogicalAtIndex(value_index);
					if (min > temp)
						min = temp;
				}
				logical_result->PushLogical(min);
			}
			else if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				int64_t min = arg1_value->IntAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					int64_t temp = arg1_value->IntAtIndex(value_index);
					if (min > temp)
						min = temp;
				}
				int_result->PushInt(min);
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				double min = arg1_value->FloatAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					double temp = arg1_value->FloatAtIndex(value_index);
					if (min > temp)
						min = temp;
				}
				float_result->PushFloat(min);
			}
			else if (arg1_type == ScriptValueType::kValueString)
			{
				string_result = new ScriptValue_String();
				result = string_result;
				
				string min = arg1_value->StringAtIndex(0);
				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					string temp = arg1_value->StringAtIndex(value_index);
					if (min > temp)
						min = temp;
				}
				string_result->PushString(min);
			}
			break;
			
#pragma mark range
		case FunctionIdentifier::rangeFunction:
			if (arg1_count == 0)
			{
				result = new ScriptValue_NULL();
			}
			else if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				int64_t max = arg1_value->IntAtIndex(0);
				int64_t min = max;

				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					int64_t temp = arg1_value->IntAtIndex(value_index);
					if (max < temp)
						max = temp;
					else if (min > temp)
						min = temp;
				}
				int_result->PushInt(min);
				int_result->PushInt(max);
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				double max = arg1_value->FloatAtIndex(0);
				double min = max;

				for (int value_index = 1; value_index < arg1_count; ++value_index)
				{
					double temp = arg1_value->FloatAtIndex(value_index);
					if (max < temp)
						max = temp;
					else if (min > temp)
						min = temp;
				}
				float_result->PushFloat(min);
				float_result->PushFloat(max);
			}
			break;
			
#pragma mark sd
		case FunctionIdentifier::sdFunction:
		{
			if (arg1_count > 1)
			{
				double mean = 0;
				double sd = 0;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					mean += arg1_value->FloatAtIndex(value_index);
				
				mean /= arg1_count;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					double temp = (arg1_value->FloatAtIndex(value_index) - mean);
					sd += temp * temp;
				}
				
				sd = sqrt(sd / (arg1_count - 1));
				float_result->PushFloat(sd);
			}
			else
			{
				result = new ScriptValue_NULL();
			}
			break;
		}
			
		// ************************************************************************************
		//
		//	vector construction functions
		//
#pragma mark *** Vector conversion functions
			
#pragma mark c
		case FunctionIdentifier::cFunction:
			result = ConcatenateScriptValues(p_function_name, p_arguments);
			break;
			
#pragma mark float
		case FunctionIdentifier::floatFunction:
			for (int64_t value_index = arg1_value->IntAtIndex(0); value_index > 0; --value_index)
				float_result->PushFloat(0.0);
			break;
			
#pragma mark integer
		case FunctionIdentifier::integerFunction:
			for (int64_t value_index = arg1_value->IntAtIndex(0); value_index > 0; --value_index)
				int_result->PushInt(0);
			break;
			
#pragma mark logical
		case FunctionIdentifier::logicalFunction:
			for (int64_t value_index = arg1_value->IntAtIndex(0); value_index > 0; --value_index)
				logical_result->PushLogical(false);
			break;
			
#pragma mark object
		case FunctionIdentifier::objectFunction:
			result = new ScriptValue_Object();
			break;
			
#pragma mark rbinom
		case FunctionIdentifier::rbinomFunction:
		{
			int64_t num_draws = arg1_value->IntAtIndex(0);
			ScriptValue *arg_size = p_arguments[1];
			ScriptValue *arg_prob = p_arguments[2];
			int arg_size_count = arg_size->Count();
			int arg_prob_count = arg_prob->Count();
			bool size_singleton = (arg_size_count == 1);
			bool prob_singleton = (arg_prob_count == 1);
			
			if (num_draws < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires n to be greater than or equal to 0." << endl << slim_terminate();
			if (!size_singleton && (arg_size_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires size to be of length 1 or n." << endl << slim_terminate();
			if (!prob_singleton && (arg_prob_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires prob to be of length 1 or n." << endl << slim_terminate();
			
			int size0 = (int)arg_size->IntAtIndex(0);
			double probability0 = arg_prob->FloatAtIndex(0);
			
			if (size_singleton && prob_singleton)
			{
				if (size0 < 0)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires size >= 0." << endl << slim_terminate();
				if ((probability0 < 0.0) || (probability0 > 1.0))
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires probability in [0.0, 1.0]." << endl << slim_terminate();
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					int_result->PushInt(gsl_ran_binomial(g_rng, probability0, size0));
			}
			else
			{
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					int size = (size_singleton ? size0 : (int)arg_size->IntAtIndex(draw_index));
					double probability = (prob_singleton ? probability0 : arg_prob->FloatAtIndex(draw_index));
					
					if (size < 0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires size >= 0." << endl << slim_terminate();
					if ((probability < 0.0) || (probability > 1.0))
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rbinom() requires probability in [0.0, 1.0]." << endl << slim_terminate();
					
					int_result->PushInt(gsl_ran_binomial(g_rng, probability, size));
				}
			}
			
			break;
		}
			
#pragma mark rep
		case FunctionIdentifier::repFunction:
			result = Execute_rep(p_function_name, p_arguments);
			break;
			
#pragma mark repEach
		case FunctionIdentifier::repEachFunction:
			result = Execute_repEach(p_function_name, p_arguments);
			break;
			
#pragma mark rexp
		case FunctionIdentifier::rexpFunction:
		{
			int64_t num_draws = arg1_value->IntAtIndex(0);
			ScriptValue *arg_rate = ((n_args >= 2) ? p_arguments[1] : nullptr);
			int arg_rate_count = (arg_rate ? arg_rate->Count() : 1);
			bool rate_singleton = (arg_rate_count == 1);
			
			if (num_draws < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rexp() requires n to be greater than or equal to 0." << endl << slim_terminate();
			if (!rate_singleton && (arg_rate_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rexp() requires rate to be of length 1 or n." << endl << slim_terminate();
			
			if (rate_singleton)
			{
				double rate0 = (arg_rate ? arg_rate->FloatAtIndex(0) : 1.0);
				double mu0 = 1.0 / rate0;
				
				if (rate0 <= 0.0)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rexp() requires rate > 0.0." << endl << slim_terminate();
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					float_result->PushFloat(gsl_ran_exponential(g_rng, mu0));
			}
			else
			{
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double rate = arg_rate->FloatAtIndex(draw_index);
					
					if (rate <= 0.0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rexp() requires rate > 0.0." << endl << slim_terminate();
					
					float_result->PushFloat(gsl_ran_exponential(g_rng, 1.0 / rate));
				}
			}
			
			break;
		}
			
#pragma mark rnorm
		case FunctionIdentifier::rnormFunction:
		{
			int64_t num_draws = arg1_value->IntAtIndex(0);
			ScriptValue *arg_mu = ((n_args >= 2) ? p_arguments[1] : nullptr);
			ScriptValue *arg_sigma = ((n_args >= 3) ? p_arguments[2] : nullptr);
			int arg_mu_count = (arg_mu ? arg_mu->Count() : 1);
			int arg_sigma_count = (arg_sigma ? arg_sigma->Count() : 1);
			bool mu_singleton = (arg_mu_count == 1);
			bool sigma_singleton = (arg_sigma_count == 1);
			
			if (num_draws < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rnorm() requires n to be greater than or equal to 0." << endl << slim_terminate();
			if (!mu_singleton && (arg_mu_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rnorm() requires mean to be of length 1 or n." << endl << slim_terminate();
			if (!sigma_singleton && (arg_sigma_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rnorm() requires sd to be of length 1 or n." << endl << slim_terminate();
			
			double mu0 = (arg_mu ? arg_mu->FloatAtIndex(0) : 0.0);
			double sigma0 = (arg_sigma ? arg_sigma->FloatAtIndex(0) : 1.0);
			
			if (mu_singleton && sigma_singleton)
			{
				if (sigma0 < 0.0)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rnorm() requires sd >= 0.0." << endl << slim_terminate();
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					float_result->PushFloat(gsl_ran_gaussian(g_rng, sigma0) + mu0);
			}
			else
			{
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double mu = (mu_singleton ? mu0 : arg_mu->FloatAtIndex(draw_index));
					double sigma = (sigma_singleton ? sigma0 : arg_sigma->FloatAtIndex(draw_index));
					
					if (sigma < 0.0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rnorm() requires sd >= 0.0." << endl << slim_terminate();
					
					float_result->PushFloat(gsl_ran_gaussian(g_rng, sigma) + mu);
				}
			}
			
			break;
		}
			
#pragma mark rpois
		case FunctionIdentifier::rpoisFunction:
		{
			int64_t num_draws = arg1_value->IntAtIndex(0);
			ScriptValue *arg_lambda = p_arguments[1];
			int arg_lambda_count = arg_lambda->Count();
			bool lambda_singleton = (arg_lambda_count == 1);
			
			if (num_draws < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rpois() requires n to be greater than or equal to 0." << endl << slim_terminate();
			if (!lambda_singleton && (arg_lambda_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rpois() requires lambda to be of length 1 or n." << endl << slim_terminate();
			
			if (lambda_singleton)
			{
				double lambda0 = arg_lambda->FloatAtIndex(0);
				
				if (lambda0 <= 0.0)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rpois() requires lambda > 0.0." << endl << slim_terminate();
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					int_result->PushInt(gsl_ran_poisson(g_rng, lambda0));		// use the GSL, not slim_fast_ran_poisson, to give the user high accuracy
			}
			else
			{
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double lambda = arg_lambda->FloatAtIndex(draw_index);
					
					if (lambda <= 0.0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function rpois() requires lambda > 0.0." << endl << slim_terminate();
					
					int_result->PushInt(gsl_ran_poisson(g_rng, lambda));
				}
			}
			
			break;
		}
			
#pragma mark runif
		case FunctionIdentifier::runifFunction:
		{
			int64_t num_draws = arg1_value->IntAtIndex(0);
			ScriptValue *arg_min = ((n_args >= 2) ? p_arguments[1] : nullptr);
			ScriptValue *arg_max = ((n_args >= 3) ? p_arguments[2] : nullptr);
			int arg_min_count = (arg_min ? arg_min->Count() : 1);
			int arg_max_count = (arg_max ? arg_max->Count() : 1);
			bool min_singleton = (arg_min_count == 1);
			bool max_singleton = (arg_max_count == 1);
			
			if (num_draws < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function runif() requires n to be greater than or equal to 0." << endl << slim_terminate();
			if (!min_singleton && (arg_min_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function runif() requires min to be of length 1 or n." << endl << slim_terminate();
			if (!max_singleton && (arg_max_count != num_draws))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function runif() requires max to be of length 1 or n." << endl << slim_terminate();
			
			double min_value0 = (arg_min ? arg_min->FloatAtIndex(0) : 0.0);
			double max_value0 = (arg_max ? arg_max->FloatAtIndex(0) : 1.0);
			double range0 = max_value0 - min_value0;
			
			if (min_singleton && max_singleton)
			{
				if (range0 < 0.0)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function runif() requires min < max." << endl << slim_terminate();
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					float_result->PushFloat(gsl_rng_uniform(g_rng) * range0 + min_value0);
			}
			else
			{
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double min_value = (min_singleton ? min_value0 : arg_min->FloatAtIndex(draw_index));
					double max_value = (max_singleton ? max_value0 : arg_max->FloatAtIndex(draw_index));
					double range = max_value - min_value;
					
					if (range < 0.0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function runif() requires min < max." << endl << slim_terminate();
					
					float_result->PushFloat(gsl_rng_uniform(g_rng) * range + min_value);
				}
			}
			
			break;
		}
			
#pragma mark sample
		case FunctionIdentifier::sampleFunction:
		{
			int64_t sample_size = p_arguments[1]->IntAtIndex(0);
			bool replace = ((n_args >= 3) ? p_arguments[2]->LogicalAtIndex(0) : false);
			
			result = arg1_value->NewMatchingType();
			
			if (sample_size < 0)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function sample() requires a sample size >= 0." << endl << slim_terminate();
			if (sample_size == 0)
				break;
			
			// the algorithm used depends on whether weights were supplied
			if (n_args >= 4)
			{
				// weights supplied
				vector<double> weights_vector;
				double weights_sum = 0.0;
				ScriptValue *arg4_value = p_arguments[3];
				int arg4_count = arg4_value->Count();
				
				if (arg4_count != arg1_count)
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function sample() requires x and weights to be the same length." << endl << slim_terminate();
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					double weight = arg4_value->FloatAtIndex(value_index);
					
					weights_vector.push_back(weight);
					weights_sum += weight;
				}
				
				// get indices of x; we sample from this vector and then look up the corresponding ScriptValue element
				vector<int> index_vector;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					index_vector.push_back(value_index);
				
				// do the sampling
				int64_t contender_count = arg1_count;
				
				for (int64_t samples_generated = 0; samples_generated < sample_size; ++samples_generated)
				{
					if (contender_count <= 0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function sample() ran out of eligible elements from which to sample." << endl << slim_terminate();
					if (weights_sum <= 0.0)
						SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function sample() encountered weights summing to <= 0." << endl << slim_terminate();
					
					double rose = gsl_rng_uniform(g_rng) * weights_sum;
					double rose_sum = 0.0;
					int rose_index;
					
					for (rose_index = 0; rose_index < contender_count - 1; ++rose_index)	// -1 so roundoff gives the result to the last contender
					{
						rose_sum += weights_vector[rose_index];
						
						if (rose <= rose_sum)
							break;
					}
					
					result->PushValueFromIndexOfScriptValue(index_vector[rose_index], arg1_value);
					
					if (!replace)
					{
						weights_sum -= weights_vector[rose_index];
						
						index_vector.erase(index_vector.begin() + rose_index);
						weights_vector.erase(weights_vector.begin() + rose_index);
						--contender_count;
					}
				}
			}
			else
			{
				// weights not supplied; use equal weights
				if (replace)
				{
					for (int64_t samples_generated = 0; samples_generated < sample_size; ++samples_generated)
						result->PushValueFromIndexOfScriptValue((int)gsl_rng_uniform_int(g_rng, arg1_count), arg1_value);
				}
				else
				{
					// get indices of x; we sample from this vector and then look up the corresponding ScriptValue element
					vector<int> index_vector;
					
					for (int value_index = 0; value_index < arg1_count; ++value_index)
						index_vector.push_back(value_index);
					
					// do the sampling
					int64_t contender_count = arg1_count;
					
					for (int64_t samples_generated = 0; samples_generated < sample_size; ++samples_generated)
					{
						if (contender_count <= 0)
							SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function sample() ran out of eligible elements from which to sample." << endl << slim_terminate();
						
						int rose_index = (int)gsl_rng_uniform_int(g_rng, contender_count);
						
						result->PushValueFromIndexOfScriptValue(index_vector[rose_index], arg1_value);
						
						index_vector.erase(index_vector.begin() + rose_index);
						--contender_count;
					}
				}
			}
			
			break;
		}
			
#pragma mark seq
		case FunctionIdentifier::seqFunction:
			result = Execute_seq(p_function_name, p_arguments);
			break;
			
#pragma mark seqAlong
		case FunctionIdentifier::seqAlongFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				int_result->PushInt(value_index);
			break;

#pragma mark string
		case FunctionIdentifier::stringFunction:
			for (int64_t value_index = arg1_value->IntAtIndex(0); value_index > 0; --value_index)
				string_result->PushString("");
			break;
			

		// ************************************************************************************
		//
		//	value inspection/manipulation functions
		//
#pragma mark *** Value inspection/manipulation functions
			
#pragma mark all
		case FunctionIdentifier::allFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				if (!arg1_value->LogicalAtIndex(value_index))
				{
					logical_result->PushLogical(false);
					break;
				}
			if (logical_result->Count() == 0)
				logical_result->PushLogical(true);
			break;
			
#pragma mark any
		case FunctionIdentifier::anyFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				if (arg1_value->LogicalAtIndex(value_index))
				{
					logical_result->PushLogical(true);
					break;
				}
			if (logical_result->Count() == 0)
				logical_result->PushLogical(false);
			break;
			
#pragma mark cat
		case FunctionIdentifier::catFunction:
		{
			string separator = ((n_args >= 2) ? p_arguments[1]->StringAtIndex(0) : " ");
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
			{
				if (value_index > 0)
					p_output_stream << separator;
				
				p_output_stream << arg1_value->StringAtIndex(value_index);
			}
			break;
		}
			
#pragma mark ifelse
		case FunctionIdentifier::ifelseFunction:
		{
			ScriptValue *arg2_value = p_arguments[1];
			ScriptValueType arg2_type = arg2_value->Type();
			int arg2_count = arg2_value->Count();
			
			ScriptValue *arg3_value = p_arguments[2];
			ScriptValueType arg3_type = arg3_value->Type();
			int arg3_count = arg3_value->Count();
			
			if (arg1_count != arg2_count || arg1_count != arg3_count)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function ifelse() requires arguments of equal length." << endl << slim_terminate();
			if (arg2_type != arg3_type)
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function ifelse() requires arguments 2 and 3 to be the same type." << endl << slim_terminate();
				
			result = arg2_value->NewMatchingType();
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
			{
				if (arg1_value->LogicalAtIndex(value_index))
					result->PushValueFromIndexOfScriptValue(value_index, arg2_value);
				else
					result->PushValueFromIndexOfScriptValue(value_index, arg3_value);
			}
			break;
		}
			
#pragma mark nchar
		case FunctionIdentifier::ncharFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				int_result->PushInt(arg1_value->StringAtIndex(value_index).length());
			break;
			
#pragma mark paste
		case FunctionIdentifier::pasteFunction:
		{
			string separator = ((n_args >= 2) ? p_arguments[1]->StringAtIndex(0) : " ");
			string result_string;
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
			{
				if (value_index > 0)
					result_string.append(separator);
				
				result_string.append(arg1_value->StringAtIndex(value_index));
			}
			
			string_result->PushString(result_string);
			break;
		}
			
#pragma mark print
		case FunctionIdentifier::printFunction:
			p_output_stream << *arg1_value << endl;
			break;
			
#pragma mark rev
		case FunctionIdentifier::revFunction:
			result = arg1_value->NewMatchingType();
			
			for (int value_index = arg1_count - 1; value_index >= 0; --value_index)
				result->PushValueFromIndexOfScriptValue(value_index, arg1_value);
			break;
			
#pragma mark size
		case FunctionIdentifier::sizeFunction:
			int_result->PushInt(arg1_value->Count());
			break;
			
#pragma mark sort
		case FunctionIdentifier::sortFunction:
			result = arg1_value->NewMatchingType();
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				result->PushValueFromIndexOfScriptValue(value_index, arg1_value);
			
			result->Sort((n_args == 1) ? true : p_arguments[1]->LogicalAtIndex(0));
			break;
			
#pragma mark sortBy
		case FunctionIdentifier::sortByFunction:
		{
			ScriptValue_Object *object_result = new ScriptValue_Object();
			
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				object_result->PushElement(arg1_value->ElementAtIndex(value_index));
			
			object_result->SortBy(p_arguments[1]->StringAtIndex(0), (n_args == 2) ? true : p_arguments[2]->LogicalAtIndex(0));
			
			result = object_result;
			break;
		}
			
#pragma mark str
		case FunctionIdentifier::strFunction:
			p_output_stream << "(" << arg1_type << ") ";
			
			if (arg1_count <= 2)
				p_output_stream << *arg1_value << endl;
			else
			{
				ScriptValue *first_value = arg1_value->GetValueAtIndex(0);
				ScriptValue *second_value = arg1_value->GetValueAtIndex(1);
				
				p_output_stream << *first_value << " " << *second_value << " ... (" << arg1_count << " values)" << endl;
				
				if (!first_value->InSymbolTable()) delete first_value;
				if (!second_value->InSymbolTable()) delete second_value;
			}
			break;
			
#pragma mark strsplit
		case FunctionIdentifier::strsplitFunction:
		{
			string joined_string = arg1_value->StringAtIndex(0);
			string separator = ((n_args >= 2) ? p_arguments[1]->StringAtIndex(0) : " ");
			string::size_type start_idx = 0, sep_idx;
			
			while (true)
			{
				sep_idx = joined_string.find(separator, start_idx);
				
				if (sep_idx == string::npos)
				{
					string_result->PushString(joined_string.substr(start_idx));
					break;
				}
				else
				{
					string_result->PushString(joined_string.substr(start_idx, sep_idx - start_idx));
					start_idx = sep_idx + separator.length();
				}
			}
			
			break;
		}
			
#pragma mark substr
		case FunctionIdentifier::substrFunction:
		{
			ScriptValue *arg_first = p_arguments[1];
			int arg_first_count = arg_first->Count();
			bool first_singleton = (arg_first_count == 1);
			
			if (!first_singleton && (arg_first_count != arg1_count))
				SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function substr() requires the size of first to be 1, or equal to the size of x." << endl << slim_terminate();
			
			int64_t first0 = arg_first->IntAtIndex(0);
			
			if (n_args >= 3)
			{
				// last supplied
				ScriptValue *arg_last = p_arguments[2];
				int arg_last_count = arg_last->Count();
				bool last_singleton = (arg_last_count == 1);
				
				if (!last_singleton && (arg_last_count != arg1_count))
					SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): function substr() requires the size of last to be 1, or equal to the size of x." << endl << slim_terminate();
				
				int64_t last0 = arg_last->IntAtIndex(0);
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					std::string str = arg1_value->StringAtIndex(value_index);
					string::size_type len = str.length();
					int clamped_first = (int)(first_singleton ? first0 : arg_first->IntAtIndex(value_index));
					int clamped_last = (int)(last_singleton ? last0 : arg_last->IntAtIndex(value_index));
					
					if (clamped_first < 0) clamped_first = 0;
					if (clamped_last >= len) clamped_last = (int)len - 1;
					
					if ((clamped_first >= len) || (clamped_last < 0) || (clamped_first > clamped_last))
						string_result->PushString("");
					else
						string_result->PushString(str.substr(clamped_first, clamped_last - clamped_first + 1));
				}
			}
			else
			{
				// last not supplied; take substrings to the end of each string
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					std::string str = arg1_value->StringAtIndex(value_index);
					string::size_type len = str.length();
					int clamped_first = (int)(first_singleton ? first0 : arg_first->IntAtIndex(value_index));
					
					if (clamped_first < 0) clamped_first = 0;
					
					if (clamped_first >= len)						
						string_result->PushString("");
					else
						string_result->PushString(str.substr(clamped_first, len));
				}
			}
			
			break;
		}
			
#pragma mark unique
		case FunctionIdentifier::uniqueFunction:
			if (arg1_count == 0)
			{
				result = arg1_value->NewMatchingType();
			}
			else if (arg1_type == ScriptValueType::kValueLogical)
			{
				logical_result = new ScriptValue_Logical();
				result = logical_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					bool value = arg1_value->LogicalAtIndex(value_index);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg1_value->LogicalAtIndex(scan_index))
							break;
					}
					
					if (scan_index == value_index)
						logical_result->PushLogical(value);
				}
			}
			else if (arg1_type == ScriptValueType::kValueInt)
			{
				int_result = new ScriptValue_Int();
				result = int_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					int64_t value = arg1_value->IntAtIndex(value_index);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg1_value->IntAtIndex(scan_index))
							break;
					}
					
					if (scan_index == value_index)
						int_result->PushInt(value);
				}
			}
			else if (arg1_type == ScriptValueType::kValueFloat)
			{
				float_result = new ScriptValue_Float();
				result = float_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					double value = arg1_value->FloatAtIndex(value_index);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg1_value->FloatAtIndex(scan_index))
							break;
					}
					
					if (scan_index == value_index)
						float_result->PushFloat(value);
				}
			}
			else if (arg1_type == ScriptValueType::kValueString)
			{
				string_result = new ScriptValue_String();
				result = string_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					string value = arg1_value->StringAtIndex(value_index);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg1_value->StringAtIndex(scan_index))
							break;
					}
					
					if (scan_index == value_index)
						string_result->PushString(value);
				}
			}
			else if (arg1_type == ScriptValueType::kValueObject)
			{
				ScriptValue_Object *object_result = new ScriptValue_Object();
				result = object_result;
				
				for (int value_index = 0; value_index < arg1_count; ++value_index)
				{
					ScriptObjectElement *value = arg1_value->ElementAtIndex(value_index);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg1_value->ElementAtIndex(scan_index))
							break;
					}
					
					if (scan_index == value_index)
						object_result->PushElement(value);
				}
			}
			break;
			
#pragma mark which
		case FunctionIdentifier::whichFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				if (arg1_value->LogicalAtIndex(value_index))
					int_result->PushInt(value_index);
			break;
			
#pragma mark whichMax
		case FunctionIdentifier::whichMaxFunction:
			if (arg1_count == 0)
			{
				result = new ScriptValue_NULL();
			}
			else
			{
				int first_index = 0;
				
				if (arg1_type == ScriptValueType::kValueLogical)
				{
					bool max = arg1_value->LogicalAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						bool temp = arg1_value->LogicalAtIndex(value_index);
						if (max < temp) { max = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueInt)
				{
					int64_t max = arg1_value->IntAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						int64_t temp = arg1_value->IntAtIndex(value_index);
						if (max < temp) { max = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueFloat)
				{
					double max = arg1_value->FloatAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						double temp = arg1_value->FloatAtIndex(value_index);
						if (max < temp) { max = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueString)
				{
					string max = arg1_value->StringAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						string temp = arg1_value->StringAtIndex(value_index);
						if (max < temp) { max = temp; first_index = value_index; }
					}
				}
				
				int_result->PushInt(first_index);
			}
			break;
			
#pragma mark whichMin
		case FunctionIdentifier::whichMinFunction:
			if (arg1_count == 0)
			{
				result = new ScriptValue_NULL();
			}
			else
			{
				int first_index = 0;
				
				if (arg1_type == ScriptValueType::kValueLogical)
				{
					bool min = arg1_value->LogicalAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						bool temp = arg1_value->LogicalAtIndex(value_index);
						if (min > temp) { min = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueInt)
				{
					int64_t min = arg1_value->IntAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						int64_t temp = arg1_value->IntAtIndex(value_index);
						if (min > temp) { min = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueFloat)
				{
					double min = arg1_value->FloatAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						double temp = arg1_value->FloatAtIndex(value_index);
						if (min > temp) { min = temp; first_index = value_index; }
					}
				}
				else if (arg1_type == ScriptValueType::kValueString)
				{
					string min = arg1_value->StringAtIndex(0);
					
					for (int value_index = 1; value_index < arg1_count; ++value_index)
					{
						string temp = arg1_value->StringAtIndex(value_index);
						if (min > temp) { min = temp; first_index = value_index; }
					}
				}
				
				int_result->PushInt(first_index);
			}
			break;
			
			
		// ************************************************************************************
		//
		//	value type testing/coercion functions
		//
#pragma mark *** Value type testing/coercion functions
			
#pragma mark asFloat
		case FunctionIdentifier::asFloatFunction:
            for (int value_index = 0; value_index < arg1_count; ++value_index)
                float_result->PushFloat(arg1_value->FloatAtIndex(value_index));
            break;
			
#pragma mark asInteger
		case FunctionIdentifier::asIntegerFunction:
            for (int value_index = 0; value_index < arg1_count; ++value_index)
                int_result->PushInt(arg1_value->IntAtIndex(value_index));
            break;
			
#pragma mark asLogical
		case FunctionIdentifier::asLogicalFunction:
			for (int value_index = 0; value_index < arg1_count; ++value_index)
				logical_result->PushLogical(arg1_value->LogicalAtIndex(value_index));
			break;
			
#pragma mark asString
		case FunctionIdentifier::asStringFunction:
            for (int value_index = 0; value_index < arg1_count; ++value_index)
                string_result->PushString(arg1_value->StringAtIndex(value_index));
            break;
			
#pragma mark element
		case FunctionIdentifier::elementFunction:
			if (arg1_value->Type() == ScriptValueType::kValueObject)
				string_result->PushString(((ScriptValue_Object *)arg1_value)->ElementType());
			else
				string_result->PushString(StringForScriptValueType(arg1_value->Type()));
			break;
			
#pragma mark isFloat
		case FunctionIdentifier::isFloatFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueFloat);
			break;
			
#pragma mark isInteger
		case FunctionIdentifier::isIntegerFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueInt);
			break;
			
#pragma mark isLogical
		case FunctionIdentifier::isLogicalFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueLogical);
			break;
			
#pragma mark isNULL
		case FunctionIdentifier::isNULLFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueNULL);
			break;
			
#pragma mark isObject
		case FunctionIdentifier::isObjectFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueObject);
			break;
			
#pragma mark isString
		case FunctionIdentifier::isStringFunction:
			logical_result->PushLogical(arg1_type == ScriptValueType::kValueString);
			break;
			
#pragma mark type
		case FunctionIdentifier::typeFunction:
			string_result->PushString(StringForScriptValueType(arg1_value->Type()));
			break;
			
			
		// ************************************************************************************
		//
		//	bookkeeping functions
		//
#pragma mark *** Bookkeeping functions
			
#pragma mark date
		case FunctionIdentifier::dateFunction:
		{
			time_t rawtime;
			struct tm *timeinfo;
			char buffer[25];	// should never be more than 10, in fact, plus a null
			
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 25, "%d-%m-%Y", timeinfo);
			
			string_result->PushString(string(buffer));
			break;
		}
			
#pragma mark function
		case FunctionIdentifier::functionFunction:
		{
			string match_string = (arg1_value ? arg1_value->StringAtIndex(0) : "");
			bool signature_found = false;
			
			// function_map_ is already alphebetized since maps keep sorted order
			for (auto functionPairIter = function_map_.begin(); functionPairIter != function_map_.end(); ++functionPairIter)
			{
				const FunctionSignature *iter_signature = functionPairIter->second;
				
				if (arg1_value && (iter_signature->function_name_.compare(match_string) != 0))
					continue;
				
				p_output_stream << *iter_signature << endl;
				signature_found = true;
			}
			
			if (arg1_value && !signature_found)
				p_output_stream << "No function signature found for \"" << match_string << "\"." << endl;
			
			break;
		}
			
#pragma mark globals
		case FunctionIdentifier::globalsFunction:
			p_output_stream << *global_symbols_;
			break;
			
#pragma mark help
		case FunctionIdentifier::helpFunction:
			p_output_stream << "Help for SLiMscript is currently unimplemented." << endl;
			break;
			
#pragma mark license
		case FunctionIdentifier::licenseFunction:
			p_output_stream << "SLiM is free software: you can redistribute it and/or" << endl;
			p_output_stream << "modify it under the terms of the GNU General Public" << endl;
			p_output_stream << "License as published by the Free Software Foundation," << endl;
			p_output_stream << "either version 3 of the License, or (at your option)" << endl;
			p_output_stream << "any later version." << endl << endl;
			
			p_output_stream << "SLiM is distributed in the hope that it will be" << endl;
			p_output_stream << "useful, but WITHOUT ANY WARRANTY; without even the" << endl;
			p_output_stream << "implied warranty of MERCHANTABILITY or FITNESS FOR" << endl;
			p_output_stream << "A PARTICULAR PURPOSE.  See the GNU General Public" << endl;
			p_output_stream << "License for more details." << endl << endl;
			
			p_output_stream << "You should have received a copy of the GNU General" << endl;
			p_output_stream << "Public License along with SLiM.  If not, see" << endl;
			p_output_stream << "<http://www.gnu.org/licenses/>." << endl;
			break;
			
#pragma mark rm
		case FunctionIdentifier::rmFunction:
		{
			vector<string> symbols_to_remove;
			
			if (n_args == 0)
				symbols_to_remove = global_symbols_->ReadWriteSymbols();
			else
				for (int value_index = 0; value_index < arg1_count; ++value_index)
					symbols_to_remove.push_back(arg1_value->StringAtIndex(value_index));
			
			for (string symbol : symbols_to_remove)
				global_symbols_->RemoveValueForSymbol(symbol, false);
			
			break;
		}
			
#pragma mark setSeed
		case FunctionIdentifier::setSeedFunction:
			InitializeRNGFromSeed((int)(arg1_value->IntAtIndex(0)));
			break;
			
#pragma mark stop
		case FunctionIdentifier::stopFunction:
			if (arg1_value)
				p_output_stream << arg1_value->StringAtIndex(0) << endl;
			
			SLIM_TERMINATION << "ERROR (ExecuteFunctionCall): stop() called." << endl << slim_terminate();
			break;
			
#pragma mark time
		case FunctionIdentifier::timeFunction:
		{
			time_t rawtime;
			struct tm *timeinfo;
			char buffer[20];		// should never be more than 8, in fact, plus a null
			
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 20, "%H:%M:%S", timeinfo);
			
			string_result->PushString(string(buffer));
			break;
		}
			
#pragma mark version
		case FunctionIdentifier::versionFunction:
			string_result->PushString("SLiMscript version 2.0a1");
			break;
			
			
		// ************************************************************************************
		//
		//	object instantiation
		//
			
#pragma mark Path
		case FunctionIdentifier::PathFunction:
			Script_PathElement *pathElement = (n_args == 1) ? (new Script_PathElement(arg1_value->StringAtIndex(0))) : (new Script_PathElement());
			result = new ScriptValue_Object(pathElement);
			pathElement->Release();
			break;
	}
	
	// Deallocate any unused result pointers
	if (null_result && (null_result != result)) delete null_result;
	if (logical_result && (logical_result != result)) delete logical_result;
	if (float_result && (float_result != result)) delete float_result;
	if (int_result && (int_result != result)) delete int_result;
	if (string_result && (string_result != result)) delete string_result;
	
	// Check the return value against the signature
	signature->CheckReturn("function", result);
	
	return result;
}

ScriptValue *ScriptInterpreter::ExecuteMethodCall(ScriptValue_Object *method_object, string const &p_method_name, vector<ScriptValue*> const &p_arguments, ostream &p_output_stream)
{
	ScriptValue *result = nullptr;
	
	// Get the function signature and check our arguments against it
	const FunctionSignature *method_signature = method_object->SignatureForMethodOfElements(p_method_name);
	bool class_method = method_signature->is_class_method;
	bool instance_method = method_signature->is_instance_method;
	
	if (!class_method && !instance_method)
		SLIM_TERMINATION << "ERROR (ScriptInterpreter::ExecuteMethodCall): internal error: " << p_method_name << " is not designated as a class method or instance method." << endl << slim_terminate();
	if (class_method && instance_method)
		SLIM_TERMINATION << "ERROR (ScriptInterpreter::ExecuteMethodCall): internal error: " << p_method_name << " is designated as both a class method and an instance method." << endl << slim_terminate();
	
	method_signature->CheckArguments("method", p_arguments);
	
	// Make the method call
	if (class_method)
		result = method_object->ExecuteClassMethodOfElements(p_method_name, p_arguments, p_output_stream, *this);
	else
		result = method_object->ExecuteInstanceMethodOfElements(p_method_name, p_arguments, p_output_stream, *this);
	
	// Check the return value against the signature
	method_signature->CheckReturn("method", result);
	
	return result;
}


































































