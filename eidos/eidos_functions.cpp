//
//  eidos_functions.cpp
//  Eidos
//
//  Created by Ben Haller on 4/6/15.
//  Copyright (c) 2015 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
//

//	This file is part of Eidos.
//
//	Eidos is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	Eidos is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with Eidos.  If not, see <http://www.gnu.org/licenses/>.


#include "eidos_functions.h"
#include "eidos_call_signature.h"
#include "eidos_test_element.h"
#include "eidos_interpreter.h"
#include "eidos_rng.h"

#include "math.h"

#include <ctime>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <stdexcept>


using std::string;
using std::vector;
using std::map;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::istream;
using std::ostream;


//
//	Construct our built-in function map
//

// We allocate all of our function signatures once and keep them forever, for faster EidosInterpreter startup
vector<const EidosFunctionSignature *> &EidosInterpreter::BuiltInFunctions(void)
{
	static vector<const EidosFunctionSignature *> *signatures = nullptr;
	
	if (!signatures)
	{
		signatures = new vector<const EidosFunctionSignature *>;
		
		// ************************************************************************************
		//
		//	math functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("abs",				EidosFunctionIdentifier::absFunction,			kEidosValueMaskNumeric))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("acos",				EidosFunctionIdentifier::acosFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("asin",				EidosFunctionIdentifier::asinFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("atan",				EidosFunctionIdentifier::atanFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("atan2",			EidosFunctionIdentifier::atan2Function,			kEidosValueMaskFloat))->AddNumeric("x")->AddNumeric("y"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("ceil",				EidosFunctionIdentifier::ceilFunction,			kEidosValueMaskFloat))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("cos",				EidosFunctionIdentifier::cosFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("exp",				EidosFunctionIdentifier::expFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("floor",			EidosFunctionIdentifier::floorFunction,			kEidosValueMaskFloat))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isFinite",			EidosFunctionIdentifier::isFiniteFunction,		kEidosValueMaskLogical))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isInfinite",		EidosFunctionIdentifier::isInfiniteFunction,	kEidosValueMaskLogical))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isNAN",			EidosFunctionIdentifier::isNaNFunction,			kEidosValueMaskLogical))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("log",				EidosFunctionIdentifier::logFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("log10",			EidosFunctionIdentifier::log10Function,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("log2",				EidosFunctionIdentifier::log2Function,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("product",			EidosFunctionIdentifier::productFunction,		kEidosValueMaskNumeric | kEidosValueMaskSingleton))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("round",			EidosFunctionIdentifier::roundFunction,			kEidosValueMaskFloat))->AddFloat("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sin",				EidosFunctionIdentifier::sinFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sqrt",				EidosFunctionIdentifier::sqrtFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sum",				EidosFunctionIdentifier::sumFunction,			kEidosValueMaskNumeric | kEidosValueMaskSingleton))->AddLogicalEquiv("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("tan",				EidosFunctionIdentifier::tanFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("trunc",			EidosFunctionIdentifier::truncFunction,			kEidosValueMaskFloat))->AddFloat("x"));
		
		
		// ************************************************************************************
		//
		//	summary statistics functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("max",				EidosFunctionIdentifier::maxFunction,			kEidosValueMaskAnyBase | kEidosValueMaskSingleton))->AddAnyBase("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("mean",				EidosFunctionIdentifier::meanFunction,			kEidosValueMaskFloat))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("min",				EidosFunctionIdentifier::minFunction,			kEidosValueMaskAnyBase | kEidosValueMaskSingleton))->AddAnyBase("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("range",			EidosFunctionIdentifier::rangeFunction,			kEidosValueMaskNumeric))->AddNumeric("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sd",				EidosFunctionIdentifier::sdFunction,			kEidosValueMaskFloat | kEidosValueMaskSingleton))->AddNumeric("x"));
		
		
		// ************************************************************************************
		//
		//	vector construction functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("c",				EidosFunctionIdentifier::cFunction,				kEidosValueMaskAny))->AddEllipsis());
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_float,	EidosFunctionIdentifier::floatFunction,			kEidosValueMaskFloat))->AddInt_S("length"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_integer,	EidosFunctionIdentifier::integerFunction,		kEidosValueMaskInt))->AddInt_S("length"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_logical,	EidosFunctionIdentifier::logicalFunction,		kEidosValueMaskLogical))->AddInt_S("length"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_object,	EidosFunctionIdentifier::objectFunction,		kEidosValueMaskObject, gEidos_UndefinedClassObject)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rbinom",			EidosFunctionIdentifier::rbinomFunction,		kEidosValueMaskInt))->AddInt_S("n")->AddInt("size")->AddFloat("prob"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rep",				EidosFunctionIdentifier::repFunction,			kEidosValueMaskAny))->AddAny("x")->AddInt_S("count"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("repEach",			EidosFunctionIdentifier::repEachFunction,		kEidosValueMaskAny))->AddAny("x")->AddInt("count"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rexp",				EidosFunctionIdentifier::rexpFunction,			kEidosValueMaskFloat))->AddInt_S("n")->AddNumeric_O("rate"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rnorm",			EidosFunctionIdentifier::rnormFunction,			kEidosValueMaskFloat))->AddInt_S("n")->AddNumeric_O("mean")->AddNumeric_O("sd"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rpois",			EidosFunctionIdentifier::rpoisFunction,			kEidosValueMaskInt))->AddInt_S("n")->AddNumeric("lambda"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("runif",			EidosFunctionIdentifier::runifFunction,			kEidosValueMaskFloat))->AddInt_S("n")->AddNumeric_O("min")->AddNumeric_O("max"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sample",			EidosFunctionIdentifier::sampleFunction,		kEidosValueMaskAny))->AddAny("x")->AddInt("size")->AddLogical_OS("replace")->AddNumeric_O("weights"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("seq",				EidosFunctionIdentifier::seqFunction,			kEidosValueMaskNumeric))->AddNumeric_S("from")->AddNumeric_S("to")->AddNumeric_OS("by"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("seqAlong",			EidosFunctionIdentifier::seqAlongFunction,		kEidosValueMaskInt))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_string,	EidosFunctionIdentifier::stringFunction,		kEidosValueMaskString))->AddInt_S("length"));
		
		
		// ************************************************************************************
		//
		//	value inspection/manipulation functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("all",				EidosFunctionIdentifier::allFunction,			kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddLogical("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("any",				EidosFunctionIdentifier::anyFunction,			kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddLogical("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("cat",				EidosFunctionIdentifier::catFunction,			kEidosValueMaskNULL))->AddAny("x")->AddString_OS("sep"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("identical",		EidosFunctionIdentifier::identicalFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x")->AddAny("y"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("ifelse",			EidosFunctionIdentifier::ifelseFunction,		kEidosValueMaskAny))->AddLogical("test")->AddAny("trueValues")->AddAny("falseValues"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("match",			EidosFunctionIdentifier::matchFunction,			kEidosValueMaskInt))->AddAny("x")->AddAny("y"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("nchar",			EidosFunctionIdentifier::ncharFunction,			kEidosValueMaskInt))->AddString("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("paste",			EidosFunctionIdentifier::pasteFunction,			kEidosValueMaskString | kEidosValueMaskSingleton))->AddAny("x")->AddString_OS("sep"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("print",			EidosFunctionIdentifier::printFunction,			kEidosValueMaskNULL))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("rev",				EidosFunctionIdentifier::revFunction,			kEidosValueMaskAny))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_size,		EidosFunctionIdentifier::sizeFunction,			kEidosValueMaskInt | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sort",				EidosFunctionIdentifier::sortFunction,			kEidosValueMaskAnyBase))->AddAnyBase("x")->AddLogical_OS("ascending"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("sortBy",			EidosFunctionIdentifier::sortByFunction,		kEidosValueMaskObject))->AddObject("x", nullptr)->AddString_S("property")->AddLogical_OS("ascending"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_str,		EidosFunctionIdentifier::strFunction,			kEidosValueMaskNULL))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("strsplit",			EidosFunctionIdentifier::strsplitFunction,		kEidosValueMaskString))->AddString_S("x")->AddString_OS("sep"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("substr",			EidosFunctionIdentifier::substrFunction,		kEidosValueMaskString))->AddString("x")->AddInt("first")->AddInt_O("last"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("unique",			EidosFunctionIdentifier::uniqueFunction,		kEidosValueMaskAny))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("which",			EidosFunctionIdentifier::whichFunction,			kEidosValueMaskInt))->AddLogical("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("whichMax",			EidosFunctionIdentifier::whichMaxFunction,		kEidosValueMaskInt))->AddAnyBase("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("whichMin",			EidosFunctionIdentifier::whichMinFunction,		kEidosValueMaskInt))->AddAnyBase("x"));
		
		
		// ************************************************************************************
		//
		//	value type testing/coercion functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("asFloat",			EidosFunctionIdentifier::asFloatFunction,		kEidosValueMaskFloat))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("asInteger",		EidosFunctionIdentifier::asIntegerFunction,		kEidosValueMaskInt))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("asLogical",		EidosFunctionIdentifier::asLogicalFunction,		kEidosValueMaskLogical))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("asString",			EidosFunctionIdentifier::asStringFunction,		kEidosValueMaskString))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("elementType",		EidosFunctionIdentifier::elementTypeFunction,	kEidosValueMaskString | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isFloat",			EidosFunctionIdentifier::isFloatFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isInteger",		EidosFunctionIdentifier::isIntegerFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isLogical",		EidosFunctionIdentifier::isLogicalFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isNULL",			EidosFunctionIdentifier::isNULLFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isObject",			EidosFunctionIdentifier::isObjectFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("isString",			EidosFunctionIdentifier::isStringFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddAny("x"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("type",				EidosFunctionIdentifier::typeFunction,			kEidosValueMaskString | kEidosValueMaskSingleton))->AddAny("x"));
		
		
		// ************************************************************************************
		//
		//	miscellaneous functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_apply,	EidosFunctionIdentifier::applyFunction,			kEidosValueMaskAny))->AddAny("x")->AddString_S("lambdaSource"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("date",				EidosFunctionIdentifier::dateFunction,			kEidosValueMaskString | kEidosValueMaskSingleton)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_executeLambda,	EidosFunctionIdentifier::executeLambdaFunction,	kEidosValueMaskAny))->AddString_S("lambdaSource")->AddLogical_OS("timed"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("function",			EidosFunctionIdentifier::functionFunction,		kEidosValueMaskNULL))->AddString_OS("functionName"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_ls,		EidosFunctionIdentifier::lsFunction,			kEidosValueMaskNULL)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("license",			EidosFunctionIdentifier::licenseFunction,		kEidosValueMaskNULL)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature(gEidosStr_rm,		EidosFunctionIdentifier::rmFunction,			kEidosValueMaskNULL))->AddString_O("variableNames"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("setSeed",			EidosFunctionIdentifier::setSeedFunction,		kEidosValueMaskNULL))->AddInt_S("seed"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("getSeed",			EidosFunctionIdentifier::getSeedFunction,		kEidosValueMaskInt | kEidosValueMaskSingleton)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("stop",				EidosFunctionIdentifier::stopFunction,			kEidosValueMaskNULL))->AddString_OS("message"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("time",				EidosFunctionIdentifier::timeFunction,			kEidosValueMaskString | kEidosValueMaskSingleton)));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("version",			EidosFunctionIdentifier::versionFunction,		kEidosValueMaskNULL)));
		
		
		// ************************************************************************************
		//
		//	filesystem access functions
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("filesAtPath",		EidosFunctionIdentifier::filesAtPathFunction,	kEidosValueMaskString))->AddString_S("path")->AddLogical_OS("fullPaths"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("readFile",			EidosFunctionIdentifier::readFileFunction,		kEidosValueMaskString))->AddString_S("filePath"));
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("writeFile",		EidosFunctionIdentifier::writeFileFunction,		kEidosValueMaskLogical | kEidosValueMaskSingleton))->AddString_S("filePath")->AddString("contents"));

		
		// ************************************************************************************
		//
		//	object instantiation
		//
		
		signatures->push_back((EidosFunctionSignature *)(new EidosFunctionSignature("_Test",			EidosFunctionIdentifier::_TestFunction,			kEidosValueMaskObject | kEidosValueMaskSingleton, gEidos_TestElementClass))->AddInt_S("yolk"));
		
		
		// alphabetize, mostly to be nice to the auto-completion feature
		std::sort(signatures->begin(), signatures->end(), CompareEidosCallSignatures);
	}
	
	return *signatures;
}

EidosFunctionMap *EidosInterpreter::BuiltInFunctionMap(void)
{
	// The built-in function map is statically allocated for faster EidosInterpreter startup
	static EidosFunctionMap *built_in_function_map = nullptr;
	
	if (!built_in_function_map)
	{
		vector<const EidosFunctionSignature *> &built_in_functions = EidosInterpreter::BuiltInFunctions();
		
		built_in_function_map = new EidosFunctionMap;
		
		for (auto sig : built_in_functions)
			built_in_function_map->insert(EidosFunctionMapPair(sig->function_name_, sig));
	}
	
	return built_in_function_map;
}


//
//	Executing function calls
//

EidosValue *ConcatenateEidosValues(EidosValue *const *const p_arguments, int p_argument_count, bool p_allow_null)
{
	// This function expects an error range to be set bracketing it externally,
	// so no blame token is needed here.
	
	EidosValueType highest_type = EidosValueType::kValueNULL;
	bool has_object_type = false, has_nonobject_type = false, all_invisible = true;
	const EidosObjectClass *element_class = nullptr;
	
	// First figure out our return type, which is the highest-promotion type among all our arguments
	for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
	{
		EidosValue *arg_value = p_arguments[arg_index];
		EidosValueType arg_type = arg_value->Type();
		
		if (!p_allow_null && (arg_type == EidosValueType::kValueNULL))
			EIDOS_TERMINATION << "ERROR (ConcatenateEidosValues): NULL is not allowed to be used in this context." << eidos_terminate(nullptr);
		
		if (arg_type > highest_type)
			highest_type = arg_type;
		
		if (!arg_value->Invisible())
			all_invisible = false;
		
		if (arg_type == EidosValueType::kValueObject)
		{
			if (arg_value->Count() > 0)		// object(0) parameters do not conflict with other object types
			{
				const EidosObjectClass *this_element_class = ((EidosValue_Object *)arg_value)->Class();
				
				if (!element_class)
				{
					// we haven't seen a (non-empty) object type yet, so remember what type we're dealing with
					element_class = this_element_class;
				}
				else
				{
					// we've already seen a object type, so check that this one is the same type
					if (element_class != this_element_class)
						EIDOS_TERMINATION << "ERROR (ConcatenateEidosValues): objects of different types cannot be mixed." << eidos_terminate(nullptr);
				}
			}
			
			has_object_type = true;
		}
		else
			has_nonobject_type = true;
	}
	
	if (has_object_type && has_nonobject_type)
		EIDOS_TERMINATION << "ERROR (ConcatenateEidosValues): object and non-object types cannot be mixed." << eidos_terminate(nullptr);
	
	// If we've got nothing but NULL, then return NULL; preserve invisibility
	if (highest_type == EidosValueType::kValueNULL)
		return (all_invisible ? gStaticEidosValueNULLInvisible : gStaticEidosValueNULL);
	
	// Create an object of the right return type, concatenate all the arguments together, and return it
	// Note that NULLs here concatenate away silently; a bit dangerous!
	if (highest_type == EidosValueType::kValueLogical)
	{
		EidosValue_Logical *result = new EidosValue_Logical();
		
		for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
		{
			EidosValue *arg_value = p_arguments[arg_index];
			
			if (arg_value->Type() != EidosValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushLogical(arg_value->LogicalAtIndex(value_index, nullptr));
		}
		
		return result;
	}
	else if (highest_type == EidosValueType::kValueInt)
	{
		EidosValue_Int_vector *result = new EidosValue_Int_vector();
		
		for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
		{
			EidosValue *arg_value = p_arguments[arg_index];
			
			if (arg_value->Type() != EidosValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushInt(arg_value->IntAtIndex(value_index, nullptr));
		}
		
		return result;
	}
	else if (highest_type == EidosValueType::kValueFloat)
	{
		EidosValue_Float_vector *result = new EidosValue_Float_vector();
		
		for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
		{
			EidosValue *arg_value = p_arguments[arg_index];
			
			if (arg_value->Type() != EidosValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushFloat(arg_value->FloatAtIndex(value_index, nullptr));
		}
		
		return result;
	}
	else if (highest_type == EidosValueType::kValueString)
	{
		EidosValue_String_vector *result = new EidosValue_String_vector();
		
		for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
		{
			EidosValue *arg_value = p_arguments[arg_index];
			
			if (arg_value->Type() != EidosValueType::kValueNULL)
				for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
					result->PushString(arg_value->StringAtIndex(value_index, nullptr));
		}
		
		return result;
	}
	else if (has_object_type)
	{
		EidosValue_Object_vector *result = new EidosValue_Object_vector();
		
		for (int arg_index = 0; arg_index < p_argument_count; ++arg_index)
		{
			EidosValue *arg_value = p_arguments[arg_index];
			
			for (int value_index = 0; value_index < arg_value->Count(); ++value_index)
				result->PushObjectElement(arg_value->ObjectElementAtIndex(value_index, nullptr));
		}
		
		return result;
	}
	else
	{
		EIDOS_TERMINATION << "ERROR (ConcatenateEidosValues): type '" << highest_type << "' is not supported by ConcatenateEidosValues()." << eidos_terminate(nullptr);
	}
	
	return nullptr;
}

EidosValue *EidosInterpreter::ExecuteFunctionCall(string const &p_function_name, const EidosFunctionSignature *p_function_signature, EidosValue *const *const p_arguments, int p_argument_count)
{
	EidosValue *result = gStaticEidosValueNULLInvisible;	// our default return value unless it gets replaced below; this is a static, so does not need to be freed
	
	// If the function call is a built-in Eidos function, we might already have a pointer to its signature cached; if not, we'll have to look it up
	if (!p_function_signature)
	{
		// Get the function signature and check our arguments against it
		auto signature_iter = function_map_->find(p_function_name);
		
		if (signature_iter == function_map_->end())
			EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): unrecognized function name " << p_function_name << "." << eidos_terminate(nullptr);
		
		p_function_signature = signature_iter->second;
	}
	
	// Check the functions arguments against the signature
	p_function_signature->CheckArguments(p_arguments, p_argument_count);
	
	// Now we look up the function again and actually execute it
	switch (p_function_signature->function_id_)
	{
		case EidosFunctionIdentifier::kNoFunction:
		{
			EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): internal logic error." << eidos_terminate(nullptr);
			break;
		}
			
		case EidosFunctionIdentifier::kDelegatedFunction:
		{
			result = p_function_signature->delegate_function_(p_function_signature->delegate_object_, p_function_name, p_arguments, p_argument_count, *this);
			break;
		}
			
			
		// ************************************************************************************
		//
		//	math functions
		//
#pragma mark -
#pragma mark Math functions
#pragma mark -
			
			
			//	(numeric)abs(numeric x)
			#pragma mark abs
			
		case EidosFunctionIdentifier::absFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_type == EidosValueType::kValueInt)
			{
				if (arg0_count == 1)
				{
					// This is an overflow-safe version of:
					//result = new EidosValue_Int_singleton_const(llabs(arg0_value->IntAtIndex(0, nullptr)));
					
					int64_t operand = arg0_value->IntAtIndex(0, nullptr);
					int64_t abs_result = llabs(operand);
					
					// llabs() man page: "The absolute value of the most negative integer remains negative."
					if (abs_result < 0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function abs() cannot take the absolute value of the most negative integer." << eidos_terminate(nullptr);
					
					result = new EidosValue_Int_singleton_const(abs_result);
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
					result = int_result;
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						// This is an overflow-safe version of:
						//int_result->PushInt(llabs(int_vec[value_index]));
						
						int64_t operand = int_vec[value_index];
						int64_t abs_result = llabs(operand);
						
						// llabs() man page: "The absolute value of the most negative integer remains negative."
						if (abs_result < 0)
							EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function abs() cannot take the absolute value of the most negative integer." << eidos_terminate(nullptr);
						
						int_result->PushInt(abs_result);
					}
				}
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				if (arg0_count == 1)
				{
					result = new EidosValue_Float_singleton_const(fabs(arg0_value->FloatAtIndex(0, nullptr)));
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
					result = float_result;
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						float_result->PushFloat(fabs(float_vec[value_index]));
				}
			}
			break;
		}
			
			
			//	(float)acos(numeric x)
			#pragma mark acos
			
		case EidosFunctionIdentifier::acosFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(acos(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(acos(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)asin(numeric x)
			#pragma mark asin
			
		case EidosFunctionIdentifier::asinFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(asin(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(asin(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)atan(numeric x)
			#pragma mark atan
			
		case EidosFunctionIdentifier::atanFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(atan(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(atan(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)atan2(numeric x, numeric y)
			#pragma mark atan2
			
		case EidosFunctionIdentifier::atan2Function:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			int arg1_count = arg1_value->Count();
			
			if (arg0_count != arg1_count)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function atan2() requires arguments of equal length." << eidos_terminate(nullptr);
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(atan2(arg0_value->FloatAtIndex(0, nullptr), arg1_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(atan2(arg0_value->FloatAtIndex(value_index, nullptr), arg1_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)ceil(float x)
			#pragma mark ceil
			
		case EidosFunctionIdentifier::ceilFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(ceil(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(ceil(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(float)cos(numeric x)
			#pragma mark cos
			
		case EidosFunctionIdentifier::cosFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(cos(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(cos(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)exp(numeric x)
			#pragma mark exp
			
		case EidosFunctionIdentifier::expFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(exp(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(exp(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)floor(float x)
			#pragma mark floor
			
		case EidosFunctionIdentifier::floorFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(floor(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(floor(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(logical)isFinite(float x)
			#pragma mark isFinite
			
		case EidosFunctionIdentifier::isFiniteFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = (isfinite(arg0_value->FloatAtIndex(0, nullptr)) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Logical *logical_result = new EidosValue_Logical();
				result = logical_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					logical_result->PushLogical(isfinite(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(logical)isInfinite(float x)
			#pragma mark isInfinite
			
		case EidosFunctionIdentifier::isInfiniteFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = (isinf(arg0_value->FloatAtIndex(0, nullptr)) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Logical *logical_result = new EidosValue_Logical();
				result = logical_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					logical_result->PushLogical(isinf(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(logical)isNAN(float x)
			#pragma mark isNAN
			
		case EidosFunctionIdentifier::isNaNFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = (isnan(arg0_value->FloatAtIndex(0, nullptr)) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Logical *logical_result = new EidosValue_Logical();
				result = logical_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					logical_result->PushLogical(isnan(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(float)log(numeric x)
			#pragma mark log
			
		case EidosFunctionIdentifier::logFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(log(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(log(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)log10(numeric x)
			#pragma mark log10
			
		case EidosFunctionIdentifier::log10Function:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(log10(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(log10(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)log2(numeric x)
			#pragma mark log2
			
		case EidosFunctionIdentifier::log2Function:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(log2(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(log2(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(numeric$)product(numeric x)
			#pragma mark product
			
		case EidosFunctionIdentifier::productFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_type == EidosValueType::kValueInt)
			{
				if (arg0_count == 1)
				{
					result = new EidosValue_Int_singleton_const(arg0_value->IntAtIndex(0, nullptr));
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					int64_t product = 1;
					double product_d = 1.0;
					bool fits_in_integer = true;
					
					// We do a tricky thing here.  We want to try to compute in integer, but switch to float if we overflow.
					// If we do overflow, we want to minimize numerical error by accumulating in integer for as long as we
					// can, and then throwing the integer accumulator over into the float accumulator only when it is about
					// to overflow.  We perform both computations in parallel, and use integer for the result if we can.
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						int64_t old_product = product;
						int64_t temp = int_vec[value_index];
						
						// Overflow detection is provided by this built-in function, supported by both GCC and clang.
						// see http://clang.llvm.org/docs/LanguageExtensions.html#checked-arithmetic-builtins and
						// see https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
						bool overflow = __builtin_smulll_overflow(old_product, temp, &product);
						
						// switch to float computation on overflow, and accumulate in the float product just before overflow
						if (overflow)
						{
							fits_in_integer = false;
							product_d *= old_product;
							product = temp;
						}
					}
					
					product_d *= product;		// multiply in whatever integer accumulation has not overflowed
					
					if (fits_in_integer)
						result = new EidosValue_Int_singleton_const(product);
					else
						result = new EidosValue_Float_singleton_const(product_d);
				}
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				if (arg0_count == 1)
				{
					result = new EidosValue_Float_singleton_const(arg0_value->FloatAtIndex(0, nullptr));
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					double product = 1;
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						product *= float_vec[value_index];
					
					result = new EidosValue_Float_singleton_const(product);
				}
			}
			break;
		}
			
			
			//	(numeric$)sum(lif x)
			#pragma mark sum
			
		case EidosFunctionIdentifier::sumFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_type == EidosValueType::kValueInt)
			{
				if (arg0_count == 1)
				{
					result = new EidosValue_Int_singleton_const(arg0_value->IntAtIndex(0, nullptr));
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					int64_t sum = 0;
					double sum_d = 0;
					bool fits_in_integer = true;
					
					// We do a tricky thing here.  We want to try to compute in integer, but switch to float if we overflow.
					// If we do overflow, we want to minimize numerical error by accumulating in integer for as long as we
					// can, and then throwing the integer accumulator over into the float accumulator only when it is about
					// to overflow.  We perform both computations in parallel, and use integer for the result if we can.
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						int64_t old_sum = sum;
						int64_t temp = int_vec[value_index];
						
						// Overflow detection is provided by this built-in function, supported by both GCC and clang.
						// see http://clang.llvm.org/docs/LanguageExtensions.html#checked-arithmetic-builtins and
						// see https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
						bool overflow = __builtin_saddll_overflow(old_sum, temp, &sum);
						
						// switch to float computation on overflow, and accumulate in the float sum just before overflow
						if (overflow)
						{
							fits_in_integer = false;
							sum_d += old_sum;
							sum = temp;		// start integer accumulation again from 0 until it overflows again
						}
					}
					
					sum_d += sum;			// add in whatever integer accumulation has not overflowed
					
					if (fits_in_integer)
						result = new EidosValue_Int_singleton_const(sum);
					else
						result = new EidosValue_Float_singleton_const(sum_d);
				}
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				if (arg0_count == 1)
				{
					result = new EidosValue_Float_singleton_const(arg0_value->FloatAtIndex(0, nullptr));
				}
				else
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					double sum = 0;
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						sum += float_vec[value_index];
					
					result = new EidosValue_Float_singleton_const(sum);
				}
			}
			else if (arg0_type == EidosValueType::kValueLogical)
			{
				// EidosValue_Logical does not have a singleton subclass, so we can always use the fast API
				const std::vector<bool> &logical_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
				int64_t sum = 0;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					sum += logical_vec[value_index];
				
				result = new EidosValue_Int_singleton_const(sum);
			}
			break;
		}
			
			
			//	(float)round(float x)
			#pragma mark round
			
		case EidosFunctionIdentifier::roundFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(round(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(round(float_vec[value_index]));
			}
			break;
		}
			
			
			//	(float)sin(numeric x)
			#pragma mark sin
			
		case EidosFunctionIdentifier::sinFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(sin(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(sin(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)sqrt(numeric x)
			#pragma mark sqrt
			
		case EidosFunctionIdentifier::sqrtFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(sqrt(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(sqrt(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)tan(numeric x)
			#pragma mark tan
			
		case EidosFunctionIdentifier::tanFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(tan(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(tan(arg0_value->FloatAtIndex(value_index, nullptr)));
			}
			break;
		}
			
			
			//	(float)trunc(float x)
			#pragma mark trunc
			
		case EidosFunctionIdentifier::truncFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(trunc(arg0_value->FloatAtIndex(0, nullptr)));
			}
			else
			{
				// We have arg0_count != 1 and arg0_value is guaranteed to be an EidosValue_Float, so we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(trunc(float_vec[value_index]));
			}
			break;
		}
			
			
		// ************************************************************************************
		//
		//	summary statistics functions
		//
#pragma mark -
#pragma mark Summary statistics functions
#pragma mark -
			
			
			//	(+$)max(+ x)
			#pragma mark max
			
		case EidosFunctionIdentifier::maxFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else if (arg0_type == EidosValueType::kValueLogical)
			{
				bool max = arg0_value->LogicalAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						bool temp = bool_vec[value_index];
						if (max < temp)
							max = temp;
					}
				}
				
				result = (max ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else if (arg0_type == EidosValueType::kValueInt)
			{
				int64_t max = arg0_value->IntAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						int64_t temp = int_vec[value_index];
						if (max < temp)
							max = temp;
					}
				}
				
				result = new EidosValue_Int_singleton_const(max);
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				double max = arg0_value->FloatAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						double temp = float_vec[value_index];
						if (max < temp)
							max = temp;
					}
				}
				
				result = new EidosValue_Float_singleton_const(max);
			}
			else if (arg0_type == EidosValueType::kValueString)
			{
				string max = arg0_value->StringAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_String_vector; we can use the fast API
					const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						const string &temp = string_vec[value_index];
						if (max < temp)
							max = temp;
					}
				}
				
				result = new EidosValue_String_singleton_const(max);
			}
			break;
		}
			
			
			//	(float)mean(numeric x)
			#pragma mark mean
			
		case EidosFunctionIdentifier::meanFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else
			{
				double sum = 0;
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					sum += arg0_value->FloatAtIndex(value_index, nullptr);
				
				result = new EidosValue_Float_singleton_const(sum / arg0_count);
			}
			break;
		}
			
			
			//	(+$)min(+ x)
			#pragma mark min
			
		case EidosFunctionIdentifier::minFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else if (arg0_type == EidosValueType::kValueLogical)
			{
				bool min = arg0_value->LogicalAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						bool temp = bool_vec[value_index];
						if (min > temp)
							min = temp;
					}
				}
				
				result = (min ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else if (arg0_type == EidosValueType::kValueInt)
			{
				int64_t min = arg0_value->IntAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						int64_t temp = int_vec[value_index];
						if (min > temp)
							min = temp;
					}
				}
				
				result = new EidosValue_Int_singleton_const(min);
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				double min = arg0_value->FloatAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						double temp = float_vec[value_index];
						if (min > temp)
							min = temp;
					}
				}
				
				result = new EidosValue_Float_singleton_const(min);
			}
			else if (arg0_type == EidosValueType::kValueString)
			{
				string min = arg0_value->StringAtIndex(0, nullptr);
				
				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_String_vector; we can use the fast API
					const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						const string &temp = string_vec[value_index];
						if (min > temp)
							min = temp;
					}
				}
				
				result = new EidosValue_String_singleton_const(min);
			}
			break;
		}
			
			
			//	(numeric)range(numeric x)
			#pragma mark range
			
		case EidosFunctionIdentifier::rangeFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else if (arg0_type == EidosValueType::kValueInt)
			{
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				int64_t max = arg0_value->IntAtIndex(0, nullptr);
				int64_t min = max;

				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
					const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						int64_t temp = int_vec[value_index];
						if (max < temp)
							max = temp;
						else if (min > temp)
							min = temp;
					}
				}
				
				int_result->PushInt(min);
				int_result->PushInt(max);
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				double max = arg0_value->FloatAtIndex(0, nullptr);
				double min = max;

				if (arg0_count > 1)
				{
					// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
					const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					
					for (int value_index = 1; value_index < arg0_count; ++value_index)
					{
						double temp = float_vec[value_index];
						if (max < temp)
							max = temp;
						else if (min > temp)
							min = temp;
					}
				}
				
				float_result->PushFloat(min);
				float_result->PushFloat(max);
			}
			break;
		}
			
			
			//	(float$)sd(numeric x)
			#pragma mark sd
			
		case EidosFunctionIdentifier::sdFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count > 1)
			{
				double mean = 0;
				double sd = 0;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					mean += arg0_value->FloatAtIndex(value_index, nullptr);
				
				mean /= arg0_count;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					double temp = (arg0_value->FloatAtIndex(value_index, nullptr) - mean);
					sd += temp * temp;
				}
				
				sd = sqrt(sd / (arg0_count - 1));
				result = new EidosValue_Float_singleton_const(sd);
			}
			else
			{
				result = gStaticEidosValueNULL;
			}
			break;
		}
			
		// ************************************************************************************
		//
		//	vector construction functions
		//
#pragma mark -
#pragma mark Vector conversion functions
#pragma mark -
			
			
			//	(*)c(...)
			#pragma mark c
			
		case EidosFunctionIdentifier::cFunction:
		{
			result = ConcatenateEidosValues(p_arguments, p_argument_count, false);
			break;
		}
			
			
			//	(float)float(integer$ length)
			#pragma mark float
			
		case EidosFunctionIdentifier::floatFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t element_count = arg0_value->IntAtIndex(0, nullptr);
			
			if (element_count < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function float() requires length to be greater than or equal to 0 (" << element_count << " supplied)." << eidos_terminate(nullptr);
			
			EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
			result = float_result;
			
			for (int64_t value_index = element_count; value_index > 0; --value_index)
				float_result->PushFloat(0.0);
			break;
		}
			
			
			//	(integer)integer(integer$ length)
			#pragma mark integer
			
		case EidosFunctionIdentifier::integerFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t element_count = arg0_value->IntAtIndex(0, nullptr);
			
			if (element_count < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function integer() requires length to be greater than or equal to 0 (" << element_count << " supplied)." << eidos_terminate(nullptr);
			
			EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
			result = int_result;
			
			for (int64_t value_index = element_count; value_index > 0; --value_index)
				int_result->PushInt(0);
			break;
		}
			
			
			//	(logical)logical(integer$ length)
			#pragma mark logical
			
		case EidosFunctionIdentifier::logicalFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t element_count = arg0_value->IntAtIndex(0, nullptr);
			
			if (element_count < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function logical() requires length to be greater than or equal to 0 (" << element_count << " supplied)." << eidos_terminate(nullptr);
			
			EidosValue_Logical *logical_result = new EidosValue_Logical();
			result = logical_result;
			
			for (int64_t value_index = element_count; value_index > 0; --value_index)
				logical_result->PushLogical(false);
			break;
		}
			
			
			//	(object<undefined>)object(void)
			#pragma mark object
			
		case EidosFunctionIdentifier::objectFunction:
		{
			result = new EidosValue_Object_vector();
			break;
		}
			
			
			//	(integer)rbinom(integer$ n, integer size, float prob)
			#pragma mark rbinom
			
		case EidosFunctionIdentifier::rbinomFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t num_draws = arg0_value->IntAtIndex(0, nullptr);
			EidosValue *arg_size = p_arguments[1];
			EidosValue *arg_prob = p_arguments[2];
			int arg_size_count = arg_size->Count();
			int arg_prob_count = arg_prob->Count();
			bool size_singleton = (arg_size_count == 1);
			bool prob_singleton = (arg_prob_count == 1);
			
			if (num_draws < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
			if (!size_singleton && (arg_size_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires size to be of length 1 or n." << eidos_terminate(nullptr);
			if (!prob_singleton && (arg_prob_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires prob to be of length 1 or n." << eidos_terminate(nullptr);
			
			int size0 = (int)arg_size->IntAtIndex(0, nullptr);
			double probability0 = arg_prob->FloatAtIndex(0, nullptr);
			
			if (size_singleton && prob_singleton)
			{
				if (size0 < 0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires size >= 0 (" << size0 << " supplied)." << eidos_terminate(nullptr);
				if ((probability0 < 0.0) || (probability0 > 1.0))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires probability in [0.0, 1.0] (" << probability0 << " supplied)." << eidos_terminate(nullptr);
				
				if (num_draws == 1)
				{
					result = new EidosValue_Int_singleton_const(gsl_ran_binomial(gEidos_rng, probability0, size0));
				}
				else
				{
					EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
					result = int_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
						int_result->PushInt(gsl_ran_binomial(gEidos_rng, probability0, size0));
				}
			}
			else
			{
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					int size = (size_singleton ? size0 : (int)arg_size->IntAtIndex(draw_index, nullptr));
					double probability = (prob_singleton ? probability0 : arg_prob->FloatAtIndex(draw_index, nullptr));
					
					if (size < 0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires size >= 0 (" << size << " supplied)." << eidos_terminate(nullptr);
					if ((probability < 0.0) || (probability > 1.0))
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rbinom() requires probability in [0.0, 1.0] (" << probability << " supplied)." << eidos_terminate(nullptr);
					
					int_result->PushInt(gsl_ran_binomial(gEidos_rng, probability, size));
				}
			}
			
			break;
		}
			
			
			//	(*)rep(* x, integer$ count)
			#pragma mark rep
			
		case EidosFunctionIdentifier::repFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			
			int64_t rep_count = arg1_value->IntAtIndex(0, nullptr);
			
			if (rep_count < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rep() requires count to be greater than or equal to 0 (" << rep_count << " supplied)." << eidos_terminate(nullptr);
			
			// the return type depends on the type of the first argument, which will get replicated
			result = arg0_value->NewMatchingType();
			
			for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
				for (int value_idx = 0; value_idx < arg0_count; value_idx++)
					result->PushValueFromIndexOfEidosValue(value_idx, arg0_value, nullptr);
			
			break;
		}
			
			
			//	(*)repEach(* x, integer count)
			#pragma mark repEach
			
		case EidosFunctionIdentifier::repEachFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			int arg1_count = arg1_value->Count();
			
			// the return type depends on the type of the first argument, which will get replicated
			result = arg0_value->NewMatchingType();
			
			if (arg1_count == 1)
			{
				int64_t rep_count = arg1_value->IntAtIndex(0, nullptr);
				
				if (rep_count < 0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function repEach() requires count to be greater than or equal to 0 (" << rep_count << " supplied)." << eidos_terminate(nullptr);
				
				for (int value_idx = 0; value_idx < arg0_count; value_idx++)
					for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
						result->PushValueFromIndexOfEidosValue(value_idx, arg0_value, nullptr);
			}
			else if (arg1_count == arg0_count)
			{
				for (int value_idx = 0; value_idx < arg0_count; value_idx++)
				{
					int64_t rep_count = arg1_value->IntAtIndex(value_idx, nullptr);
					
					if (rep_count < 0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function repEach() requires all elements of count to be greater than or equal to 0 (" << rep_count << " supplied)." << eidos_terminate(nullptr);
					
					for (int rep_idx = 0; rep_idx < rep_count; rep_idx++)
						result->PushValueFromIndexOfEidosValue(value_idx, arg0_value, nullptr);
				}
			}
			else
			{
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function repEach() requires that parameter count's size() either (1) be equal to 1, or (2) be equal to the size() of its first argument." << eidos_terminate(nullptr);
			}
			
			break;
		}
			
			
			//	(float)rexp(integer$ n, [numeric rate])
			#pragma mark rexp
			
		case EidosFunctionIdentifier::rexpFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t num_draws = arg0_value->IntAtIndex(0, nullptr);
			EidosValue *arg_rate = ((p_argument_count >= 2) ? p_arguments[1] : nullptr);
			int arg_rate_count = (arg_rate ? arg_rate->Count() : 1);
			bool rate_singleton = (arg_rate_count == 1);
			
			if (num_draws < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rexp() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
			if (!rate_singleton && (arg_rate_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rexp() requires rate to be of length 1 or n." << eidos_terminate(nullptr);
			
			if (rate_singleton)
			{
				double rate0 = (arg_rate ? arg_rate->FloatAtIndex(0, nullptr) : 1.0);
				double mu0 = 1.0 / rate0;
				
				if (rate0 <= 0.0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rexp() requires rate > 0.0 (" << rate0 << " supplied)." << eidos_terminate(nullptr);
				
				if (num_draws == 1)
				{
					result = new EidosValue_Float_singleton_const(gsl_ran_exponential(gEidos_rng, mu0));
				}
				else
				{
					EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
					result = float_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
						float_result->PushFloat(gsl_ran_exponential(gEidos_rng, mu0));
				}
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double rate = arg_rate->FloatAtIndex(draw_index, nullptr);
					
					if (rate <= 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rexp() requires rate > 0.0 (" << rate << " supplied)." << eidos_terminate(nullptr);
					
					float_result->PushFloat(gsl_ran_exponential(gEidos_rng, 1.0 / rate));
				}
			}
			
			break;
		}
			
			
			//	(float)rnorm(integer$ n, [numeric mean], [numeric sd])
			#pragma mark rnorm
			
		case EidosFunctionIdentifier::rnormFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t num_draws = arg0_value->IntAtIndex(0, nullptr);
			EidosValue *arg_mu = ((p_argument_count >= 2) ? p_arguments[1] : nullptr);
			EidosValue *arg_sigma = ((p_argument_count >= 3) ? p_arguments[2] : nullptr);
			int arg_mu_count = (arg_mu ? arg_mu->Count() : 1);
			int arg_sigma_count = (arg_sigma ? arg_sigma->Count() : 1);
			bool mu_singleton = (arg_mu_count == 1);
			bool sigma_singleton = (arg_sigma_count == 1);
			
			if (num_draws < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rnorm() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
			if (!mu_singleton && (arg_mu_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rnorm() requires mean to be of length 1 or n." << eidos_terminate(nullptr);
			if (!sigma_singleton && (arg_sigma_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rnorm() requires sd to be of length 1 or n." << eidos_terminate(nullptr);
			
			double mu0 = (arg_mu ? arg_mu->FloatAtIndex(0, nullptr) : 0.0);
			double sigma0 = (arg_sigma ? arg_sigma->FloatAtIndex(0, nullptr) : 1.0);
			
			if (mu_singleton && sigma_singleton)
			{
				if (sigma0 < 0.0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rnorm() requires sd >= 0.0 (" << sigma0 << " supplied)." << eidos_terminate(nullptr);
				
				if (num_draws == 1)
				{
					result = new EidosValue_Float_singleton_const(gsl_ran_gaussian(gEidos_rng, sigma0) + mu0);
				}
				else
				{
					EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
					result = float_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
						float_result->PushFloat(gsl_ran_gaussian(gEidos_rng, sigma0) + mu0);
				}
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double mu = (mu_singleton ? mu0 : arg_mu->FloatAtIndex(draw_index, nullptr));
					double sigma = (sigma_singleton ? sigma0 : arg_sigma->FloatAtIndex(draw_index, nullptr));
					
					if (sigma < 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rnorm() requires sd >= 0.0 (" << sigma << " supplied)." << eidos_terminate(nullptr);
					
					float_result->PushFloat(gsl_ran_gaussian(gEidos_rng, sigma) + mu);
				}
			}
			
			break;
		}
			
			
			//	(integer)rpois(integer$ n, numeric lambda)
			#pragma mark rpois
			
		case EidosFunctionIdentifier::rpoisFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t num_draws = arg0_value->IntAtIndex(0, nullptr);
			EidosValue *arg_lambda = p_arguments[1];
			int arg_lambda_count = arg_lambda->Count();
			bool lambda_singleton = (arg_lambda_count == 1);
			
			if (num_draws < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rpois() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
			if (!lambda_singleton && (arg_lambda_count != num_draws))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rpois() requires lambda to be of length 1 or n." << eidos_terminate(nullptr);
			
			// Here we ignore USE_GSL_POISSON and always use the GSL.  This is because we don't know whether lambda (otherwise known as mu) is
			// small or large, and because we don't know what level of accuracy is demanded by whatever the user is doing with the deviates,
			// and so forth; it makes sense to just rely on the GSL for maximal accuracy and reliability.
			
			if (lambda_singleton)
			{
				double lambda0 = arg_lambda->FloatAtIndex(0, nullptr);
				
				if (lambda0 <= 0.0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rpois() requires lambda > 0.0 (" << lambda0 << " supplied)." << eidos_terminate(nullptr);
				
				if (num_draws == 1)
				{
					result = new EidosValue_Int_singleton_const(gsl_ran_poisson(gEidos_rng, lambda0));
				}
				else
				{
					EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
					result = int_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
						int_result->PushInt(gsl_ran_poisson(gEidos_rng, lambda0));
				}
			}
			else
			{
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				for (int draw_index = 0; draw_index < num_draws; ++draw_index)
				{
					double lambda = arg_lambda->FloatAtIndex(draw_index, nullptr);
					
					if (lambda <= 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function rpois() requires lambda > 0.0 (" << lambda << " supplied)." << eidos_terminate(nullptr);
					
					int_result->PushInt(gsl_ran_poisson(gEidos_rng, lambda));
				}
			}
			
			break;
		}
			
			
			//	(float)runif(integer$ n, [numeric min], [numeric max])
			#pragma mark runif
			
		case EidosFunctionIdentifier::runifFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t num_draws = arg0_value->IntAtIndex(0, nullptr);
			
			if (p_argument_count == 1)
			{
				// With no min or max, we can streamline quite a bit
				if (num_draws == 1)
				{
					result = new EidosValue_Float_singleton_const(gsl_rng_uniform(gEidos_rng));
				}
				else
				{
					if (num_draws < 0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
					
					EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
					result = float_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
						float_result->PushFloat(gsl_rng_uniform(gEidos_rng));
				}
			}
			else
			{
				EidosValue *arg_min = ((p_argument_count >= 2) ? p_arguments[1] : nullptr);
				EidosValue *arg_max = ((p_argument_count >= 3) ? p_arguments[2] : nullptr);
				int arg_min_count = (arg_min ? arg_min->Count() : 1);
				int arg_max_count = (arg_max ? arg_max->Count() : 1);
				bool min_singleton = (arg_min_count == 1);
				bool max_singleton = (arg_max_count == 1);
				
				if (num_draws < 0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires n to be greater than or equal to 0 (" << num_draws << " supplied)." << eidos_terminate(nullptr);
				if (!min_singleton && (arg_min_count != num_draws))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires min to be of length 1 or n." << eidos_terminate(nullptr);
				if (!max_singleton && (arg_max_count != num_draws))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires max to be of length 1 or n." << eidos_terminate(nullptr);
				
				double min_value0 = (arg_min ? arg_min->FloatAtIndex(0, nullptr) : 0.0);
				double max_value0 = (arg_max ? arg_max->FloatAtIndex(0, nullptr) : 1.0);
				double range0 = max_value0 - min_value0;
				
				if (min_singleton && max_singleton)
				{
					if (range0 < 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires min < max." << eidos_terminate(nullptr);
					
					if (num_draws == 1)
					{
						result = new EidosValue_Float_singleton_const(gsl_rng_uniform(gEidos_rng) * range0 + min_value0);
					}
					else
					{
						EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
						result = float_result;
						
						for (int draw_index = 0; draw_index < num_draws; ++draw_index)
							float_result->PushFloat(gsl_rng_uniform(gEidos_rng) * range0 + min_value0);
					}
				}
				else
				{
					EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
					result = float_result;
					
					for (int draw_index = 0; draw_index < num_draws; ++draw_index)
					{
						double min_value = (min_singleton ? min_value0 : arg_min->FloatAtIndex(draw_index, nullptr));
						double max_value = (max_singleton ? max_value0 : arg_max->FloatAtIndex(draw_index, nullptr));
						double range = max_value - min_value;
						
						if (range < 0.0)
							EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function runif() requires min < max." << eidos_terminate(nullptr);
						
						float_result->PushFloat(gsl_rng_uniform(gEidos_rng) * range + min_value);
					}
				}
			}
			
			break;
		}
			
			
			//	(*)sample(* x, integer size, [logical$ replace], [numeric weights])
			#pragma mark sample
			
		case EidosFunctionIdentifier::sampleFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			int64_t sample_size = p_arguments[1]->IntAtIndex(0, nullptr);
			bool replace = ((p_argument_count >= 3) ? p_arguments[2]->LogicalAtIndex(0, nullptr) : false);
			
			if (sample_size < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() requires a sample size >= 0 (" << sample_size << " supplied)." << eidos_terminate(nullptr);
			if (sample_size == 0)
			{
				result = arg0_value->NewMatchingType();
				break;
			}
			
			if (arg0_count == 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() provided with insufficient elements (0 supplied)." << eidos_terminate(nullptr);
			
			if (!replace && (arg0_count < sample_size))
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() provided with insufficient elements (" << arg0_count << " supplied, " << sample_size << " needed)." << eidos_terminate(nullptr);
			
			result = arg0_value->NewMatchingType();
			
			// the algorithm used depends on whether weights were supplied
			if (p_argument_count >= 4)
			{
				// weights supplied
				vector<double> weights_vector;
				double weights_sum = 0.0;
				EidosValue *arg3_value = p_arguments[3];
				int arg3_count = arg3_value->Count();
				
				if (arg3_count != arg0_count)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() requires x and weights to be the same length." << eidos_terminate(nullptr);
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					double weight = arg3_value->FloatAtIndex(value_index, nullptr);
					
					if (weight < 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() requires all weights to be non-negative (" << weight << " supplied)." << eidos_terminate(nullptr);
					
					weights_vector.push_back(weight);
					weights_sum += weight;
				}
				
				// get indices of x; we sample from this vector and then look up the corresponding EidosValue element
				vector<int> index_vector;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					index_vector.push_back(value_index);
				
				// do the sampling
				int64_t contender_count = arg0_count;
				
				for (int64_t samples_generated = 0; samples_generated < sample_size; ++samples_generated)
				{
					if (contender_count <= 0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() ran out of eligible elements from which to sample." << eidos_terminate(nullptr);
					if (weights_sum <= 0.0)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function sample() encountered weights summing to <= 0." << eidos_terminate(nullptr);
					
					double rose = gsl_rng_uniform(gEidos_rng) * weights_sum;
					double rose_sum = 0.0;
					int rose_index;
					
					for (rose_index = 0; rose_index < contender_count - 1; ++rose_index)	// -1 so roundoff gives the result to the last contender
					{
						rose_sum += weights_vector[rose_index];
						
						if (rose <= rose_sum)
							break;
					}
					
					result->PushValueFromIndexOfEidosValue(index_vector[rose_index], arg0_value, nullptr);
					
					if (!replace)
					{
						weights_sum -= weights_vector[rose_index];	// possible source of numerical error
						
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
						result->PushValueFromIndexOfEidosValue((int)gsl_rng_uniform_int(gEidos_rng, arg0_count), arg0_value, nullptr);
				}
				else
				{
					// get indices of x; we sample from this vector and then look up the corresponding EidosValue element
					vector<int> index_vector;
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						index_vector.push_back(value_index);
					
					// do the sampling
					int64_t contender_count = arg0_count;
					
					for (int64_t samples_generated = 0; samples_generated < sample_size; ++samples_generated)
					{
						// this error should never occur, since we checked the count above
						if (contender_count <= 0)
							EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): (internal error) function sample() ran out of eligible elements from which to sample." << eidos_terminate(nullptr);
						
						int rose_index = (int)gsl_rng_uniform_int(gEidos_rng, contender_count);
						
						result->PushValueFromIndexOfEidosValue(index_vector[rose_index], arg0_value, nullptr);
						
						index_vector.erase(index_vector.begin() + rose_index);
						--contender_count;
					}
				}
			}
			
			break;
		}
			
			
			//	(numeric)seq(numeric$ from, numeric$ to, [numeric$ by])
			#pragma mark seq
			
		case EidosFunctionIdentifier::seqFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			EidosValue *arg1_value = p_arguments[1];
			EidosValueType arg1_type = arg1_value->Type();
			EidosValue *arg2_value = ((p_argument_count == 3) ? p_arguments[2] : nullptr);
			EidosValueType arg2_type = (arg2_value ? arg2_value->Type() : EidosValueType::kValueInt);
			
			if ((arg0_type == EidosValueType::kValueFloat) || (arg1_type == EidosValueType::kValueFloat) || (arg2_type == EidosValueType::kValueFloat))
			{
				// float return case
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				double first_value = arg0_value->FloatAtIndex(0, nullptr);
				double second_value = arg1_value->FloatAtIndex(0, nullptr);
				double default_by = ((first_value < second_value) ? 1 : -1);
				double by_value = (arg2_value ? arg2_value->FloatAtIndex(0, nullptr) : default_by);
				
				if (by_value == 0.0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function seq() requires by != 0." << eidos_terminate(nullptr);
				if (((first_value < second_value) && (by_value < 0)) || ((first_value > second_value) && (by_value > 0)))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function seq() by has incorrect sign." << eidos_terminate(nullptr);
				
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
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				int64_t first_value = arg0_value->IntAtIndex(0, nullptr);
				int64_t second_value = arg1_value->IntAtIndex(0, nullptr);
				int64_t default_by = ((first_value < second_value) ? 1 : -1);
				int64_t by_value = (arg2_value ? arg2_value->IntAtIndex(0, nullptr) : default_by);
				
				if (by_value == 0)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function seq() requires by != 0." << eidos_terminate(nullptr);
				if (((first_value < second_value) && (by_value < 0)) || ((first_value > second_value) && (by_value > 0)))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function seq() by has incorrect sign." << eidos_terminate(nullptr);
				
				if (by_value > 0)
					for (int64_t seq_value = first_value; seq_value <= second_value; seq_value += by_value)
						int_result->PushInt(seq_value);
				else
					for (int64_t seq_value = first_value; seq_value >= second_value; seq_value += by_value)
						int_result->PushInt(seq_value);
			}
			
			break;
		}
			
			
			//	(integer)seqAlong(* x)
			#pragma mark seqAlong
			
		case EidosFunctionIdentifier::seqAlongFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
			result = int_result;
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				int_result->PushInt(value_index);
			break;
		}
			
			
			//	(string)string(integer$ length)
			#pragma mark string
			
		case EidosFunctionIdentifier::stringFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int64_t element_count = arg0_value->IntAtIndex(0, nullptr);
			
			if (element_count < 0)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function string() requires length to be greater than or equal to 0 (" << element_count << " supplied)." << eidos_terminate(nullptr);
			
			EidosValue_String_vector *string_result = new EidosValue_String_vector();
			result = string_result;
			
			for (int64_t value_index = element_count; value_index > 0; --value_index)
				string_result->PushString(gEidosStr_empty_string);
			break;
		}
			

		// ************************************************************************************
		//
		//	value inspection/manipulation functions
		//
#pragma mark -
#pragma mark Value inspection/manipulation functions
#pragma mark -
			
			
			//	(logical$)all(logical x)
			#pragma mark all
			
		case EidosFunctionIdentifier::allFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
			
			result = gStaticEidosValue_LogicalT;
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				if (!bool_vec[value_index])
				{
					result = gStaticEidosValue_LogicalF;
					break;
				}
			
			break;
		}
			
			
			//	(logical$)any(logical x)
			#pragma mark any
			
		case EidosFunctionIdentifier::anyFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
			
			result = gStaticEidosValue_LogicalF;
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				if (bool_vec[value_index])
				{
					result = gStaticEidosValue_LogicalT;
					break;
				}
			
			break;
		}
			
			
			//	(void)cat(* x, [string$ sep])
			#pragma mark cat
			
		case EidosFunctionIdentifier::catFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValueType arg0_type = arg0_value->Type();
			std::ostringstream &output_stream = ExecutionOutputStream();
			string separator = ((p_argument_count >= 2) ? p_arguments[1]->StringAtIndex(0, nullptr) : gEidosStr_space_string);
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
			{
				if (value_index > 0)
					output_stream << separator;
				
				if (arg0_type == EidosValueType::kValueObject)
					output_stream << *arg0_value->ObjectElementAtIndex(value_index, nullptr);
				else
					output_stream << arg0_value->StringAtIndex(value_index, nullptr);
			}
			break;
		}
			
			
			//	(logical$)identical(* x, * y)
			#pragma mark identical
			
		case EidosFunctionIdentifier::identicalFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			EidosValueType arg1_type = arg1_value->Type();
			int arg1_count = arg1_value->Count();
			
			if ((arg0_type != arg1_type) || (arg0_count != arg1_count))
			{
				result = gStaticEidosValue_LogicalF;
				break;
			}
			
			result = gStaticEidosValue_LogicalT;
			
			if (arg0_type == EidosValueType::kValueNULL)
				break;
			
			if (arg0_count == 1)
			{
				// Handle singleton comparison separately, to allow the use of the fast vector API below
				if (arg0_type == EidosValueType::kValueLogical)
				{
					if (arg0_value->LogicalAtIndex(0, nullptr) != arg1_value->LogicalAtIndex(0, nullptr))
						result = gStaticEidosValue_LogicalF;
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					if (arg0_value->IntAtIndex(0, nullptr) != arg1_value->IntAtIndex(0, nullptr))
						result = gStaticEidosValue_LogicalF;
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					if (arg0_value->FloatAtIndex(0, nullptr) != arg1_value->FloatAtIndex(0, nullptr))
						result = gStaticEidosValue_LogicalF;
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					if (arg0_value->StringAtIndex(0, nullptr) != arg1_value->StringAtIndex(0, nullptr))
						result = gStaticEidosValue_LogicalF;
				}
				else if (arg0_type == EidosValueType::kValueObject)
				{
					if (arg0_value->ObjectElementAtIndex(0, nullptr) != arg1_value->ObjectElementAtIndex(0, nullptr))
						result = gStaticEidosValue_LogicalF;
				}
			}
			else
			{
				// We have arg0_count != 1, so we can use the fast vector API; we want identical() to be very fast since it is a common bottleneck
				if (arg0_type == EidosValueType::kValueLogical)
				{
					const std::vector<bool> &bool_vec0 = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
					const std::vector<bool> &bool_vec1 = dynamic_cast<EidosValue_Logical *>(arg1_value)->LogicalVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						if (bool_vec0[value_index] != bool_vec1[value_index])
						{
							result = gStaticEidosValue_LogicalF;
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					const std::vector<int64_t> &int_vec0 = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					const std::vector<int64_t> &int_vec1 = dynamic_cast<EidosValue_Int_vector *>(arg1_value)->IntVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						if (int_vec0[value_index] != int_vec1[value_index])
						{
							result = gStaticEidosValue_LogicalF;
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					const std::vector<double> &float_vec0 = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					const std::vector<double> &float_vec1 = dynamic_cast<EidosValue_Float_vector *>(arg1_value)->FloatVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						if (float_vec0[value_index] != float_vec1[value_index])
						{
							result = gStaticEidosValue_LogicalF;
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					const std::vector<std::string> &string_vec0 = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
					const std::vector<std::string> &string_vec1 = dynamic_cast<EidosValue_String_vector *>(arg1_value)->StringVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						if (string_vec0[value_index] != string_vec1[value_index])
						{
							result = gStaticEidosValue_LogicalF;
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueObject)
				{
					const std::vector<EidosObjectElement *> &objelement_vec0 = dynamic_cast<EidosValue_Object_vector *>(arg0_value)->ObjectElementVector();
					const std::vector<EidosObjectElement *> &objelement_vec1 = dynamic_cast<EidosValue_Object_vector *>(arg1_value)->ObjectElementVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						if (objelement_vec0[value_index] != objelement_vec1[value_index])
						{
							result = gStaticEidosValue_LogicalF;
							break;
						}
				}
			}
			break;
		}
			
			
			//	(*)ifelse(logical test, * trueValues, * falseValues)
			#pragma mark ifelse
			
		case EidosFunctionIdentifier::ifelseFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
			
			EidosValue *arg1_value = p_arguments[1];
			EidosValueType arg1_type = arg1_value->Type();
			int arg1_count = arg1_value->Count();
			
			EidosValue *arg2_value = p_arguments[2];
			EidosValueType arg2_type = arg2_value->Type();
			int arg2_count = arg2_value->Count();
			
			if (arg0_count != arg1_count || arg0_count != arg2_count)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function ifelse() requires arguments of equal length." << eidos_terminate(nullptr);
			if (arg1_type != arg2_type)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function ifelse() requires arguments 2 and 3 to be the same type (" << arg1_type << " and " << arg2_type << " supplied)." << eidos_terminate(nullptr);
			
			result = arg1_value->NewMatchingType();
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
			{
				if (bool_vec[value_index])
					result->PushValueFromIndexOfEidosValue(value_index, arg1_value, nullptr);
				else
					result->PushValueFromIndexOfEidosValue(value_index, arg2_value, nullptr);
			}
			break;
		}
			
			
			//	(integer)match(* x, * y)
			#pragma mark match
			
		case EidosFunctionIdentifier::matchFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			EidosValueType arg1_type = arg1_value->Type();
			int arg1_count = arg1_value->Count();
			
			if (arg0_type != arg1_type)
				EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function match() requires arguments x and table to be the same type." << eidos_terminate(nullptr);
			
			if (arg0_type == EidosValueType::kValueNULL)
			{
				result = new EidosValue_Int_vector();
				break;
			}
			
			if ((arg0_count == 1) && (arg1_count == 1))
			{
				// Handle singleton matching separately, to allow the use of the fast vector API below
				if (arg0_type == EidosValueType::kValueLogical)
					result = new EidosValue_Int_singleton_const(arg0_value->LogicalAtIndex(0, nullptr) == arg1_value->LogicalAtIndex(0, nullptr) ? 0 : -1);
				else if (arg0_type == EidosValueType::kValueInt)
					result = new EidosValue_Int_singleton_const(arg0_value->IntAtIndex(0, nullptr) == arg1_value->IntAtIndex(0, nullptr) ? 0 : -1);
				else if (arg0_type == EidosValueType::kValueFloat)
					result = new EidosValue_Int_singleton_const(arg0_value->FloatAtIndex(0, nullptr) == arg1_value->FloatAtIndex(0, nullptr) ? 0 : -1);
				else if (arg0_type == EidosValueType::kValueString)
					result = new EidosValue_Int_singleton_const(arg0_value->StringAtIndex(0, nullptr) == arg1_value->StringAtIndex(0, nullptr) ? 0 : -1);
				else if (arg0_type == EidosValueType::kValueObject)
					result = new EidosValue_Int_singleton_const(arg0_value->ObjectElementAtIndex(0, nullptr) == arg1_value->ObjectElementAtIndex(0, nullptr) ? 0 : -1);
			}
			else if (arg0_count == 1)	// && (arg1_count != 1)
			{
				int table_index;
				
				if (arg0_type == EidosValueType::kValueLogical)
				{
					bool value0 = arg0_value->LogicalAtIndex(0, nullptr);
					const std::vector<bool> &bool_vec1 = dynamic_cast<EidosValue_Logical *>(arg1_value)->LogicalVector();
					
					for (table_index = 0; table_index < arg1_count; ++table_index)
						if (value0 == bool_vec1[table_index])
						{
							result = new EidosValue_Int_singleton_const(table_index);
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					int64_t value0 = arg0_value->IntAtIndex(0, nullptr);
					const std::vector<int64_t> &int_vec1 = dynamic_cast<EidosValue_Int_vector *>(arg1_value)->IntVector();
					
					for (table_index = 0; table_index < arg1_count; ++table_index)
						if (value0 == int_vec1[table_index])
						{
							result = new EidosValue_Int_singleton_const(table_index);
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					double value0 = arg0_value->FloatAtIndex(0, nullptr);
					const std::vector<double> &float_vec1 = dynamic_cast<EidosValue_Float_vector *>(arg1_value)->FloatVector();
					
					for (table_index = 0; table_index < arg1_count; ++table_index)
						if (value0 == float_vec1[table_index])
						{
							result = new EidosValue_Int_singleton_const(table_index);
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					std::string value0 = arg0_value->StringAtIndex(0, nullptr);
					const std::vector<std::string> &string_vec1 = dynamic_cast<EidosValue_String_vector *>(arg1_value)->StringVector();
					
					for (table_index = 0; table_index < arg1_count; ++table_index)
						if (value0 == string_vec1[table_index])
						{
							result = new EidosValue_Int_singleton_const(table_index);
							break;
						}
				}
				else if (arg0_type == EidosValueType::kValueObject)
				{
					EidosObjectElement *value0 = arg0_value->ObjectElementAtIndex(0, nullptr);
					const std::vector<EidosObjectElement *> &objelement_vec1 = dynamic_cast<EidosValue_Object_vector *>(arg1_value)->ObjectElementVector();
					
					for (table_index = 0; table_index < arg1_count; ++table_index)
						if (value0 == objelement_vec1[table_index])
						{
							result = new EidosValue_Int_singleton_const(table_index);
							break;
						}
				}
				
				if (table_index == arg1_count)
					result = new EidosValue_Int_singleton_const(-1);
			}
			else if (arg1_count == 1)	// && (arg0_count != 1)
			{
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				if (arg0_type == EidosValueType::kValueLogical)
				{
					bool value1 = arg1_value->LogicalAtIndex(0, nullptr);
					const std::vector<bool> &bool_vec0 = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						int_result->PushInt(bool_vec0[value_index] == value1 ? 0 : -1);
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					int64_t value1 = arg1_value->IntAtIndex(0, nullptr);
					const std::vector<int64_t> &int_vec0 = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						int_result->PushInt(int_vec0[value_index] == value1 ? 0 : -1);
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					double value1 = arg1_value->FloatAtIndex(0, nullptr);
					const std::vector<double> &float_vec0 = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						int_result->PushInt(float_vec0[value_index] == value1 ? 0 : -1);
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					std::string value1 = arg1_value->StringAtIndex(0, nullptr);
					const std::vector<std::string> &string_vec0 = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						int_result->PushInt(string_vec0[value_index] == value1 ? 0 : -1);
				}
				else if (arg0_type == EidosValueType::kValueObject)
				{
					EidosObjectElement *value1 = arg1_value->ObjectElementAtIndex(0, nullptr);
					const std::vector<EidosObjectElement *> &objelement_vec0 = dynamic_cast<EidosValue_Object_vector *>(arg0_value)->ObjectElementVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
						int_result->PushInt(objelement_vec0[value_index] == value1 ? 0 : -1);
				}
			}
			else						// ((arg0_count != 1) && (arg1_count != 1))
			{
				// We can use the fast vector API; we want match() to be very fast since it is a common bottleneck
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				int table_index;
				
				if (arg0_type == EidosValueType::kValueLogical)
				{
					const std::vector<bool> &bool_vec0 = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
					const std::vector<bool> &bool_vec1 = dynamic_cast<EidosValue_Logical *>(arg1_value)->LogicalVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						for (table_index = 0; table_index < arg1_count; ++table_index)
							if (bool_vec0[value_index] == bool_vec1[table_index])
								break;
						
						int_result->PushInt(table_index == arg1_count ? -1 : table_index);
					}
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					const std::vector<int64_t> &int_vec0 = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
					const std::vector<int64_t> &int_vec1 = dynamic_cast<EidosValue_Int_vector *>(arg1_value)->IntVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						for (table_index = 0; table_index < arg1_count; ++table_index)
							if (int_vec0[value_index] == int_vec1[table_index])
								break;
						
						int_result->PushInt(table_index == arg1_count ? -1 : table_index);
					}
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					const std::vector<double> &float_vec0 = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
					const std::vector<double> &float_vec1 = dynamic_cast<EidosValue_Float_vector *>(arg1_value)->FloatVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						for (table_index = 0; table_index < arg1_count; ++table_index)
							if (float_vec0[value_index] == float_vec1[table_index])
								break;
						
						int_result->PushInt(table_index == arg1_count ? -1 : table_index);
					}
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					const std::vector<std::string> &string_vec0 = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
					const std::vector<std::string> &string_vec1 = dynamic_cast<EidosValue_String_vector *>(arg1_value)->StringVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						for (table_index = 0; table_index < arg1_count; ++table_index)
							if (string_vec0[value_index] == string_vec1[table_index])
								break;
						
						int_result->PushInt(table_index == arg1_count ? -1 : table_index);
					}
				}
				else if (arg0_type == EidosValueType::kValueObject)
				{
					const std::vector<EidosObjectElement *> &objelement_vec0 = dynamic_cast<EidosValue_Object_vector *>(arg0_value)->ObjectElementVector();
					const std::vector<EidosObjectElement *> &objelement_vec1 = dynamic_cast<EidosValue_Object_vector *>(arg1_value)->ObjectElementVector();
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						for (table_index = 0; table_index < arg1_count; ++table_index)
							if (objelement_vec0[value_index] == objelement_vec1[table_index])
								break;
						
						int_result->PushInt(table_index == arg1_count ? -1 : table_index);
					}
				}
			}
			break;
		}
			
			
			//	(integer)nchar(string x)
			#pragma mark nchar
			
		case EidosFunctionIdentifier::ncharFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Int_singleton_const(arg0_value->StringAtIndex(0, nullptr).length());
			}
			else
			{
				const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
				
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					int_result->PushInt(string_vec[value_index].length());
			}
			break;
		}
			
			
			//	(string$)paste(* x, [string$ sep])
			#pragma mark paste
			
		case EidosFunctionIdentifier::pasteFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValueType arg0_type = arg0_value->Type();
			string separator = ((p_argument_count >= 2) ? p_arguments[1]->StringAtIndex(0, nullptr) : gEidosStr_space_string);
			string result_string;
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
			{
				if (value_index > 0)
					result_string.append(separator);
				
				if (arg0_type == EidosValueType::kValueObject)
				{
					std::ostringstream oss;
					
					oss << *arg0_value->ObjectElementAtIndex(value_index, nullptr);
					
					result_string.append(oss.str());
				}
				else
					result_string.append(arg0_value->StringAtIndex(value_index, nullptr));
			}
			
			result = new EidosValue_String_singleton_const(result_string);
			break;
		}
			
			
			//	(void)print(* x)
			#pragma mark print
			
		case EidosFunctionIdentifier::printFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			
			ExecutionOutputStream() << *arg0_value << endl;
			break;
		}
			
			
			//	(*)rev(* x)
			#pragma mark rev
			
		case EidosFunctionIdentifier::revFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			result = arg0_value->NewMatchingType();
			
			for (int value_index = arg0_count - 1; value_index >= 0; --value_index)
				result->PushValueFromIndexOfEidosValue(value_index, arg0_value, nullptr);
			break;
		}
			
			
			//	(integer$)size(* x)
			#pragma mark size
			
		case EidosFunctionIdentifier::sizeFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			
			result = new EidosValue_Int_singleton_const(arg0_value->Count());
			break;
		}
			
			
			//	(+)sort(+ x, [logical$ ascending])
			#pragma mark sort
			
		case EidosFunctionIdentifier::sortFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			result = arg0_value->NewMatchingType();
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				result->PushValueFromIndexOfEidosValue(value_index, arg0_value, nullptr);
			
			result->Sort((p_argument_count == 1) ? true : p_arguments[1]->LogicalAtIndex(0, nullptr));
			break;
		}
			
			
			//	(object)sortBy(object x, string$ property, [logical$ ascending])
			#pragma mark sortBy
			
		case EidosFunctionIdentifier::sortByFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue_Object_vector *object_result = new EidosValue_Object_vector();
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				object_result->PushObjectElement(arg0_value->ObjectElementAtIndex(value_index, nullptr));
			
			object_result->SortBy(p_arguments[1]->StringAtIndex(0, nullptr), (p_argument_count == 2) ? true : p_arguments[2]->LogicalAtIndex(0, nullptr));
			
			result = object_result;
			break;
		}
			
			
			//	(void)str(* x)
			#pragma mark str
			
		case EidosFunctionIdentifier::strFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			std::ostringstream &output_stream = ExecutionOutputStream();
			
			output_stream << "(" << arg0_type << ") ";
			
			if (arg0_count <= 2)
				output_stream << *arg0_value << endl;
			else
			{
				EidosValue *first_value = arg0_value->GetValueAtIndex(0, nullptr);
				EidosValue *second_value = arg0_value->GetValueAtIndex(1, nullptr);
				
				output_stream << *first_value << gEidosStr_space_string << *second_value << " ... (" << arg0_count << " values)" << endl;
				
				if (first_value->IsTemporary()) delete first_value;
				if (second_value->IsTemporary()) delete second_value;
			}
			break;
		}
			
			
			//	(string)strsplit(string$ x, [string$ sep])
			#pragma mark strsplit
			
		case EidosFunctionIdentifier::strsplitFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValue_String_vector *string_result = new EidosValue_String_vector();
			result = string_result;
			
			string joined_string = arg0_value->StringAtIndex(0, nullptr);
			string separator = ((p_argument_count >= 2) ? p_arguments[1]->StringAtIndex(0, nullptr) : gEidosStr_space_string);
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
			
			
			//	(string)substr(string x, integer first, [integer last])
			#pragma mark substr
			
		case EidosFunctionIdentifier::substrFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				const std::string &string_value = arg0_value->StringAtIndex(0, nullptr);
				string::size_type len = string_value.length();
				EidosValue *arg_first = p_arguments[1];
				int arg_first_count = arg_first->Count();
				
				if (arg_first_count != arg0_count)
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function substr() requires the size of first to be 1, or equal to the size of x." << eidos_terminate(nullptr);
				
				int64_t first0 = arg_first->IntAtIndex(0, nullptr);
				
				if (p_argument_count >= 3)
				{
					// last supplied
					EidosValue *arg_last = p_arguments[2];
					int arg_last_count = arg_last->Count();
					
					if (arg_last_count != arg0_count)
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function substr() requires the size of last to be 1, or equal to the size of x." << eidos_terminate(nullptr);
					
					int64_t last0 = arg_last->IntAtIndex(0, nullptr);
					
					int clamped_first = (int)first0;
					int clamped_last = (int)last0;
					
					if (clamped_first < 0) clamped_first = 0;
					if (clamped_last >= len) clamped_last = (int)len - 1;
					
					if ((clamped_first >= len) || (clamped_last < 0) || (clamped_first > clamped_last))
						result = new EidosValue_String_singleton_const(gEidosStr_empty_string);
					else
						result = new EidosValue_String_singleton_const(string_value.substr(clamped_first, clamped_last - clamped_first + 1));
				}
				else
				{
					// last not supplied; take substrings to the end of each string
					int clamped_first = (int)first0;
					
					if (clamped_first < 0) clamped_first = 0;
					
					if (clamped_first >= len)						
						result = new EidosValue_String_singleton_const(gEidosStr_empty_string);
					else
						result = new EidosValue_String_singleton_const(string_value.substr(clamped_first, len));
				}
			}
			else
			{
				const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
				EidosValue *arg_first = p_arguments[1];
				int arg_first_count = arg_first->Count();
				bool first_singleton = (arg_first_count == 1);
				
				if (!first_singleton && (arg_first_count != arg0_count))
					EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function substr() requires the size of first to be 1, or equal to the size of x." << eidos_terminate(nullptr);
				
				EidosValue_String_vector *string_result = new EidosValue_String_vector();
				result = string_result;
				
				int64_t first0 = arg_first->IntAtIndex(0, nullptr);
				
				if (p_argument_count >= 3)
				{
					// last supplied
					EidosValue *arg_last = p_arguments[2];
					int arg_last_count = arg_last->Count();
					bool last_singleton = (arg_last_count == 1);
					
					if (!last_singleton && (arg_last_count != arg0_count))
						EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): function substr() requires the size of last to be 1, or equal to the size of x." << eidos_terminate(nullptr);
					
					int64_t last0 = arg_last->IntAtIndex(0, nullptr);
					
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						std::string str = string_vec[value_index];
						string::size_type len = str.length();
						int clamped_first = (int)(first_singleton ? first0 : arg_first->IntAtIndex(value_index, nullptr));
						int clamped_last = (int)(last_singleton ? last0 : arg_last->IntAtIndex(value_index, nullptr));
						
						if (clamped_first < 0) clamped_first = 0;
						if (clamped_last >= len) clamped_last = (int)len - 1;
						
						if ((clamped_first >= len) || (clamped_last < 0) || (clamped_first > clamped_last))
							string_result->PushString(gEidosStr_empty_string);
						else
							string_result->PushString(str.substr(clamped_first, clamped_last - clamped_first + 1));
					}
				}
				else
				{
					// last not supplied; take substrings to the end of each string
					for (int value_index = 0; value_index < arg0_count; ++value_index)
					{
						std::string str = string_vec[value_index];
						string::size_type len = str.length();
						int clamped_first = (int)(first_singleton ? first0 : arg_first->IntAtIndex(value_index, nullptr));
						
						if (clamped_first < 0) clamped_first = 0;
						
						if (clamped_first >= len)						
							string_result->PushString(gEidosStr_empty_string);
						else
							string_result->PushString(str.substr(clamped_first, len));
					}
				}
			}
			
			break;
		}
			
			
			//	(*)unique(* x)
			#pragma mark unique
			
		case EidosFunctionIdentifier::uniqueFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = arg0_value->NewMatchingType();
			}
			else if (arg0_count == 1)
			{
				result = arg0_value->CopyValues();
			}
			else if (arg0_type == EidosValueType::kValueLogical)
			{
				const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
				bool containsF = false, containsT = false;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					if (bool_vec[value_index])
						containsT = true;
					else
						containsF = true;
				}
				
				if (containsF && !containsT)
					result = gStaticEidosValue_LogicalF;
				else if (containsT && !containsF)
					result = gStaticEidosValue_LogicalT;
				else if (!containsT && !containsF)
					result = new EidosValue_Logical();
				else	// containsT && containsF
				{
					// In this case, we need to be careful to preserve the order of occurrence
					EidosValue_Logical *logical_result = new EidosValue_Logical();
					result = logical_result;
					
					if (bool_vec[0])
					{
						logical_result->PushLogical(true);
						logical_result->PushLogical(false);
					}
					else
					{
						logical_result->PushLogical(false);
						logical_result->PushLogical(true);
					}
				}
			}
			else if (arg0_type == EidosValueType::kValueInt)
			{
				// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
				const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					int64_t value = int_vec[value_index];
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == int_vec[scan_index])
							break;
					}
					
					if (scan_index == value_index)
						int_result->PushInt(value);
				}
			}
			else if (arg0_type == EidosValueType::kValueFloat)
			{
				// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
				const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					double value = float_vec[value_index];
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == float_vec[scan_index])
							break;
					}
					
					if (scan_index == value_index)
						float_result->PushFloat(value);
				}
			}
			else if (arg0_type == EidosValueType::kValueString)
			{
				// We have arg0_count != 1, so the type of arg0_value must be EidosValue_String_vector; we can use the fast API
				const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
				EidosValue_String_vector *string_result = new EidosValue_String_vector();
				result = string_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					string value = string_vec[value_index];
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == string_vec[scan_index])
							break;
					}
					
					if (scan_index == value_index)
						string_result->PushString(value);
				}
			}
			else if (arg0_type == EidosValueType::kValueObject)
			{
				EidosValue_Object_vector *object_result = new EidosValue_Object_vector();
				result = object_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					EidosObjectElement *value = arg0_value->ObjectElementAtIndex(value_index, nullptr);
					int scan_index;
					
					for (scan_index = 0; scan_index < value_index; ++scan_index)
					{
						if (value == arg0_value->ObjectElementAtIndex(scan_index, nullptr))
							break;
					}
					
					if (scan_index == value_index)
						object_result->PushObjectElement(value);
				}
			}
			break;
		}
			
			
			//	(integer)which(logical x)
			#pragma mark which
			
		case EidosFunctionIdentifier::whichFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
			EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
			result = int_result;
			
			for (int value_index = 0; value_index < arg0_count; ++value_index)
				if (bool_vec[value_index])
					int_result->PushInt(value_index);
			break;
		}
			
			
			//	(integer)whichMax(+ x)
			#pragma mark whichMax
			
		case EidosFunctionIdentifier::whichMaxFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else
			{
				int first_index = 0;
				
				if (arg0_type == EidosValueType::kValueLogical)
				{
					bool max = arg0_value->LogicalAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
						const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							bool temp = bool_vec[value_index];
							if (max < temp) { max = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					int64_t max = arg0_value->IntAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
						const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							int64_t temp = int_vec[value_index];
							if (max < temp) { max = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					double max = arg0_value->FloatAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
						const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							double temp = float_vec[value_index];
							if (max < temp) { max = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					string max = arg0_value->StringAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_String_vector; we can use the fast API
						const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							const string &temp = string_vec[value_index];
							if (max < temp) { max = temp; first_index = value_index; }
						}
					}
				}
				
				result = new EidosValue_Int_singleton_const(first_index);
			}
			break;
		}
			
			
			//	(integer)whichMin(+ x)
			#pragma mark whichMin
			
		case EidosFunctionIdentifier::whichMinFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 0)
			{
				result = gStaticEidosValueNULL;
			}
			else
			{
				int first_index = 0;
				
				if (arg0_type == EidosValueType::kValueLogical)
				{
					bool min = arg0_value->LogicalAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
						const std::vector<bool> &bool_vec = dynamic_cast<EidosValue_Logical *>(arg0_value)->LogicalVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							bool temp = bool_vec[value_index];
							if (min > temp) { min = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueInt)
				{
					int64_t min = arg0_value->IntAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Int_vector; we can use the fast API
						const std::vector<int64_t> &int_vec = dynamic_cast<EidosValue_Int_vector *>(arg0_value)->IntVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							int64_t temp = int_vec[value_index];
							if (min > temp) { min = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueFloat)
				{
					double min = arg0_value->FloatAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_Float_vector; we can use the fast API
						const std::vector<double> &float_vec = dynamic_cast<EidosValue_Float_vector *>(arg0_value)->FloatVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							double temp = float_vec[value_index];
							if (min > temp) { min = temp; first_index = value_index; }
						}
					}
				}
				else if (arg0_type == EidosValueType::kValueString)
				{
					string min = arg0_value->StringAtIndex(0, nullptr);
					
					if (arg0_count > 1)
					{
						// We have arg0_count != 1, so the type of arg0_value must be EidosValue_String_vector; we can use the fast API
						const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg0_value)->StringVector();
						
						for (int value_index = 1; value_index < arg0_count; ++value_index)
						{
							const string &temp = string_vec[value_index];
							if (min > temp) { min = temp; first_index = value_index; }
						}
					}
				}
				
				result = new EidosValue_Int_singleton_const(first_index);
			}
			break;
		}
			
			
		// ************************************************************************************
		//
		//	value type testing/coercion functions
		//
#pragma mark -
#pragma mark Value type testing/coercion functions
#pragma mark -
			
			
			//	(float)asFloat(* x)
			#pragma mark asFloat
			
		case EidosFunctionIdentifier::asFloatFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Float_singleton_const(arg0_value->FloatAtIndex(0, nullptr));
			}
			else
			{
				EidosValue_Float_vector *float_result = new EidosValue_Float_vector();
				result = float_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					float_result->PushFloat(arg0_value->FloatAtIndex(value_index, nullptr));
			}
            break;
		}
			
			
			//	(integer)asInteger(* x)
			#pragma mark asInteger
			
		case EidosFunctionIdentifier::asIntegerFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_Int_singleton_const(arg0_value->IntAtIndex(0, nullptr));
			}
			else
			{
				EidosValue_Int_vector *int_result = new EidosValue_Int_vector();
				result = int_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					int_result->PushInt(arg0_value->IntAtIndex(value_index, nullptr));
			}
            break;
		}
			
			
			//	(logical)asLogical(* x)
			#pragma mark asLogical
			
		case EidosFunctionIdentifier::asLogicalFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = (arg0_value->LogicalAtIndex(0, nullptr) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF);
			}
			else
			{
				EidosValue_Logical *logical_result = new EidosValue_Logical();
				result = logical_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					logical_result->PushLogical(arg0_value->LogicalAtIndex(value_index, nullptr));
			}
			break;
		}
			
			
			//	(string)asString(* x)
			#pragma mark asString
			
		case EidosFunctionIdentifier::asStringFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			
			if (arg0_count == 1)
			{
				result = new EidosValue_String_singleton_const(arg0_value->StringAtIndex(0, nullptr));
			}
			else
			{
				EidosValue_String_vector *string_result = new EidosValue_String_vector();
				result = string_result;
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					string_result->PushString(arg0_value->StringAtIndex(value_index, nullptr));
			}
            break;
		}
			
			
			//	(string$)elementType(* x)
			#pragma mark elementType
			
		case EidosFunctionIdentifier::elementTypeFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			
			result = new EidosValue_String_singleton_const(arg0_value->ElementType());
			break;
		}
			
			
			//	(logical$)isFloat(* x)
			#pragma mark isFloat
			
		case EidosFunctionIdentifier::isFloatFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueFloat) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(logical$)isInteger(* x)
			#pragma mark isInteger
			
		case EidosFunctionIdentifier::isIntegerFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueInt) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(logical$)isLogical(* x)
			#pragma mark isLogical
			
		case EidosFunctionIdentifier::isLogicalFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueLogical) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(logical$)isNULL(* x)
			#pragma mark isNULL
			
		case EidosFunctionIdentifier::isNULLFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueNULL) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(logical$)isObject(* x)
			#pragma mark isObject
			
		case EidosFunctionIdentifier::isObjectFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueObject) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(logical$)isString(* x)
			#pragma mark isString
			
		case EidosFunctionIdentifier::isStringFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosValueType arg0_type = arg0_value->Type();
			
			result = (arg0_type == EidosValueType::kValueString) ? gStaticEidosValue_LogicalT : gStaticEidosValue_LogicalF;
			break;
		}
			
			
			//	(string$)type(* x)
			#pragma mark type
			
		case EidosFunctionIdentifier::typeFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			
			result = new EidosValue_String_singleton_const(StringForEidosValueType(arg0_value->Type()));
			break;
		}
			
			
		// ************************************************************************************
		//
		//	filesystem access functions
		//
#pragma mark -
#pragma mark Filesystem access functions
#pragma mark -
			
			
			//	(string)filesAtPath(string$ path, [logical$ fullPaths])
			#pragma mark filesAtPath
			
		case EidosFunctionIdentifier::filesAtPathFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			string base_path = arg0_value->StringAtIndex(0, nullptr);
			int base_path_length = (int)base_path.length();
			bool base_path_ends_in_slash = (base_path_length > 0) && (base_path[base_path_length-1] == '/');
			string path = EidosResolvedPath(base_path);
			bool fullPaths = (p_argument_count >= 2) ? p_arguments[1]->LogicalAtIndex(0, nullptr) : false;
			
			// this code modified from GNU: http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
			// I'm not sure if it works on Windows... sigh...
			DIR *dp;
			struct dirent *ep;
			
			dp = opendir(path.c_str());
			
			if (dp != NULL)
			{
				EidosValue_String_vector *string_result = new EidosValue_String_vector();
				result = string_result;
				
				while ((ep = readdir(dp)))
				{
					string filename = ep->d_name;
					
					if (fullPaths)
						filename = base_path + (base_path_ends_in_slash ? "" : "/") + filename;
					
					string_result->PushString(filename);
				}
				
				(void)closedir(dp);
			}
			else
			{
				// not a fatal error, just a warning log
				ExecutionOutputStream() << "WARNING (ExecuteFunctionCall): function filesAtPath() could not open path " << path << "." << endl;
				result = gStaticEidosValueNULL;
			}
			break;
		}
			
			
			//	(string)readFile(string$ filePath)
			#pragma mark readFile
			
		case EidosFunctionIdentifier::readFileFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			string base_path = arg0_value->StringAtIndex(0, nullptr);
			string file_path = EidosResolvedPath(base_path);
			
			// read the contents in
			std::ifstream file_stream(file_path.c_str());
			
			if (!file_stream.is_open())
			{
				// not a fatal error, just a warning log
				ExecutionOutputStream() << "WARNING (ExecuteFunctionCall): function readFile() could not read file at path " << file_path << "." << endl;
				result = gStaticEidosValueNULL;
			}
			else
			{
				EidosValue_String_vector *string_result = new EidosValue_String_vector();
				result = string_result;
				
				string line;
				
				while (getline(file_stream, line))
					string_result->PushString(line);
				
				if (file_stream.bad())
				{
					// not a fatal error, just a warning log
					ExecutionOutputStream() << "WARNING (ExecuteFunctionCall): function readFile() encountered stream errors while reading file at path " << file_path << "." << endl;
					if (string_result->IsTemporary()) delete string_result;
					
					result = gStaticEidosValueNULL;
				}
			}
			break;
		}
			
			
			//	(logical$)writeFile(string$ filePath, string contents)
			#pragma mark writeFile
			
		case EidosFunctionIdentifier::writeFileFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			string base_path = arg0_value->StringAtIndex(0, nullptr);
			string file_path = EidosResolvedPath(base_path);
			
			// the second argument is the file contents to write
			EidosValue *arg1_value = p_arguments[1];
			int arg1_count = arg1_value->Count();
			
			// write the contents out
			std::ofstream file_stream(file_path.c_str());
			
			if (!file_stream.is_open())
			{
				// Not a fatal error, just a warning log
				ExecutionOutputStream() << "WARNING (ExecuteFunctionCall): function writeFile() could not write to file at path " << file_path << "." << endl;
				result = gStaticEidosValue_LogicalF;
			}
			else
			{
				if (arg1_count == 1)
				{
					file_stream << arg1_value->StringAtIndex(0, nullptr) << endl;
				}
				else
				{
					const std::vector<std::string> &string_vec = dynamic_cast<EidosValue_String_vector *>(arg1_value)->StringVector();
					
					for (int value_index = 0; value_index < arg1_count; ++value_index)
						file_stream << string_vec[value_index] << endl;
				}
				
				if (file_stream.bad())
				{
					// Not a fatal error, just a warning log
					ExecutionOutputStream() << "WARNING (ExecuteFunctionCall): function writeFile() encountered stream errors while writing to file at path " << file_path << "." << endl;
					result = gStaticEidosValue_LogicalF;
				}
				else
				{
					result = gStaticEidosValue_LogicalT;
				}
			}
			break;
		}
			
			
		// ************************************************************************************
		//
		//	miscellaneous functions
		//
#pragma mark -
#pragma mark Miscellaneous functions
#pragma mark -
		
			
			//	(*)apply(* x, string$ lambdaSource)
#pragma mark apply
			
		case EidosFunctionIdentifier::applyFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			int arg0_count = arg0_value->Count();
			EidosValue *arg1_value = p_arguments[1];
			EidosScript script(arg1_value->StringAtIndex(0, nullptr));
			vector<EidosValue*> results;
			
			// Errors in lambdas should be reported for the lambda script, not for the calling script,
			// if possible.  In the GUI this does not work well, however; there, errors should be
			// reported as occurring in the call to apply().  Here we save off the current
			// error context and set up the error context for reporting errors inside the lambda,
			// in case that is possible; see how exceptions are handled below.
			int error_start_save = gEidosCharacterStartOfError;
			int error_end_save = gEidosCharacterEndOfError;
			EidosScript *current_script_save = gEidosCurrentScript;
			bool executing_runtime_script_save = gEidosExecutingRuntimeScript;
			
			gEidosCharacterStartOfError = -1;
			gEidosCharacterEndOfError = -1;
			gEidosCurrentScript = &script;
			gEidosExecutingRuntimeScript = true;
			
			// Tokenize, parse, and execute inside try/catch so we can handle errors well
			try
			{
				script.Tokenize();
				script.ParseInterpreterBlockToAST();
				
				EidosSymbolTable &symbols = GetSymbolTable();								// get our own symbol table
				EidosInterpreter interpreter(script, symbols, this->eidos_context_);		// give the interpreter the symbol table
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
				{
					EidosValue *apply_value = arg0_value->GetValueAtIndex(value_index, nullptr);
					
					// Set the iterator variable "applyValue" to the value; the symbol table takes ownership here
					// and set the value to external-temporary, so it is not ours any more!
					symbols.SetValueForSymbol(gEidosStr_applyValue, apply_value);
					
					// Get the result
					EidosValue *apply_result = interpreter.EvaluateInterpreterBlock(false);
					
					// If it is a temporary value, then we are fine, because that means that we own it; if it
					// is external-permanent that is also fine because that means it will live long enough; if
					// it is marked external-temporary, then that is a problem, because we need to keep it past
					// the scope where it would be valid (since it could be a reference to something in the
					// symbol table that might go away before we collate our results), so in that case we
					// make a copy that we own all by ourselves.  A specific case where this bites us is if
					// the lambda returns applyValue; that is an external-temporary reference to a symbol table
					// entry that will be freed the next time around this loop, invalidating the result pointer.
					// There could be other cases too, though, like a lambda that calls rm().  To be safe, we
					// have to do this, because the way this loop works means that values are kept for longer
					// than the guaranteed external-temporary lifetime.
					if (apply_result->IsExternalTemporary())
						apply_result = apply_result->CopyValues();
					
					results.push_back(apply_result);
				}
				
				// We do not want a leftover applyValue symbol in the symbol table, so we remove it now
				symbols.RemoveValueForSymbol(gEidosStr_applyValue, false);
				
				// Assemble all the individual results together, just as c() does
				ExecutionOutputStream() << interpreter.ExecutionOutput();
				result = ConcatenateEidosValues(results.data(), (int)results.size(), true);
				
				// Now we just need to dispose of our temporary EidosValues
				for (EidosValue *temp_value : results)
					if (temp_value->IsTemporary()) delete temp_value;
			}
			catch (std::runtime_error err)
			{
				// If exceptions throw, then we want to set up the error information to highlight the
				// apply() that failed, since we can't highlight the actual error.  (If exceptions
				// don't throw, this catch block will never be hit; exit() will already have been called
				// and the error will have been reported from the context of the lambda script string.)
				if (gEidosTerminateThrows)
				{
					gEidosCharacterStartOfError = error_start_save;
					gEidosCharacterEndOfError = error_end_save;
					gEidosCurrentScript = current_script_save;
					gEidosExecutingRuntimeScript = executing_runtime_script_save;
				}
				
				throw;
			}
			
			// Restore the normal error context in the event that no exception occurring within the lambda
			gEidosCharacterStartOfError = error_start_save;
			gEidosCharacterEndOfError = error_end_save;
			gEidosCurrentScript = current_script_save;
			gEidosExecutingRuntimeScript = executing_runtime_script_save;
			
			break;
		}
			
			
			//	(string$)date(void)
			#pragma mark date
			
		case EidosFunctionIdentifier::dateFunction:
		{
			time_t rawtime;
			struct tm *timeinfo;
			char buffer[25];	// should never be more than 10, in fact, plus a null
			
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 25, "%d-%m-%Y", timeinfo);
			
			result = new EidosValue_String_singleton_const(string(buffer));
			break;
		}
			
			
			//	(*)executeLambda(string$ lambdaSource, [logical$ timed])
			#pragma mark executeLambda
			
		case EidosFunctionIdentifier::executeLambdaFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			EidosScript script(arg0_value->StringAtIndex(0, nullptr));
			bool timed = (p_argument_count >= 2) ? p_arguments[1]->LogicalAtIndex(0, nullptr) : false;
			clock_t begin = 0, end = 0;
			
			// Errors in lambdas should be reported for the lambda script, not for the calling script,
			// if possible.  In the GUI this does not work well, however; there, errors should be
			// reported as occurring in the call to executeLambda().  Here we save off the current
			// error context and set up the error context for reporting errors inside the lambda,
			// in case that is possible; see how exceptions are handled below.
			int error_start_save = gEidosCharacterStartOfError;
			int error_end_save = gEidosCharacterEndOfError;
			EidosScript *current_script_save = gEidosCurrentScript;
			bool executing_runtime_script_save = gEidosExecutingRuntimeScript;
			
			gEidosCharacterStartOfError = -1;
			gEidosCharacterEndOfError = -1;
			gEidosCurrentScript = &script;
			gEidosExecutingRuntimeScript = true;
			
			// Tokenize, parse, and execute inside try/catch so we can handle errors well
			try
			{
				script.Tokenize();
				script.ParseInterpreterBlockToAST();
				
				EidosSymbolTable &symbols = GetSymbolTable();								// get our own symbol table
				EidosInterpreter interpreter(script, symbols, this->eidos_context_);		// give the interpreter the symbol table
				
				if (timed)
					begin = clock();
				
				result = interpreter.EvaluateInterpreterBlock(false);
				
				if (timed)
					end = clock();
				
				ExecutionOutputStream() << interpreter.ExecutionOutput();
			}
			catch (std::runtime_error err)
			{
				// If exceptions throw, then we want to set up the error information to highlight the
				// executeLambda() that failed, since we can't highlight the actual error.  (If exceptions
				// don't throw, this catch block will never be hit; exit() will already have been called
				// and the error will have been reported from the context of the lambda script string.)
				if (gEidosTerminateThrows)
				{
					gEidosCharacterStartOfError = error_start_save;
					gEidosCharacterEndOfError = error_end_save;
					gEidosCurrentScript = current_script_save;
					gEidosExecutingRuntimeScript = executing_runtime_script_save;
				}
				
				throw;
			}
			
			// Restore the normal error context in the event that no exception occurring within the lambda
			gEidosCharacterStartOfError = error_start_save;
			gEidosCharacterEndOfError = error_end_save;
			gEidosCurrentScript = current_script_save;
			gEidosExecutingRuntimeScript = executing_runtime_script_save;
			
			if (timed)
			{
				double time_spent = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
				
				ExecutionOutputStream() << "// ********** executeLambda() elapsed time: " << time_spent << std::endl;
			}
			
			break;
		}
			
			
			//	(void)function([string$ functionName])
			#pragma mark function
			
		case EidosFunctionIdentifier::functionFunction:
		{
			EidosValue *arg0_value = (p_argument_count >= 1) ? p_arguments[0] : nullptr;
			std::ostringstream &output_stream = ExecutionOutputStream();
			string match_string = (arg0_value ? arg0_value->StringAtIndex(0, nullptr) : gEidosStr_empty_string);
			bool signature_found = false;
			
			// function_map_ is already alphebetized since maps keep sorted order
			for (auto functionPairIter = function_map_->begin(); functionPairIter != function_map_->end(); ++functionPairIter)
			{
				const EidosFunctionSignature *iter_signature = functionPairIter->second;
				
				if (arg0_value && (iter_signature->function_name_.compare(match_string) != 0))
					continue;
				
				if (!arg0_value && (iter_signature->function_name_.substr(0, 1).compare("_") == 0))
					continue;	// skip internal functions that start with an underscore, unless specifically requested
				
				output_stream << *iter_signature << endl;
				signature_found = true;
			}
			
			if (arg0_value && !signature_found)
				output_stream << "No function signature found for \"" << match_string << "\"." << endl;
			
			break;
		}
			
			
			//	(void)ls(void)
			#pragma mark ls
			
		case EidosFunctionIdentifier::lsFunction:
		{
			ExecutionOutputStream() << global_symbols_;
			break;
		}
			
			
			//	(void)license(void)
			#pragma mark license
			
		case EidosFunctionIdentifier::licenseFunction:
		{
			std::ostringstream &output_stream = ExecutionOutputStream();
			
			output_stream << "Eidos is free software: you can redistribute it and/or" << endl;
			output_stream << "modify it under the terms of the GNU General Public" << endl;
			output_stream << "License as published by the Free Software Foundation," << endl;
			output_stream << "either version 3 of the License, or (at your option)" << endl;
			output_stream << "any later version." << endl << endl;
			
			output_stream << "Eidos is distributed in the hope that it will be" << endl;
			output_stream << "useful, but WITHOUT ANY WARRANTY; without even the" << endl;
			output_stream << "implied warranty of MERCHANTABILITY or FITNESS FOR" << endl;
			output_stream << "A PARTICULAR PURPOSE.  See the GNU General Public" << endl;
			output_stream << "License for more details." << endl << endl;
			
			output_stream << "You should have received a copy of the GNU General" << endl;
			output_stream << "Public License along with Eidos.  If not, see" << endl;
			output_stream << "<http://www.gnu.org/licenses/>." << endl;
			
			if (gEidosContextLicense.length())
			{
				output_stream << endl << "------------------------------------------------------" << endl << endl;
				output_stream << gEidosContextLicense << endl;
			}
			
			break;
		}
			
			
			//	(void)rm([string variableNames])
			#pragma mark rm
			
		case EidosFunctionIdentifier::rmFunction:
		{
			vector<string> symbols_to_remove;
			
			if (p_argument_count == 0)
				symbols_to_remove = global_symbols_.ReadWriteSymbols();
			else
			{
				EidosValue *arg0_value = p_arguments[0];
				int arg0_count = arg0_value->Count();
				
				for (int value_index = 0; value_index < arg0_count; ++value_index)
					symbols_to_remove.push_back(arg0_value->StringAtIndex(value_index, nullptr));
			}
			
			for (string &symbol : symbols_to_remove)
				global_symbols_.RemoveValueForSymbol(symbol, false);
			
			break;
		}
			
			
			//	(void)setSeed(integer$ seed)
			#pragma mark setSeed
			
		case EidosFunctionIdentifier::setSeedFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			
			EidosInitializeRNGFromSeed(arg0_value->IntAtIndex(0, nullptr));
			break;
		}
			
			
			//	(integer$)getSeed(void)
			#pragma mark getSeed
			
		case EidosFunctionIdentifier::getSeedFunction:
		{
			result = new EidosValue_Int_singleton_const(gEidos_rng_last_seed);
			break;
		}
			
			
			//	(void)stop([string$ message])
			#pragma mark stop
			
		case EidosFunctionIdentifier::stopFunction:
		{
			if (p_argument_count >= 1)
				ExecutionOutputStream() << p_arguments[0]->StringAtIndex(0, nullptr) << endl;
			
			EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteFunctionCall): stop() called." << eidos_terminate(nullptr);
			break;
		}
			
			
			//	(string$)time(void)
			#pragma mark time
			
		case EidosFunctionIdentifier::timeFunction:
		{
			time_t rawtime;
			struct tm *timeinfo;
			char buffer[20];		// should never be more than 8, in fact, plus a null
			
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 20, "%H:%M:%S", timeinfo);
			
			result = new EidosValue_String_singleton_const(string(buffer));
			break;
		}
			
			
			//	(void)version(void)
			#pragma mark version
			
		case EidosFunctionIdentifier::versionFunction:
		{
			std::ostringstream &output_stream = ExecutionOutputStream();
			
			output_stream << "Eidos version 1.0a1" << endl;
			
			if (gEidosContextVersion.length())
				output_stream << gEidosContextVersion << endl;
			break;
		}
			
			
		// ************************************************************************************
		//
		//	object instantiation
		//
			
			
			//	(object<_TestElement>$)_Test(integer$ yolk)
			#pragma mark _TestFunction
			
		case EidosFunctionIdentifier::_TestFunction:
		{
			EidosValue *arg0_value = p_arguments[0];
			Eidos_TestElement *testElement = new Eidos_TestElement(arg0_value->IntAtIndex(0, nullptr));
			result = new EidosValue_Object_singleton_const(testElement);
			testElement->Release();
			break;
		}
			
	}
	
	// Check the return value against the signature
	p_function_signature->CheckReturn(result);
	
	return result;
}

EidosValue *EidosInterpreter::ExecuteMethodCall(EidosValue_Object *p_method_object, EidosGlobalStringID p_method_id, EidosValue *const *const p_arguments, int p_argument_count)
{
	EidosValue *result = nullptr;
	
	// Get the function signature and check our arguments against it
	const EidosObjectClass *object_class = p_method_object->Class();
	const EidosMethodSignature *method_signature = object_class->SignatureForMethod(p_method_id);
	
	if (!method_signature)
		EIDOS_TERMINATION << "ERROR (EidosInterpreter::ExecuteMethodCall): method " << StringForEidosGlobalStringID(p_method_id) << "() is not defined on object element type " << object_class->ElementType() << "." << eidos_terminate(nullptr);
	
	method_signature->CheckArguments(p_arguments, p_argument_count);
	
	// Make the method call
	if (method_signature->is_class_method)
		result = object_class->ExecuteClassMethod(p_method_id, p_arguments, p_argument_count, *this);
	else
		result = p_method_object->ExecuteInstanceMethodOfElements(p_method_id, p_arguments, p_argument_count, *this);
	
	// Check the return value against the signature
	method_signature->CheckReturn(result);
	
	return result;
}


































































