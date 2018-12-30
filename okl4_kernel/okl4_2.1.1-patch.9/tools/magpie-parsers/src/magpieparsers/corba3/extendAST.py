import sys, antlr
from antlr import *
from magpieparsers.parser_common import Node

class BaseAST(AST):

    verboseStringConversion = False
    tokenNames = None

    def __init__(self):
        self.down  = None ## kid
        self.right = None ## sibling
        self.node = None #Node(None) ## the 'real' AST
        
	#print 'BaseAST init'

    def addChild(self,node):
        if node:
            #print 'Adding child: ',node
            t = rightmost(self.down)
            self.node.add_child(node.node)
            if t:
                t.right = node
            else:
                assert not self.down
                self.down = node

    def getNumberOfChildren(self):
        t = self.down
        n = 0
        while t:
            n += 1
            t = t.right
        return n

    def doWorkForFindAll(self,v,target,partialMatch):
        sibling = self

        while sibling:
            c1 = partialMatch and sibling.equalsTreePartial(target)
            if c1:
                v.append(sibling)
            else:
                c2 = not partialMatch and sibling.equalsTree(target)
                if c2:
                    v.append(sibling)

            ### regardless of match or not, check any children for matches
            if sibling.getFirstChild():
                sibling.getFirstChild().doWorkForFindAll(v,target,partialMatch)

            sibling = sibling.getNextSibling()

    ### Is node t equal to 'self' in terms of token type and text?
    def equals(self,t):
        if not t:
            return False
        return self.getText() == t.getText() and self.getType() == t.getType()

    ### Is t an exact structural and equals() match of this tree.  The
    ### 'self' reference is considered the start of a sibling list.
    ###
    def equalsList(self, t):
        return cmptree(self, t, partial=False)

    ### Is 't' a subtree of this list?
    ### The siblings of the root are NOT ignored.
    ###
    def equalsListPartial(self,t):
        return cmptree(self,t,partial=True)

    ### Is tree rooted at 'self' equal to 't'?  The siblings
    ### of 'self' are ignored.
    ###
    def equalsTree(self, t):
        return self.equals(t) and \
               cmptree(self.getFirstChild(), t.getFirstChild(), partial=False)

    ### Is 't' a subtree of the tree rooted at 'self'?  The siblings
    ### of 'self' are ignored.
    ###
    def equalsTreePartial(self, t):
        if not t:
            return True
        return self.equals(t) and cmptree(
           self.getFirstChild(), t.getFirstChild(), partial=True)

    ### Walk the tree looking for all exact subtree matches.  Return
    ### an ASTEnumerator that lets the caller walk the list
    ### of subtree roots found herein.
    def findAll(self,target):
        roots = []

        ### the empty tree cannot result in an enumeration
        if not target:
            return None
        # find all matches recursively
        self.doWorkForFindAll(roots, target, False)
        return roots

    ### Walk the tree looking for all subtrees.  Return
    ###  an ASTEnumerator that lets the caller walk the list
    ###  of subtree roots found herein.
    def findAllPartial(self,sub):
        roots = []

        ### the empty tree cannot result in an enumeration
        if not sub:
            return None

        self.doWorkForFindAll(roots, sub, True)  ### find all matches recursively
        return roots

    ### Get the first child of this node None if not children
    def getFirstChild(self):
        return self.down

    ### Get the next sibling in line after this one
    def getNextSibling(self):
        return self.right

    ### Get the token text for this node
    def getText(self):
        return ""

    ### Get the token type for this node
    def getType(self):
        return 0

    def getLine(self):
        return 0

    def getColumn(self):
        return 0

    ### Remove all children */
    def removeChildren(self):
        self.down = None
        self.node.children = []

    def setFirstChild(self,c):
        self.down = c
        #print 'setting the first child of ', self.node.type, '( ', self.node.leaf, ' ) = ', c.node.type, '  ', c.node.leaf, '  ?? ', c, '  currentChildren are: ', self.node.children
        self.node.add_child(c.node)

    def setNextSibling(self, n):
        self.right = n
        #print 'setting the next child of ', self.node.type, '( ', self.node.leaf, ' ) = ', n.node.type, '  ', n.node.leaf, '  ?? ', n, '  currentChildren are: ', self.node.children
        #self.node.add_child(n.node)

    ### Set the token text for this node
    def setText(self, text):
        pass

    ### Set the token type for this node
    def setType(self, ttype):
        pass

    ### static
    def setVerboseStringConversion(verbose,names):
        verboseStringConversion = verbose
        tokenNames = names
    setVerboseStringConversion = staticmethod(setVerboseStringConversion)

    ### Return an array of strings that maps token ID to it's text.
    ##  @since 2.7.3
    def getTokenNames():
        return tokenNames

    def toString(self):
        return self.getText()
        
    ### return tree as lisp string - sibling included
    def toStringList(self):
        ts = self.toStringTree()
        sib = self.getNextSibling()
        if sib:
            ts += sib.toStringList()
        return ts

    __str__ = toStringList

    ### return tree as string - siblings ignored
    def toStringTree(self):
        ts = ""
        kid = self.getFirstChild()
        if kid:
            ts += " ("
        ts += " " + self.toString()
        if kid:
            ts += kid.toStringList()
            ts += " )"
        return ts

###xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx###
###                       extendAST                                ###
###xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx###

### extendAST AST node implementation
class extendAST(BaseAST):
    def __init__(self,token=None):
        super(extendAST,self).__init__()
        self.ttype = INVALID_TYPE
        self.text  = "<no text>"
        self.initialize(token)
	#print 'extendAST init', token
        #assert self.text

    ### Get the token text for this node
    def getText(self):
        return self.text

    ### Get the token type for this node
    def getType(self):
        return self.ttype

    def initialize(self,*args):
        if not args:
            return

        arg0 = args[0]
        
	#print 'in extendAST.initialize', arg0, '  ', self.node
        if isinstance(arg0,int):
            arg1 = args[1]
            self.setType(arg0)
            self.setText(arg1)
            return

        if isinstance(arg0,AST) or isinstance(arg0,Token):
            self.setText(arg0.getText())
            self.setType(arg0.getType())
            return

    ### Set the token text for this node
    def setText(self,text_):
        assert isinstance(text_,str)
        #print 'setText = ', text_
        self.text = text_

    ### Set the token type for this node
    def setType(self,ttype_):
        assert isinstance(ttype_,int)
        #print 'ttype = ', ttype_
        self.ttype = ttype_
