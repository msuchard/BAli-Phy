/*
   Copyright (C) 2004-2009 Benjamin Redelings

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

#ifndef MCMC_H
#define MCMC_H

#include <iostream>
#include <vector>
#include <valarray>
#include <string>
#include <map>
#include "mytypes.H"
#include "parameters.H"
#include "rng.H"
#include "util.H"
#include "proposals.H"
// how to have different models, with different moves
// and possibly moves between models?

namespace MCMC {

  //---------------------- Move Stats ---------------------//
  struct Result {
    std::valarray<int> counts;
    std::valarray<double> totals;

    int size() const {return counts.size();}

    void inc(const Result&);

    Result() {}
    Result(bool);
    Result(int,int=1);
  };

  class MoveStats: public std::map<std::string,Result>
  {
  public:
    void inc(const string&, const Result&);
  };

  std::ostream& operator<<(const std::ostream& o, const MoveStats& Stats);

  //---------------------- Simple Move  ---------------------//
  typedef void (*atomic_move)(Parameters&,MoveStats&);
  typedef void (*atomic_move_arg)(Parameters&,MoveStats&,int);

  //---------------- Move's w/ sub-moves --------------------//
  class Move: public Cloneable 
  {
    bool enabled_;

  public:
    string name;

    vector<string> attributes;

    double iterations;

    /// Make a copy of this object
    virtual Move* clone() const =0;

    /// Is this move enabled?
    bool enabled() {return enabled_;}

    /// Enable this move
    void enable() {enabled_=true;}

    /// Disable this move
    void disable() {enabled_=false;}
    
    /// Enable this move or any submove with attribute 's'
    virtual void enable(const string& s);

    /// Disable this move or any submove with attribute 's'
    virtual void disable(const string& s);

    /// Set up for an iteration of length l, return number of subiterations
    virtual int reset(double l) =0;

    /// Do a complete iteration (a top-level routine)
    virtual void iterate(Parameters&,MoveStats&) =0;

    /// Do the i-th iteration for this round (not a top-level routine)
    virtual void iterate(Parameters&,MoveStats&,int i) =0;

    /// Show enabled-ness for this move and submoves
    virtual void show_enabled(std::ostream&,int depth=0) const;

    /// construct a new move called 's'
    Move(const string& s);
    Move(const string& s, const string& v);

    virtual ~Move() {}
  };

  // FIXME? We could make this inherit from virtual public Move...
  //    but that seems to introduce problems...
  // We could also move the code to this class, and call it from the classes
  //    that use it.

  /// A collection of moves
  class MoveGroupBase {
  protected:
    /// This list of moves
    std::vector<OwnedPointer<Move> > moves;

    /// This weight of each move
    std::vector<double> lambda;

  public:
    int nmoves() const {return moves.size();}
    void add(double,const Move& m,bool=true);

    MoveGroupBase() {}
  };

  /// A Move which runs parts of a number of submoves each round
  class MoveGroup: public Move,public MoveGroupBase {
  protected:

    /// An ordered list of submoves to run this round
    vector<int> order;

    /// suborder[i] is the n-th time we've run order[i]
    vector<int> suborder;
    
    double sum() const;

    /// Setup 'order' and 'suborder' for this round
    virtual void getorder(double l)=0;
  public:

    MoveGroup* clone() const =0;

    void enable(const string&);
    void disable(const string&);

    int reset(double);
    void iterate(Parameters&,MoveStats&);
    void iterate(Parameters&,MoveStats&,int);

    void show_enabled(std::ostream&,int depth=0) const;

    MoveGroup(const string& s):Move(s) {}
    MoveGroup(const string& s, const string& v):Move(s,v) {}

    virtual ~MoveGroup() {}
  };

  /// A Move which runs all submoves each round
  class MoveAll: public MoveGroup {
    void getorder(double l);
  public:
    MoveAll* clone() const {return new MoveAll(*this);}

    MoveAll(const string& s):MoveGroup(s) {}
    MoveAll(const string& s, const string& v):MoveGroup(s,v) {}
    virtual ~MoveAll() {}
  };

  /// A Move which runs one submoves each round
  class MoveOne: public MoveGroup {
    int choose() const;
    void getorder(double l);
  public:
    MoveOne* clone() const {return new MoveOne(*this);}

    MoveOne(const string& s):MoveGroup(s) {}
    MoveOne(const string& s, const string& v):MoveGroup(s,v) {}
    virtual ~MoveOne() {}
  };

  /// A Move which runs a specific moves each round
  class SingleMove: public Move {
    atomic_move m;
  public:
    SingleMove* clone() const {return new SingleMove(*this);}

    int reset(double lambda);

    void iterate(Parameters& P,MoveStats& Stats) { iterate(P,Stats,0); }

    void iterate(Parameters& P,MoveStats&,int);

    SingleMove(atomic_move m1,const string& s)
      :Move(s),m(m1) {}
    SingleMove(atomic_move m1,const string& s, const string& v)
      :Move(s,v),m(m1) {}
  };


  /// A Move which runs a specific moves each round
  class MH_Move: public Move {
    OwnedPointer<Proposal> proposal;
  public:
    MH_Move* clone() const {return new MH_Move(*this);}

    int reset(double lambda);

    void iterate(Parameters& P,MoveStats& Stats) { iterate(P,Stats,0); }

    void iterate(Parameters& P,MoveStats&,int);

    MH_Move(const Proposal& P,const string& s)
      :Move(s),proposal(P) {}
    MH_Move(const Proposal& P,const string& s, const string& v)
      :Move(s,v),proposal(P) {}
  };

  // Improve: make W into a FUNCTION to determine the initial width
  class Slice_Move: public Move 
  {
    int index;
    bool lower_bound;
    double lower;
    bool upper_bound;
    double upper;
    double W;
    double (*window)(const Parameters&);

    double (*transform)(double);
    double (*inverse)(double);

  public:
    Slice_Move* clone() const {return new Slice_Move(*this);}

    int reset(double lambda);

    void iterate(Parameters& P,MoveStats& Stats) { iterate(P,Stats,0); }

    void iterate(Parameters& P,MoveStats&,int);

    Slice_Move(const string& s,int i,
	       bool lb,double l,bool ub,double u,double W_);

    Slice_Move(const string& s, const string& v,int i,
	       bool lb,double l,bool ub,double u,double W_);

    Slice_Move(const string& s,int i,
	       bool lb,double l,bool ub,double u,double (*W_)(const Parameters&));

    Slice_Move(const string& s, const string& v,int i,
	       bool lb,double l,bool ub,double u,double (*W_)(const Parameters&));

    Slice_Move(const string& s,int i,
	       bool lb,double l,bool ub,double u,double W_,
	       double(*f1)(double),
	       double(*f2)(double));

    Slice_Move(const string& s, const string& v,int i,
	       bool lb,double l,bool ub,double u,double W_,
	       double(*f1)(double),
	       double(*f2)(double));
  };


  /// A move which takes an integer argument from a supplied list
  class MoveArg: public Move {
    /// The ordered list of args to operate on this round
    vector<int> order;

  public:
    MoveArg* clone() const=0;

    /// A list of arguments to be passed to submoves
    vector<int> args;

    int reset(double);
    void iterate(Parameters&,MoveStats&);
    void iterate(Parameters&,MoveStats&,int);

    /// Operate on the 'a'-th arg
    virtual void operator()(Parameters&,MoveStats&,int a)=0;

    MoveArg(const string& s):Move(s) { }
    MoveArg(const string& s, const string& v):Move(s,v) { }

    virtual ~MoveArg() {}
  };


  /// Apply this move to Each arg

  /// Apply this move to Each arg, choosing for each arg from
  /// among the moves that apply to that arg.
  class MoveEach: public MoveArg, protected MoveGroupBase {

    /// The n-th arg of this move, is the subarg[m][n]-th arg of move m
    vector< vector<int> > subarg;

    /// Get the sum of the weights for the i-th arg
    double sum(int i) const;

    /// Choose an submove to apply to arg i
    int choose(int i) const;

    /// Is the arg-th arg is available for the m-th move?
    bool submove_has_arg(int m,int arg) const {return subarg[m][arg] != -1;}
  public:
    MoveEach* clone() const {return new MoveEach(*this);}

    void add(double l,const MoveArg& m,bool=true);

    void enable(const string&);
    void disable(const string&);

    void operator()(Parameters&,MoveStats&,int);
    
    void show_enabled(std::ostream&,int depth=0) const;

    MoveEach(const string& s):MoveArg(s) {}
    MoveEach(const string& s,const string& v):MoveArg(s,v) {}

    virtual ~MoveEach() {}
  };

  /// A single move with an integer argument, and the arguments it takes
  class MoveArgSingle: public MoveArg {
    atomic_move_arg m;
  public:
    MoveArgSingle* clone() const {return new MoveArgSingle(*this);}

    void operator()(Parameters&,MoveStats&,int);

    MoveArgSingle(const string& s,atomic_move_arg m1,const vector<int>& a)
      :MoveArg(s),m(m1) 
    {args=a;}

    MoveArgSingle(const string& s,const string& v,atomic_move_arg m1,const vector<int>& a)
      :MoveArg(s,v),m(m1) 
    {args=a;}

    ~MoveArgSingle() {}
  };



  /// A Sampler: based on a collection of moves to run every iteration
  class Sampler: public MoveAll, public MoveStats {

  public:
    /// Run the sampler for 'max' iterations
    void go(Parameters& P, int subsample, int max, 
	    std::ostream&,std::ostream&,std::ostream&,std::ostream&,std::vector<std::ostream*>& files);

    Sampler(const string& s)
      :MoveAll(s) {};
  };

}

#endif
