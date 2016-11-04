// Copyright Hugh Perkins 2016

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "flowcontrolinstructions.h"

#include <iostream>

namespace cocl {
namespace flowcontrol {

static int nextId = 0;

using namespace std;
using namespace cocl;
using namespace cocl::flowcontrol;
using namespace llvm;

Block::Block() {
    this->id = nextId;
    nextId++;
}
string Block::blockType() const {
    return "Block";
}
void Block::replaceIncoming(Block *oldIncoming, Block *newIncoming) {
    int id = 0;
    bool found = false;
    for(auto it=incoming.begin(); it != incoming.end(); it++) {
        Block *block = *it;
        if(block == oldIncoming) {
            found = true;
            break;
        }
        id++;
    }
    if(found) {
        incoming[id] = newIncoming;
        return;
    }
    throw runtime_error("illegal parameters");
}

string RootBlock::blockType() const {
    return "RootBlock";
}
void RootBlock::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "RootBlock " << this->id << endl;
    if(seen.find(first) == seen.end()) {
        first->dump(seen, indent + "  ");
    }
}
void RootBlock::replaceChild(Block *oldChild, Block *newChild) {
    assert(first == oldChild);
    first = newChild;
}
int RootBlock::numSuccessors() {
    // if(first != 0) {
    //     return 1;
    // }
    return 0;
}
Block *RootBlock::getSuccessor(int idx) {
    throw runtime_error("illegal parameters");
    // return first;
}

int For::numSuccessors() {
    if(next != 0) {
        return 1;
    }
    return 0;
}
Block *For::getSuccessor(int idx) {
    if(idx > 0) {
        throw runtime_error("illegal request");
    }
    return next;
}
void For::replaceChild(Block *oldChild, Block *newChild) {
    if(next == oldChild) {
        next = newChild;
        return;
    }
    throw runtime_error("couldnt find old child");
}
void For::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "For " << this->id << endl;
    cout << indent << "  Pre:" << endl;
    preBlock->dump(seen, indent + "    ");
    cout << indent << "  Body:" << endl;
    bodyBlock->dump(seen, indent + "    ");
    if(next != 0) {
        if(seen.find(next) == seen.end()) {
            next->dump(seen, indent);
        } else {
            cout << "(*" << next->id << endl;
        }
    }
}

string If::blockType() const {
    return "If";
}
void If::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "If " << this->id << endl;
    cout << indent << "  True:" << endl;
    if(seen.find(trueBlock) == seen.end()) {
        trueBlock->dump(seen, indent + "    ");
    } else {
        cout << indent << "    (*" << trueBlock->id << endl;
    }
    if(falseBlock != 0) {
        cout << indent << "  False:" << endl;
        if(seen.find(falseBlock) == seen.end()) {
            falseBlock->dump(seen, indent + "    ");
        } else {
            cout << indent << "    (*" << falseBlock->id << endl;
        }
    }
}
void If::replaceChild(Block *oldChild, Block *newChild) {
    if(trueBlock == oldChild) {
        trueBlock = newChild;
        return;
    }
    if(falseBlock == oldChild) {
        falseBlock = newChild;
        return;
    }
    throw runtime_error("couldnt find old child");
}
int If::numSuccessors() {
    int count = 0;
    if(trueBlock != 0) {
        count++;
    }
    if(falseBlock != 0) {
        count++;
    }
    return count;
}
Block *If::getSuccessor(int idx) {
    if(idx == 0) {
        if(trueBlock != 0) {
            return trueBlock;
        } else {
            return falseBlock;
        }
    }
    if(idx == 1) {
        if(trueBlock != 0) {
            return falseBlock;
        }
    }
    throw runtime_error("illegal request");
}

string ConditionalBranch::blockType() const {
    return "ConditionalBranch";
}
void ConditionalBranch::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "ConditionalBranch " << this->id << endl;
    cout << indent << "  True:" << endl;
    if(seen.find(trueNext) == seen.end()) {
        trueNext->dump(seen, indent + "    ");
    } else {
        cout << indent << "    (*" << trueNext->id << endl;
    }
    if(falseNext != 0) {
        cout << indent << "  False:" << endl;
        if(seen.find(falseNext) == seen.end()) {
            falseNext->dump(seen, indent + "    ");
        } else {
            cout << indent << "    (*" << falseNext->id << endl;
        }
    }
}
void ConditionalBranch::replaceChild(Block *oldChild, Block *newChild) {
    if(trueNext == oldChild) {
        trueNext = newChild;
        return;
    }
    if(falseNext == oldChild) {
        falseNext = newChild;
        return;
    }
    throw runtime_error("couldnt find old child");
}
int ConditionalBranch::numSuccessors() {
    int count = 0;
    if(trueNext != 0) {
        count++;
    }
    if(falseNext != 0) {
        count++;
    }
    return count;
}
Block *ConditionalBranch::getSuccessor(int idx) {
    if(idx == 0) {
        if(trueNext != 0) {
            return trueNext;
        } else {
            return falseNext;
        }
    }
    if(idx == 1) {
        if(trueNext != 0) {
            return falseNext;
        }
    }
    throw runtime_error("illegal request");
}

string BasicBlockBlock::blockType() const {
    return "BasicBlockBlock";
}
void BasicBlockBlock::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "BasicBlockBlock " << this->id << endl;
    if(next == 0) {
        return;
    }
    if(seen.find(next) == seen.end()) {
        next->dump(seen, indent);
    } else {
        cout << indent << "(*" << next->id << endl;
    }
}
void BasicBlockBlock::replaceChild(Block *oldChild, Block *newChild) {
    if(next == oldChild) {
        next = newChild;
        return;
    }
    throw runtime_error("couldnt find old child");
}
int BasicBlockBlock::numSuccessors() {
    int count = 0;
    if(next != 0) {
        count++;
    }
    return count;
}
Block *BasicBlockBlock::getSuccessor(int idx) {
    return next;
}

string Sequence::blockType() const {
    return "Sequence";
}
void Sequence::dump(set<const Block *> &seen, string indent) const {
    seen.insert(this);
    cout << indent << "Sequence " << this->id << endl;
    for(auto it = children.begin(); it != children.end(); it++) {
        Block *child = *it;
        if(seen.find(child) == seen.end()) {
            child->dump(seen, indent + "  ");
        } else {
            cout << indent << "  (*" << child->id << endl;
        }
    }
    if(next != 0) {
        if(seen.find(next) == seen.end()) {
            next->dump(seen, indent + "");
        } else {
            cout << indent << "(*" << next->id << endl;
        }
    }
}
void Sequence::replaceChild(Block *oldChild, Block *newChild) {
    // int i = 0;
    if(next == oldChild) {
        next = newChild;
        return;
    }
    // bool foundChild = false;
    // for(auto it = children.begin(); it != children.end(); it++) {
    //     Block *child = *it;
    //     if(child == oldChild) {
    //         foundChild = true;
    //         break;
    //     }
    //     i++;
    // }
    // if(foundChild) {
    //     children[i] = newChild;
    //     return;
    // }
    throw runtime_error("couldnt find old child");
}
int Sequence::numSuccessors() {
    int count = 0;
    if(next != 0) {
        count++;
    }
    return count;
}
Block *Sequence::getSuccessor(int idx) {
    return next;
}

string ReturnBlock::blockType() const {
    return "ReturnBlock";
}
void ReturnBlock::dump(set<const Block *> &seen, string indent) const {
    cout << indent << "ReturnBlock " << this->id << endl;
}
void ReturnBlock::replaceChild(Block *oldChild, Block *newChild) {
    throw runtime_error("couldnt find old child");
}
int ReturnBlock::numSuccessors() {
    return 0;
}
Block *ReturnBlock::getSuccessor(int idx) {
    throw runtime_error("illegal request");
}

} // flowcontrol
} // cocl