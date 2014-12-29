//
//  substitution.cpp
//  SLiM
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014 Philipp Messer.  All rights reserved.
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


#include "substitution.h"

#include <iostream>


Substitution::Substitution(const Mutation &p_mutation, int p_fixation_time) :
	mutation_type_ptr_(p_mutation.mutation_type_ptr_), position_(p_mutation.position_), selection_coeff_(p_mutation.selection_coeff_), subpop_index_(p_mutation.subpop_index_), generation_(p_mutation.generation_), fixation_time_(p_fixation_time)
{
}

void Substitution::print() const
{ 
	std::cout << " m" << mutation_type_ptr_->mutation_type_id_ << " " << position_+1 << " " << selection_coeff_ << " " << mutation_type_ptr_->dominance_coeff_ << " p" << subpop_index_ << " " << generation_ << " "<< fixation_time_ << std::endl; 
}




































































