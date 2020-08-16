BEGIN {
	FS = ","
	print "#pragma once\n"
	print "// AUTO GENERATED FILE, DO NOT EDIT BY HAND"
	print "#define INSTRUCTIONS \\"
}

/0x.+/ {
	print "	INST(" $2 ", AM_" $3 ", " $1", " $4 ") \\"
}
