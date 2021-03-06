/*
   Copyright (C) 2009 Benjamin Redelings

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

#include "consensus-tree.H"

#include <map>
#include <list>

#include "myexception.H"
#include "statistics.H"
#include "util.H"

using std::vector;
using std::string;
using std::map;
using std::list;
using std::pair;

using std::endl;
using std::cerr;

using boost::dynamic_bitset;


void add_partitions_and_counts(const vector<tree_sample>& samples, int index, map<dynamic_bitset<>,p_counts>& counts)
{
  const tree_sample& sample = samples[index];

  vector<string> names = sample.names();

  typedef map<dynamic_bitset<>,p_counts> container_t;

  for(int i=0;i<sample.trees.size();i++) 
  {
    const vector<dynamic_bitset<> >& T = sample.trees[i].partitions;

    // for each partition in the next tree
    dynamic_bitset<> partition(names.size());
    for(int b=0;b<T.size();b++) 
    {
      partition = T[b];

      if (not partition[0])
	partition.flip();

      // Look up record for this partition
      container_t::iterator record = counts.find(partition);
      if (record == counts.end()) {
	counts.insert(container_t::value_type(partition,p_counts(samples.size())));
	record = counts.find(partition);
	assert(record != counts.end());
      }

      //      cerr<<" dist = "<<index<<" topology = "<<i<<" branch = "<<b<<"    split="<<partition<<endl;
    
      // Record this tree as having the partition if we haven't already done so. 
      p_counts& pc = record->second;
      //      cerr<<"    "<<join(pc.counts,' ')<<endl;
      pc.counts[index] ++;
      //      cerr<<"    "<<join(pc.counts,' ')<<endl;
    }
  }
}

map<dynamic_bitset<>,p_counts> get_multi_partitions_and_counts(const vector<tree_sample>& samples)
{
  map<dynamic_bitset<>,p_counts> partitions;

  for(int i=0;i<samples.size();i++)
    add_partitions_and_counts(samples, i, partitions);

  return partitions;
}

struct p_count {
  int count;
  int last_tree;
  p_count(): count(0),last_tree(-1) {}
};

vector<pair<Partition,unsigned> > 
get_Ml_partitions_and_counts(const tree_sample& sample,double l,const dynamic_bitset<>&  mask) 
{
  // find the first bit
  int first = mask.find_first();

  if (l <= 0.0)
    throw myexception()<<"Consensus level must be > 0.0";
  if (l > 1.0)
    throw myexception()<<"Consensus level must be <= 1.0";

  // use a sorted list of <partition,count>, sorted by partition.
  typedef map<dynamic_bitset<>,p_count> container_t;
  container_t counts;

  // use a linked list of pointers to <partition,count> records.
  list<container_t::iterator> majority;

  vector<string> names = sample.names();

  unsigned count = 0;

  for(int i=0;i<sample.trees.size();i++) 
  {
    const vector<dynamic_bitset<> >& T = sample.trees[i].partitions;

    unsigned min_old = std::min(1+(unsigned)(l*count),count);

    count ++;
    unsigned min_new = std::min(1+(unsigned)(l*count),count);

    // for each partition in the next tree
    dynamic_bitset<> partition(names.size());
    for(int b=0;b<T.size();b++) 
    {
      partition = T[b];

      if (not partition[first])
	partition.flip();

      partition &= mask;

      // Look up record for this partition
      container_t::iterator record = counts.find(partition);
      if (record == counts.end()) {
	counts.insert(container_t::value_type(partition,p_count()));
	record = counts.find(partition);
	assert(record != counts.end());
      }

      // FIXME - we are doing the lookup twice
      p_count& pc = record->second;
      int& C2 = pc.count;
      int C1 = C2;
      if (pc.last_tree != i) {
	pc.last_tree=i;
	C2 ++;
      }
      
      // add the partition if it wasn't good before, but is now
      if ((C1==0 or C1<min_old) and C2 >= min_new)
	majority.push_back(record);
    }


    // for partition in the majority tree
    for(typeof(majority.begin()) p = majority.begin();p != majority.end();) {
      if ((*p)->second.count < min_new) {
	typeof(p) old = p;
	p++;
	majority.erase(old);
      }
      else
	p++;
    }
  }

  vector<pair<Partition,unsigned> > partitions;
  partitions.reserve( 2*names.size() );
  for(typeof(majority.begin()) p = majority.begin();p != majority.end();p++) {
    const dynamic_bitset<>& partition =(*p)->first;
 
    Partition pi(names,partition,mask);
    unsigned p_count = (*p)->second.count;

    if (valid(pi))
      partitions.push_back(pair<Partition,unsigned>(pi,p_count));
  }

  return partitions;
}


vector<pair<Partition,unsigned> > 
get_Ml_partitions_and_counts(const tree_sample& sample,double l) 
{
  dynamic_bitset<> mask(sample.names().size());
  mask.flip();
  return get_Ml_partitions_and_counts(sample,l,mask);
}

vector<Partition> 
remove_counts(const vector<pair<Partition,unsigned> >& partitions_and_counts) 
{
  vector<Partition> partitions;
  for(int i=0;i<partitions_and_counts.size();i++)
    partitions.push_back(partitions_and_counts[i].first);

  return partitions;
}


vector<Partition>
get_Ml_partitions(const tree_sample& sample,double l) 
{
  return remove_counts(get_Ml_partitions_and_counts(sample,l));
}

void add_unique(list<dynamic_bitset<> >& masks,const list<dynamic_bitset<> >& old_masks,
		const dynamic_bitset<>& mask) 
{
  // don't add the mask unless contains internal partitions (it could be all 0)
  if (mask.count() < 4) return;

  // don't add the mask if we already have that mask
  foreach(m,masks)
    if (*m == mask) return;

  foreach(m,old_masks)
    if (*m == mask) return;

  // otherwise, add the mask
  masks.push_front(mask);
}


void add_unique(list<dynamic_bitset<> >& masks,const dynamic_bitset<>& mask) 
{
  return add_unique(masks,list<dynamic_bitset<> >(),mask);
}


/// find out which partitions imply the sub-partitions, prefering the most likely.
vector<int> match(const vector<pair<Partition,unsigned> >& full_partitions,
		  const vector<pair<Partition,unsigned> >& sub_partitions)
{
  vector<int> m(sub_partitions.size(),-1);

  for(int i=0;i<sub_partitions.size();i++)
    for(int j=0;j<full_partitions.size();j++) 
    {
      // things that imply us cannot have a higher probability
      if (full_partitions[j].second > sub_partitions[i].second)
	continue;

      // skip this possible parent if it isn't as good as one we've found so far.
      if (m[i] == -1 or full_partitions[j].second > full_partitions[m[i]].second)
      {
	if (implies(full_partitions[j].first,sub_partitions[i].first))
	  m[i] = j;
      }
    }

  return m;
}


// also construct list "who is implied by whom"

// consider only pulling out combinations of branches pointing to the same node

vector<pair<Partition,unsigned> > 
get_Ml_sub_partitions_and_counts(const tree_sample& sample,double l,const dynamic_bitset<>& mask,
				 double min_rooting,int depth) 
{
  // get list of branches to consider cutting
  //   FIXME - consider 4n-12 most probable partitions, here?
  //         - Perhaps NOT, though.
  vector<Partition> partitions_c50 = get_Ml_partitions(sample, 0.5);
  SequenceTree c50 = get_mf_tree(sample.names(),partitions_c50);
  vector<const_branchview> branches = branches_from_leaves(c50);  

  // construct unit masks
  // - unit masks are masks that come directly from a supported branch (full, or partial)
  list< dynamic_bitset<> > unit_masks;
  for(int b=0;b<branches.size();b++)
    add_unique(unit_masks, mask & branch_partition(c50,branches[b]) );

  // construct beginning masks
  list<dynamic_bitset<> > masks = unit_masks;
  list<dynamic_bitset<> > old_masks = unit_masks;

  // start collecting partitions at M[l]
  vector<pair<Partition,unsigned> > partitions = get_Ml_partitions_and_counts(sample,l,mask);

  // any good mask should be combined w/ other good masks
  list<dynamic_bitset<> > good_masks;
  for(int iterations=0;not masks.empty();iterations++)
  {
    vector<pair<Partition,unsigned> > full_partitions = partitions;

    if (log_verbose) cerr<<"iteration: "<<iterations<<"   depth: "<<depth<<"   masks: "<<masks.size()<<endl;
    list<dynamic_bitset<> > new_good_masks;
    list<dynamic_bitset<> > new_unit_masks;

    // get sub-partitions for each mask
    vector<Partition> all_sub_partitions;
    foreach(m,masks) 
    {
      // get sub-partitions of *m 
      vector<pair<Partition,unsigned> > sub_partitions = get_Ml_partitions_and_counts(sample,l,*m);
    
      // match up sub-partitions and full partitions
      // FIXME - aren't we RE-doing a lot of work, here?
      vector<int> parents = match(full_partitions,sub_partitions);

      // check for partitions with increased support when *m is unplugged
      double rooting=1.0;
      for(int i=0;i<sub_partitions.size();i++) 
      {
	if (not informative(sub_partitions[i].first))
	  continue;
	    
	double r = 1;
	if (parents[i] == -1) {
	  r = (l*sample.size())/double(sub_partitions[i].second);
	}
	else {
	  r = full_partitions[parents[i]].second/double(sub_partitions[i].second);
	  assert(r <= 1.0);
	}

	double OD = statistics::odds(sub_partitions[i].second-5,sample.size(),10);

	// actually, considering bad rooting of low-probability edges may be a better (or alternate)
	// strategy to unplugging edges that are only slightly bad.

	// Determination of rooting probabilities seems to have the largest effect on computation time
	//  - thus, in the long run, new_good_masks has a larger effect than new_unit_masks.
	//  - actually, this makes kind of makes sense...
	//    + new_unit_masks can add partitions they reveal under fairly weak conditions.
	//    + however, unless a new unit mask ends up being a good_mask, it won't trigger the quadratic behavior.

	// What happens when we consider unplugging ratios for branches (now) supported at level l<0.5?
	if (r < min_rooting and OD > 0.5) {
	  add_unique(new_unit_masks,unit_masks,sub_partitions[i].first.group1);
	  add_unique(new_unit_masks,unit_masks,sub_partitions[i].first.group2);
	  rooting = std::min(rooting,r);
	}

	// Store the new sub-partitions we found
	if (r < 0.999 or 
	    (parents[i] != -1 and statistics::odds_ratio(sub_partitions[i].second,
							 full_partitions[parents[i]].second,
							 sample.size(),
							 10) > 1.1)
	    )
	  partitions.push_back(sub_partitions[i]);
      }

      // check if any of our branches make this branch badly rooted
      if (rooting < min_rooting)
	new_good_masks.push_front(*m);
    }

    old_masks.insert(old_masks.end(),masks.begin(),masks.end());
    masks.clear();
    masks = new_unit_masks;

    if (log_verbose) cerr<<"new unit_masks = "<<new_unit_masks.size()<<endl;

    if (depth == 0) continue;

    // FIXME!! We need to find a way to consider only masks which are
    // 'close' togther - defined in terms of the number and support 
    // of branches that are in the between them.

    // fixme - do a convolution here - e.g. 2->1+1 3->1+2 4 ->1+3,2+2
    // otherwise we do 1  - 1,2 - 1,2,3,4 - 1,2,3,4,5,6,7,8
    good_masks.insert(good_masks.end(),new_good_masks.begin(),new_good_masks.end());
    foreach(i,new_good_masks)
      foreach(j,good_masks)
        if (*i != *j)
          add_unique(masks,old_masks,*i & *j);

    // what will we operate on next time? 
    // - perhaps change to look at pairs of branches connected to a node
    // - perhaps depth 3 could be pairs of branches of distance 1
    // - should I use the M[0.5] tree here, or the M[l] tree?
    if (iterations < depth-1) {

      foreach(i,old_masks)
	foreach(j,unit_masks)
	  add_unique(masks,old_masks,*i & *j);

      // old good_masks were considered with unit_masks last_time
      foreach(i,new_good_masks)
      	foreach(j,unit_masks)
      	  add_unique(masks,old_masks,*i & *j);

      // old good_masks were considered with unit_masks already
      foreach(i,old_masks)
	foreach(j,new_good_masks)
	  add_unique(masks,old_masks,*i & *j);
    }

    //cerr<<"   new good masks = "<<new_good_masks.size()<<"    new unit masks = "<<new_unit_masks.size()<<endl;
    //cerr<<"       good masks = "<<good_masks.size()    <<"       total masks = "<<old_masks.size()<<"       found = "<<partitions.size()<<endl;


  }

  return partitions;
}

vector<pair<Partition,unsigned> > 
get_Ml_sub_partitions_and_counts(const tree_sample& sample,double l,double min_rooting,int depth) 
{
  dynamic_bitset<> mask(sample.names().size());
  mask.flip();
  return get_Ml_sub_partitions_and_counts(sample,l,mask,min_rooting,depth);
  
}

vector<Partition> 
get_Ml_sub_partitions(const tree_sample& sample,double l,double min_rooting,int depth) 
{
  return remove_counts(get_Ml_sub_partitions_and_counts(sample,l,min_rooting,depth));
}


