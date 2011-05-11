#include <math.h>
#include "net/PhasedBroadcast.h"
#include "net/net.h"
#include "lb/LbmParameters.h"

namespace hemelb
{
  namespace net
  {
    PhasedBroadcast::PhasedBroadcast(Net* iNet,
                                     const lb::SimulationState * iSimState,
                                     unsigned int spreadFactor)
    {
      mNet = iNet;
      mSimState = iSimState;

      // Initialise member variables.
      mTreeDepth = 0;
      mMyDepth = 0;
      mSpreadFactor = spreadFactor;

      // Calculate the correct values for the depth variables.
      proc_t noSeenToThisDepth = 1;
      proc_t noAtCurrentDepth = 1;

      hemelb::topology::NetworkTopology* netTop = hemelb::topology::NetworkTopology::Instance();

      while (noSeenToThisDepth < netTop->GetProcessorCount())
      {
        // Go down a level. I.e. increase the depth of the tree, to a new level which has M times
        // as many nodes on it.
        ++mTreeDepth;
        noAtCurrentDepth *= spreadFactor;
        noSeenToThisDepth += noAtCurrentDepth;

        // If this node is at the current depth, it must have a rank lower than or equal to the highest
        // rank at the current depth but greater than the highest rank at the previous depth.
        if (noSeenToThisDepth > netTop->GetLocalRank() && ( (noSeenToThisDepth - noAtCurrentDepth)
            <= netTop->GetLocalRank()))
        {
          mMyDepth = mTreeDepth;
        }
      }

      // In a M-tree, with a root of 0, each node N's parent has rank floor((N-1) / M)
      if (netTop->GetLocalRank() == 0)
      {
        mParent = NOPARENT;
      }
      else
      {
        mParent = (netTop->GetLocalRank() - 1) / spreadFactor;
      }

      // The children of a node N in a M-tree with root 0 are those in the range (M*N)+1,...,(M*N) + M
      for (unsigned int child = (spreadFactor * netTop->GetLocalRank()) + 1; child <= spreadFactor
          * (1 + netTop->GetLocalRank()); ++child)
      {
        if (child < (unsigned int) netTop->GetProcessorCount())
        {
          mChildren.push_back(child);
        }
      }
    }

    /*
     * PROGRAMME
     * ---------
     *
     * For 0-indexed cycle counts and 0-indexed depths, tree depth = N.
     *
     * Iterations 0 to (N-1) are nodes at depth (it) passing down to nodes at depth (it + 1).
     * Action happens on all nodes at the end of iteration (N-1).
     * Iterations N to (2N-1) are nodes at depth (2N - it) passing up to nodes at depth (2N - it - 1).
     */
    void PhasedBroadcast::RequestComms()
    {
      unsigned int iCycleNumber = Get0IndexedIterationNumber();

      // Passing down the tree.
      if (iCycleNumber < mTreeDepth)
      {
        if (mMyDepth == iCycleNumber)
        {
          ProgressToChildren();
        }
        else if (mMyDepth == (iCycleNumber + 1))
        {
          ProgressFromParent();
        }
      }
      // Passing up the tree.
      else
      {
        if (mMyDepth == (2 * mTreeDepth - iCycleNumber))
        {
          ProgressToParent();
        }
        else if (mMyDepth == (2 * mTreeDepth - iCycleNumber - 1))
        {
          ProgressFromChildren();
        }
      }
    }

    void PhasedBroadcast::PostReceive()
    {
      unsigned int iCycleNumber = Get0IndexedIterationNumber();
      // Passing down the tree.
      if (mMyDepth == (iCycleNumber + 1))
      {
        PostReceiveFromParent();
      }
      // Passing up the tree to this node.
      else if (mMyDepth == (2 * mTreeDepth - iCycleNumber - 1))
      {
        PostReceiveFromChildren();

        // If this node is the root of the tree, it must act.
        if (topology::NetworkTopology::Instance()->GetLocalRank() == 0)
        {
          TopNodeAction();
        }
      }

      // If we're halfway through the programme, all top-down changes have occurred and
      // can be applied on all nodes at once safely.
      if (iCycleNumber == (mTreeDepth - 1))
      {
        Effect();
      }
    }

    unsigned long PhasedBroadcast::Get0IndexedIterationNumber() const
    {
      if (mTreeDepth > 0)
      {
        return (mSimState->TimeStep - 1) % GetRoundTripLength();
      }
      else
      {
        return 0;
      }
    }

    unsigned int PhasedBroadcast::GetRoundTripLength() const
    {
      return 2 * mTreeDepth;
    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::Reset()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::ProgressFromChildren()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::ProgressFromParent()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::ProgressToChildren()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::ProgressToParent()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::Effect()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::TopNodeAction()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::PostReceiveFromChildren()
    {

    }

    /**
     * The default version of this method does nothing.
     */
    void PhasedBroadcast::PostReceiveFromParent()
    {

    }

  }
}
