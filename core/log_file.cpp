//
//  log_file.cpp
//  SLiM
//
//  Created by Ben Haller on 11/2/20.
//  Copyright (c) 2020-2022 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/slim/
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


#include "log_file.h"

#include <utility>
#include <algorithm>
#include <vector>
#include <iomanip>

#include "slim_globals.h"
#include "slim_sim.h"
#include "subpopulation.h"


//
//	LogFile
//
#pragma mark -
#pragma mark LogFile
#pragma mark -

LogFile::LogFile(SLiMSim &p_sim) : sim_(p_sim)
{
}

LogFile::~LogFile(void)
{
}

void LogFile::ConfigureFile(const std::string &p_filePath, std::vector<const std::string *> &p_initialContents, bool p_append, bool p_compress, const std::string &p_sep)
{
	user_file_path_ = p_filePath;
	
	// correct the user-visible path to end in ".gz" if it doesn't already
	if (p_compress && !Eidos_string_hasSuffix(user_file_path_, ".gz"))
		user_file_path_.append(".gz");
	
	// Resolve a ~ at the start of the path
	resolved_file_path_ = Eidos_ResolvedPath(user_file_path_);
	
	// Convert to an absolute path so we do not depend on the current working directory, which could change
	if ((resolved_file_path_.length() > 0) && (resolved_file_path_[0] != '/'))
	{
		std::string current_dir = Eidos_CurrentDirectory();
		size_t current_dir_length = current_dir.length();
		
		if (current_dir_length > 0)
		{
			// Figure out whether we need to append a '/' to the CWD or not; I'm not sure whether this is standard
			if (current_dir[current_dir_length - 1] == '/')
				resolved_file_path_ = current_dir + resolved_file_path_;
			else
				resolved_file_path_ = current_dir + "/" + resolved_file_path_;
		}
		else
		{
			EIDOS_TERMINATION << "ERROR (LogFile::ConfigureFile): current working directory seems to be invalid." << EidosTerminate();
		}
	}
	
	compress_ = p_compress;
	sep_ = p_sep;
	
	// We always open the file for writing (or appending) synchronously and write out the initial contents, if any
	Eidos_WriteToFile(resolved_file_path_, p_initialContents, p_append, p_compress, EidosFileFlush::kForceFlush);
}

void LogFile::SetLogInterval(bool p_autologging_enabled, int64_t p_logInterval)
{
	if (p_autologging_enabled && (p_logInterval < 1))
		EIDOS_TERMINATION << "ERROR (LogFile::SetLogInterval): the log interval must be >= 1 (or NULL, to disable automatic logging)." << EidosTerminate();
	
	autologging_enabled_ = p_autologging_enabled;
	log_interval_ = p_autologging_enabled ? p_logInterval : 0;
	autolog_start_ = sim_.Generation();
}

void LogFile::SetFlushInterval(bool p_explicit_flushing, int64_t p_flushInterval)
{
	if (p_explicit_flushing && (p_flushInterval < 1))
		EIDOS_TERMINATION << "ERROR (LogFile::SetFlushInterval): the flush interval must be >= 1 (or NULL, to request the default flushing behavior)." << EidosTerminate();
	
	explicit_flushing_ = p_explicit_flushing;
	flush_interval_ = p_flushInterval;
}

EidosValue_SP LogFile::_GeneratedValue_Generation(const LogFileGeneratorInfo &p_generator_info)
{
#pragma unused(p_generator_info)
	slim_generation_t generation = sim_.Generation();
	
	return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(generation));
}

EidosValue_SP LogFile::_GeneratedValue_GenerationStage(const LogFileGeneratorInfo &p_generator_info)
{
#pragma unused(p_generator_info)
	SLiMGenerationStage generation_stage = sim_.GenerationStage();
	std::string stage_string = StringForSLiMGenerationStage(generation_stage);
	
	return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_String_singleton(stage_string));
}

EidosValue_SP LogFile::_GeneratedValue_PopulationSexRatio(const LogFileGeneratorInfo &p_generator_info)
{
#pragma unused(p_generator_info)
	if (sim_.SexEnabled())
	{
		slim_popsize_t total_individuals = 0, total_males = 0;
		
		for (auto &subpop_iter : sim_.ThePopulation().subpops_)
		{
			Subpopulation *subpop = subpop_iter.second;
			slim_popsize_t subpop_size = subpop->CurrentSubpopSize();
			slim_popsize_t first_male_index = subpop->CurrentFirstMaleIndex();
			
			total_individuals += subpop_size;
			total_males += (subpop_size - first_male_index);
		}
		
		double sex_ratio = (total_individuals == 0) ? 0.0 : (total_males / (double)total_individuals);
		return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Float_singleton(sex_ratio));
	}
	else
	{
		// no dictionary entry, which will produce NULL
		return gStaticEidosValueNULL;
	}
}

EidosValue_SP LogFile::_GeneratedValue_PopulationSize(const LogFileGeneratorInfo &p_generator_info)
{
#pragma unused(p_generator_info)
	slim_popsize_t total_individuals = 0;
	
	for (auto &subpop_iter : sim_.ThePopulation().subpops_)
		total_individuals += (subpop_iter.second)->CurrentSubpopSize();
	
	return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(total_individuals));
}

EidosValue_SP LogFile::_GeneratedValue_SubpopulationSexRatio(const LogFileGeneratorInfo &p_generator_info)
{
	Subpopulation *subpop = sim_.SubpopulationWithID(p_generator_info.objectid_);
	
	if (sim_.SexEnabled() && subpop)
	{
		slim_popsize_t subpop_size = subpop->CurrentSubpopSize();
		slim_popsize_t first_male_index = subpop->CurrentFirstMaleIndex();
		double sex_ratio = (subpop_size == 0) ? 0.0 : ((subpop_size - first_male_index) / (double)subpop_size);
		return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Float_singleton(sex_ratio));
	}
	else
	{
		// no dictionary entry, which will produce NULL
		return gStaticEidosValueNULL;
	}
}

EidosValue_SP LogFile::_GeneratedValue_SubpopulationSize(const LogFileGeneratorInfo &p_generator_info)
{
	Subpopulation *subpop = sim_.SubpopulationWithID(p_generator_info.objectid_);
	
	if (subpop)
	{
		slim_popsize_t subpop_size = subpop->CurrentSubpopSize();
		return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(subpop_size));
	}
	else
	{
		// no dictionary entry, which will produce NULL
		return gStaticEidosValueNULL;
	}
}

EidosValue_SP LogFile::_GeneratedValue_CustomScript(const LogFileGeneratorInfo &p_generator_info)
{
	// See, e.g., Subpopulation::ApplyGlobalFitnessCallbacks() for comments on running scripts
	EidosScript *generator_script = p_generator_info.script_;
	EidosErrorContext error_context_save = gEidosErrorContext;
	gEidosErrorContext = EidosErrorContext{{-1, -1, -1, -1}, generator_script, true};
	
	EidosValue_SP result_SP;
	
	try
	{
		EidosSymbolTable callback_symbols(EidosSymbolTableType::kContextConstantsTable, &sim_.SymbolTable());
		EidosSymbolTable client_symbols(EidosSymbolTableType::kLocalVariablesTable, &callback_symbols);
		EidosFunctionMap &function_map = sim_.FunctionMap();
		EidosInterpreter interpreter(*generator_script, client_symbols, function_map, &sim_, SLIM_OUTSTREAM, SLIM_ERRSTREAM);
		
		callback_symbols.InitializeConstantSymbolEntry(gID_context, p_generator_info.context_);
		
		result_SP = interpreter.EvaluateInterpreterBlock(false, true);	// do not print output, return the last statement value
		
		if (result_SP->Type() == EidosValueType::kValueObject)
			EIDOS_TERMINATION << "ERROR (LogFile::_GeneratedValue_CustomScript): a LogFile generator script for addCustomColumn() may not return type object." << EidosTerminate(nullptr);
		if ((result_SP->Type() != EidosValueType::kValueNULL) && (result_SP->Count() != 1))
			EIDOS_TERMINATION << "ERROR (LogFile::_GeneratedValue_CustomScript): a LogFile generator script for addCustomColumn() must return a singleton value, or NULL." << EidosTerminate(nullptr);
	}
	catch (...)
	{
		if (gEidosTerminateThrows)
			gEidosErrorContext = error_context_save;
		throw;
	}
	
	gEidosErrorContext = error_context_save;
	
	return result_SP;
}

void LogFile::_GeneratedValues_CustomMeanAndSD(const LogFileGeneratorInfo &p_generator_info, EidosValue_SP *p_generated_value_1, EidosValue_SP *p_generated_value_2)
{
	// See, e.g., Subpopulation::ApplyGlobalFitnessCallbacks() for comments on running scripts
	EidosScript *generator_script = p_generator_info.script_;
	EidosErrorContext error_context_save = gEidosErrorContext;
	gEidosErrorContext = EidosErrorContext{{-1, -1, -1, -1}, generator_script, true};
	
	EidosValue_SP result_SP;
	
	try
	{
		EidosSymbolTable callback_symbols(EidosSymbolTableType::kContextConstantsTable, &sim_.SymbolTable());
		EidosSymbolTable client_symbols(EidosSymbolTableType::kLocalVariablesTable, &callback_symbols);
		EidosFunctionMap &function_map = sim_.FunctionMap();
		EidosInterpreter interpreter(*generator_script, client_symbols, function_map, nullptr, SLIM_OUTSTREAM, SLIM_ERRSTREAM);
		
		callback_symbols.InitializeConstantSymbolEntry(gID_context, p_generator_info.context_);
		
		result_SP = interpreter.EvaluateInterpreterBlock(false, true);	// do not print output, return the last statement value
		
		if ((result_SP->Type() != EidosValueType::kValueInt) && (result_SP->Type() != EidosValueType::kValueFloat) && (result_SP->Type() != EidosValueType::kValueNULL))
			EIDOS_TERMINATION << "ERROR (LogFile::_GeneratedValues_CustomMeanAndSD): a LogFile generator script for addMeanSDColumns() must return a vector of type integer or float, or NULL." << EidosTerminate(nullptr);
		
		if (result_SP->Count() == 0)
		{
			// A zero-length result vector, including NULL, will write out NA for mean and sd
			*p_generated_value_1 = gStaticEidosValueNULL;
			*p_generated_value_2 = gStaticEidosValueNULL;
		}
		else
		{
			// A non-zero result vector gets evaluated for its mean and sd (sd==NA if length 1)
			// We just use eidos_functions here, since it does exactly what we want anyway
			std::vector<EidosValue_SP> argument_vec;
			
			argument_vec.emplace_back(result_SP);
			
			if (result_SP->Count() == 1)
			{
				*p_generated_value_1 = Eidos_ExecuteFunction_mean(argument_vec, interpreter);
				*p_generated_value_2 = gStaticEidosValueNULL;
			}
			else
			{
				*p_generated_value_1 = Eidos_ExecuteFunction_mean(argument_vec, interpreter);
				*p_generated_value_2 = Eidos_ExecuteFunction_sd(argument_vec, interpreter);
			}
		}
	}
	catch (...)
	{
		if (gEidosTerminateThrows)
			gEidosErrorContext = error_context_save;
		throw;
	}
	
	gEidosErrorContext = error_context_save;
}

void LogFile::_OutputValue(std::ostringstream &p_out, EidosValue *p_value)
{
	EidosValueType type = p_value->Type();
	
	if (type == EidosValueType::kValueNULL)
	{
		// NULL gets logged as NA; mixes paradigms a bit, but seems useful
		p_out << "NA";
	}
	else
	{
		// Use EidosValue to write the value.  However, we want to control the precision of float output.
		// Note that this is not thread-safe.
		int old_precision = gEidosFloatOutputPrecision;
		gEidosFloatOutputPrecision = float_precision_;
		
		p_out << *p_value;			// FIXME this doesn't handle string quoting well at present
		
		gEidosFloatOutputPrecision = old_precision;
	}
}

void LogFile::AppendNewRow(void)
{
	std::vector<const std::string *> line_vec;
	std::string header_line;
	std::string row_line;
	
	// Gather all generators into our Dictionary
	RemoveAllKeys();
	
	// Generate the header row if needed
	if (!header_logged_)
	{
		std::ostringstream ss;
		bool first_column = true;
		
#ifdef SLIMGUI
		std::vector<std::string> gui_line;
#endif
		
		for (const std::string &column_name : column_names_)
		{
			if (!first_column)
				ss << sep_;
			first_column = false;
			ss << column_name;
			
#ifdef SLIMGUI
			std::ostringstream gui_ss;
			gui_ss << column_name;
			gui_line.emplace_back(gui_ss.str());
#endif
		}
		
		header_line = ss.str();
		line_vec.emplace_back(&header_line);
		
#ifdef SLIMGUI
		emitted_lines_.emplace_back(std::move(gui_line));
#endif
		
		// Having emitted the header line, we lock ourselves to prevent inconsistencies in the emitted table
		header_logged_ = true;
	}
	
	// Generate the text of the row from the Dictionary entries
	{
		std::ostringstream ss;
		int column_index = 0;
		
#ifdef SLIMGUI
		std::vector<std::string> gui_line;
#endif
		
		for (const LogFileGeneratorInfo &generator : generator_info_)
		{
			EidosValue_SP generated_value;
			
			switch (generator.type_)
			{
				case LogFileGeneratorType::kGenerator_Generation:
					generated_value = _GeneratedValue_Generation(generator);
					break;
				case LogFileGeneratorType::kGenerator_GenerationStage:
					generated_value = _GeneratedValue_GenerationStage(generator);
					break;
				case LogFileGeneratorType::kGenerator_PopulationSexRatio:
					generated_value = _GeneratedValue_PopulationSexRatio(generator);
					break;
				case LogFileGeneratorType::kGenerator_PopulationSize:
					generated_value = _GeneratedValue_PopulationSize(generator);
					break;
				case LogFileGeneratorType::kGenerator_SubpopulationSexRatio:
					generated_value = _GeneratedValue_SubpopulationSexRatio(generator);
					break;
				case LogFileGeneratorType::kGenerator_SubpopulationSize:
					generated_value = _GeneratedValue_SubpopulationSize(generator);
					break;
				case LogFileGeneratorType::kGenerator_CustomScript:
					generated_value = _GeneratedValue_CustomScript(generator);
					break;
				case LogFileGeneratorType::kGenerator_CustomMeanAndSD:
				{
					// This requires special-casing because it generates two columns
					EidosValue_SP generated_value_1, generated_value_2;
					
					_GeneratedValues_CustomMeanAndSD(generator, &generated_value_1, &generated_value_2);
					
					// emit generated_value_1
					if (column_index != 0)
						ss << sep_;
					
					_OutputValue(ss, generated_value_1.get());
					
#ifdef SLIMGUI
					std::ostringstream gui_ss;
					_OutputValue(gui_ss, generated_value_1.get());
					gui_line.emplace_back(gui_ss.str());
#endif
					
					if (generated_value_1->Type() != EidosValueType::kValueNULL)
						SetKeyValue(column_names_[column_index], std::move(generated_value_1));
					
					column_index++;
					
					// let the code below emit generated_value_2
					generated_value = generated_value_2;

					break;
				}
			}
			
			// Emit the generated value and add it to our Dictionary state
			if (column_index != 0)
				ss << sep_;
			
			_OutputValue(ss, generated_value.get());
			
#ifdef SLIMGUI
			std::ostringstream gui_ss;
			_OutputValue(gui_ss, generated_value.get());
			gui_line.emplace_back(gui_ss.str());
#endif
			
			if (generated_value->Type() != EidosValueType::kValueNULL)
				SetKeyValue(column_names_[column_index], std::move(generated_value));
			
			column_index++;
		}
		
		row_line = ss.str();
		line_vec.emplace_back(&row_line);
		
#ifdef SLIMGUI
		emitted_lines_.emplace_back(std::move(gui_line));
#endif
	}
	
	ContentsChanged("LogFile::AppendNewRow()");
	
	// Write out the row
	EidosFileFlush flush = EidosFileFlush::kDefaultFlush;
	
	if (explicit_flushing_)
	{
		unflushed_row_count_++;
		
		if (unflushed_row_count_ >= flush_interval_)
		{
			flush = EidosFileFlush::kForceFlush;
			unflushed_row_count_ = 0;
		}
		else
		{
			flush = EidosFileFlush::kNoFlush;
		}
	}
	
	Eidos_WriteToFile(resolved_file_path_, line_vec, true, compress_, flush);
}

void LogFile::GenerationEndCallout(void)
{
	if (autologging_enabled_)
	{
		slim_generation_t generation = sim_.Generation();
		
		if ((generation - autolog_start_) % log_interval_ == 0)
			AppendNewRow();
	}
}

EidosValue_SP LogFile::AllKeys(void) const
{
	// We want to return the column names in order, so we have to override EidosDictionaryUnretained here
	// Our column_names_ vector should correspond to EidosDictionaryUnretained's state, just with a fixed order
	if (!header_logged_)
		return gStaticEidosValue_String_ZeroVec;
	
	int column_count = (int)column_names_.size();
	EidosValue_String_vector *string_result = (new (gEidosValuePool->AllocateChunk()) EidosValue_String_vector())->Reserve(column_count);
	
	for (const std::string &column_name : column_names_)
		string_result->PushString(column_name);
	
	return EidosValue_SP(string_result);
}

const EidosClass *LogFile::Class(void) const
{
	return gSLiM_LogFile_Class;
}

void LogFile::Print(std::ostream &p_ostream) const
{
	p_ostream << Class()->ClassName() << "<" << user_file_path_ << ">";
}

EidosValue_SP LogFile::GetProperty(EidosGlobalStringID p_property_id)
{
	// All of our strings are in the global registry, so we can require a successful lookup
	switch (p_property_id)
	{
			// constants
		//case gEidosID_allKeys:	// not technically overridden here, but we override AllKeys() to provide new behavior
		case gEidosID_filePath:
			return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_String_singleton(user_file_path_));
		case gID_logInterval:
			return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(log_interval_));
			
			// variables
		case gID_precision:
			return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(float_precision_));
		case gID_tag:
		{
			slim_usertag_t tag_value = tag_value_;
			
			if (tag_value == SLIM_TAG_UNSET_VALUE)
				EIDOS_TERMINATION << "ERROR (LogFile::GetProperty): property tag accessed on simulation object before being set." << EidosTerminate();
			
			return EidosValue_SP(new (gEidosValuePool->AllocateChunk()) EidosValue_Int_singleton(tag_value));
		}
			
			// all others, including gID_none
		default:
			return super::GetProperty(p_property_id);
	}
}

void LogFile::SetProperty(EidosGlobalStringID p_property_id, const EidosValue &p_value)
{
	// All of our strings are in the global registry, so we can require a successful lookup
	switch (p_property_id)
	{
		case gID_precision:
		{
			int64_t value = p_value.IntAtIndex(0, nullptr);
			
			if ((value < 1) || (value > 22))
				EIDOS_TERMINATION << "ERROR (LogFile::SetProperty): property precision must be in [1,22]." << EidosTerminate();
			
			float_precision_ = (int)value;
			
			return;
		}
		case gID_tag:
		{
			slim_usertag_t value = SLiMCastToUsertagTypeOrRaise(p_value.IntAtIndex(0, nullptr));
			
			tag_value_ = value;
			return;
		}
		
		default:
		{
			return super::SetProperty(p_property_id, p_value);
		}
	}
}

EidosValue_SP LogFile::ExecuteInstanceMethod(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
	switch (p_method_id)
	{
			// our own methods
		case gID_addCustomColumn:				return ExecuteMethod_addCustomColumn(p_method_id, p_arguments, p_interpreter);
		case gID_addGeneration:					return ExecuteMethod_addGeneration(p_method_id, p_arguments, p_interpreter);
		case gID_addGenerationStage:			return ExecuteMethod_addGenerationStage(p_method_id, p_arguments, p_interpreter);
		case gID_addMeanSDColumns:				return ExecuteMethod_addMeanSDColumns(p_method_id, p_arguments, p_interpreter);
		case gID_addPopulationSexRatio:			return ExecuteMethod_addPopulationSexRatio(p_method_id, p_arguments, p_interpreter);
		case gID_addPopulationSize:				return ExecuteMethod_addPopulationSize(p_method_id, p_arguments, p_interpreter);
		case gID_addSubpopulationSexRatio:		return ExecuteMethod_addSubpopulationSexRatio(p_method_id, p_arguments, p_interpreter);
		case gID_addSubpopulationSize:			return ExecuteMethod_addSubpopulationSize(p_method_id, p_arguments, p_interpreter);
		case gID_flush:							return ExecuteMethod_flush(p_method_id, p_arguments, p_interpreter);
		case gID_logRow:						return ExecuteMethod_logRow(p_method_id, p_arguments, p_interpreter);
		case gID_setLogInterval:				return ExecuteMethod_setLogInterval(p_method_id, p_arguments, p_interpreter);
		case gID_setFilePath:					return ExecuteMethod_setFilePath(p_method_id, p_arguments, p_interpreter);
			
			// overrides from Dictionary
		case gEidosID_addKeysAndValuesFrom:		return ExecuteMethod_addKeysAndValuesFrom(p_method_id, p_arguments, p_interpreter);
		case gEidosID_appendKeysAndValuesFrom:	return ExecuteMethod_appendKeysAndValuesFrom(p_method_id, p_arguments, p_interpreter);
		case gEidosID_clearKeysAndValues:		return ExecuteMethod_clearKeysAndValues(p_method_id, p_arguments, p_interpreter);
		case gEidosID_setValue:					return ExecuteMethod_setValue(p_method_id, p_arguments, p_interpreter);
		default:								return super::ExecuteInstanceMethod(p_method_id, p_arguments, p_interpreter);
	}
}

void LogFile::RaiseForLockedHeader(const std::string &p_caller_name)
{
	EIDOS_TERMINATION << "ERROR (" << p_caller_name << "): this LogFile has already emitted its header line, so new data generators cannot be added." << EidosTerminate(nullptr);
}

//	*********************	- (void)addCustomColumn(string$ columnName, string$ source, [* context = NULL])
EidosValue_SP LogFile::ExecuteMethod_addCustomColumn(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addCustomColumn");
	
	EidosValue_String *columnName_value = (EidosValue_String *)p_arguments[0].get();
	EidosValue_String *source_value = (EidosValue_String *)p_arguments[1].get();
	EidosValue_SP context_value = p_arguments[2];
	
	const std::string &column_name = columnName_value->StringRefAtIndex(0, nullptr);
	const std::string &source = source_value->StringRefAtIndex(0, nullptr);
	
	// See, e.g., Subpopulation::ApplyGlobalFitnessCallbacks() for comments on parsing/running script blocks
	EidosErrorContext error_context_save = gEidosErrorContext;
	EidosScript *source_script = new EidosScript(source, -1);
	
	gEidosErrorContext = EidosErrorContext{{-1, -1, -1, -1}, source_script, true};
	
	try {
		source_script->Tokenize();
		source_script->ParseInterpreterBlockToAST(false);
	}
	catch (...)
	{
		if (gEidosTerminateThrows)
			gEidosErrorContext = error_context_save;
		
		delete source_script;
		source_script = nullptr;
		
		EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_addCustomColumn): tokenize/parse error in script for addCustomColumn()." << EidosTerminate();
	}
	
	gEidosErrorContext = error_context_save;
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_CustomScript, source_script, -1, context_value});
	column_names_.emplace_back(column_name);
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addGeneration()
EidosValue_SP LogFile::ExecuteMethod_addGeneration(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addGeneration");
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_Generation, nullptr, -1, EidosValue_SP()});
	
	column_names_.emplace_back("generation");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addGenerationStage()
EidosValue_SP LogFile::ExecuteMethod_addGenerationStage(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addGenerationStage");
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_GenerationStage, nullptr, -1, EidosValue_SP()});
	column_names_.emplace_back("gen_stage");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addMeanSDColumns(string$ columnName, string$ source, [* context = NULL])
EidosValue_SP LogFile::ExecuteMethod_addMeanSDColumns(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addMeanSDColumns");
	
	EidosValue_String *columnName_value = (EidosValue_String *)p_arguments[0].get();
	EidosValue_String *source_value = (EidosValue_String *)p_arguments[1].get();
	EidosValue_SP context_value = p_arguments[2];
	
	const std::string &column_name = columnName_value->StringRefAtIndex(0, nullptr);
	const std::string &source = source_value->StringRefAtIndex(0, nullptr);
	
	EidosErrorContext error_context_save = gEidosErrorContext;
	EidosScript *source_script = new EidosScript(source, -1);
	
	gEidosErrorContext = EidosErrorContext{{-1, -1, -1, -1}, source_script, true};
	
	try {
		source_script->Tokenize();
		source_script->ParseInterpreterBlockToAST(false);
	}
	catch (...)
	{
		if (gEidosTerminateThrows)
			gEidosErrorContext = error_context_save;
		
		delete source_script;
		source_script = nullptr;
		
		EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_addCustomColumn): tokenize/parse error in script for addMeanSDColumns()." << EidosTerminate();
	}
	
	gEidosErrorContext = error_context_save;
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_CustomMeanAndSD, source_script, -1, context_value});
	column_names_.emplace_back(column_name + "_mean");
	column_names_.emplace_back(column_name + "_sd");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addPopulationSexRatio()
EidosValue_SP LogFile::ExecuteMethod_addPopulationSexRatio(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addPopulationSexRatio");
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_PopulationSexRatio, nullptr, -1, EidosValue_SP()});
	column_names_.emplace_back("sex_ratio");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addPopulationSize()
EidosValue_SP LogFile::ExecuteMethod_addPopulationSize(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addPopulationSize");
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_PopulationSize, nullptr, -1, EidosValue_SP()});
	column_names_.emplace_back("num_individuals");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addSubpopulationSexRatio(io<Subpopulation>$ subpop)
EidosValue_SP LogFile::ExecuteMethod_addSubpopulationSexRatio(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addSubpopulationSexRatio");
	
	// Extract the subpopulation id; we allow reference to nonexistent subpopulations, which is unusual, so there's no function to use
	EidosValue_SP subpop_value = p_arguments[0];
	slim_objectid_t subpop_id;
	
	if (subpop_value->Type() == EidosValueType::kValueInt)
	{
		subpop_id = SLiMCastToObjectidTypeOrRaise(subpop_value->IntAtIndex(0, nullptr));
	}
	else
	{
#if DEBUG
		// Use dynamic_cast<> only in DEBUG since it is hella slow
		// the class of the object here should be guaranteed by the caller anyway
		Subpopulation *subpop = dynamic_cast<Subpopulation *>(subpop_value->ObjectElementAtIndex(0, nullptr));
#else
		Subpopulation *subpop = (Subpopulation *)(subpop_value->ObjectElementAtIndex(0, nullptr));
#endif
		
		subpop_id = subpop->subpopulation_id_;
	}
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_SubpopulationSexRatio, nullptr, subpop_id, EidosValue_SP()});
	column_names_.emplace_back(SLiMEidosScript::IDStringWithPrefix('p', subpop_id) + "_sex_ratio");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)addSubpopulationSize(io<Subpopulation>$ subpop)
EidosValue_SP LogFile::ExecuteMethod_addSubpopulationSize(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_interpreter)
	if (header_logged_)
		RaiseForLockedHeader("LogFile::ExecuteMethod_addSubpopulationSize");
	
	// Extract the subpopulation id; we allow reference to nonexistent subpopulations, which is unusual, so there's no function to use
	EidosValue_SP subpop_value = p_arguments[0];
	slim_objectid_t subpop_id;
	
	if (subpop_value->Type() == EidosValueType::kValueInt)
	{
		subpop_id = SLiMCastToObjectidTypeOrRaise(subpop_value->IntAtIndex(0, nullptr));
	}
	else
	{
#if DEBUG
		// Use dynamic_cast<> only in DEBUG since it is hella slow
		// the class of the object here should be guaranteed by the caller anyway
		Subpopulation *subpop = dynamic_cast<Subpopulation *>(subpop_value->ObjectElementAtIndex(0, nullptr));
#else
		Subpopulation *subpop = (Subpopulation *)(subpop_value->ObjectElementAtIndex(0, nullptr));
#endif
		
		subpop_id = subpop->subpopulation_id_;
	}
	
	generator_info_.emplace_back(LogFileGeneratorInfo{LogFileGeneratorType::kGenerator_SubpopulationSize, nullptr, subpop_id, EidosValue_SP()});
	column_names_.emplace_back(SLiMEidosScript::IDStringWithPrefix('p', subpop_id) + "_num_individuals");
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)flush(void)
EidosValue_SP LogFile::ExecuteMethod_flush(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	Eidos_FlushFile(resolved_file_path_);
	unflushed_row_count_ = 0;
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)logRow(void)
EidosValue_SP LogFile::ExecuteMethod_logRow(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	AppendNewRow();
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)setLogInterval([Ni$ logInterval = NULL])
EidosValue_SP LogFile::ExecuteMethod_setLogInterval(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_interpreter)
	EidosValue *logInterval_value = p_arguments[0].get();
	bool autologging = false;
	int64_t logInterval = 0;
	
	if (logInterval_value->Type() == EidosValueType::kValueNULL)
	{
		// NULL turns off autologging
		autologging = false;
		logInterval = 0;
	}
	else
	{
		autologging = true;
		logInterval = logInterval_value->IntAtIndex(0, nullptr);
	}
	
	SetLogInterval(autologging, logInterval);
	
	return gStaticEidosValueVOID;
}

//	*********************	- (void)setFilePath(string$ filePath, [Ns initialContents = NULL], [logical$ append = F], [Nl$ compress = NULL], [Ns$ sep = NULL])
EidosValue_SP LogFile::ExecuteMethod_setFilePath(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_interpreter)
	EidosValue_String *filePath_value = (EidosValue_String *)p_arguments[0].get();
	EidosValue *initialContents_value = p_arguments[1].get();
	EidosValue *append_value = p_arguments[2].get();
	EidosValue *compress_value = p_arguments[3].get();
	EidosValue_String *sep_value = (EidosValue_String *)p_arguments[4].get();
	
	// Note that the parameters and their interpretation is different from SLiMSim::ExecuteMethod_createLogFile();
	// in particular, NULL here means "keep the existing value"
	const std::string &filePath = filePath_value->StringRefAtIndex(0, nullptr);
	std::vector<const std::string *> initialContents;
	bool append = append_value->LogicalAtIndex(0, nullptr);
	bool do_compress = compress_;
	std::string sep = sep_;
	
	if (initialContents_value->Type() != EidosValueType::kValueNULL)
	{
		EidosValue_String *ic_string_value = (EidosValue_String *)initialContents_value;
		int ic_count = initialContents_value->Count();
		
		for (int ic_index = 0; ic_index < ic_count; ++ic_index)
			initialContents.emplace_back(&ic_string_value->StringRefAtIndex(ic_index, nullptr));
	}
	
	if (compress_value->Type() != EidosValueType::kValueNULL)
		do_compress = compress_value->LogicalAtIndex(0, nullptr);
	
	if (sep_value->Type() != EidosValueType::kValueNULL)
		sep = sep_value->StringRefAtIndex(0, nullptr);
	
	ConfigureFile(filePath, initialContents, append, do_compress, sep);
	
	return gStaticEidosValueVOID;
}

EidosValue_SP LogFile::ExecuteMethod_addKeysAndValuesFrom(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_addKeysAndValuesFrom): LogFile manages its dictionary entries; they cannot be modified by the user." << EidosTerminate(nullptr);
}

EidosValue_SP LogFile::ExecuteMethod_appendKeysAndValuesFrom(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_appendKeysAndValuesFrom): LogFile manages its dictionary entries; they cannot be modified by the user." << EidosTerminate(nullptr);
}

EidosValue_SP LogFile::ExecuteMethod_clearKeysAndValues(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_clearKeysAndValues): LogFile manages its dictionary entries; they cannot be modified by the user." << EidosTerminate(nullptr);
}

EidosValue_SP LogFile::ExecuteMethod_setValue(EidosGlobalStringID p_method_id, const std::vector<EidosValue_SP> &p_arguments, EidosInterpreter &p_interpreter)
{
#pragma unused (p_method_id, p_arguments, p_interpreter)
	EIDOS_TERMINATION << "ERROR (LogFile::ExecuteMethod_setValue): LogFile manages its dictionary entries; they cannot be modified by the user." << EidosTerminate(nullptr);
}


//
//	LogFile_Class
//
#pragma mark -
#pragma mark LogFile_Class
#pragma mark -

EidosClass *gSLiM_LogFile_Class = nullptr;


const std::vector<EidosPropertySignature_CSP> *LogFile_Class::Properties(void) const
{
	static std::vector<EidosPropertySignature_CSP> *properties = nullptr;
	
	if (!properties)
	{
		properties = new std::vector<EidosPropertySignature_CSP>(*super::Properties());
		
		properties->emplace_back((EidosPropertySignature *)(new EidosPropertySignature(gEidosStr_filePath,			true,	kEidosValueMaskString | kEidosValueMaskSingleton)));
		properties->emplace_back((EidosPropertySignature *)(new EidosPropertySignature(gStr_logInterval,			true,	kEidosValueMaskInt | kEidosValueMaskSingleton)));
		properties->emplace_back((EidosPropertySignature *)(new EidosPropertySignature(gStr_tag,					false,	kEidosValueMaskInt | kEidosValueMaskSingleton)));
		properties->emplace_back((EidosPropertySignature *)(new EidosPropertySignature(gStr_precision,				false,	kEidosValueMaskInt | kEidosValueMaskSingleton)));
		
		std::sort(properties->begin(), properties->end(), CompareEidosPropertySignatures);
	}
	
	return properties;
}

const std::vector<EidosMethodSignature_CSP> *LogFile_Class::Methods(void) const
{
	static std::vector<EidosMethodSignature_CSP> *methods = nullptr;
	
	if (!methods)
	{
		methods = new std::vector<EidosMethodSignature_CSP>(*super::Methods());
		
		// our own methods
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addCustomColumn, kEidosValueMaskVOID))->AddString_S("columnName")->AddString_S(gEidosStr_source)->AddAny_O("context", gStaticEidosValueNULL));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addGeneration, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addGenerationStage, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addMeanSDColumns, kEidosValueMaskVOID))->AddString_S("columnName")->AddString_S(gEidosStr_source)->AddAny_O("context", gStaticEidosValueNULL));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addPopulationSexRatio, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addPopulationSize, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addSubpopulationSexRatio, kEidosValueMaskVOID))->AddIntObject_S(gStr_subpop, gSLiM_Subpopulation_Class));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_addSubpopulationSize, kEidosValueMaskVOID))->AddIntObject_S(gStr_subpop, gSLiM_Subpopulation_Class));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_flush, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_logRow, kEidosValueMaskVOID)));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_setLogInterval, kEidosValueMaskVOID))->AddInt_OSN("logInterval", gStaticEidosValueNULL));
		methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gStr_setFilePath, kEidosValueMaskVOID))->AddString_S(gEidosStr_filePath)->AddString_ON("initialContents", gStaticEidosValueNULL)->AddLogical_OS("append", gStaticEidosValue_LogicalF)->AddLogical_OSN("compress", gStaticEidosValueNULL)->AddString_OSN("sep", gStaticEidosValueNULL));
		
		// overrides of Dictionary methods; these should not be declared again, to avoid a duplicate in the methods table
		//methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gEidosStr_addKeysAndValuesFrom, kEidosValueMaskVOID))->AddObject_S(gEidosStr_source, nullptr));
		//methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gEidosStr_appendKeysAndValuesFrom, kEidosValueMaskVOID))->AddObject(gEidosStr_source, nullptr));
		//methods->emplace_back((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gEidosStr_clearKeysAndValues, kEidosValueMaskVOID)));
		//methods->emplace_back(((EidosInstanceMethodSignature *)(new EidosInstanceMethodSignature(gEidosStr_setValue, kEidosValueMaskVOID))->AddString_S("key")->AddAny("value")));
		
		std::sort(methods->begin(), methods->end(), CompareEidosCallSignatures);
	}
	
	return methods;
}



































