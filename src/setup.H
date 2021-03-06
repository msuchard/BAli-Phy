/*
   Copyright (C) 2004-2007 Benjamin Redelings

This file is part of BAli-Phy.

BAli-Phy is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

BAli-Phy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with BAli-Phy; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef SETUP_H
#define SETUP_H

#include <vector>
#include <valarray>
#include "smodel.H"
#include "imodel.H"
#include "alignment.H"
#include "sequencetree.H"
#include "clone.H"
#include <boost/program_options.hpp>

/// Reorder internal sequences of A to correspond to standardized node names for T
alignment standardize(const alignment& A, const SequenceTree& T);

/// Estimate the empirical frequencies of different letters from the alignment, with pseudocounts
std::valarray<double> empirical_frequencies(const boost::program_options::variables_map& args,
					    const alignment& A);

/// Estimate the empirical frequencies of different letters from the alignment, with pseudocounts
std::valarray<double> empirical_frequencies(const boost::program_options::variables_map& args,
					    const vector<alignment>& A);

//----------------- Load trees and alignments ---------------------//


void remap_T_indices(SequenceTree& T1, const vector<std::string>& names);
void remap_T_indices(SequenceTree& T1, const alignment& T2);
void remap_T_indices(SequenceTree& T1, const SequenceTree& T2);

/// Map leaf nodes of T to the leaf sequences of A
void link(alignment& A,SequenceTree& T,bool internal_sequences=true);

/// Map leaf nodes of T to the leaf sequences of A
void link(alignment& A,RootedSequenceTree& T,bool internal_sequences=true);

void load_As_and_T(const boost::program_options::variables_map& args,
		   std::vector<alignment>& A,SequenceTree& T,bool internal_sequences=true);

void load_As_and_T(const boost::program_options::variables_map& args,
		   std::vector<alignment>& A,RootedSequenceTree& T,bool internal_sequences=true);
/// Load an alignment A, generate a random SequenceTree T, and then link them.
void load_As_and_random_T(const boost::program_options::variables_map& args,
			  std::vector<alignment>& A,SequenceTree& T,bool internal_sequences=true);


/// Load and link() an alignment A and SequenceTree T
void load_A_and_T(const boost::program_options::variables_map& args,
		  alignment& A,SequenceTree& T,bool internal_sequences=true);

/// Load and link() an alignment A and SequenceTree T
void load_A_and_T(const boost::program_options::variables_map& args,
		  alignment& A,RootedSequenceTree& T,bool internal_sequences=true);

/// Load an alignment A, generate a random SequenceTree T, and then link them.
void load_A_and_random_T(const boost::program_options::variables_map& args,
			 alignment& A,SequenceTree& T,bool internal_sequences=true);

SequenceTree load_constraint_tree(const std::string& filename,const std::vector<string>&);


/// Count how many times each letter occurs in A
std::valarray<double> letter_counts(const alignment& A);

/// Count how many times letter 'l' occurss in A (including *,-,?)
int letter_count(const alignment& A,int l);

/// Load statements from a config file.
void load_bali_phy_rc(boost::program_options::variables_map&,
		      const boost::program_options::options_description&);

//------------------------ Substitution stuff ----------------------//

/// Return a substitution::Model specified by args
OwnedPointer<substitution::MultiModel> get_smodel(const boost::program_options::variables_map& args,
						  const alphabet& a,const std::valarray<double>& default_frequencies);

/// Return a substitution::Model specified by name
OwnedPointer<substitution::MultiModel> get_smodel(const boost::program_options::variables_map& args,
						  const string&,
						  const alignment& A);

/// Return a substitution::Model specified by args
OwnedPointer<substitution::MultiModel> get_smodel(const boost::program_options::variables_map& args,
						  const alignment& A);

/// Return a substitution::Model specified by name
OwnedPointer<substitution::MultiModel> get_smodel(const boost::program_options::variables_map& args,
						  const string&,
						  const vector<alignment>& A);

//--------------------------- Indel stuff --------------------------//
OwnedPointer<IndelModel> get_imodel(string);

#endif 
