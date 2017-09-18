// -*- mode: c++; -*-
#ifndef HEMELBSETUPTOOL_SECTIONTREEBUILDER_H
#define HEMELBSETUPTOOL_SECTIONTREEBUILDER_H

#include "MaskTree.h"
#include "SectionTree.h"

// This will build a section tree from a MaskTree (i.e. the result of
// flood filling a domain) and it's source EdgeTree (that contains all
// the boundary condition data)
class SectionTreeBuilder : public MaskTree::ConstVisitor {
public:
  using Int = MaskTree::Int;
  
  static inline Int LocalOffset(const MaskTree::Node& n) {
    return SectionTree::LocalOffset(n.X(), n.Y(), n.Z(), n.Level());
  }
  
  SectionTreeBuilder(const MaskTree& mask, const FluidTree& edges);
  
  SectionTree::Ptr operator()();
  
  virtual void Arrive(MaskTree::ConstNodePtr np);
  virtual void Depart(MaskTree::ConstNodePtr n);
  
  inline SectionTree::IndT GetSectionSize() const {
    return offsets[0];
  }

private:
  const MaskTree& maskTree;
  const FluidTree& edgeTree;
  
  const Int nLevels;
  const unsigned nEdgeSites;
  // Current insertion index for each level
  // Has size == nLevels +1
  std::vector<SectionTree::IndT> offsets;
  std::vector<FluidTree::ConstNodePtr> edge_ptrs;
  
  SectionTree::IndT wall_normal_offset;
  SectionTree::IndT links_offset;
  
  SectionTree::Ptr output;
};

#endif
