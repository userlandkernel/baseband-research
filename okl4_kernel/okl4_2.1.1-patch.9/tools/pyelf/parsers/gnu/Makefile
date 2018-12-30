GNU_LDS_FILE=gnu_lds.lds
OUTPUTFILE=.
DATADIR="test/data"

#DEBUG=-traceParser

all:	
	runantlr -o $(OUTPUTFILE) $(DEBUG) gnu_ld_l.g 
	runantlr -o $(OUTPUTFILE) $(DEBUG) gnu_ld_p.g 
	-touch $(OUTPUTFILE)/__init__.py


#run generated parser python script
test_p:	all
	 python $(OUTPUTFILE)/gnu_ld_p.py < $(DATADIR)/$(GNU_LDS_FILE)

#run generated lexer python script
test_l:	all
	 python $(OUTPUTFILE)/gnu_ld_l.py < $(DATADIR)/$(GNU_LDS_FILE)


clean:
	-rm -f gnu_ld_l.py* gnu_ld_p.py* GNU_LDTokenTypes.txt gnu_ld_pTokenTypes.txt gnu_ld_wTokenTypes.txt gnu_ld_l.txt
