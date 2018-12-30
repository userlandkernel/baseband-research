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

import copy, os
from elf.util import align_up, align_down

def xmlbool(bool):
    """Convert a boolean into the string expected by the elfweaver XML."""
    # Is there a smarter way of doing this?
    if bool:
        return "true"
    else:
        return "false"

def uniq(lst):
    # This is ineffcient but keeps it in order
    new_lst = []
    for each in lst:
        if each not in new_lst:
            new_lst.append(each)
    return new_lst


class mkstr:
    def __init__(self, str):
        self.str = str

    def __call__(self, targets, source, env):
        return self.str

    def __str__(self):
        return self.str

def contains_any(needles, haystack):
    """ Returns True if any of the needles are in haystack

    >>> contains_any([1, 2, 3], [50, 37, 12, 1])
    True
    >>> contains_any([1, 2, 3], [50, 37, 12])
    False
    """
    for needle in needles:
        if needle in haystack:
            return True
    return False

def list2str(list):
    """Given a list return a string"""
    return "[" + ", ".join([str(x) for x in list]) + "]"

def make_global(obj, name=None):
    """ Inserts an object into the global namespace. By default
    the name is determined by the objects __name__ attribute. Alternatively
    a name can be specified.

    >>> make_global(1, "One")
    >>> One
    1
    """
    if name:
        globals()[name] = obj
    else:
        globals()[obj.__name__] = obj

class IdentityMapping:
    """ This is an object that can be used to generate an identity mapping.
    That is it is an object that has the same interface as a normal dictionary,
    but every lookup returns the key itself. E.g:

    >>> identity_mapping = IdentityMapping()
    >>> identity_mapping[1]
    1
    >>> identity_mapping["foo"]
    'foo'
    """
    def __getitem__(self, key):
        return key

identity_mapping = IdentityMapping()

class OrderedDict:
    """A very simple insertion ordered dictionary so we can gets
    out keys in insertion order. This is used because we need to link
    against libraries in the order in which the are added"""
    
    def __init__(self):
        self.dict = {}
        self.insert_order = []

    def __setitem__(self, key, value):
        self.dict[key] = value
        self.insert_order.append(key)

    def __getitem__(self, key):
        return self.dict[key]

    def keys(self):
        return self.insert_order

    def __copy__(self):
        new = OrderedDict()
        new.dict = copy.copy(self.dict)
        new.insert_order = copy.copy(self.insert_order)
        return new

    def items(self):
        return [self.dict[key] for key in self.keys()]

class ListFunction:
    def __init__(self, fn):
        self.fn = fn

    def __getitem__(self, idx):
        return self.fn()[idx]

def markup(template, output, environment):
	"""
	This function reads a template. The template format is:
	{{python code:}} execute "python code"
	{{?python code}}: evaluate "python code" and replace the whole thing with the result.
	{{
	python code
	...
	}}: execute "python code". 
	"""		
	environment['out'] = output
	
	window_start = 0
	window_end = 0
	eof = len(template)

	STATE_NORMAL = 0
	STATE_EXECCODE = 1
	STATE_EVALCODE = 2
	STATE_LOOP = 3
	STATE_IF = 4

	state = STATE_NORMAL

	while window_start < eof:
		if state == STATE_NORMAL:
			window_end = template.find('{{', window_start)
			if window_end == -1:
				window_end = eof
			output.write(template[window_start:window_end])
			window_start = window_end
			if template[window_start:window_start + 9] == '{{ABORT}}':
				return False
			if template[window_start:window_start + 4] == '{{if':
				state = STATE_IF
				window_start += 4
			elif template[window_start:window_start + 3] == '{{?':
				state = STATE_EVALCODE
				window_start += 3
			elif template[window_start:window_start + 3] == '{{*':
				state = STATE_LOOP
				window_start += 3
			elif template[window_start:window_start + 2] == '{{':
				state = STATE_EXECCODE
				window_start += 2
		elif state == STATE_EVALCODE:
			window_end = template.find('}}', window_start)
			if window_end != -1:
				code = template[window_start:window_end]
				output.write(str(eval(code, environment)))
				window_start = window_end + 2
			state = STATE_NORMAL
		elif state == STATE_EXECCODE:
			window_end = template.find('}}', window_start)
			if window_end != -1:
				code = template[window_start:window_end]
				exec code in environment
				window_start = window_end + 2
			state = STATE_NORMAL
		elif state == STATE_IF:
			window_end = template.find('fi}}', window_start)
			ifcode_end = template.find(':', window_start)
			code = template[window_start:ifcode_end]
			if eval(code, environment):
				result = markup(template[ifcode_end+1:window_end], output, environment)
				if result is False:
					return False
			window_start = window_end + 4
			state = STATE_NORMAL
		elif state == STATE_LOOP:
			window_end = template.find('*}}', window_start)
			loopcode = template[window_start:window_end]
			for item in environment['LOOP']:
				environment['LOOPITEM'] = item
				result = markup(loopcode, output, environment)
				if result is False:
					return False
			window_start = window_end + 3
			state = STATE_NORMAL
	return True

# Remove pyc files from a directory
def pyc_clean(dir):
    def rmglob(arg, top, names):
        rmlist = [ top + os.path.sep + x for x in names if x.endswith(arg)]
        for x in rmlist:
            print "Removing", x
            os.remove(x)
    os.path.walk(dir, rmglob, ".pyc")

class LibraryNotFound(Exception):
    pass

class AllocatorException(Exception):
    pass

class Allocator:
    """
    A simple allocator for physical memory.

    This allocator can be used to allocate regions of memory with
    certain alignment requirements and to ensure that memory regions
    at fixed locations do not have a dynamic allocation placed on top 
    of them.
    """
    def __init__(self, free_list, min_alloc_size):
        """
        __init__(free_list, min_alloc_size)

        free_list is a array of tuples.  Each touple specifies the
        start and end address, in bytes, of a region of free memory.

        min_alloc_size is the smallest size, in bytes, that can be
        allocated.  Alignment must be a multiple of this size.
        """
        self.freelist = free_list[:]
        self.min_alloc = min_alloc_size

    def free_memory(self):
        return self.freelist

    def mark(self, base, end):
        """
        mark(base, end) -> base
        
        Remove the given range from the free list.  This is used to
        record were fixed address objects are located.

        An exception is raised if the range crosses a free/used
        border.

        It is *not* an error if the range is wholly outside the
        freelist.
        """

        base = align_down(base, self.min_alloc)
        end = align_up(end, self.min_alloc)-1

#        if self is phys_alloc:
#            print "Mark: 0x%x 0x%x" % (base, end)

        for (free_start, free_end) in self.freelist:
            # if used range is in this free range
            if base >= free_start and base <= free_end:
                # check used block not going past free range
                if end > free_end:
                    raise AllocatorException, "Used block (0x%x, 0x%x) outside avail range (0x%x, 0x%x)." % (base, end, free_start, free_end)

                index = self.freelist.index((free_start, free_end))
                self.freelist.remove((free_start, free_end))

                # align ranges to page size
                free_start = align_down(free_start, self.min_alloc)
                free_end = align_up(free_end, self.min_alloc)
                base = align_down(base, self.min_alloc)
                end = align_up(end, self.min_alloc)

                # need to divide free range into zero, one or two
                # new ranges
                if free_start < base:
                    self.freelist.insert(index, (free_start, base-1))
                    index += 1

                if end < free_end:
                    self.freelist.insert(index, (end, free_end-1))

#        if self is phys_alloc:
#            print " =free:", [(hex(x), hex(y)) for (x, y) in self.freelist]

        return base

    def alloc(self, size, alignment, virt):
        """
        alloc(size, alignment, virt) -> long

        Allocate a block of memory from the freelist and return the
        start address.

        size - The minimum size of the block in bytes.
        alignment - The virtual-physical alignment.
        virt - The virtual address where the data in the block will be
        stored.
        """
        if alignment < self.min_alloc:
            raise AllocatorException, \
                  "Alignment (0x%x) less than the min page size (0x%x)." \
                  % (alignment, self.min_alloc)

        if alignment % self.min_alloc != 0:
            raise AllocatorException, \
                  "Alignment (0x%x) is not a multiple of min page size (0x%x)." \
                  % (alignment, self.min_alloc)

        offset = virt % alignment
        alloc_start = -1

#        if self is phys_alloc:
#            print "Alloc: size: 0x%x, alignment: 0x%x" % (size, alignment)

        for (free_start, free_end) in self.freelist:
            alloc_start = align_down(free_start, alignment) + offset
            alloc_end   = alloc_start + size - 1

#            print "alloc_start: 0x%x, free_start: 0x%x, alloc_end: 0x%x, free_end: 0x%x" % (alloc_start, free_start, alloc_end, free_end)
            # If alignment pushes the block below the start of
            # the free region, bump it up
            if alloc_start < free_start:
                alloc_start += alignment
                alloc_end += alignment

#            print "alloc_start2: 0x%x, free_start2: 0x%x, alloc_end2: 0x%x, free_end2: 0x%x" % (alloc_start, free_start, alloc_end, free_end)
            # If the range is within free memory...
            if alloc_start >= free_start and alloc_end <= free_end:
                index = self.freelist.index((free_start, free_end))
                self.freelist.remove((free_start, free_end))

                # need to divide free range into zero, one or two
                # new ranges
                if free_start < alloc_start:
                    self.freelist.insert(index, (free_start,
                                                 align_down(alloc_start,
                                                            alignment) - 1))
                    index += 1

                if alloc_end <= free_end:
                    self.freelist.insert(index, (alloc_end, free_end))
                else:
                    alloc_start += alignment

                break
            else:
                alloc_start = -1

        if alloc_start == -1:
            raise AllocatorException, "Out of Memory"

#        if self is phys_alloc:
#            print " -free:", [(hex(x), hex(y)) for (x, y) in self.freelist]

        return alloc_start

    def round_pages(self):
        """
        round_pages()

        remove any partially used pages from the freelist
        """
        alignment = self.min_alloc
        scan_list = True

        while (scan_list):
            scan_list = False
            for (free_start, free_end) in self.freelist:
                alloc_start = free_start
                align_end   = align_up(alloc_start+1, alignment)
                if (align_end < free_end):
                    alloc_end = align_end
                else:
                    alloc_end = free_end
                #print " round:", hex(free_start), "..", hex(free_end)

                if (alloc_start < alloc_end) and (alloc_end-alloc_start < alignment):
                    #print "  - rm:", hex(alloc_start), "..", hex(alloc_end)
                    scan_list = True

                    index = self.freelist.index((free_start, free_end))
                    self.freelist.remove((free_start, free_end))

                    if alloc_end < free_end:
                        self.freelist.insert(index, (alloc_end, free_end))
                        index += 1

        return

    def next_avail(self):
        """
        Return the base address of the biggest block of memory in
        the free list.
        """

        def sort_by_size(x, y):
            return int((x[1] - x[0]) - (y[1] - y[0]))

        freelist = self.freelist[:]
        freelist.sort(cmp = sort_by_size)

        return freelist[-1][0]

def parse_addr_file(filename):
    """Parse an address file, called filename.
    An address file is in the form:
    <name>: <base0>,<end0> <base1>,<end1>
    <name>: <base0>,<end0> <base1>,<end1>

    Each line represents a set of disjoint set of memory with
    a particular name. The each item in the set is a range of
    memory.

    This function returns a dictionary. The dictionary's keys
    are the name. The dictionary value is a list of tuples, with
    each tuple being of the form (base, end).
    """
    f = open(filename)
    result = {}
    for line in f.readlines():
        name, values = line.split(":")
        addrs = []
        values = values.split()
        for each in values:
            base, end = each.split(",")
            addrs.append((long(base, 0), long(end, 0)))
        result[name] = addrs
    return result

def markup(template, output, environment):
	"""
	This function reads a template. The template format is:
	{{python code:}} execute "python code"
	{{?python code}}: evaluate "python code" and replace the whole thing with the result.
	{{
	python code
	...
	}}: execute "python code". 
	"""		
	environment['out'] = output
	
	window_start = 0
	window_end = 0
	eof = len(template)

	STATE_NORMAL = 0
	STATE_EXECCODE = 1
	STATE_EVALCODE = 2
	STATE_LOOP = 3
	STATE_IF = 4

	state = STATE_NORMAL

	while window_start < eof:
		if state == STATE_NORMAL:
			window_end = template.find('{{', window_start)
			if window_end == -1:
				window_end = eof
			output.write(template[window_start:window_end])
			window_start = window_end
			if template[window_start:window_start + 9] == '{{ABORT}}':
				return False
			if template[window_start:window_start + 4] == '{{if':
				state = STATE_IF
				window_start += 4
			elif template[window_start:window_start + 3] == '{{?':
				state = STATE_EVALCODE
				window_start += 3
			elif template[window_start:window_start + 3] == '{{*':
				state = STATE_LOOP
				window_start += 3
			elif template[window_start:window_start + 2] == '{{':
				state = STATE_EXECCODE
				window_start += 2
		elif state == STATE_EVALCODE:
			window_end = template.find('}}', window_start)
			if window_end != -1:
				code = template[window_start:window_end]
				output.write(str(eval(code, environment)))
				window_start = window_end + 2
			state = STATE_NORMAL
		elif state == STATE_EXECCODE:
			window_end = template.find('}}', window_start)
			if window_end != -1:
				code = template[window_start:window_end]
				exec code in environment
				window_start = window_end + 2
			state = STATE_NORMAL
		elif state == STATE_IF:
			window_end = template.find('fi}}', window_start)
			ifcode_end = template.find(':', window_start)
			code = template[window_start:ifcode_end]
			if eval(code, environment):
				result = markup(template[ifcode_end+1:window_end], output, environment)
				if result is False:
					return False
			window_start = window_end + 4
			state = STATE_NORMAL
		elif state == STATE_LOOP:
			window_end = template.find('*}}', window_start)
			loopcode = template[window_start:window_end]
			for item in environment['LOOP']:
				environment['LOOPITEM'] = item
				result = markup(loopcode, output, environment)
				if result is False:
					return False
			window_start = window_end + 3
			state = STATE_NORMAL
	return True

