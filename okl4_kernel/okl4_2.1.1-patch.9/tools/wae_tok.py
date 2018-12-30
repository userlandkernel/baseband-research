#!/usr/bin/python
##############################################################################
# Copyright (c) 2006, National ICT Australia
##############################################################################
# Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
# All rights reserved.
# 
# 1. Redistribution and use of OKL4 (Software) in source and binary
# forms, with or without modification, are permitted provided that the
# following conditions are met:
# 
#     (a) Redistributions of source code must retain this clause 1
#         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
#         (Licence Terms) and the above copyright notice.
# 
#     (b) Redistributions in binary form must reproduce the above
#         copyright notice and the Licence Terms in the documentation and/or
#         other materials provided with the distribution.
# 
#     (c) Redistributions in any form must be accompanied by information on
#         how to obtain complete source code for:
#        (i) the Software; and
#        (ii) all accompanying software that uses (or is intended to
#        use) the Software whether directly or indirectly.  Such source
#        code must:
#        (iii) either be included in the distribution or be available
#        for no more than the cost of distribution plus a nominal fee;
#        and
#        (iv) be licensed by each relevant holder of copyright under
#        either the Licence Terms (with an appropriate copyright notice)
#        or the terms of a licence which is approved by the Open Source
#        Initative.  For an executable file, "complete source code"
#        means the source code for all modules it contains and includes
#        associated build and other files reasonably required to produce
#        the executable.
# 
# 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
# LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
# IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
# EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
# THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
# BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
# PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
# THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
# 
# 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


import string

class tokeniser:

	def __init__( self, eots ):
		self.eots = eots
		self.tokens = []
		
	# create an escapte character
	def esc_char( self, ch ):
		if( ch == "\"" or ch == "\\" ):
			return ch
		else:
			return "?"

	# is a char whitespace?
	def is_a( self, a, ch ):
		return string.find( a, ch ) != -1
	
	# is a char whitespace?
	def is_white( self, ch ):
		return self.is_a( string.whitespace, ch )

	# is char an end of token?
	def is_eot( self, ch ):
		if( self.is_white( ch ) ):
			return 1
		if( self.is_a( self.eots, ch ) ):
			return 1

		return 0
		
	def eat_string_literal( self, s ):
		idx = 1  # skip leading "
		tok = ""
		while( 1 ):
			cur = s[idx]
			if( cur == "\"" ):
				break
			if( tok == "\\" ):
				idx = idx + 1
				cur = esc_char( s[idx] )
				
			tok = tok + cur
			idx = idx + 1
			
		self.tokens.append( "\"" + tok + "\"" )
		return idx + 1
		
	def eat_whitespace( self, s ):
		# just skip one-by-one and don't add any tokens
		return 1
	
	def eat_token( self, s ):
		if( self.is_eot( s[0] ) ):
			self.tokens.append( s[0] )
			return 1

		tok = ""
		idx = 0
		while( idx < len( s )  ):
			cur = s[idx]
			if( self.is_eot(cur) ):
				break
			tok = tok + cur
			idx = idx + 1

		self.tokens.append( tok )
		return idx

	def tokenise( self, line ):
		idx = 0
		while idx < len( line ):
			cur = line[idx]
			
			if( cur == "\"" ):
 				idx = idx + self.eat_string_literal( line[idx:] )
			elif( self.is_white( cur ) ):
				idx = idx + self.eat_whitespace( line[idx:] )
			else:
				idx = idx + self.eat_token( line[idx:] )

		#for each in self.tokens:
		#	print "%s" % each

if __name__ == "__main__":
	t = tokeniser("@<>:()*+")
	s = "atoi(\"37.69 b\") + 57*8.5"
	print "about to tokenise '%s'" % s 
	t.tokenise( s )
	print t.tokens

	t.tokens = []
	s = "pants 32@0x1230 : "
	print "about to tokenise '%s'" % s 
	t.tokenise( s )
	print t.tokens

	t.tokens = []
	s = "pants 32 @ 1230 : "
	print "about to tokenise '%s'" % s 
	t.tokenise( s )
	print t.tokens
