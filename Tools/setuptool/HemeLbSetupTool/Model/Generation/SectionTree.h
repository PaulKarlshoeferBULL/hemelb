// -*- mode: c++; -*-
#ifndef HEMELBSETUPTOOL_SECTIONTREE_H
#define HEMELBSETUPTOOL_SECTIONTREE_H

#include <vector>
#include <list>
#include <fstream>

#include "Oct.h"
#include "MaskTree.h"
#include "FluidSiteTree.h"

// This is a flattened Octree that at the lowest level stores zero or
// more Sections, in the meaning of PETSc.


// This is a pair of integers (offset and a count) and some data. The
// integers index into the data.
namespace H5 {
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;
}
template <class T>
struct Section {
  typedef uint64_t IndT;

  void append() {
    offsets.push_back(data.size());
    counts.push_back(0);
  }
  
  template <class... Args>
  void append(Args&&... args) {
    offsets.push_back(data.size());
    counts.push_back(1);
    data.emplace_back(std::forward<Args>(args)...);
  }

  void write(H5::GroupPtr grp) const;
  
  std::vector<IndT> offsets;
  std::vector<IndT> counts;
  std::vector<T> data;
};

// The tree data is stored in nLevel vectors

// Each branch node in the tree is 8 indices giving the offsets of the
// children in the next level down's vector.

// Leaf nodes are the sections.

// To keep addressing consistent we add an empty vector for the leaf nodes.

class SectionTree {
public:
  typedef std::shared_ptr<SectionTree> Ptr;

  typedef MaskTree::Int Int;
  typedef uint64_t IndT;
  
  typedef std::vector<IndT> TreeLevel;
  typedef std::vector<TreeLevel> Tree;
  
  static constexpr IndT NA() {return ~0;};
  
  static inline Int LocalOffset(Int i, Int j, Int k, Int lvl) {
    Int xbit = (i >> lvl) & 1;
    Int ybit = (j >> lvl) & 1;
    Int zbit = (k >> lvl) & 1;
    return (xbit << 2) | (ybit << 1) | zbit;
  }
  
  IndT FindIndex(Int i, Int j, Int k) const;
  
  void AddSection(std::string& name);

  const Tree& GetTree() const;
  
  void Write(const std::string& fn) const;
  
  
private:
  
  friend class SectionTreeBuilder;
  friend class SectionTreeTests;
  
  SectionTree(size_t nl);
  
  Int nLevels;
  Tree indices;
  Tree counts;
  IndT total;
  
  Section<SVector> wall_normals;
  Section<std::array<Link, 26>> links;
  
};


#endif