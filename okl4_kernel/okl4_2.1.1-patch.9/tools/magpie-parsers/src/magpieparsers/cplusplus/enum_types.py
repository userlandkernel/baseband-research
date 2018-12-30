
# Type qualifiers (was C++ enum TypeQualifier)
# XXX nfd removed jul 2005, we now use strings.

# Storage classes (was C++ enum StorageClass)
# XXX nfd removed jul 2005, we now use strings.

# Decl specifiers (was C++ enum DeclSpecifier)
dsInvalid = 0
dsVIRTUAL = 1
dsINLINE = 2
dsEXPLICIT = 3
dsFRIEND = 4

# For type specifiers
# can't bitwise-OR enum elements together, this must be an int; damn!
# XXX nfd removed jul 2005, we now use strings.

# JEL 3/26/96 changed to allow ORing of values
qiInvalid = 0x0
qiType = 0x1	# = includes = enum, = class, = typedefs, = namespace
qiDtor = 0x2
qiCtor = 0x4
qiOperator = 0x8
qiPtrMember = 0x10
qiVar = 0x20
qiFun = 0x40

