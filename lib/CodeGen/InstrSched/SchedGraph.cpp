//===- SchedGraph.cpp - Scheduling Graph Implementation -------------------===//
//
// Scheduling graph based on SSA graph plus extra dependence edges capturing
// dependences due to machine resources (machine registers, CC registers, and
// any others).
//
//===----------------------------------------------------------------------===//

#include "SchedGraph.h"
#include "llvm/CodeGen/InstrSelection.h"
#include "llvm/CodeGen/MachineCodeForInstruction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/Target/MachineRegInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/MachineInstrInfo.h"
#include "llvm/Function.h"
#include "llvm/iOther.h"
#include "Support/StringExtras.h"
#include "Support/STLExtras.h"

using std::vector;
using std::pair;
using std::cerr;

//*********************** Internal Data Structures *************************/

// The following two types need to be classes, not typedefs, so we can use
// opaque declarations in SchedGraph.h
// 
struct RefVec: public vector< pair<SchedGraphNode*, int> > {
  typedef vector< pair<SchedGraphNode*, int> >::      iterator       iterator;
  typedef vector< pair<SchedGraphNode*, int> >::const_iterator const_iterator;
};

struct RegToRefVecMap: public hash_map<int, RefVec> {
  typedef hash_map<int, RefVec>::      iterator       iterator;
  typedef hash_map<int, RefVec>::const_iterator const_iterator;
};

struct ValueToDefVecMap: public hash_map<const Instruction*, RefVec> {
  typedef hash_map<const Instruction*, RefVec>::      iterator       iterator;
  typedef hash_map<const Instruction*, RefVec>::const_iterator const_iterator;
};

// 
// class SchedGraphEdge
// 

/*ctor*/
SchedGraphEdge::SchedGraphEdge(SchedGraphNode* _src,
			       SchedGraphNode* _sink,
			       SchedGraphEdgeDepType _depType,
			       unsigned int     _depOrderType,
			       int _minDelay)
  : src(_src),
    sink(_sink),
    depType(_depType),
    depOrderType(_depOrderType),
    minDelay((_minDelay >= 0)? _minDelay : _src->getLatency()),
    val(NULL)
{
  assert(src != sink && "Self-loop in scheduling graph!");
  src->addOutEdge(this);
  sink->addInEdge(this);
}


/*ctor*/
SchedGraphEdge::SchedGraphEdge(SchedGraphNode*  _src,
			       SchedGraphNode*  _sink,
			       const Value*     _val,
			       unsigned int     _depOrderType,
			       int              _minDelay)
  : src(_src),
    sink(_sink),
    depType(ValueDep),
    depOrderType(_depOrderType),
    minDelay((_minDelay >= 0)? _minDelay : _src->getLatency()),
    val(_val)
{
  assert(src != sink && "Self-loop in scheduling graph!");
  src->addOutEdge(this);
  sink->addInEdge(this);
}


/*ctor*/
SchedGraphEdge::SchedGraphEdge(SchedGraphNode*  _src,
			       SchedGraphNode*  _sink,
			       unsigned int     _regNum,
			       unsigned int     _depOrderType,
			       int             _minDelay)
  : src(_src),
    sink(_sink),
    depType(MachineRegister),
    depOrderType(_depOrderType),
    minDelay((_minDelay >= 0)? _minDelay : _src->getLatency()),
    machineRegNum(_regNum)
{
  assert(src != sink && "Self-loop in scheduling graph!");
  src->addOutEdge(this);
  sink->addInEdge(this);
}


/*ctor*/
SchedGraphEdge::SchedGraphEdge(SchedGraphNode* _src,
			       SchedGraphNode* _sink,
			       ResourceId      _resourceId,
			       int             _minDelay)
  : src(_src),
    sink(_sink),
    depType(MachineResource),
    depOrderType(NonDataDep),
    minDelay((_minDelay >= 0)? _minDelay : _src->getLatency()),
    resourceId(_resourceId)
{
  assert(src != sink && "Self-loop in scheduling graph!");
  src->addOutEdge(this);
  sink->addInEdge(this);
}

/*dtor*/
SchedGraphEdge::~SchedGraphEdge()
{
}

void SchedGraphEdge::dump(int indent) const {
  cerr << std::string(indent*2, ' ') << *this; 
}


// 
// class SchedGraphNode
// 

/*ctor*/
SchedGraphNode::SchedGraphNode(unsigned int _nodeId,
                               const BasicBlock*   _bb,
			       const MachineInstr* _minstr,
                               int   indexInBB,
			       const TargetMachine& target)
  : nodeId(_nodeId),
    bb(_bb),
    minstr(_minstr),
    origIndexInBB(indexInBB),
    latency(0)
{
  if (minstr)
    {
      MachineOpCode mopCode = minstr->getOpCode();
      latency = target.getInstrInfo().hasResultInterlock(mopCode)
	? target.getInstrInfo().minLatency(mopCode)
	: target.getInstrInfo().maxLatency(mopCode);
    }
}


/*dtor*/
SchedGraphNode::~SchedGraphNode()
{
  // for each node, delete its out-edges
  std::for_each(beginOutEdges(), endOutEdges(),
                deleter<SchedGraphEdge>);
}

void SchedGraphNode::dump(int indent) const {
  cerr << std::string(indent*2, ' ') << *this; 
}


inline void
SchedGraphNode::addInEdge(SchedGraphEdge* edge)
{
  inEdges.push_back(edge);
}


inline void
SchedGraphNode::addOutEdge(SchedGraphEdge* edge)
{
  outEdges.push_back(edge);
}

inline void
SchedGraphNode::removeInEdge(const SchedGraphEdge* edge)
{
  assert(edge->getSink() == this);
  
  for (iterator I = beginInEdges(); I != endInEdges(); ++I)
    if ((*I) == edge)
      {
	inEdges.erase(I);
	break;
      }
}

inline void
SchedGraphNode::removeOutEdge(const SchedGraphEdge* edge)
{
  assert(edge->getSrc() == this);
  
  for (iterator I = beginOutEdges(); I != endOutEdges(); ++I)
    if ((*I) == edge)
      {
	outEdges.erase(I);
	break;
      }
}


// 
// class SchedGraph
// 


/*ctor*/
SchedGraph::SchedGraph(const BasicBlock* bb,
		       const TargetMachine& target)
{
  bbVec.push_back(bb);
  buildGraph(target);
}


/*dtor*/
SchedGraph::~SchedGraph()
{
  for (const_iterator I = begin(); I != end(); ++I)
    delete I->second;
  delete graphRoot;
  delete graphLeaf;
}


void
SchedGraph::dump() const
{
  cerr << "  Sched Graph for Basic Blocks: ";
  for (unsigned i=0, N=bbVec.size(); i < N; i++)
    {
      cerr << (bbVec[i]->hasName()? bbVec[i]->getName() : "block")
	   << " (" << bbVec[i] << ")"
	   << ((i == N-1)? "" : ", ");
    }
  
  cerr << "\n\n    Actual Root nodes : ";
  for (unsigned i=0, N=graphRoot->outEdges.size(); i < N; i++)
    cerr << graphRoot->outEdges[i]->getSink()->getNodeId()
	 << ((i == N-1)? "" : ", ");
  
  cerr << "\n    Graph Nodes:\n";
  for (const_iterator I=begin(); I != end(); ++I)
    cerr << "\n" << *I->second;
  
  cerr << "\n";
}


void
SchedGraph::eraseIncomingEdges(SchedGraphNode* node, bool addDummyEdges)
{
  // Delete and disconnect all in-edges for the node
  for (SchedGraphNode::iterator I = node->beginInEdges();
       I != node->endInEdges(); ++I)
    {
      SchedGraphNode* srcNode = (*I)->getSrc();
      srcNode->removeOutEdge(*I);
      delete *I;
      
      if (addDummyEdges &&
	  srcNode != getRoot() &&
	  srcNode->beginOutEdges() == srcNode->endOutEdges())
	{ // srcNode has no more out edges, so add an edge to dummy EXIT node
	  assert(node != getLeaf() && "Adding edge that was just removed?");
	  (void) new SchedGraphEdge(srcNode, getLeaf(),
		    SchedGraphEdge::CtrlDep, SchedGraphEdge::NonDataDep, 0);
	}
    }
  
  node->inEdges.clear();
}

void
SchedGraph::eraseOutgoingEdges(SchedGraphNode* node, bool addDummyEdges)
{
  // Delete and disconnect all out-edges for the node
  for (SchedGraphNode::iterator I = node->beginOutEdges();
       I != node->endOutEdges(); ++I)
    {
      SchedGraphNode* sinkNode = (*I)->getSink();
      sinkNode->removeInEdge(*I);
      delete *I;
      
      if (addDummyEdges &&
	  sinkNode != getLeaf() &&
	  sinkNode->beginInEdges() == sinkNode->endInEdges())
	{ //sinkNode has no more in edges, so add an edge from dummy ENTRY node
	  assert(node != getRoot() && "Adding edge that was just removed?");
	  (void) new SchedGraphEdge(getRoot(), sinkNode,
		    SchedGraphEdge::CtrlDep, SchedGraphEdge::NonDataDep, 0);
	}
    }
  
  node->outEdges.clear();
}

void
SchedGraph::eraseIncidentEdges(SchedGraphNode* node, bool addDummyEdges)
{
  this->eraseIncomingEdges(node, addDummyEdges);	
  this->eraseOutgoingEdges(node, addDummyEdges);	
}


void
SchedGraph::addDummyEdges()
{
  assert(graphRoot->outEdges.size() == 0);
  
  for (const_iterator I=begin(); I != end(); ++I)
    {
      SchedGraphNode* node = (*I).second;
      assert(node != graphRoot && node != graphLeaf);
      if (node->beginInEdges() == node->endInEdges())
	(void) new SchedGraphEdge(graphRoot, node, SchedGraphEdge::CtrlDep,
				  SchedGraphEdge::NonDataDep, 0);
      if (node->beginOutEdges() == node->endOutEdges())
	(void) new SchedGraphEdge(node, graphLeaf, SchedGraphEdge::CtrlDep,
				  SchedGraphEdge::NonDataDep, 0);
    }
}


void
SchedGraph::addCDEdges(const TerminatorInst* term,
		       const TargetMachine& target)
{
  const MachineInstrInfo& mii = target.getInstrInfo();
  MachineCodeForInstruction &termMvec = MachineCodeForInstruction::get(term);
  
  // Find the first branch instr in the sequence of machine instrs for term
  // 
  unsigned first = 0;
  while (! mii.isBranch(termMvec[first]->getOpCode()) &&
         ! mii.isReturn(termMvec[first]->getOpCode()))
    ++first;
  assert(first < termMvec.size() &&
	 "No branch instructions for terminator?  Ok, but weird!");
  if (first == termMvec.size())
    return;

  SchedGraphNode* firstBrNode = getGraphNodeForInstr(termMvec[first]);

  // Add CD edges from each instruction in the sequence to the
  // *last preceding* branch instr. in the sequence 
  // Use a latency of 0 because we only need to prevent out-of-order issue.
  // 
  for (unsigned i = termMvec.size(); i > first+1; --i)
    {
      SchedGraphNode* toNode = getGraphNodeForInstr(termMvec[i-1]);
      assert(toNode && "No node for instr generated for branch/ret?");
      
      for (unsigned j = i-1; j != 0; --j) 
	if (mii.isBranch(termMvec[j-1]->getOpCode()) ||
            mii.isReturn(termMvec[j-1]->getOpCode()))
	  {
	    SchedGraphNode* brNode = getGraphNodeForInstr(termMvec[j-1]);
	    assert(brNode && "No node for instr generated for branch/ret?");
	    (void) new SchedGraphEdge(brNode, toNode, SchedGraphEdge::CtrlDep,
				      SchedGraphEdge::NonDataDep, 0);
	    break;			// only one incoming edge is enough
	  }
    }
  
  // Add CD edges from each instruction preceding the first branch
  // to the first branch.  Use a latency of 0 as above.
  // 
  for (unsigned i = first; i != 0; --i)
    {
      SchedGraphNode* fromNode = getGraphNodeForInstr(termMvec[i-1]);
      assert(fromNode && "No node for instr generated for branch?");
      (void) new SchedGraphEdge(fromNode, firstBrNode, SchedGraphEdge::CtrlDep,
				SchedGraphEdge::NonDataDep, 0);
    }
  
  // Now add CD edges to the first branch instruction in the sequence from
  // all preceding instructions in the basic block.  Use 0 latency again.
  // 
  const BasicBlock* bb = firstBrNode->getBB();
  const MachineBasicBlock& mvec = MachineBasicBlock::get(bb);
  for (unsigned i=0, N=mvec.size(); i < N; i++) 
    {
      if (mvec[i] == termMvec[first])   // reached the first branch
        break;
      
      SchedGraphNode* fromNode = this->getGraphNodeForInstr(mvec[i]);
      if (fromNode == NULL)
        continue;			// dummy instruction, e.g., PHI
      
      (void) new SchedGraphEdge(fromNode, firstBrNode,
                                SchedGraphEdge::CtrlDep,
                                SchedGraphEdge::NonDataDep, 0);
      
      // If we find any other machine instructions (other than due to
      // the terminator) that also have delay slots, add an outgoing edge
      // from the instruction to the instructions in the delay slots.
      // 
      unsigned d = mii.getNumDelaySlots(mvec[i]->getOpCode());
      assert(i+d < N && "Insufficient delay slots for instruction?");
      
      for (unsigned j=1; j <= d; j++)
        {
          SchedGraphNode* toNode = this->getGraphNodeForInstr(mvec[i+j]);
          assert(toNode && "No node for machine instr in delay slot?");
          (void) new SchedGraphEdge(fromNode, toNode,
                                    SchedGraphEdge::CtrlDep,
                                    SchedGraphEdge::NonDataDep, 0);
        }
    }
}

static const int SG_LOAD_REF  = 0;
static const int SG_STORE_REF = 1;
static const int SG_CALL_REF  = 2;

static const unsigned int SG_DepOrderArray[][3] = {
  { SchedGraphEdge::NonDataDep,
            SchedGraphEdge::AntiDep,
                        SchedGraphEdge::AntiDep },
  { SchedGraphEdge::TrueDep,
            SchedGraphEdge::OutputDep,
                        SchedGraphEdge::TrueDep | SchedGraphEdge::OutputDep },
  { SchedGraphEdge::TrueDep,
            SchedGraphEdge::AntiDep | SchedGraphEdge::OutputDep,
                        SchedGraphEdge::TrueDep | SchedGraphEdge::AntiDep
                                                | SchedGraphEdge::OutputDep }
};


// Add a dependence edge between every pair of machine load/store/call
// instructions, where at least one is a store or a call.
// Use latency 1 just to ensure that memory operations are ordered;
// latency does not otherwise matter (true dependences enforce that).
// 
void
SchedGraph::addMemEdges(const vector<SchedGraphNode*>& memNodeVec,
			const TargetMachine& target)
{
  const MachineInstrInfo& mii = target.getInstrInfo();
  
  // Instructions in memNodeVec are in execution order within the basic block,
  // so simply look at all pairs <memNodeVec[i], memNodeVec[j: j > i]>.
  // 
  for (unsigned im=0, NM=memNodeVec.size(); im < NM; im++)
    {
      MachineOpCode fromOpCode = memNodeVec[im]->getOpCode();
      int fromType = mii.isCall(fromOpCode)? SG_CALL_REF
                       : mii.isLoad(fromOpCode)? SG_LOAD_REF
                                               : SG_STORE_REF;
      for (unsigned jm=im+1; jm < NM; jm++)
	{
          MachineOpCode toOpCode = memNodeVec[jm]->getOpCode();
          int toType = mii.isCall(toOpCode)? SG_CALL_REF
                         : mii.isLoad(toOpCode)? SG_LOAD_REF
                                               : SG_STORE_REF;
          
          if (fromType != SG_LOAD_REF || toType != SG_LOAD_REF)
            (void) new SchedGraphEdge(memNodeVec[im], memNodeVec[jm],
                                      SchedGraphEdge::MemoryDep,
                                      SG_DepOrderArray[fromType][toType], 1);
        }
    }
} 

// Add edges from/to CC reg instrs to/from call instrs.
// Essentially this prevents anything that sets or uses a CC reg from being
// reordered w.r.t. a call.
// Use a latency of 0 because we only need to prevent out-of-order issue,
// like with control dependences.
// 
void
SchedGraph::addCallCCEdges(const vector<SchedGraphNode*>& memNodeVec,
                           MachineBasicBlock& bbMvec,
                           const TargetMachine& target)
{
  const MachineInstrInfo& mii = target.getInstrInfo();
  vector<SchedGraphNode*> callNodeVec;
  
  // Find the call instruction nodes and put them in a vector.
  for (unsigned im=0, NM=memNodeVec.size(); im < NM; im++)
    if (mii.isCall(memNodeVec[im]->getOpCode()))
      callNodeVec.push_back(memNodeVec[im]);
  
  // Now walk the entire basic block, looking for CC instructions *and*
  // call instructions, and keep track of the order of the instructions.
  // Use the call node vec to quickly find earlier and later call nodes
  // relative to the current CC instruction.
  // 
  int lastCallNodeIdx = -1;
  for (unsigned i=0, N=bbMvec.size(); i < N; i++)
    if (mii.isCall(bbMvec[i]->getOpCode()))
      {
        ++lastCallNodeIdx;
        for ( ; lastCallNodeIdx < (int)callNodeVec.size(); ++lastCallNodeIdx)
          if (callNodeVec[lastCallNodeIdx]->getMachineInstr() == bbMvec[i])
            break;
        assert(lastCallNodeIdx < (int)callNodeVec.size() && "Missed Call?");
      }
    else if (mii.isCCInstr(bbMvec[i]->getOpCode()))
      { // Add incoming/outgoing edges from/to preceding/later calls
        SchedGraphNode* ccNode = this->getGraphNodeForInstr(bbMvec[i]);
        int j=0;
        for ( ; j <= lastCallNodeIdx; j++)
          (void) new SchedGraphEdge(callNodeVec[j], ccNode,
                                    MachineCCRegsRID, 0);
        for ( ; j < (int) callNodeVec.size(); j++)
          (void) new SchedGraphEdge(ccNode, callNodeVec[j],
                                    MachineCCRegsRID, 0);
      }
}


void
SchedGraph::addMachineRegEdges(RegToRefVecMap& regToRefVecMap,
			       const TargetMachine& target)
{
  assert(bbVec.size() == 1 && "Only handling a single basic block here");
  
  // This assumes that such hardwired registers are never allocated
  // to any LLVM value (since register allocation happens later), i.e.,
  // any uses or defs of this register have been made explicit!
  // Also assumes that two registers with different numbers are
  // not aliased!
  // 
  for (RegToRefVecMap::iterator I = regToRefVecMap.begin();
       I != regToRefVecMap.end(); ++I)
    {
      int regNum        = (*I).first;
      RefVec& regRefVec = (*I).second;
      
      // regRefVec is ordered by control flow order in the basic block
      for (unsigned i=0; i < regRefVec.size(); ++i)
	{
	  SchedGraphNode* node = regRefVec[i].first;
	  unsigned int opNum   = regRefVec[i].second;
	  bool isDef = node->getMachineInstr()->operandIsDefined(opNum);
	  bool isDefAndUse =
            node->getMachineInstr()->operandIsDefinedAndUsed(opNum);
          
          for (unsigned p=0; p < i; ++p)
            {
              SchedGraphNode* prevNode = regRefVec[p].first;
              if (prevNode != node)
                {
                  unsigned int prevOpNum = regRefVec[p].second;
                  bool prevIsDef =
                    prevNode->getMachineInstr()->operandIsDefined(prevOpNum);
                  bool prevIsDefAndUse =
                    prevNode->getMachineInstr()->operandIsDefinedAndUsed(prevOpNum);
                  if (isDef)
                    {
                      if (prevIsDef)
                        new SchedGraphEdge(prevNode, node, regNum,
                                           SchedGraphEdge::OutputDep);
                      if (!prevIsDef || prevIsDefAndUse)
                        new SchedGraphEdge(prevNode, node, regNum,
                                           SchedGraphEdge::AntiDep);
                    }
                  
                  if (prevIsDef)
                    if (!isDef || isDefAndUse)
                      new SchedGraphEdge(prevNode, node, regNum,
                                         SchedGraphEdge::TrueDep);
                }
            }
        }
    }
}


// Adds dependences to/from refNode from/to all other defs
// in the basic block.  refNode may be a use, a def, or both.
// We do not consider other uses because we are not building use-use deps.
// 
void
SchedGraph::addEdgesForValue(SchedGraphNode* refNode,
                             const RefVec& defVec,
                             const Value* defValue,
                             bool  refNodeIsDef,
                             bool  refNodeIsDefAndUse,
                             const TargetMachine& target)
{
  bool refNodeIsUse = !refNodeIsDef || refNodeIsDefAndUse;
  
  // Add true or output dep edges from all def nodes before refNode in BB.
  // Add anti or output dep edges to all def nodes after refNode.
  for (RefVec::const_iterator I=defVec.begin(), E=defVec.end(); I != E; ++I)
    {
      if ((*I).first == refNode)
        continue;                       // Dont add any self-loops
      
      if ((*I).first->getOrigIndexInBB() < refNode->getOrigIndexInBB())
        { // (*).first is before refNode
          if (refNodeIsDef)
            (void) new SchedGraphEdge((*I).first, refNode, defValue,
                                      SchedGraphEdge::OutputDep);
          if (refNodeIsUse)
            (void) new SchedGraphEdge((*I).first, refNode, defValue,
                                      SchedGraphEdge::TrueDep);
        }
      else
        { // (*).first is after refNode
          if (refNodeIsDef)
            (void) new SchedGraphEdge(refNode, (*I).first, defValue,
                                      SchedGraphEdge::OutputDep);
          if (refNodeIsUse)
            (void) new SchedGraphEdge(refNode, (*I).first, defValue,
                                      SchedGraphEdge::AntiDep);
        }
    }
}


void
SchedGraph::addEdgesForInstruction(const MachineInstr& MI,
                                   const ValueToDefVecMap& valueToDefVecMap,
				   const TargetMachine& target)
{
  SchedGraphNode* node = getGraphNodeForInstr(&MI);
  if (node == NULL)
    return;
  
  // Add edges for all operands of the machine instruction.
  // 
  for (unsigned i = 0, numOps = MI.getNumOperands(); i != numOps; ++i)
    {
      switch (MI.getOperandType(i))
	{
	case MachineOperand::MO_VirtualRegister:
	case MachineOperand::MO_CCRegister:
	  if (const Instruction* srcI =
              dyn_cast_or_null<Instruction>(MI.getOperand(i).getVRegValue()))
            {
              ValueToDefVecMap::const_iterator I = valueToDefVecMap.find(srcI);
              if (I != valueToDefVecMap.end())
                addEdgesForValue(node, I->second, srcI,
                                 MI.operandIsDefined(i),
                                 MI.operandIsDefinedAndUsed(i), target);
            }
	  break;
	  
	case MachineOperand::MO_MachineRegister:
	  break; 
	  
	case MachineOperand::MO_SignExtendedImmed:
	case MachineOperand::MO_UnextendedImmed:
	case MachineOperand::MO_PCRelativeDisp:
	  break;	// nothing to do for immediate fields
	  
	default:
	  assert(0 && "Unknown machine operand type in SchedGraph builder");
	  break;
	}
    }
  
  // Add edges for values implicitly used by the machine instruction.
  // Examples include function arguments to a Call instructions or the return
  // value of a Ret instruction.
  // 
  for (unsigned i=0, N=MI.getNumImplicitRefs(); i < N; ++i)
    if (! MI.implicitRefIsDefined(i) ||
        MI.implicitRefIsDefinedAndUsed(i))
      if (const Instruction *srcI =
          dyn_cast_or_null<Instruction>(MI.getImplicitRef(i)))
        {
          ValueToDefVecMap::const_iterator I = valueToDefVecMap.find(srcI);
          if (I != valueToDefVecMap.end())
            addEdgesForValue(node, I->second, srcI,
                             MI.implicitRefIsDefined(i),
                             MI.implicitRefIsDefinedAndUsed(i), target);
        }
}


void
SchedGraph::findDefUseInfoAtInstr(const TargetMachine& target,
                                  SchedGraphNode* node,
                                  vector<SchedGraphNode*>& memNodeVec,
                                  RegToRefVecMap& regToRefVecMap,
                                  ValueToDefVecMap& valueToDefVecMap)
{
  const MachineInstrInfo& mii = target.getInstrInfo();
  
  
  MachineOpCode opCode = node->getOpCode();
  if (mii.isLoad(opCode) || mii.isStore(opCode) || mii.isCall(opCode))
    memNodeVec.push_back(node);
  
  // Collect the register references and value defs. for explicit operands
  // 
  const MachineInstr& minstr = *node->getMachineInstr();
  for (int i=0, numOps = (int) minstr.getNumOperands(); i < numOps; i++)
    {
      const MachineOperand& mop = minstr.getOperand(i);
      
      // if this references a register other than the hardwired
      // "zero" register, record the reference.
      if (mop.getType() == MachineOperand::MO_MachineRegister)
        {
          int regNum = mop.getMachineRegNum();
	  if (regNum != target.getRegInfo().getZeroRegNum())
            regToRefVecMap[mop.getMachineRegNum()].push_back(
                                                  std::make_pair(node, i));
          continue;                     // nothing more to do
	}
      
      // ignore all other non-def operands
      if (! minstr.operandIsDefined(i))
	continue;
      
      // We must be defining a value.
      assert((mop.getType() == MachineOperand::MO_VirtualRegister ||
              mop.getType() == MachineOperand::MO_CCRegister)
             && "Do not expect any other kind of operand to be defined!");
      
      const Instruction* defInstr = cast<Instruction>(mop.getVRegValue());
      valueToDefVecMap[defInstr].push_back(std::make_pair(node, i)); 
    }
  
  // 
  // Collect value defs. for implicit operands.  The interface to extract
  // them assumes they must be virtual registers!
  // 
  for (int i=0, N = (int) minstr.getNumImplicitRefs(); i < N; ++i)
    if (minstr.implicitRefIsDefined(i))
      if (const Instruction* defInstr =
          dyn_cast_or_null<Instruction>(minstr.getImplicitRef(i)))
        {
          valueToDefVecMap[defInstr].push_back(std::make_pair(node, -i)); 
        }
}


void
SchedGraph::buildNodesforBB(const TargetMachine& target,
                            const BasicBlock* bb,
                            vector<SchedGraphNode*>& memNodeVec,
                            RegToRefVecMap& regToRefVecMap,
                            ValueToDefVecMap& valueToDefVecMap)
{
  const MachineInstrInfo& mii = target.getInstrInfo();
  
  // Build graph nodes for each VM instruction and gather def/use info.
  // Do both those together in a single pass over all machine instructions.
  const MachineBasicBlock& mvec = MachineBasicBlock::get(bb);
  for (unsigned i=0; i < mvec.size(); i++)
    if (! mii.isDummyPhiInstr(mvec[i]->getOpCode()))
      {
        SchedGraphNode* node = new SchedGraphNode(getNumNodes(), bb,
                                                  mvec[i], i, target);
        this->noteGraphNodeForInstr(mvec[i], node);
        
        // Remember all register references and value defs
        findDefUseInfoAtInstr(target, node,
                              memNodeVec, regToRefVecMap,valueToDefVecMap);
      }
  
#undef REALLY_NEED_TO_SEARCH_SUCCESSOR_PHIS
#ifdef REALLY_NEED_TO_SEARCH_SUCCESSOR_PHIS
  // This is a BIG UGLY HACK.  IT NEEDS TO BE ELIMINATED.
  // Look for copy instructions inserted in this BB due to Phi instructions
  // in the successor BBs.
  // There MUST be exactly one copy per Phi in successor nodes.
  // 
  for (BasicBlock::succ_const_iterator SI=bb->succ_begin(), SE=bb->succ_end();
       SI != SE; ++SI)
    for (BasicBlock::const_iterator PI=(*SI)->begin(), PE=(*SI)->end();
         PI != PE; ++PI)
      {
        if ((*PI)->getOpcode() != Instruction::PHINode)
          break;                        // No more Phis in this successor
        
        // Find the incoming value from block bb to block (*SI)
        int bbIndex = cast<PHINode>(*PI)->getBasicBlockIndex(bb);
        assert(bbIndex >= 0 && "But I know bb is a predecessor of (*SI)?");
        Value* inVal = cast<PHINode>(*PI)->getIncomingValue(bbIndex);
        assert(inVal != NULL && "There must be an in-value on every edge");
        
        // Find the machine instruction that makes a copy of inval to (*PI).
        // This must be in the current basic block (bb).
        const MachineCodeForVMInstr& mvec = MachineBasicBlock::get(*PI);
        const MachineInstr* theCopy = NULL;
        for (unsigned i=0; i < mvec.size() && theCopy == NULL; i++)
          if (! mii.isDummyPhiInstr(mvec[i]->getOpCode()))
            // not a Phi: assume this is a copy and examine its operands
            for (int o=0, N=(int) mvec[i]->getNumOperands(); o < N; o++)
              {
                const MachineOperand& mop = mvec[i]->getOperand(o);
                
                if (mvec[i]->operandIsDefined(o))
                  assert(mop.getVRegValue() == (*PI) && "dest shd be my Phi");
                
                if (! mvec[i]->operandIsDefined(o) ||
                    NOT NEEDED? mvec[i]->operandIsDefinedAndUsed(o))
                  if (mop.getVRegValue() == inVal)
                    { // found the copy!
                      theCopy = mvec[i];
                      break;
                    }
              }
        
        // Found the dang instruction.  Now create a node and do the rest...
        if (theCopy != NULL)
          {
            SchedGraphNode* node = new SchedGraphNode(getNumNodes(), bb,
                                            theCopy, origIndexInBB++, target);
            this->noteGraphNodeForInstr(theCopy, node);
            findDefUseInfoAtInstr(target, node,
                                  memNodeVec, regToRefVecMap,valueToDefVecMap);
          }
      }
#endif  //REALLY_NEED_TO_SEARCH_SUCCESSOR_PHIS
}


void
SchedGraph::buildGraph(const TargetMachine& target)
{
  const BasicBlock* bb = bbVec[0];
  
  assert(bbVec.size() == 1 && "Only handling a single basic block here");
  
  // Use this data structure to note all machine operands that compute
  // ordinary LLVM values.  These must be computed defs (i.e., instructions). 
  // Note that there may be multiple machine instructions that define
  // each Value.
  ValueToDefVecMap valueToDefVecMap;
  
  // Use this data structure to note all memory instructions.
  // We use this to add memory dependence edges without a second full walk.
  // 
  // vector<const Instruction*> memVec;
  vector<SchedGraphNode*> memNodeVec;
  
  // Use this data structure to note any uses or definitions of
  // machine registers so we can add edges for those later without
  // extra passes over the nodes.
  // The vector holds an ordered list of references to the machine reg,
  // ordered according to control-flow order.  This only works for a
  // single basic block, hence the assertion.  Each reference is identified
  // by the pair: <node, operand-number>.
  // 
  RegToRefVecMap regToRefVecMap;
  
  // Make a dummy root node.  We'll add edges to the real roots later.
  graphRoot = new SchedGraphNode(0, NULL, NULL, -1, target);
  graphLeaf = new SchedGraphNode(1, NULL, NULL, -1, target);

  //----------------------------------------------------------------
  // First add nodes for all the machine instructions in the basic block
  // because this greatly simplifies identifying which edges to add.
  // Do this one VM instruction at a time since the SchedGraphNode needs that.
  // Also, remember the load/store instructions to add memory deps later.
  //----------------------------------------------------------------
  
  buildNodesforBB(target, bb, memNodeVec, regToRefVecMap, valueToDefVecMap);
  
  //----------------------------------------------------------------
  // Now add edges for the following (all are incoming edges except (4)):
  // (1) operands of the machine instruction, including hidden operands
  // (2) machine register dependences
  // (3) memory load/store dependences
  // (3) other resource dependences for the machine instruction, if any
  // (4) output dependences when multiple machine instructions define the
  //     same value; all must have been generated from a single VM instrn
  // (5) control dependences to branch instructions generated for the
  //     terminator instruction of the BB. Because of delay slots and
  //     2-way conditional branches, multiple CD edges are needed
  //     (see addCDEdges for details).
  // Also, note any uses or defs of machine registers.
  // 
  //----------------------------------------------------------------
      
  MachineBasicBlock& bbMvec = MachineBasicBlock::get(bb);
  
  // First, add edges to the terminator instruction of the basic block.
  this->addCDEdges(bb->getTerminator(), target);
      
  // Then add memory dep edges: store->load, load->store, and store->store.
  // Call instructions are treated as both load and store.
  this->addMemEdges(memNodeVec, target);

  // Then add edges between call instructions and CC set/use instructions
  this->addCallCCEdges(memNodeVec, bbMvec, target);
  
  // Then add incoming def-use (SSA) edges for each machine instruction.
  for (unsigned i=0, N=bbMvec.size(); i < N; i++)
    addEdgesForInstruction(*bbMvec[i], valueToDefVecMap, target);
  
#ifdef NEED_SEPARATE_NONSSA_EDGES_CODE
  // Then add non-SSA edges for all VM instructions in the block.
  // We assume that all machine instructions that define a value are
  // generated from the VM instruction corresponding to that value.
  // TODO: This could probably be done much more efficiently.
  for (BasicBlock::const_iterator II = bb->begin(); II != bb->end(); ++II)
    this->addNonSSAEdgesForValue(*II, target);
#endif //NEED_SEPARATE_NONSSA_EDGES_CODE
  
  // Then add edges for dependences on machine registers
  this->addMachineRegEdges(regToRefVecMap, target);
  
  // Finally, add edges from the dummy root and to dummy leaf
  this->addDummyEdges();		
}


// 
// class SchedGraphSet
// 

/*ctor*/
SchedGraphSet::SchedGraphSet(const Function* _function,
			     const TargetMachine& target) :
  method(_function)
{
  buildGraphsForMethod(method, target);
}


/*dtor*/
SchedGraphSet::~SchedGraphSet()
{
  // delete all the graphs
  for(iterator I = begin(), E = end(); I != E; ++I)
    delete *I;  // destructor is a friend
}


void
SchedGraphSet::dump() const
{
  cerr << "======== Sched graphs for function `" << method->getName()
       << "' ========\n\n";
  
  for (const_iterator I=begin(); I != end(); ++I)
    (*I)->dump();
  
  cerr << "\n====== End graphs for function `" << method->getName()
       << "' ========\n\n";
}


void
SchedGraphSet::buildGraphsForMethod(const Function *F,
				    const TargetMachine& target)
{
  for (Function::const_iterator BI = F->begin(); BI != F->end(); ++BI)
    addGraph(new SchedGraph(BI, target));
}


std::ostream &operator<<(std::ostream &os, const SchedGraphEdge& edge)
{
  os << "edge [" << edge.src->getNodeId() << "] -> ["
     << edge.sink->getNodeId() << "] : ";
  
  switch(edge.depType) {
  case SchedGraphEdge::CtrlDep:		os<< "Control Dep"; break;
  case SchedGraphEdge::ValueDep:        os<< "Reg Value " << edge.val; break;
  case SchedGraphEdge::MemoryDep:	os<< "Memory Dep"; break;
  case SchedGraphEdge::MachineRegister: os<< "Reg " <<edge.machineRegNum;break;
  case SchedGraphEdge::MachineResource: os<<"Resource "<<edge.resourceId;break;
  default: assert(0); break;
  }
  
  os << " : delay = " << edge.minDelay << "\n";
  
  return os;
}

std::ostream &operator<<(std::ostream &os, const SchedGraphNode& node)
{
  os << std::string(8, ' ')
     << "Node " << node.nodeId << " : "
     << "latency = " << node.latency << "\n" << std::string(12, ' ');
  
  if (node.getMachineInstr() == NULL)
    os << "(Dummy node)\n";
  else
    {
      os << *node.getMachineInstr() << "\n" << std::string(12, ' ');
      os << node.inEdges.size() << " Incoming Edges:\n";
      for (unsigned i=0, N=node.inEdges.size(); i < N; i++)
	  os << std::string(16, ' ') << *node.inEdges[i];
  
      os << std::string(12, ' ') << node.outEdges.size()
         << " Outgoing Edges:\n";
      for (unsigned i=0, N=node.outEdges.size(); i < N; i++)
        os << std::string(16, ' ') << *node.outEdges[i];
    }
  
  return os;
}
